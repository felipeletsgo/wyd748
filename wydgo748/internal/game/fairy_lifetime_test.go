package game

import (
	"encoding/json"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestFairyLifetimePausesAcrossInventoryAndRelogin(t *testing.T) {
	w, p, _, session := newTimedSwapWorld(t, nil)
	w.items[3914] = premiumTestDef(3914, fairySlot)
	p.Char.Inv[0].Index = 3914
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, fairySlot))
	started := p.Char.Equip[fairySlot]
	if started.ExpiresUnix == 0 {
		t.Fatal("fairy was not activated")
	}
	clock := w.clock.(*fakeClock)
	clock.Advance(time.Hour)
	w.onSwapItem(session, timedSwapPacket(placeEquip, fairySlot, placeInv, 0))
	paused := p.Char.Inv[0]
	if paused.PausedUnix != w.now().Unix() {
		t.Fatalf("unequip did not pause lifetime: %+v", paused)
	}
	remaining := paused.ExpiresUnix - paused.PausedUnix
	encoded, err := json.Marshal(p.Account)
	if err != nil {
		t.Fatal(err)
	}
	var restored model.Account
	if err := json.Unmarshal(encoded, &restored); err != nil {
		t.Fatal(err)
	}
	p.Account, p.Char = &restored, &restored.Chars[0]
	clock.Advance(30 * 24 * time.Hour)
	if removed := expireTimedAccountItems(p.Account, w.now()); len(removed) != 0 {
		t.Fatal("paused fairy expired in inventory")
	}
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, fairySlot))
	resumed := p.Char.Equip[fairySlot]
	if resumed.UID != started.UID || resumed.PausedUnix != 0 || resumed.ExpiresUnix-w.now().Unix() != remaining {
		t.Fatalf("remaining lifetime changed after relogin: %+v", resumed)
	}
	clock.Advance(time.Duration(remaining) * time.Second)
	if removed := expireTimedAccountItems(p.Account, w.now()); len(removed) != 1 || p.Char.Equip[fairySlot].Index != 0 {
		t.Fatal("resumed fairy did not expire at the remaining equipped lifetime")
	}
}

func TestFairyLifetimeMigratesLiveStoredItemsWithoutResurrection(t *testing.T) {
	w, p, _, _ := newTimedSwapWorld(t, nil)
	now := w.now()
	live := model.Item{Index: 3914, UID: "11111111111141118111111111111111", ActivatedUnix: now.Add(-time.Hour).Unix(), ExpiresUnix: now.Add(time.Hour).Unix()}
	expired := live
	expired.UID = "22222222222242228222222222222222"
	expired.ExpiresUnix = now.Unix()
	p.Account.Cargo[0], p.Account.Cargo[1] = live, expired
	carried := live
	carried.UID = "44444444444444448444444444444444"
	p.Account.Chars = append(p.Account.Chars, model.Char{Inv: [model.MaxCarry]model.Item{carried}})
	// Appending can reallocate the character slice.
	p.Char = &p.Account.Chars[p.CharSlot]
	if changed, err := w.planTimedEquipmentForEnter(p, now); err != nil || !changed {
		t.Fatalf("stored fairy migration failed: changed=%v err=%v", changed, err)
	}
	if p.Account.Cargo[0].PausedUnix != now.Unix() || p.Account.Chars[1].Inv[0].PausedUnix != now.Unix() {
		t.Fatal("entry did not pause live fairies in Cargo and inactive character inventory")
	}
	if p.Account.Cargo[1] != (model.Item{}) {
		t.Fatal("entry resurrected an already expired fairy")
	}
	if removed := expireTimedAccountItems(p.Account, now.Add(30*24*time.Hour)); len(removed) != 0 {
		t.Fatal("stored paused fairy was consumed")
	}
}

func TestCriticalEquipmentFairyPausePublishesOnlyAfterAcknowledgement(t *testing.T) {
	for _, resume := range []bool{false, true} {
		t.Run(map[bool]string{false: "pause", true: "resume"}[resume], func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			seedCriticalEquipment(w, p)
			w.items[3914] = premiumTestDef(3914, fairySlot)
			now := w.now()
			item := model.Item{Index: 3914, UID: "33333333333343338333333333333333", ActivatedUnix: now.Add(-2 * time.Hour).Unix(), ExpiresUnix: now.Add(time.Hour).Unix()}
			st, sp, dt, dp := byte(placeEquip), byte(fairySlot), byte(placeInv), byte(8)
			if resume {
				item.PausedUnix = now.Add(-time.Hour).Unix()
				p.Char.Inv[8] = item
				st, sp, dt, dp = dt, dp, st, sp
			} else {
				p.Char.Equip[fairySlot] = item
			}
			before := accountStateSnapshot(p.Account)
			w.onSwapItem(p.Session, criticalCargoSwapRequest(p, st, sp, dt, dp))
			write := awaitCriticalWrite(t, backend.entered)
			planned := &write.accounts[0].Chars[p.CharSlot]
			want := item
			if resume {
				want.ExpiresUnix += now.Unix() - want.PausedUnix
				want.PausedUnix = 0
				if planned.Equip[fairySlot] != want {
					t.Fatal("resume proposal did not preserve the frozen remaining lifetime")
				}
			} else {
				want.PausedUnix = now.Unix()
				if planned.Inv[8] != want {
					t.Fatal("unequip proposal did not freeze the fairy")
				}
			}
			for phase := 0; phase < 2; phase++ {
				if phase == 1 {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("fairy state was published before acknowledgement")
				}
			}
			w.processCriticalCompletions(1)
			if p.Char.Inv != planned.Inv || p.Char.Equip != planned.Equip || len(queue.pending) != 0 || w.shuttingDown {
				t.Fatal("acknowledgement did not apply the committed fairy state")
			}
		})
	}
}

func TestFairyPauseRollsBackWhenEquipmentSaveFails(t *testing.T) {
	w, p, store, session := newTimedSwapWorld(t, nil)
	w.items[3914] = premiumTestDef(3914, fairySlot)
	p.Char.Inv[0].Index = 3914
	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, fairySlot))
	before := p.Char.Equip[fairySlot]
	store.err = errors.New("save rejected")
	w.onSwapItem(session, timedSwapPacket(placeEquip, fairySlot, placeInv, 0))
	if p.Char.Equip[fairySlot] != before || p.Char.Inv[0] != (model.Item{}) {
		t.Fatal("failed save published a paused fairy")
	}
}
