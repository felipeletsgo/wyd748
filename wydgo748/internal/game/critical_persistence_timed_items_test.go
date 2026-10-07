package game

import (
	"bytes"
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func seedCriticalTimedExpiry(w *World, p *Player) {
	seedCriticalEquipment(w, p)
	p.Account.Chars = append(p.Account.Chars, model.Char{Name: "Inactive", UID: "inactive-expiry"})
	p.Char = &p.Account.Chars[p.CharSlot]
	expired := func(index uint16, uid string) model.Item {
		return model.Item{Index: index, UID: uid, ActivatedUnix: w.now().Add(-time.Hour).Unix(), ExpiresUnix: w.now().Unix()}
	}
	p.Char.Equip[fairySlot] = expired(4300, "expired-fairy")
	p.Char.Equip[mountSlot] = criticalBabyMount()
	p.Char.Equip[mountSlot].UID = "expired-mount"
	p.Char.Equip[mountSlot].ActivatedUnix, p.Char.Equip[mountSlot].ExpiresUnix = w.now().Add(-time.Hour).Unix(), w.now().Unix()
	p.Char.Inv[3] = expired(4300, "expired-inventory")
	p.Account.Chars[1].Equip[fairySlot] = expired(4300, "expired-inactive-equipped")
	p.Account.Chars[1].Inv[8] = expired(4300, "expired-inactive-carried")
	p.Account.Cargo[7] = expired(4300, "expired-cargo")
	p.Account.Cargo[8] = expired(4300, "future-cargo")
	p.Account.Cargo[8].ExpiresUnix = w.now().Add(time.Hour).Unix()
	w.syncCriaPet(p)
	for p.Session.QueuedPacketsForTest() > 0 {
		p.Session.DequeuePacketForTest()
	}
}

func assertCriticalTimedExpiryPublication(t *testing.T, p *Player, removals []timedItemRemoval, removedPet *Mob) {
	t.Helper()
	expected := make([][]byte, 0, len(removals)+5)
	activeEquip := false
	for _, removal := range removals {
		if removal.charIndex == p.CharSlot || removal.place == placeStorage {
			expected = append(expected, wire.SendItem(p.ID, removal.place, removal.slot, model.Item{}))
		}
		activeEquip = activeEquip || removal.charIndex == p.CharSlot && removal.place == placeEquip
	}
	expected = append(expected, wire.MessagePanel("One or more timed items have expired."))
	if activeEquip {
		expected = append(expected, playerScorePacket(p), wire.UpdateEtc(p.ID, *p.Char))
		if removedPet != nil {
			expected = append(expected, wire.RemoveMob(removedPet.ID, 0))
		}
		expected = append(expected, playerAppearancePacket(p))
	}
	for sequence, want := range expected {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) {
			t.Fatalf("missing expiry response %d", sequence)
		}
		header := wire.ParseHeader(packet)
		if header.Type != wire.ParseHeader(want).Type || header.ID != wire.ParseHeader(want).ID ||
			int(header.Size) != len(want) || !bytes.Equal(packet[12:], want[12:]) {
			t.Fatalf("expiry response %d differs in order, recipient, size or full payload", sequence)
		}
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("unexpected additional expiry effect")
	}
}

