package game

import (
	"bytes"
	"encoding/binary"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func seedCriticalEquipment(w *World, p *Player) {
	if w.items == nil {
		w.items = make(map[uint16]model.ItemDef)
	}
	w.items[4300] = premiumTestDef(4300, fairySlot)
	w.items[model.MountBabyBase] = premiumTestDef(model.MountBabyBase, mountSlot)
	p.Char.Inv[3] = model.Item{Index: 4300, UID: "timed-fairy"}
	p.Char.Inv[4] = model.Item{Index: model.MountBabyBase, UID: "timed-mount"}
	w.rng = fixedRNG{value: 7}
}

func assertCriticalEquipmentPublication(t *testing.T, p *Player, request []byte, followerPackets ...[]byte) {
	t.Helper()
	expected := [][]byte{
		wire.SwapItem(p.ID, request[12], request[13], request[14], request[15], binary.LittleEndian.Uint16(request[16:18])),
		playerScorePacket(p), wire.UpdateEtc(p.ID, *p.Char),
	}
	expected = append(expected, followerPackets...)
	expected = append(expected, playerAppearancePacket(p))
	for sequence, want := range expected {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) {
			t.Fatalf("missing equipment response %d", sequence)
		}
		header := wire.ParseHeader(packet)
		if header.Type != wire.ParseHeader(want).Type || header.ID != wire.ParseHeader(want).ID ||
			int(header.Size) != len(want) || !bytes.Equal(packet[12:], want[12:]) {
			t.Fatalf("equipment response %d differs in order, recipient, size or full payload", sequence)
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("unexpected additional equipment response")
	}
}

func TestCriticalEquipmentSwapDetachedDeadlineAndMountPublication(t *testing.T) {
	for _, scenario := range []string{"fairy", "fresh-mount", "initialized-mount", "unequip", "same-slot", "existing-deadline", "untimed"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			seedCriticalEquipment(w, p)
			st, sp, dt, dp := byte(placeInv), byte(3), byte(placeEquip), byte(fairySlot)
			var previousPet *Mob
			switch scenario {
			case "fresh-mount", "initialized-mount":
				sp, dp = 4, mountSlot
				if scenario == "initialized-mount" {
					p.Char.Inv[4] = criticalBabyMount()
					p.Char.Inv[4].UID = "initialized-mount"
				}
			case "unequip":
				p.Char.Equip[mountSlot] = criticalBabyMount()
				p.Char.Equip[mountSlot].UID = "equipped-mount"
				w.syncCriaPet(p)
				previousPet = w.findCriaPet(p.ID)
				if previousPet == nil {
					t.Fatal("fixture did not spawn the existing follower")
				}
				st, sp, dt, dp = placeEquip, mountSlot, placeInv, 8
				for p.Session.QueuedPacketsForTest() > 0 {
					p.Session.DequeuePacketForTest()
				}
			case "same-slot":
				p.Char.Equip[fairySlot], p.Char.Inv[3] = p.Char.Inv[3], model.Item{}
				st, sp, dt, dp = placeEquip, fairySlot, placeEquip, fairySlot
			case "existing-deadline":
				p.Char.Inv[3].ActivatedUnix, p.Char.Inv[3].ExpiresUnix = w.now().Add(-time.Hour).Unix(), w.now().Add(time.Hour).Unix()
			case "untimed":
				def := w.items[4300]
				def.StaticEffects = []model.StaticEffect{{Name: "EF_CLASS", Value: 15}}
				w.items[4300] = def
			}
			p.EggIncubationUID, p.NextEggIncubationTick = "previous-egg", w.now().Add(time.Hour)
			before := accountStateSnapshot(p.Account)
			account, character := p.Account, p.Char
			request := criticalCargoSwapRequest(p, st, sp, dt, dp)
			w.onSwapItem(p.Session, request)
			write := awaitCriticalWrite(t, backend.entered)
			if len(write.accounts) != 1 || write.accounts[0] == p.Account || write.character != nil || write.instances != nil {
				t.Fatal("equipment worker did not receive a detached account-only proposal")
			}
			committed := &write.accounts[0].Chars[p.CharSlot]
			if scenario != "unequip" && scenario != "untimed" {
				item := committed.Equip[dp]
				if item.ActivatedUnix <= 0 || item.ExpiresUnix <= item.ActivatedUnix {
					t.Fatal("proposal omitted the timed UID deadline")
				}
				if scenario == "existing-deadline" && item != before.Chars[0].Inv[3] {
					t.Fatal("existing deadline was restarted")
				}
			}
			if scenario == "fresh-mount" && (committed.Equip[mountSlot].MountHP() <= 0 ||
				committed.Equip[mountSlot].MountLongev() != 17 || committed.Equip[mountSlot].MountFood() != 30) {
				t.Fatal("fresh mount was not materialized before persistence")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.onSwapItem(p.Session, request)
				w.syncCriaPet(p)
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 ||
					p.EggIncubationUID != "previous-egg" || w.findCriaPet(p.ID) != previousPet {
					t.Fatalf("%s changed mount effects, published or repeated the write", phase)
				}
			}
			// A different RNG at delivery proves that acknowledgement cannot reroll.
			w.rng = fixedRNG{value: 1}
			w.processCriticalCompletions(1)
			if p.Account != account || p.Char != character || p.Char.Inv != committed.Inv || p.Char.Equip != committed.Equip ||
				len(queue.pending) != 0 || w.shuttingDown || w.criticalPlayerReserved(p) || fallback.saves != 0 {
				t.Fatal("equipment completion failed to apply exactly the committed slots")
			}
			if dp == mountSlot || sp == mountSlot {
				if p.EggIncubationUID != "" || !p.NextEggIncubationTick.IsZero() {
					t.Fatal("committed mount move did not clear the egg clock")
				}
			} else if p.EggIncubationUID != "previous-egg" {
				t.Fatal("unrelated equipment move reset the egg clock")
			}
			pet := w.findCriaPet(p.ID)
			if scenario == "fresh-mount" || scenario == "initialized-mount" {
				if pet == nil || pet.HP != uint32(p.Char.Equip[mountSlot].MountHP()) {
					t.Fatal("guarded follower reconciliation did not run after acknowledgement")
				}
			} else if scenario == "unequip" && (pet != nil || !previousPet.Dead) {
				t.Fatal("committed unequip retained a living follower")
			}
			var followerPackets [][]byte
			if pet != nil && previousPet == nil && p.hasVisible(pet.ID) {
				anct := pet.Def.Equip.AncientCodes()
				followerPackets = append(followerPackets, wire.CreateMobVisual(pet.ID, pet.Def.Name, pet.X, pet.Y,
					pet.Def.Mesh(), anct[:], mobPublicExtendedAt(pet, w.now()), pet.Affects[:], 0))
			} else if previousPet != nil {
				followerPackets = append(followerPackets, wire.RemoveMob(previousPet.ID, 0))
			}
			assertCriticalEquipmentPublication(t, p, request, followerPackets...)
		})
	}
}

