package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCriticalInstanceConsumersRestoreBeforeAdmission(t *testing.T) {
	for _, flow := range []string{"room-move", "quiz", "big-cube", "expiry-exit"} {
		for _, outcome := range []string{"commit", "rejected", "unknown"} {
			t.Run(flow+"/"+outcome, func(t *testing.T) {
				w, leader, member, _, clock := instanceTestWorld()
				cfg := *instanceTestRule().Instance
				cfg.ExitX, cfg.ExitY = 2050, 2050
				quiz := &model.VolatileInstanceQuiz{Answer: true, TrueX: 2200, TrueY: 2200,
					FalseX: 2300, FalseY: 2300, RewardExp: 100, DurationSeconds: 10}
				cfg.Stages = []model.VolatileInstanceStage{{Name: "Question", X: 2250, Y: 2250, Quiz: quiz}}
				if flow == "big-cube" {
					cfg.StateMachine = "big_cube"
				}
				inst := &ItemInstance{Config: cfg, RuntimeID: cfg.ID, LeaderID: leader.ID,
					MemberIDs: []uint16{leader.ID, member.ID}, QuizAt: clock.Now().Add(time.Second),
					QuizDeadline: clock.Now().Add(time.Second), MobIDs: make(map[uint16]struct{})}
				w.itemInstances[inst.RuntimeID] = inst
				claimCriticalTestPlayers(t, w, leader, member)
				for _, p := range []*Player{leader, member} {
					w.indexPlayerCharacter(p)
					w.updatePlayerSpatial(p)
				}
				w.rebuildPlayerInstanceIndex()
				before := []*model.Account{accountStateSnapshot(leader.Account), accountStateSnapshot(member.Account)}
				oldPositions := [][2]uint16{{leader.X, leader.Y}, {member.X, member.Y}}
				oldInstance := cloneCriticalInstanceMembership(w).itemInstances[inst.RuntimeID]
				gate, release := criticalGate(t)
				base := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
				if outcome == "rejected" {
					base.failure = criticalCertifiedRejection(t)
				} else if outcome == "unknown" {
					base.failure = errors.New("instance transaction outcome unavailable")
				}
				q := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(q)
				switch flow {
				case "room-move":
					if !w.moveInstanceMembersAtomic(inst, cfg.Stages[0], nil, nil) {
						t.Fatal("room move was not admitted")
					}
				case "quiz":
					if !w.resolveItemInstanceQuiz(inst, clock.Now()) {
						t.Fatal("quiz answer was not admitted")
					}
				case "big-cube":
					if !w.resolveBigCubeQuestion(inst, clock.Now()) {
						t.Fatal("Big Cube answer was not admitted")
					}
				case "expiry-exit":
					w.queueCriticalInstanceExit(inst.RuntimeID, inst)
				}
				proposal := awaitCriticalWrite(t, base.entered)
				if proposal.instances == nil || len(proposal.accounts) != 2 {
					t.Fatal("transition did not submit a combined detached proposal")
				}
				assertRestored := func() {
					t.Helper()
					for i, p := range []*Player{leader, member} {
						if !reflect.DeepEqual(accountStateSnapshot(p.Account), before[i]) ||
							[2]uint16{p.X, p.Y} != oldPositions[i] || p.Session.QueuedPacketsForTest() != 0 {
							t.Fatal("pending transition exposed player state or confirmation")
						}
					}
					if !reflect.DeepEqual(inst, oldInstance) || w.itemInstances[inst.RuntimeID] != inst {
						t.Fatal("pending transition exposed speculative instance state")
					}
				}
				assertRestored()
				release()
				awaitReadyCriticalResults(t, q, 1)
				assertRestored()
				w.processCriticalCompletions(1)
				if outcome == "commit" {
					for _, p := range []*Player{leader, member} {
						var want *model.Account
						for _, a := range proposal.accounts {
							if a.Name == p.Account.Name {
								want = a
							}
						}
						// Durable accounts intentionally normalize entry coordinates;
						// active positions come from the actor-owned detached plan.
						if want == nil || p.X != p.Char.X || p.Y != p.Char.Y ||
							[2]uint16{p.X, p.Y} == oldPositions[int(p.ID)-1] || p.Session.QueuedPacketsForTest() == 0 {
							t.Fatal("confirmed transition did not install and publish its destination")
						}
						if p.Char != &p.Account.Chars[p.CharSlot] {
							t.Fatal("transition broke the live character/account pointer identity")
						}
					}
					if len(q.pending) != 0 || w.shuttingDown {
						t.Fatal("successful transition retained reservations or stopped the World")
					}
				} else {
					assertRestored()
					if outcome == "unknown" {
						if !w.shuttingDown || !w.criticalInstanceReserved() || len(q.pending) != 1 {
							t.Fatal("unknown instance result escaped global containment")
						}
					} else if w.shuttingDown || len(q.pending) != 0 {
						t.Fatal("certified rejection did not release admission safely")
					}
				}
			})
		}
	}
}

