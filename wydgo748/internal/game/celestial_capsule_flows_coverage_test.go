package game

import (
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func TestUseCelestialCapsuleRejectsInvalidAccountStates(t *testing.T) {
	tests := []struct {
		name  string
		setup func(*World, *Player)
	}{
		{
			name: "non-Celestial evolution",
			setup: func(_ *World, p *Player) {
				p.Char.Evolution = "mortal"
			},
		},
		{
			name: "missing character UID",
			setup: func(_ *World, p *Player) {
				p.Char.UID = ""
			},
		},
		{
			name: "missing progression score",
			setup: func(_ *World, p *Player) {
				p.Char.Score = nil
			},
		},
		{
			name: "character already sealed",
			setup: func(_ *World, p *Player) {
				p.Account.CelestialCapsules = append(p.Account.CelestialCapsules, model.CelestialCapsule{
					ID: 2, SourceUID: p.Char.UID,
				})
			},
		},
		{
			name: "cargo full",
			setup: func(_ *World, p *Player) {
				for i := range p.Account.Cargo {
					p.Account.Cargo[i] = model.Item{Index: 1}
				}
			},
		},
		{
			name: "invalid active character slot",
			setup: func(_ *World, p *Player) {
				p.CharSlot = len(p.Account.Chars)
			},
		},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, p, _ := celestialExtractionFixture(t)
			p.Char.Evolution = "celestial"
			tc.setup(w, p)
			beforeAccount := accountStateSnapshot(p.Account)
			beforeWorld := p.InWorld

			w.useCelestialCapsule(p.Session, p, &p.Char.Inv[0], 0)

			if !reflect.DeepEqual(accountStateSnapshot(p.Account), beforeAccount) || p.InWorld != beforeWorld || p.Char == nil {
				t.Fatalf("rejected seal use changed actor state: inWorld=%v char=%+v account=%+v",
					p.InWorld, p.Char, p.Account)
			}
			if p.Session.QueuedPacketsForTest() == 0 {
				t.Fatal("rejected seal use did not resynchronize the client")
			}
		})
	}
}

func TestCriticalCelestialExtractionSettlesOnlyAfterPersistence(t *testing.T) {
	for _, tc := range []struct {
		name      string
		failure   error
		admit     bool
		committed bool
	}{
		{name: "committed", admit: true, committed: true},
		{name: "certified rejection", failure: criticalCertifiedRejection(t), admit: true},
		{name: "admission closed", admit: false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, seal := celestialExtractionFixture(t)
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("failed to claim the extraction account session")
			}
			base := &criticalTestStore{entered: make(chan criticalWrite, 1), failure: tc.failure}
			backend := &criticalStateTestStore{&criticalBatchTestStore{base}}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			if !tc.admit {
				queue.closeAdmission()
			}
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			uncommitted := accountStateSnapshot(p.Account)
			uncommitted.Chars = append(uncommitted.Chars, model.Char{})

			w.onPutoutSeal(p.Session, putoutSealPacket(placeInv, 0, 2100, 2100, "Restored"))

			if tc.admit {
				proposal := awaitCriticalWrite(t, base.entered)
				if len(proposal.accounts) != 1 || proposal.accounts[0] == p.Account ||
					len(proposal.accounts[0].Chars) != 2 || proposal.accounts[0].Chars[1].Name != "Restored" ||
					proposal.character == nil || proposal.character.uid != p.Char.UID {
					t.Fatalf("extraction write did not contain the detached account and active sidecar: %+v", proposal)
				}
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), uncommitted) || !p.InWorld || p.Char == nil ||
					p.Char.Inv[0] != seal || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("uncommitted extraction changed the live actor or sent a result")
				}
				w.applyCriticalCompletion(awaitCriticalResult(t, queue))
			} else if !reflect.DeepEqual(accountStateSnapshot(p.Account), uncommitted) {
				t.Fatal("rejected extraction admission changed existing account data")
			}

			if tc.committed {
				if p.InWorld || p.Char != nil || len(p.Account.CelestialCapsules) != 0 ||
					p.Account.Chars[1].Name != "Restored" || p.Account.Chars[0].Inv[0] != (model.Item{}) {
					t.Fatalf("committed extraction was not installed: inWorld=%v chars=%+v capsules=%+v",
						p.InWorld, p.Account.Chars, p.Account.CelestialCapsules)
				}
				if _, held := w.charNames["restored"]; !held {
					t.Fatal("committed character name was not retained")
				}
				return
			}

			if !p.InWorld || p.Char == nil || p.Char != &p.Account.Chars[0] ||
				p.Char.Inv[0] != seal || len(p.Account.Chars) != 2 || p.Account.Chars[1].Name != "" ||
				len(p.Account.CelestialCapsules) != 1 {
				t.Fatalf("failed extraction did not restore the source seal and character: inWorld=%v chars=%+v capsules=%+v",
					p.InWorld, p.Account.Chars, p.Account.CelestialCapsules)
			}
			if _, held := w.charNames["restored"]; held {
				t.Fatal("failed extraction left its name reserved")
			}
			if p.Session.QueuedPacketsForTest() == 0 {
				t.Fatal("failed extraction did not report the rejection")
			}
		})
	}
}

func TestCriticalCelestialSealingAdmissionFailureRestoresState(t *testing.T) {
	w, p, _, queue, _ := criticalCelestialQueueFixture(t, 399)
	p.Char.Evolution = "celestial"
	p.Char.Inv[0] = model.Item{Index: model.CelestialSealItem}
	w.items = map[uint16]model.ItemDef{
		model.CelestialSealItem: {Index: model.CelestialSealItem, Name: "Spirit's Seal"},
	}
	w.volatiles = model.VolatileCatalog{
		Items:     map[uint16]model.VolatileRule{model.CelestialSealItem: {Action: "celestial_capsule", Consume: true}},
		ItemCodes: map[uint16]int{model.CelestialSealItem: 206},
	}
	queue.closeAdmission()
	before := accountStateSnapshot(p.Account)

	w.onUseItem(p.Session, useItemPacket(0, 0))
	if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !p.InWorld || p.Char == nil ||
		p.Char.Inv[0].Index != model.CelestialSealItem || p.Session.QueuedPacketsForTest() == 0 || len(queue.pending) != 0 {
		t.Fatal("rejected critical seal admission did not restore the live character and source item")
	}
}
