package game

import (
	"bytes"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func criticalSummonFixture(w *World, owner *Player) *Mob {
	summon := &Mob{ID: 1201, X: owner.X, Y: owner.Y, HP: 1000, GenerIndex: -1,
		SummonerID: owner.ID, SummonKind: summonKindBM, SummonRange: 6,
		Def: testNPCDef(model.Score{MaxHP: 1000, CurHP: 1000, MaxMP: 100,
			Attack: 1000, Accuracy: 1000})}
	w.appendMobInstance(summon)
	w.registerMobSpatial(summon)
	return summon
}

func TestCriticalSummonOwnerPausesCombatFollowAndRebinding(t *testing.T) {
	for _, scenario := range []string{"mob-combat", "player-combat", "mount-follow", "bm-follow", "thorn-wall"} {
		t.Run(scenario, func(t *testing.T) {
			w, owner, target, mob := manaControlCombatFixture(t, true)
			summon := criticalSummonFixture(w, owner)
			summon.InstanceID = "previous-runtime"
			switch scenario {
			case "mob-combat":
				owner.CombatTargetID = mob.ID
			case "player-combat":
				owner.CombatTargetID = target.ID
			case "mount-follow", "bm-follow":
				if scenario == "mount-follow" {
					summon.SummonKind = summonKindMount
				}
				oldX, oldY := summon.X, summon.Y
				summon.X += 8
				w.moveMobSpatial(summon, oldX, oldY)
			case "thorn-wall":
				summon.SummonKind = summonKindThornWall
			}
			beforeSummon, beforeMob := *summon, *mob
			beforeOwner, beforeTarget := cloneCharacterState(owner.Char), cloneCharacterState(target.Char)
			command := owner.CombatTargetID
			drainPlayerPackets(owner)
			drainPlayerPackets(target)
			release, queue := holdCriticalCombatParticipant(t, w, owner)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.tickSummonCombat(w.now())
				assertCriticalCombatUnchanged(t, owner, beforeOwner, 0, time.Time{})
				assertCriticalCombatUnchanged(t, target, beforeTarget, 0, time.Time{})
				if !reflect.DeepEqual(*summon, beforeSummon) || !reflect.DeepEqual(*mob, beforeMob) || owner.CombatTargetID != command {
					t.Fatalf("%s changed summon, target mob or owner command", phase)
				}
			}
			w.processCriticalCompletions(1)
			w.tickSummonCombat(w.now())
			if summon.InstanceID == beforeSummon.InstanceID {
				t.Fatal("summon did not rebind after owner acknowledgement")
			}
			switch scenario {
			case "mob-combat":
				if mob.HP >= beforeMob.HP || summon.NextAttack.IsZero() {
					t.Fatal("summon mob attack did not resume")
				}
			case "player-combat":
				if playerCurHP(target.Char) >= playerCurHP(&beforeTarget) || playerCurMP(target.Char) >= playerCurMP(&beforeTarget) {
					t.Fatal("summon PvP and Mana Control did not resume")
				}
			case "mount-follow", "bm-follow":
				if summon.X == beforeSummon.X && summon.Y == beforeSummon.Y {
					t.Fatal("summon follow movement did not resume")
				}
			}
		})
	}
}

func TestCriticalSummonTargetReservationRetainsBothCommands(t *testing.T) {
	for _, scenario := range []string{"primary", "fallback", "invalid-primary"} {
		t.Run(scenario, func(t *testing.T) {
			w, owner, target, _ := manaControlCombatFixture(t, true)
			summon := criticalSummonFixture(w, owner)
			summon.TargetID = target.ID
			summon.InstanceID = "previous-runtime"
			owner.LastAttackerID = target.ID
			if scenario == "primary" {
				owner.CombatTargetID = target.ID
			} else if scenario == "invalid-primary" {
				owner.CombatTargetID = 999
			}
			command, fallback, beforeSummon := owner.CombatTargetID, owner.LastAttackerID, *summon
			beforeOwner, beforeTarget := cloneCharacterState(owner.Char), cloneCharacterState(target.Char)
			release, queue := holdCriticalCombatParticipant(t, w, target)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				if got := w.ownerSummonTarget(owner); !got.paused || got.id != 0 {
					t.Fatal("reserved target was not recognized as a paused command")
				}
				w.tickSummonCombat(w.now())
				assertCriticalCombatUnchanged(t, owner, beforeOwner, fallback, time.Time{})
				assertCriticalCombatUnchanged(t, target, beforeTarget, 0, time.Time{})
				if owner.CombatTargetID != command || owner.LastAttackerID != fallback || !reflect.DeepEqual(*summon, beforeSummon) {
					t.Fatalf("%s consumed a command or summon deadline", phase)
				}
			}
			w.processCriticalCompletions(1)
			w.tickSummonCombat(w.now())
			if playerCurHP(target.Char) >= playerCurHP(&beforeTarget) || summon.TargetID != target.ID || summon.NextAttack.IsZero() {
				t.Fatal("retained summon command did not resume after acknowledgement")
			}
		})
	}
}

