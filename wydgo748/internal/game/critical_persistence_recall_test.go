package game

import (
	"errors"
	"maps"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCriticalRecallDetachedDepartureAndRevival(t *testing.T) {
	for _, room := range []bool{false, true} {
		for _, outcome := range []string{"commit", "rejection", "unknown", "stale-runtime", "closed"} {
			t.Run(map[bool]string{false: "public/", true: "water/"}[room]+outcome, func(t *testing.T) {
				w, p, _ := handlerTestWorld(t)
				w.clock = newFakeClock(time.Date(2026, 10, 5, 12, 0, 0, 0, time.UTC))
				p.Char.UID = "recall-owner"
				w.playersByCharacterUID[p.Char.UID] = p
				if !w.claimAccountSession(p.Session, p.Account.Name) {
					t.Fatal("recall account admission failed")
				}
				setPlayerCurHP(p.Char, 0)
				p.DeadAt = w.now().Add(-10 * time.Second)
				var inst *ItemInstance
				if room {
					w.itemInstances = make(map[string]*ItemInstance)
					w.pendingInstanceMembers = make(map[string]map[string]struct{})
					w.pendingInstanceLeaders = make(map[string]string)
					inst = &ItemInstance{Config: model.VolatileInstance{ID: "water-recall"}, RuntimeID: "water-recall",
						LeaderID: p.ID, MemberIDs: []uint16{p.ID}, MemberCharacterUIDs: []string{p.Char.UID},
						LeaderCharacterUID: p.Char.UID, HardDeadline: w.now().Add(time.Hour)}
					w.itemInstances[inst.RuntimeID] = inst
					w.setPlayerInstanceIndex(p.ID, inst.RuntimeID)
					w.pendingInstanceMembers[inst.RuntimeID] = map[string]struct{}{p.Char.UID: {}}
					w.pendingInstanceLeaders[inst.RuntimeID] = p.Char.UID
				}
				gate, release := criticalGate(t)
				base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
				if outcome == "rejection" {
					base.failure = criticalCertifiedRejection(t)
				} else if outcome == "unknown" {
					base.failure = errors.New("recall commit acknowledgement lost")
				}
				q := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 4)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(q)
				fallback := &guildFlowStore{}
				w.store = fallback
				before := accountStateSnapshot(p.Account)
				membership := cloneCriticalInstanceMembership(w)
				oldX, oldY, deadAt := p.X, p.Y, p.DeadAt
				if !w.recallPlayer(p, "restart") {
					t.Fatal("recall proposal was not admitted")
				}
				proposal := awaitCriticalWrite(t, base.entered)
				if playerCurHP(&proposal.accounts[0].Chars[p.CharSlot]) == 0 || (proposal.instances != nil) != room {
					t.Fatal("recall omitted revival or aggregate departure")
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, q, 1)
					}
					repeated := w.recallPlayer(p, "repeat")
					// Compare normalized clones: a World without rooms keeps nil maps.
					current := cloneCriticalInstanceMembership(w)
					if repeated || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
						p.X != oldX || p.Y != oldY || p.DeadAt != deadAt ||
						!reflect.DeepEqual(current.itemInstances, membership.itemInstances) ||
						!reflect.DeepEqual(current.pendingInstanceMembers, membership.pendingInstanceMembers) ||
						!maps.Equal(w.playerInstance, membership.playerInstance) || len(base.entered) != 0 ||
						p.Session.QueuedPacketsForTest() != 0 {
						t.Fatalf("%s recall changed or published retained state", phase)
					}
				}
				if outcome == "stale-runtime" {
					if room {
						replacement := *inst
						w.itemInstances[inst.RuntimeID] = &replacement
					} else {
						p.Char.UID = "replacement-recall-owner"
					}
					before = accountStateSnapshot(p.Account)
				} else if outcome == "closed" {
					p.Session.Close()
				}
				w.processCriticalCompletions(1)
				success := outcome == "commit" || outcome == "closed"
				if success {
					if w.shuttingDown || !reflect.DeepEqual(accountPersistenceSnapshot(p.Account), proposal.accounts[0]) ||
						!p.DeadAt.IsZero() || playerCurHP(p.Char) == 0 || len(q.pending) != 0 {
						t.Fatal("recall did not install its durable proposal")
					}
					if room && (w.itemInstances[inst.RuntimeID] != inst || itemInstanceHasMember(inst, p.ID) ||
						w.playerRuntimeInstanceID(p.ID) != "" || len(w.pendingInstanceMembers[inst.RuntimeID]) != 0 ||
						!reflect.DeepEqual(w.instanceStateSnapshot(), proposal.instances)) {
						t.Fatal("acknowledged departure retained room ownership or changed runtime identity")
					}
				} else {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.X != oldX || p.Y != oldY || p.DeadAt != deadAt {
						t.Fatal("unverified recall installed position or revival")
					}
					if outcome == "rejection" {
						if w.shuttingDown || len(q.pending) != 0 || w.criticalPlayerReserved(p) || w.criticalInstanceReserved() {
							t.Fatal("certified rejection did not release the recall reservation")
						}
					} else if !w.shuttingDown || len(q.pending) != 1 || !w.criticalPlayerReserved(p) {
						t.Fatal("unverified recall lost containment")
					}
				}
				if fallback.saves != 0 || fallback.gameSaves != 0 {
					t.Fatal("recall saved synchronously on World")
				}
				if outcome != "closed" {
					assertInstanceHUDCleared(t, p, room && success)
				}
			})
		}
	}
}