func TestCriticalTimedExpiryDetachedAllLocationsAndSelectedCharacter(t *testing.T) {
	for _, scenario := range []string{"first-character", "second-character", "inventory-only", "inactive-only", "cargo-only"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			seedCriticalTimedExpiry(w, p)
			if scenario == "second-character" {
				p.Account.Chars[0], p.Account.Chars[1] = p.Account.Chars[1], p.Account.Chars[0]
				p.CharSlot, p.Char = 1, &p.Account.Chars[1]
			}
			if scenario != "first-character" && scenario != "second-character" {
				for i := range p.Account.Chars {
					p.Account.Chars[i].Equip = [model.MaxEquipSlots]model.Item{}
					p.Account.Chars[i].Inv = [model.MaxCarry]model.Item{}
				}
				p.Account.Cargo[7] = model.Item{}
				item := model.Item{Index: 4300, UID: "only-expired", ActivatedUnix: w.now().Add(-time.Hour).Unix(), ExpiresUnix: w.now().Unix()}
				switch scenario {
				case "inventory-only":
					p.Char.Inv[3] = item
				case "inactive-only":
					p.Account.Chars[1].Inv[8] = item
				case "cargo-only":
					p.Account.Cargo[7] = item
				}
				w.syncCriaPet(p)
				for p.Session.QueuedPacketsForTest() > 0 {
					p.Session.DequeuePacketForTest()
				}
			}
			before := accountStateSnapshot(p.Account)
			account, character := p.Account, p.Char
			previousPet := w.findCriaPet(p.ID)
			expected := accountStateSnapshot(p.Account)
			removals := expireTimedAccountItems(expected, w.now())
			wantCount := 6
			if scenario != "first-character" && scenario != "second-character" {
				wantCount = 1
			}
			if len(removals) != wantCount {
				t.Fatal("fixture does not cover the intended expired locations")
			}
			w.tickTimedItems(w.now())
			proposal := awaitCriticalWrite(t, backend.entered)
			if len(proposal.accounts) != 1 || proposal.accounts[0] == p.Account || proposal.character != nil ||
				proposal.instances != nil || !reflect.DeepEqual(proposal.accounts[0], accountPersistenceSnapshot(expected)) {
				t.Fatal("worker did not receive the exact detached account-wide expiry proposal")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.tickTimedItems(w.now().Add(timedItemSweepInterval * 2))
				w.syncCriaPet(p)
				assertCriticalCargoUnchanged(t, p, before, fallback)
				if p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || len(backend.entered) != 0 ||
					!w.criticalPlayerReserved(p) || w.findCriaPet(p.ID) != previousPet {
					t.Fatalf("%s published expiry, changed followers, repeated or released the account", phase)
				}
			}
			w.processCriticalCompletions(1)
			if wantCount == 6 {
				w.recalcPlayer(&expected.Chars[p.CharSlot])
			}
			if p.Account != account || p.Char != character || w.shuttingDown || w.criticalPlayerReserved(p) || len(queue.pending) != 0 ||
				!reflect.DeepEqual(accountStateSnapshot(p.Account), expected) || fallback.saves != 0 || p.Account.Cargo[8].UID != "future-cargo" {
				t.Fatal("expiry acknowledgement lost identity, unrelated state, score or future item")
			}
			if previousPet != nil && (!previousPet.Dead || w.findCriaPet(p.ID) != nil) {
				t.Fatal("expiry retained the removed mount follower")
			}
			assertCriticalTimedExpiryPublication(t, p, removals, previousPet)
			w.tickTimedItems(w.now().Add(timedItemSweepInterval * 4))
			if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("later sweep replayed acknowledged expiry")
			}
		})
	}
}