func TestCriticalSummonKillPreflightsPartyRewardRecipient(t *testing.T) {
	for _, command := range []string{"primary", "fallback"} {
		t.Run(command, func(t *testing.T) {
			w, owner, mob, store := killRewardTestWorld(t, nil)
			w.clock = newFakeClock(time.Date(2026, 10, 4, 12, 0, 0, 0, time.UTC))
			mob.HP = 100
			recipient, _ := networkedTestPlayer(2, "RewardRecipient", 2102, 2100)
			w.players[recipient.Session], w.playersByID[recipient.ID] = recipient, recipient
			w.indexPlayerCharacter(recipient)
			w.updatePlayerSpatial(recipient)
			party := &Party{Members: []*Player{owner, recipient}}
			owner.Party, recipient.Party = party, party
			summon := criticalSummonFixture(w, owner)
			if command == "primary" {
				owner.CombatTargetID = mob.ID
			} else {
				owner.CombatTargetID, owner.LastAttackerID = 999, mob.ID
			}
			primary, fallback := owner.CombatTargetID, owner.LastAttackerID
			beforeSummon, beforeMob := *summon, *mob
			beforeOwner, beforeRecipient := cloneCharacterState(owner.Char), cloneCharacterState(recipient.Char)
			drainPlayerPackets(owner)
			drainPlayerPackets(recipient)
			claimCriticalTestPlayers(t, w, owner)
			release, queue := holdCriticalCombatParticipant(t, w, recipient)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.tickSummonCombat(w.now())
				assertCriticalCombatUnchanged(t, owner, beforeOwner, fallback, time.Time{})
				assertCriticalCombatUnchanged(t, recipient, beforeRecipient, 0, time.Time{})
				if !reflect.DeepEqual(*mob, beforeMob) || !reflect.DeepEqual(*summon, beforeSummon) ||
					owner.CombatTargetID != primary || owner.LastAttackerID != fallback || store.saves != 0 {
					t.Fatalf("%s mutated/published damage before preflighting the reward recipient", phase)
				}
			}
			w.processCriticalCompletions(1)
			w.tickSummonCombat(w.now())
			// The lethal summon hit submits one detached two-account reward.
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if !mob.Dead || store.saves != 0 || len(queue.pending) != 0 ||
				owner.Char.Exp <= beforeOwner.Exp || recipient.Char.Exp <= beforeRecipient.Exp {
				t.Fatal("whole-party kill reward did not commit after acknowledgement")
			}
		})
	}
}

func TestCriticalSummonCreationRejectsReservedOwnerBeforeChanges(t *testing.T) {
	for _, scenario := range []string{"bm-cast", "contract-plan"} {
		t.Run(scenario, func(t *testing.T) {
			w, owner, target, _ := manaControlCombatFixture(t, false)
			beforeOwner := cloneCharacterState(owner.Char)
			mobCount, summonCount := len(w.mobs), len(w.summons)
			create := func() bool {
				if scenario == "bm-cast" {
					return w.castSummon(owner, model.SkillDef{InstanceValue: 1}, 30)
				}
				plan, ok := w.planContractSummon(owner, &model.VolatileSummon{Name: "Contract", Face: 206, HP: 100})
				return ok && plan != nil
			}
			release, queue := holdCriticalCombatParticipant(t, w, owner)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				if create() || len(w.mobs) != mobCount || len(w.summons) != summonCount {
					t.Fatalf("%s created a summon for a reserved owner", phase)
				}
				assertCriticalCombatUnchanged(t, owner, beforeOwner, 0, time.Time{})
				if target.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("rejected summon creation published to an observer")
				}
			}
			w.processCriticalCompletions(1)
			if !create() {
				t.Fatal("summon creation did not resume after acknowledgement")
			}
		})
	}
}

