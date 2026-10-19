package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCriticalKillSurvivesInstanceExpiryBeforeAcknowledgement(t *testing.T) {
	for _, outcome := range []string{"commit", "rejection"} {
		t.Run(outcome, func(t *testing.T) {
			w, p, mob, _ := killRewardTestWorld(t, nil)
			peer, _ := networkedTestPlayer(2, "ExpiryPeer", 2102, 2100)
			w.players[peer.Session], w.playersByID[peer.ID] = peer, peer
			w.indexPlayerCharacter(peer)
			w.updatePlayerSpatial(peer)
			claimCriticalTestPlayers(t, w, p, peer)
			inst := &ItemInstance{
				RuntimeID: "expiry-test", Config: model.VolatileInstance{ID: "expiry-test",
					Mode: "shared_timed_zone", FinishPolicy: "respawn_until_timeout", ExitX: 2100, ExitY: 2100},
				// Put an unreserved member first to cover rollback when exit
				// admission then discovers the killer's pending reservation.
				MemberIDs: []uint16{peer.ID, p.ID}, MobIDs: map[uint16]struct{}{mob.ID: {}}, Remaining: 1,
				HardDeadline: w.now().Add(time.Second),
			}
			w.itemInstances = map[string]*ItemInstance{inst.RuntimeID: inst}
			mob.InstanceID = inst.RuntimeID
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if outcome == "rejection" {
				backend.failure = criticalCertifiedRejection(t)
			}
			queue := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{backend}}, 1, 4)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			w.killMobState(p, mob, 500, 37)
			awaitCriticalWrite(t, backend.entered)
			peerBefore := accountStateSnapshot(peer.Account)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				packets := p.Session.QueuedPacketsForTest()
				w.tickItemInstances(inst.HardDeadline)
				if w.mobsByID[mob.ID] != mob || w.itemInstances[inst.RuntimeID] != inst ||
					p.Session.QueuedPacketsForTest() != packets || len(queue.pending) != 1 {
					t.Fatalf("%s: expiry changed an unpublished kill before acknowledgement", phase)
				}
				if !reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore) ||
					peer.X != 2102 || peer.Y != 2100 || peer.Session.QueuedPacketsForTest() != 0 {
					t.Fatalf("%s: busy expiry partially moved or notified another group member", phase)
				}
			}
			w.processCriticalCompletions(1)
			if w.shuttingDown || w.criticalPlayerReserved(p) || len(queue.pending) != 0 {
				t.Fatal("instance expiry invalidated kill settlement or retained its reservation")
			}
			if outcome == "commit" {
				if w.mobsByID[mob.ID] == mob || p.Char.Exp == 0 {
					t.Fatal("committed kill did not grant its reward and retire its mob")
				}
			} else if w.mobsByID[mob.ID] != mob || mob.Dead || mob.HP != 37 || p.Char.Exp != 0 {
				t.Fatal("certified rejection did not restore the unpublished kill")
			}
			// Once the kill settles, expiry retries normally without replaying
			// its reward, regardless of the original transaction outcome.
			exp := p.Char.Exp
			backend.failure = nil
			w.tickItemInstances(inst.HardDeadline)
			awaitCriticalWrite(t, backend.entered)
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if w.itemInstances[inst.RuntimeID] != nil || w.mobsByID[mob.ID] != nil ||
				w.shuttingDown || len(queue.pending) != 0 || p.Char.Exp != exp {
				t.Fatal("expiry did not recover after kill settlement or repeated its reward")
			}
			w.tickItemInstances(inst.HardDeadline.Add(time.Second))
			if len(queue.pending) != 0 || p.Char.Exp != exp {
				t.Fatal("closed instance repeated its exit or reward")
			}
		})
	}
}