func TestCriticalTimedExpiryAdmissionRetryAndUnknownOutcomeContainment(t *testing.T) {
	for _, scenario := range []string{"full", "stopping", "closed", "no-expiry", "store-error", "store-panic", "identity"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, fallback, backend, queue, release := criticalCargoFixture(t)
			seedCriticalTimedExpiry(w, p)
			switch scenario {
			case "full":
				for _, name := range []string{"other-a", "other-b"} {
					if _, err := w.criticalWrites.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}, func(criticalPersistenceResult) error { return nil }); err != nil {
						t.Fatal(err)
					}
				}
				awaitCriticalWrite(t, backend.entered)
			case "stopping":
				queue.closeAdmission()
			case "closed":
				p.Session.Close()
			case "no-expiry":
				expireTimedAccountItems(p.Account, w.now())
				w.syncCriaPet(p)
			case "store-error":
				backend.failure = errors.New("expiry commit acknowledgement lost")
			case "store-panic":
				backend.panicAfterWrite = true
			}
			for p.Session.QueuedPacketsForTest() > 0 {
				p.Session.DequeuePacketForTest()
			}
			before := accountStateSnapshot(p.Account)
			previousPet := w.findCriaPet(p.ID)
			w.tickTimedItems(w.now())
			unknown := scenario == "store-error" || scenario == "store-panic" || scenario == "identity"
			if unknown {
				awaitCriticalWrite(t, backend.entered)
				if scenario == "identity" {
					p.criticalLifecycle = &criticalPlayerLifecycle{}
				}
				release()
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
				if !w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() || len(queue.pending) != 1 {
					t.Fatal("unknown/stale expiry escaped containment")
				}
			} else if w.criticalPlayerReserved(p) || len(backend.entered) != 0 {
				t.Fatal("rejected/no-op expiry admitted a write or reserved the account")
			}
			assertCriticalCargoUnchanged(t, p, before, fallback)
			if p.Session.QueuedPacketsForTest() != 0 || w.findCriaPet(p.ID) != previousPet {
				t.Fatal("failed expiry published removals or follower effects")
			}
			if scenario != "full" {
				return
			}
			release()
			awaitReadyCriticalResults(t, queue, 2)
			w.processCriticalCompletions(2)
			awaitCriticalWrite(t, backend.entered) // Second unrelated admitted job.
			w.tickTimedItems(w.now().Add(timedItemSweepInterval))
			awaitCriticalWrite(t, backend.entered)
			assertCriticalCargoUnchanged(t, p, before, fallback)
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if len(queue.pending) != 0 || w.shuttingDown || p.Char.Equip[fairySlot].Index != 0 {
				t.Fatal("rejected expiry did not recover at the next eligible sweep")
			}
		})
	}
}

func TestCriticalTimedExpiryCommittedLifecycleSnapshots(t *testing.T) {
	testCriticalItemLifecycle(t, seedCriticalTimedExpiry, func(w *World, _ *Player) { w.tickTimedItems(w.now()) })
}

func TestCriticalTimedExpiryActualWorldLoopProgressAndUnknownOutcome(t *testing.T) {
	for _, unknown := range []bool{false, true} {
		name, failure := "success", error(nil)
		if unknown {
			name, failure = "unknown", errors.New("expiry commit acknowledgement lost")
		}
		t.Run(name, func(t *testing.T) {
			var before *model.Account
			w, p, unrelatedUID, backend, queue, ticks, release, stop := criticalEconomyLoopFixture(t, failure, func(w *World, p *Player) {
				seedCriticalTimedExpiry(w, p)
				before = accountStateSnapshot(p.Account)
			})
			deliverCriticalLoopTick(t, ticks)
			proposal := awaitCriticalWrite(t, backend.entered)
			for range 3 {
				deliverCriticalLoopTick(t, ticks)
				overview := readCriticalLoopOverview(t, w)
				progress := false
				for _, player := range overview.Players {
					if player.UID == unrelatedUID && player.HP > 500 {
						progress = true
					}
				}
				if overview.State != "running" || !progress || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("held expiry blocked World or confirmed early")
				}
			}
			release()
			deadline := time.Now().Add(2 * time.Second)
			for time.Now().Before(deadline) {
				overview := readCriticalLoopOverview(t, w)
				if unknown && overview.State != "running" || !unknown && p.Session.QueuedPacketsForTest() > 0 {
					break
				}
			}
			stop()
			if len(backend.entered) != 0 || w.mobTickCounter < 4 {
				t.Fatal("actual sweep replayed or did not advance unrelated ticks")
			}
			if unknown {
				if !w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() || len(queue.pending) != 1 ||
					p.Session.QueuedPacketsForTest() != 0 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
					t.Fatal("actual unknown expiry applied, published or released ownership")
				}
				return
			}
			if w.shuttingDown || len(queue.pending) != 0 || p.Char.Equip[fairySlot] != proposal.accounts[0].Chars[0].Equip[fairySlot] || p.Account.Cargo[7].Index != 0 {
				t.Fatal("actual expiry failed to acknowledge committed removals")
			}
		})
	}
}