func TestCriticalSummonPreservesPrimaryPriorityOverReservedFallback(t *testing.T) {
	w, owner, target, mob := manaControlCombatFixture(t, false)
	summon := criticalSummonFixture(w, owner)
	owner.CombatTargetID, owner.LastAttackerID = mob.ID, target.ID
	beforeTarget, beforeHP := cloneCharacterState(target.Char), mob.HP
	release, queue := holdCriticalCombatParticipant(t, w, target)
	for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		if got := w.ownerSummonTarget(owner); got.paused || got.mob != mob {
			t.Fatalf("%s blocked an unrelated higher-priority command", phase)
		}
	}
	w.tickSummonCombat(w.now())
	if mob.HP >= beforeHP || summon.TargetID != mob.ID || owner.LastAttackerID != target.ID ||
		!reflect.DeepEqual(*target.Char, beforeTarget) {
		t.Fatal("primary command did not advance independently of the reserved fallback")
	}
}

func TestCriticalSummonIgnoresReservedIneligibleRewardMembers(t *testing.T) {
	for _, scenario := range []string{"dead", "other-sector", "experience-cap"} {
		t.Run(scenario, func(t *testing.T) {
			w, owner, target, mob := manaControlCombatFixture(t, false)
			party := &Party{Members: []*Player{owner, target}}
			owner.Party, target.Party = party, party
			switch scenario {
			case "dead":
				setPlayerCurHP(target.Char, 0)
			case "other-sector":
				target.X, target.Char.X = 2300, 2300
				w.updatePlayerSpatial(target)
			case "experience-cap":
				table := progressionTable(target.Char)
				target.Char.Exp = table[len(table)-1]
			}
			criticalSummonFixture(w, owner)
			owner.CombatTargetID = mob.ID
			beforeHP, beforeTarget := mob.HP, cloneCharacterState(target.Char)
			release, queue := holdCriticalCombatParticipant(t, w, target)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				if w.criticalKillParticipantsReserved(owner) {
					t.Fatalf("%s treated an ineligible member as a reward participant", phase)
				}
			}
			w.tickSummonCombat(w.now())
			if mob.HP >= beforeHP || !reflect.DeepEqual(*target.Char, beforeTarget) {
				t.Fatal("unrelated summon attack did not advance without changing the reserved member")
			}
		})
	}
}

func TestSummonTemplatesSpawnEnglishNamesWithoutChangingFaces(t *testing.T) {
	names := []string{"Condor", "Boar", "Wolf", "Bear", "Tiger", "Gorilla", "Black Dragon", "Succubus"}
	faces := []uint16{206, 225, 226, 227, 244, 245, 307, 396}
	for i, name := range names {
		t.Run(name, func(t *testing.T) {
			w, owner, _, _ := manaControlCombatFixture(t, false)
			if !w.castSummon(owner, model.SkillDef{InstanceValue: i + 1}, 80) || len(w.summons) == 0 {
				t.Fatal("fixture did not materialize the summon template")
			}
			for _, summon := range w.summons {
				if summon.Def.Name != name+"^" || summon.Def.Equip.Rosto.Index != faces[i] {
					t.Fatal("summon display name or established face ID changed unexpectedly")
				}
			}
		})
	}
}

func TestSummonDeathNotificationUsesEnglishKillerKind(t *testing.T) {
	w, owner, target, _ := manaControlCombatFixture(t, false)
	criticalSummonFixture(w, owner)
	owner.CombatTargetID = target.ID
	setPlayerCurHP(target.Char, 1)
	w.tickSummonCombat(w.now())
	if playerCurHP(target.Char) != 0 {
		t.Fatal("fixture did not exercise lethal summon PvP")
	}
	found := false
	for {
		packet, ok := target.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid encrypted summon combat packet")
		}
		found = found || bytes.Contains(packet, []byte("!You were killed by player \"Attacker\"."))
	}
	if !found {
		t.Fatal("lethal summon PvP did not publish the English death notification")
	}
}