func TestCriticalInstanceExpiryRetiresEntitiesOnlyAfterAcknowledgement(t *testing.T) {
	for _, outcome := range []string{"commit", "rejection", "unknown"} {
		t.Run(outcome, func(t *testing.T) {
			w, leader, member, _, clock := instanceTestWorld()
			claimCriticalTestPlayers(t, w, leader, member)
			inst := &ItemInstance{
				RuntimeID: "expiry-test", Config: model.VolatileInstance{ID: "expiry-test",
					Mode: "shared_timed_zone", FinishPolicy: "respawn_until_timeout", ExitX: 2050, ExitY: 2050},
				MemberIDs: []uint16{leader.ID, member.ID}, MobIDs: map[uint16]struct{}{1400: {}},
				NPCIDs: map[uint16]struct{}{1401: {}}, Remaining: 1, HardDeadline: clock.Now().Add(time.Second),
			}
			w.itemInstances[inst.RuntimeID] = inst
			mob := &Mob{ID: 1400, X: leader.X + 1, Y: leader.Y, HP: 100, GenerIndex: -1,
				InstanceID: inst.RuntimeID, Def: &model.NPCDef{Name: "ExpiryMob", Tipo: model.TipoMonstro}}
			npc := &Mob{ID: 1401, X: leader.X + 2, Y: leader.Y, HP: 100, GenerIndex: -1,
				InstanceID: inst.RuntimeID, Def: &model.NPCDef{Name: "ExpiryNPC"}}
			for _, entity := range []*Mob{mob, npc} {
				w.mobs = append(w.mobs, entity)
				w.registerMobSpatial(entity)
				leader.show(entity.ID)
				member.show(entity.ID)
			}
			w.rebuildPlayerInstanceIndex()
			before := []*model.Account{accountStateSnapshot(leader.Account), accountStateSnapshot(member.Account)}
			positions := [][2]uint16{{leader.X, leader.Y}, {member.X, member.Y}}
			gate, release := criticalGate(t)
			backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if outcome == "rejection" {
				backend.failure = criticalCertifiedRejection(t)
			} else if outcome == "unknown" {
				backend.failure = errors.New("instance exit acknowledgement unavailable")
			}
			queue := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{backend}}, 1, 4)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			assertUnpublished := func() {
				t.Helper()
				if w.itemInstances[inst.RuntimeID] != inst || w.mobsByID[mob.ID] != mob ||
					w.mobsByID[npc.ID] != npc || mob.Dead {
					t.Fatal("unconfirmed exit retired the instance or its entities")
				}
				for i, p := range []*Player{leader, member} {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before[i]) ||
						[2]uint16{p.X, p.Y} != positions[i] || p.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("unconfirmed exit changed a member or published teardown to an observer")
					}
				}
			}
			w.tickItemInstances(inst.HardDeadline)
			proposal := awaitCriticalWrite(t, backend.entered)
			if len(proposal.accounts) != 2 || proposal.instances == nil {
				t.Fatal("exit did not persist the entire group and instance together")
			}
			assertUnpublished()
			w.tickItemInstances(inst.HardDeadline.Add(time.Second))
			if len(queue.pending) != 1 {
				t.Fatal("pending expiry submitted a duplicate transition")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			assertUnpublished()
			w.processCriticalCompletions(1)
			if outcome != "commit" {
				assertUnpublished()
				if outcome == "unknown" {
					if !w.shuttingDown || !w.criticalInstanceReserved() || len(queue.pending) != 1 {
						t.Fatal("unknown exit escaped fail-closed containment")
					}
				} else if w.shuttingDown || w.criticalInstanceReserved() || len(queue.pending) != 0 {
					t.Fatal("certified exit rejection prevented safe retry")
				}
				return
			}
			if w.itemInstances[inst.RuntimeID] != nil || w.mobsByID[mob.ID] != nil ||
				w.mobsByID[npc.ID] != nil || w.shuttingDown || len(queue.pending) != 0 {
				t.Fatal("committed exit did not retire its entities and release reservations")
			}
			for _, p := range []*Player{leader, member} {
				if p.X != 2050 || p.Y != 2050 || p.Char.X != p.X || p.Char.Y != p.Y ||
					p.Session.QueuedPacketsForTest() == 0 || w.criticalPlayerReserved(p) {
					t.Fatal("committed group exit did not publish its durable destination")
				}
			}
		})
	}
}
