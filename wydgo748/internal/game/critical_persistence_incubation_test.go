package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func criticalIncubationFixture(t *testing.T, failure error) (*World, *Player, *criticalTestStore, *criticalPersistence, func()) {
	t.Helper()
	w, p, _, backend, queue, release := criticalRecoveryFixture(t, failure)
	p.Char.CP = 0
	p.NextCPRecovery = w.now().Add(time.Hour)
	p.Char.Equip[mountSlot] = model.Item{Index: model.MountEggBase, UID: "incubating-egg"}
	setEggDelay(&p.Char.Equip[mountSlot], 3)
	p.EggIncubationUID, p.NextEggIncubationTick = "incubating-egg", w.now().Add(-time.Second)
	// This stale follower must not be reconciled during the same tick that
	// reserves the egg. It is removed by the next eligible tick after commit.
	pet := criticalSummonFixture(w, p)
	pet.SummonKind, pet.Def.Equip.Rosto.Index = summonKindMount, criaPetFaceBase
	p.NextMountTick = w.now().Add(time.Hour)
	return w, p, backend, queue, release
}

func TestCriticalEggHoursApplyOnlyAfterAcknowledgement(t *testing.T) {
	for _, hours := range []int{1, 2, 3, 7} {
		t.Run(time.Duration(hours).String(), func(t *testing.T) {
			w, p, backend, queue, release := criticalIncubationFixture(t, nil)
			now := w.now()
			p.NextEggIncubationTick = now.Add(-time.Duration(hours-1) * time.Hour)
			before, eggDeadline, foodDeadline := cloneCharacterState(p.Char), p.NextEggIncubationTick, p.NextMountTick
			pet := w.findCriaPet(p.ID)
			w.tickPlayerMounts(now)
			write := awaitCriticalWrite(t, backend.entered)
			charged := min(hours, 3)
			if eggDelay(write.accounts[0].Chars[p.CharSlot].Equip[mountSlot]) != 3-charged || write.accounts[0] == p.Account {
				t.Fatal("worker did not receive detached whole-hour proposal")
			}
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.tickPlayerMounts(now)
				if !reflect.DeepEqual(*p.Char, before) || !p.NextEggIncubationTick.Equal(eggDeadline) ||
					p.EggIncubationUID != "incubating-egg" || !p.NextMountTick.Equal(foodDeadline) ||
					w.findCriaPet(p.ID) != pet || pet.Dead || p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 {
					t.Fatalf("%s mutated clocks, reconciled follower, published or repeated save", phase)
				}
			}
			w.processCriticalCompletions(1)
			if eggDelay(p.Char.Equip[mountSlot]) != 3-charged || w.criticalPlayerReserved(p) || p.Session.QueuedPacketsForTest() != 2 {
				t.Fatal("completion omitted committed countdown, confirmation or fence release")
			}
			if charged == 3 {
				if p.EggIncubationUID != "" || !p.NextEggIncubationTick.IsZero() {
					t.Fatal("completed incubation retained its fractional clock")
				}
			} else if p.EggIncubationUID != "incubating-egg" || !p.NextEggIncubationTick.Equal(eggDeadline.Add(time.Duration(charged)*time.Hour)) {
				t.Fatal("partial incubation discarded fractional progress")
			}
			w.tickPlayerMounts(now)
			if w.findCriaPet(p.ID) != nil || !pet.Dead || !p.NextMountTick.IsZero() || len(queue.pending) != 0 {
				t.Fatal("postcommit tick did not reconcile clocks/follower or repeated charged hours")
			}
		})
	}
}

func TestCriticalEggAdmissionRejectionPreservesProposalAndClocks(t *testing.T) {
	for _, scenario := range []string{"full", "stopping", "identity", "missing-account"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, backend, queue, _ := criticalIncubationFixture(t, nil)
			switch scenario {
			case "full":
				for _, name := range []string{"occupied-a", "occupied-b"} {
					if _, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
						t.Fatal(err)
					}
				}
				awaitCriticalWrite(t, backend.entered)
			case "stopping":
				queue.closeAdmission()
			case "identity":
				delete(w.accountSessions, accountSessionKey(p.Account.Name))
			case "missing-account":
				p.Account = nil
			}
			before, deadline, foodDeadline, pending := cloneCharacterState(p.Char), p.NextEggIncubationTick, p.NextMountTick, len(queue.pending)
			// The direct producer boundary is being rejected. An unreserved
			// outer tick is still allowed to reconcile unrelated follower state.
			w.tickEquippedEggIncubation(p, w.now())
			if !reflect.DeepEqual(*p.Char, before) || !p.NextEggIncubationTick.Equal(deadline) ||
				p.EggIncubationUID != "incubating-egg" || !p.NextMountTick.Equal(foodDeadline) ||
				p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != pending || p.criticalLifecycle != nil {
				t.Fatal("rejection mutated, confirmed, reserved or attempted inline persistence")
			}
		})
	}
}

func TestCriticalEggUnknownCommitRetainsCountdownAndOwnership(t *testing.T) {
	w, p, backend, queue, release := criticalIncubationFixture(t, errors.New("commit acknowledgement lost"))
	before, deadline, foodDeadline := cloneCharacterState(p.Char), p.NextEggIncubationTick, p.NextMountTick
	pet := w.findCriaPet(p.ID)
	w.tickPlayerMounts(w.now())
	awaitCriticalWrite(t, backend.entered)
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	w.tickPlayerMounts(w.now())
	if !reflect.DeepEqual(*p.Char, before) || !p.NextEggIncubationTick.Equal(deadline) ||
		!p.NextMountTick.Equal(foodDeadline) || w.findCriaPet(p.ID) != pet || pet.Dead ||
		p.Session.QueuedPacketsForTest() != 0 || len(queue.pending) != 1 || !w.criticalPlayerReserved(p) ||
		!p.PersistencePoisoned || !p.Session.IsClosed() || !errors.Is(w.shutdownErr, errCriticalPersistenceIndeterminate) {
		t.Fatal("unknown incubation commit confirmed, retried, reconciled or released ownership")
	}
}