func TestCriticalEquipmentSwapRejectsAndContainsUnknownOutcomes(t *testing.T) {
	for _, scenario := range []string{"invalid-slot", "class", "missing-uid", "expired", "invalid-deadline", "full", "stopping", "store-error", "store-panic", "identity"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			seedCriticalEquipment(w, p)
			sp := byte(3)
			switch scenario {
			case "invalid-slot":
				sp = 63
			case "class":
				def := w.items[4300]
				def.StaticEffects = []model.StaticEffect{{Name: "EF_CLASS", Value: 2}}
				w.items[4300] = def
			case "missing-uid":
				p.Char.Inv[3].UID = ""
			case "expired":
				p.Char.Inv[3].ActivatedUnix, p.Char.Inv[3].ExpiresUnix = w.now().Add(-time.Hour).Unix(), w.now().Unix()
			case "invalid-deadline":
				p.Char.Inv[3].ExpiresUnix = w.now().Add(time.Hour).Unix()
			case "full":
				for _, name := range []string{"other-a", "other-b"} {
					if _, err := w.criticalWrites.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}, func(criticalPersistenceResult) error { return nil }); err != nil {
						t.Fatal(err)
					}
				}
				awaitCriticalWrite(t, backend.entered)
			case "stopping":
				queue.closeAdmission()
			case "store-error":
				backend.failure = errors.New("commit acknowledgement lost")
			case "store-panic":
				backend.panicAfterWrite = true
			}
			before := accountStateSnapshot(p.Account)
			request := criticalCargoSwapRequest(p, placeInv, sp, placeEquip, fairySlot)
			w.onSwapItem(p.Session, request)
			unknown := scenario == "store-error" || scenario == "store-panic" || scenario == "identity"
			if unknown {
				awaitCriticalWrite(t, backend.entered)
				if scenario == "identity" {
					p.criticalLifecycle = &criticalPlayerLifecycle{}
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if !w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() || len(queue.pending) != 1 ||
					p.Session.QueuedPacketsForTest() != 0 || w.findCriaPet(p.ID) != nil {
					t.Fatal("failed/obsolete commit published or released instead of containment")
				}
				w.onSwapItem(p.Session, request)
			} else {
				if w.criticalPlayerReserved(p) || len(backend.entered) != 0 {
					t.Fatal("rejected move reserved the participant or wrote to the store")
				}
				for p.Session.QueuedPacketsForTest() > 0 {
					packet, _ := p.Session.DequeuePacketForTest()
					if !wire.Decrypt(packet) || (wire.ParseHeader(packet).Type != wire.OpSendItem && wire.ParseHeader(packet).Type != wire.OpMessagePanel) {
						t.Fatal("rejection emitted a success/equipment effect packet")
					}
				}
			}
			assertCriticalCargoUnchanged(t, p, before, fallback)
		})
	}
}

func TestCriticalEquipmentSwapActualDispatchKeepsWorldResponsive(t *testing.T) {
	w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, nil, seedCriticalEquipment)
	request := criticalCargoSwapRequest(p, placeInv, 3, placeEquip, fairySlot)
	deliver := func() {
		select {
		case w.commands <- command{s: p.Session, pkt: request}:
		case <-time.After(time.Second):
			t.Fatal("World did not accept the equipment intention")
		}
	}
	deliver()
	proposal := awaitCriticalWrite(t, backend.entered)
	for range 3 {
		deliverCriticalLoopPing(t, w, p)
		deliverCriticalLoopTick(t, ticks)
		overview := readCriticalLoopOverview(t, w)
		regenerated := false
		for _, player := range overview.Players {
			if player.UID == unrelatedUID && player.HP > 500 {
				regenerated = true
			}
		}
		if overview.State != "running" || !regenerated || p.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("held equipment save blocked the actor or confirmed early")
		}
	}
	release()
	deadline := time.Now().Add(2 * time.Second)
	for p.Session.QueuedPacketsForTest() < 4 && time.Now().Before(deadline) {
		readCriticalLoopOverview(t, w)
	}
	stop() // Join before reading mutable actor state.
	if len(backend.entered) != 0 || len(queue.pending) != 0 || w.mobTickCounter < 3 || w.shuttingDown ||
		p.Char.Equip[fairySlot] != proposal.accounts[0].Chars[0].Equip[fairySlot] {
		t.Fatal("actual dispatch failed to settle the committed equipment once")
	}
	assertCriticalEquipmentPublication(t, p, request)
}
