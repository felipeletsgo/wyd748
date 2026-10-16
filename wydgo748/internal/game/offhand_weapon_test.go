package game

import (
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Removing the main weapon of a pair moves the off-hand weapon to slot 6,
// as W2PP _MSG_TradingItem does; a shield stays in slot 7.
func TestRemovingMainWeaponPromotesOffhandWeapon(t *testing.T) {
	for _, tc := range []struct {
		name        string
		offhandPos  int
		wantPromote bool
	}{{"weapon", 192, true}, {"shield", 128, false}} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			w.items[2493] = model.ItemDef{Index: 2493, Pos: 192}
			w.items[2861] = model.ItemDef{Index: 2861, Pos: tc.offhandPos}
			p.Char.Equip[6] = model.Item{Index: 2493, UID: "main"}
			p.Char.Equip[7] = model.Item{Index: 2861, UID: "offhand"}
			p.Char.Inv[0] = model.Item{}

			w.onSwapItem(p.Session, criticalCargoSwapRequest(p, placeEquip, 6, placeInv, 0))

			if p.Char.Inv[0].UID != "main" {
				t.Fatalf("main weapon was not unequipped: %+v", p.Char.Inv[0])
			}
			if tc.wantPromote && (p.Char.Equip[6].UID != "offhand" || p.Char.Equip[7].Index != 0) {
				t.Fatalf("off-hand weapon was not promoted: 6=%+v 7=%+v", p.Char.Equip[6], p.Char.Equip[7])
			}
			if !tc.wantPromote && (p.Char.Equip[6].Index != 0 || p.Char.Equip[7].UID != "offhand") {
				t.Fatalf("shield moved: 6=%+v 7=%+v", p.Char.Equip[6], p.Char.Equip[7])
			}
		})
	}
}

func TestRemovingOffhandKeepsMainWeapon(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.items[2493] = model.ItemDef{Index: 2493, Pos: 192}
	p.Char.Equip[6] = model.Item{Index: 2493, UID: "main"}
	p.Char.Equip[7] = model.Item{Index: 2493, UID: "offhand"}
	w.onSwapItem(p.Session, criticalCargoSwapRequest(p, placeEquip, 7, placeInv, 0))
	if p.Char.Equip[6].UID != "main" || p.Char.Equip[7].Index != 0 || p.Char.Inv[0].UID != "offhand" {
		t.Fatal("removing one of two identical weapons changed the wrong identity")
	}
}

func TestOffhandPromotionRollsBackFailedSave(t *testing.T) {
	w, p, backend := handlerTestWorld(t)
	w.items[2493] = model.ItemDef{Index: 2493, Pos: 192}
	w.items[2861] = model.ItemDef{Index: 2861, Pos: 192}
	p.Char.Equip[6] = model.Item{Index: 2493, UID: "main", Eff: [6]byte{43, 9}}
	p.Char.Equip[7] = model.Item{Index: 2861, UID: "offhand", Eff: [6]byte{43, 230}}
	before := accountStateSnapshot(p.Account)
	backend.err = errors.New("save unavailable")
	w.onSwapItem(p.Session, criticalCargoSwapRequest(p, placeEquip, 6, placeInv, 0))
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || backend.saves != 1 {
		t.Fatal("failed promotion did not restore both hands and the inventory")
	}
	for p.Session.QueuedPacketsForTest() > 0 {
		packet, _ := p.Session.DequeuePacketForTest()
		if !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpSendItem && wire.ParseHeader(packet).Type != wire.OpMessagePanel {
			t.Fatal("failed promotion published a successful move or appearance")
		}
	}
}

func TestCriticalOffhandPromotionSettlement(t *testing.T) {
	for _, outcome := range []string{"commit", "rejection", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			w.items = map[uint16]model.ItemDef{2493: {Index: 2493, Pos: 192}, 2861: {Index: 2861, Pos: 192}}
			p.Char.Equip[6] = model.Item{Index: 2493, UID: "main", Eff: [6]byte{43, 9}}
			p.Char.Equip[7] = model.Item{Index: 2861, UID: "offhand", Eff: [6]byte{43, 230}}
			p.Char.Inv[0] = model.Item{}
			before := accountStateSnapshot(p.Account)
			if outcome == "rejection" {
				backend.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				backend.failure = errors.New("commit acknowledgement lost")
			}
			request := criticalCargoSwapRequest(p, placeEquip, 6, placeInv, 0)
			w.onSwapItem(p.Session, request)
			proposal := awaitCriticalWrite(t, backend.entered)
			prepared := proposal.accounts[0].Chars[p.CharSlot]
			if prepared.Equip[6] != before.Chars[p.CharSlot].Equip[7] || prepared.Equip[7].Index != 0 ||
				prepared.Inv[0] != before.Chars[p.CharSlot].Equip[6] {
				t.Fatal("promotion proposal lost an item, effect or identity")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.onSwapItem(p.Session, request)
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
					t.Fatal("pending promotion published or submitted twice")
				}
			}
			w.processCriticalCompletions(1)
			if outcome == "commit" {
				if p.Char.Equip != prepared.Equip || p.Char.Inv != prepared.Inv || len(queue.pending) != 0 {
					t.Fatal("committed promotion differs from the saved proposal")
				}
				assertCriticalPickupPacket(t, p, wire.SwapItem(p.ID, placeEquip, 6, placeInv, 0, 1102))
				assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeEquip, 6, p.Char.Equip[6]))
				assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeEquip, 7, p.Char.Equip[7]))
				assertCriticalPickupPacket(t, p, playerScorePacket(p))
				assertCriticalPickupPacket(t, p, wire.UpdateEtc(p.ID, *p.Char))
				assertCriticalPickupPacket(t, p, playerAppearancePacket(p))
				if p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("promotion published an extra equipment response")
				}
			} else {
				assertCriticalCargoUnchanged(t, p, before, fallback)
				// Equipment writes retain the existing conservative containment
				// policy for every failed acknowledgement, including rejections.
				if !w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("failed promotion was not contained without publication")
				}
			}
			packets := p.Session.QueuedPacketsForTest()
			w.processCriticalCompletions(1)
			if p.Session.QueuedPacketsForTest() != packets || len(backend.entered) != 0 || fallback.saves != 0 {
				t.Fatal("promotion completion replayed or saved inline")
			}
		})
	}
}