func TestCriticalUxmalRewardPublicationAndMarker(t *testing.T) {
	for _, full := range []bool{false, true} {
		for _, outcome := range []string{"commit", "rejected", "unknown", "changed-ground-commit", "changed-ground-rejected", "admission-stopped"} {
			if !full && (outcome == "changed-ground-commit" || outcome == "changed-ground-rejected") {
				continue
			}
			name := "inventory/"
			if full {
				name = "ground/"
			}
			t.Run(name+outcome, func(t *testing.T) {
				w, leader, member, _, clock := uxmalTestWorld()
				for i, p := range []*Player{leader, member} {
					p.Char.UID = []string{"00000000-0000-0000-0000-000000000001", "00000000-0000-0000-0000-000000000002"}[i]
					w.indexPlayerCharacter(p)
					if full {
						for slot := range p.Char.Inv {
							p.Char.Inv[slot] = model.Item{Index: 7001}
						}
					}
				}
				claimCriticalTestPlayers(t, w, leader, member)
				inst := &ItemInstance{Config: *w.uxmal, RuntimeID: "uxmal:0:0", LeaderID: leader.ID,
					MemberIDs: []uint16{leader.ID, member.ID}}
				w.itemInstances[inst.RuntimeID] = inst
				// Establish the existing party's visibility before measuring new
				// packets; ordinary initial peer spawns are not reward publication.
				for _, p := range []*Player{leader, member} {
					w.refreshPlayerVisibility(p)
					for p.Session.QueuedPacketsForTest() > 0 {
						p.Session.DequeuePacketForTest()
					}
				}
				before := []*model.Account{accountStateSnapshot(leader.Account), accountStateSnapshot(member.Account)}
				gate, release := criticalGate(t)
				base := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
				if outcome == "rejected" || outcome == "changed-ground-rejected" {
					base.failure = criticalCertifiedRejection(t)
				} else if outcome == "unknown" {
					base.failure = errors.New("Uxmal reward acknowledgement lost")
				}
				q := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(q)
				if outcome == "admission-stopped" {
					w.shuttingDown = true
					if w.grantUxmalRewards(inst, clock.Now()) || len(w.groundItems) != 0 || inst.RewardGranted || len(q.pending) != 0 {
						t.Fatal("failed admission retained allocations or a reward marker")
					}
					for i, p := range []*Player{leader, member} {
						if !reflect.DeepEqual(accountStateSnapshot(p.Account), before[i]) || p.Session.QueuedPacketsForTest() != 0 {
							t.Fatal("failed admission changed the account or published rewards")
						}
					}
					return
				}
				if !w.grantUxmalRewards(inst, clock.Now()) {
					t.Fatal("reward was not admitted")
				}
				proposal := awaitCriticalWrite(t, base.entered)
				if proposal.instances == nil || len(proposal.accounts) != 2 || len(proposal.instances.Instances) != 0 {
					t.Fatal("reward did not preserve the combined write and process-local Uxmal contract")
				}
				assertHidden := func() {
					t.Helper()
					if inst.RewardGranted || !inst.ExitAt.IsZero() {
						t.Fatal("reward marker became live before confirmation")
					}
					for i, p := range []*Player{leader, member} {
						w.refreshPlayerVisibility(p)
						if !reflect.DeepEqual(accountStateSnapshot(p.Account), before[i]) || p.Session.QueuedPacketsForTest() != 0 {
							t.Fatal("unconfirmed rewards became visible")
						}
					}
					for _, g := range w.groundItems {
						if !g.criticalReserved() {
							t.Fatal("pending ground reward lost its reservation")
						}
					}
				}
				assertHidden()
				if outcome == "changed-ground-commit" || outcome == "changed-ground-rejected" {
					for _, g := range w.groundItems {
						g.X++
						break
					}
				}
				release()
				awaitReadyCriticalResults(t, q, 1)
				assertHidden()
				w.processCriticalCompletions(1)
				switch outcome {
				case "commit":
					if !inst.RewardGranted || inst.ExitAt.IsZero() || len(q.pending) != 0 || w.shuttingDown {
						t.Fatal("confirmed reward was not installed")
					}
					for _, p := range []*Player{leader, member} {
						if p.Session.QueuedPacketsForTest() == 0 {
							t.Fatal("confirmed reward was not published")
						}
					}
					if full && len(w.groundItems) != 3 {
						t.Fatal("full inventories did not retain all three rewards")
					}
					for _, g := range w.groundItems {
						if g.criticalReserved() {
							t.Fatal("confirmed ground reward stayed reserved")
						}
					}
					if !w.grantUxmalRewards(inst, clock.Now()) || len(q.pending) != 0 {
						t.Fatal("repeated completed grant submitted another write")
					}
				case "rejected":
					assertHidden()
					if len(w.groundItems) != 0 || inst.Remaining != 1 || len(q.pending) != 0 || w.shuttingDown {
						t.Fatal("rejection leaked allocations or prevented a safe retry")
					}
				case "unknown", "changed-ground-commit", "changed-ground-rejected":
					assertHidden()
					if !w.shuttingDown || !w.criticalInstanceReserved() || len(q.pending) != 1 || full && len(w.groundItems) != 3 {
						t.Fatal("unknown reward escaped containment")
					}
				}
			})
		}
	}
}
