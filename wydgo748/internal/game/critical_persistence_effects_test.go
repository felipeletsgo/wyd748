package game

import (
	"reflect"
	"strconv"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCriticalSupportPreflightsEveryParticipant(t *testing.T) {
	for _, index := range []int{27, 31, 42, 25, 11} {
		for _, role := range []string{"caster", "first-target", "last-target"} {
			t.Run(strconv.Itoa(index)+"/"+role, func(t *testing.T) {
				w, caster, target, _ := manaControlCombatFixture(t, false)
				first, _ := networkedTestPlayer(3, "SupportFirst", 2101, 2100)
				w.players[first.Session], w.playersByID[first.ID] = first, first
				w.indexPlayerCharacter(first)
				w.updatePlayerSpatial(first)
				if index == 42 {
					target.X, target.Y = 2110, 2100
					target.Char.X, target.Char.Y = target.X, target.Y
					w.updatePlayerSpatial(target)
				}
				participants := []*Player{caster, first, target}
				for _, p := range participants {
					setPlayerCurHP(p.Char, 100)
					if index == 31 && p != caster {
						setPlayerCurHP(p.Char, 0)
						p.DeadAt = w.now()
					}
					if index == 25 {
						p.Char.Affects[0] = model.Affect{Type: 5, ExpiresAt: w.now().Add(time.Minute)}
					}
					drainPlayerPackets(p)
				}
				reserved := map[string]*Player{"caster": caster, "first-target": first, "last-target": target}[role]
				snapshots := make([]model.Char, len(participants))
				deadlines := make([]time.Time, len(participants))
				positions := make([][2]uint16, len(participants))
				for i, p := range participants {
					snapshots[i], deadlines[i] = cloneCharacterState(p.Char), p.DeadAt
					positions[i] = [2]uint16{p.X, p.Y}
				}
				skill := model.SkillDef{Index: index, AffectType: 11, AffectValue: 10, AffectTime: 10}
				release, queue := holdCriticalCombatParticipant(t, w, reserved)
				for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, queue, 1)
					}
					if got := w.applySupportSkillToTargets(caster, []*Player{first, target}, skill, 50); len(got) != 0 {
						t.Fatalf("%s partially applied a reserved support cast", phase)
					}
					for i, p := range participants {
						assertCriticalCombatUnchanged(t, p, snapshots[i], 0, deadlines[i])
						if positions[i] != [2]uint16{p.X, p.Y} {
							t.Fatal("reserved support participant moved")
						}
					}
				}
				w.processCriticalCompletions(1)
				if got := w.applySupportSkillToTargets(caster, []*Player{first, target}, skill, 50); len(got) != 2 {
					t.Fatal("whole support cast did not resume after acknowledgement")
				}
				switch index {
				case 27:
					if playerCurHP(target.Char) <= 100 {
						t.Fatal("healing did not resume")
					}
				case 31:
					if playerCurHP(target.Char) == 0 || !target.DeadAt.IsZero() || playerCurMP(caster.Char) != 0 {
						t.Fatal("revival or caster cost did not resume")
					}
				case 42:
					if positions[2] == [2]uint16{target.X, target.Y} || target.Char.X != target.X || target.Char.Y != target.Y || target.Session.QueuedPacketsForTest() == 0 {
						t.Fatal("teleport position and publication did not resume")
					}
				case 25:
					if target.Char.Affects[0].Type != 0 {
						t.Fatal("cleansing did not resume")
					}
				case 11:
					if activePlayerAffectAt(target.Char, 11, w.now()) == nil {
						t.Fatal("buff did not resume")
					}
				}
			})
		}
	}
}

func TestCriticalSupportRejectsInvalidParticipantsBeforeMutation(t *testing.T) {
	for _, scenario := range []string{"nil-caster", "caster-without-character", "nil-first-target", "nil-last-target", "first-without-character", "last-without-character"} {
		t.Run(scenario, func(t *testing.T) {
			w, caster, target, _ := manaControlCombatFixture(t, false)
			first, _ := networkedTestPlayer(3, "InvalidSupportFirst", 2101, 2100)
			participants := []*Player{caster, first, target}
			original := make([]*model.Char, len(participants))
			snapshots := make([]model.Char, len(participants))
			for i, p := range participants {
				setPlayerCurHP(p.Char, 100)
				original[i], snapshots[i] = p.Char, cloneCharacterState(p.Char)
				drainPlayerPackets(p)
			}
			caller, targets := caster, []*Player{first, target}
			switch scenario {
			case "nil-caster":
				caller = nil
			case "caster-without-character":
				caster.Char = nil
			case "nil-first-target":
				targets[0] = nil
			case "nil-last-target":
				targets[1] = nil
			case "first-without-character":
				first.Char = nil
			case "last-without-character":
				target.Char = nil
			}
			if got := w.applySupportSkillToTargets(caller, targets, model.SkillDef{Index: 27, InstanceValue: 100}, 50); len(got) != 0 {
				t.Fatal("invalid participant set partially applied support")
			}
			for i, p := range participants {
				if !reflect.DeepEqual(*original[i], snapshots[i]) || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("invalid participant set mutated or published character state")
				}
			}
		})
	}
}

func TestCriticalPartySupportRejectsBeforeManaAndCooldown(t *testing.T) {
	for _, role := range []string{"caster", "first-target", "last-target", "explicit-target"} {
		t.Run(role, func(t *testing.T) {
			w, caster, target, _ := manaControlCombatFixture(t, false)
			first, _ := networkedTestPlayer(3, "PartySupportFirst", 2101, 2100)
			w.players[first.Session], w.playersByID[first.ID] = first, first
			w.indexPlayerCharacter(first)
			w.updatePlayerSpatial(first)
			party := &Party{Members: []*Player{caster, first, target}}
			participants := party.Members
			for _, p := range participants {
				p.Party = party
				setPlayerCurHP(p.Char, 100)
				drainPlayerPackets(p)
			}
			caster.Char.Class, caster.Char.LearnedSkill = 1, 1<<3
			caster.SkillReady = make(map[int]time.Time)
			w.skills = map[int]model.SkillDef{27: {Index: 27, Party: 1, MaxTarget: 3, Range: 6, ManaSpent: 10, Delay: 5, InstanceValue: 100}}
			req := skillCastRequest{Skill: 27}
			reserved := map[string]*Player{"caster": caster, "first-target": first, "last-target": target, "explicit-target": target}[role]
			if role == "explicit-target" {
				req.TargetID = target.ID
				skill := w.skills[27]
				skill.Party = 0
				w.skills[27] = skill
			}
			snapshots := make([]model.Char, len(participants))
			for i, p := range participants {
				snapshots[i] = cloneCharacterState(p.Char)
			}
			release, queue := holdCriticalCombatParticipant(t, w, reserved)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.onSkillAttack(caster, req)
				for i, p := range participants {
					assertCriticalCombatUnchanged(t, p, snapshots[i], 0, time.Time{})
				}
				if len(caster.SkillReady) != 0 {
					t.Fatal("rejected cast consumed its cooldown")
				}
			}
			w.processCriticalCompletions(1)
			w.onSkillAttack(caster, req)
			if playerCurMP(caster.Char) >= playerCurMP(&snapshots[0]) || playerCurHP(target.Char) <= 100 || !caster.SkillReady[27].After(w.now()) {
				t.Fatal("real skill handler did not resume healing, cost and cooldown")
			}
		})
	}
}

func TestCriticalPlayerEffectTicksFreezeUntilAcknowledgement(t *testing.T) {
	for _, kind := range []byte{17, 20, 22, 23, 4} {
		t.Run(strconv.Itoa(int(kind)), func(t *testing.T) {
			w, owner, p, mob := manaControlCombatFixture(t, true)
			setPlayerCurHP(p.Char, 500)
			p.Char.Affects[1] = model.Affect{Type: kind, Level: 20, Value: 10, ExpiresAt: w.now().Add(time.Minute), NextTick: w.now().Add(-time.Second)}
			if kind == 20 {
				p.Char.Affects[1].OwnerID, p.Char.Affects[1].OwnerCharacterUID = owner.ID, owner.Char.UID
			}
			if kind == 4 {
				p.Char.Affects[1].ExpiresAt = w.now().Add(-time.Second)
			}
			snapshot := cloneCharacterState(p.Char)
			mobHP := mob.HP
			release, queue := holdCriticalCombatParticipant(t, w, p)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.tickPlayerAffects(w.now())
				assertCriticalCombatUnchanged(t, p, snapshot, 0, time.Time{})
				if mob.HP != mobHP || owner.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("reserved player effect damaged a mob or published to its owner")
				}
			}
			w.processCriticalCompletions(1)
			w.tickPlayerAffects(w.now())
			if kind == 4 {
				if p.Char.Affects[1].Type != 0 {
					t.Fatal("expired affect was not removed after acknowledgement")
				}
			} else if !p.Char.Affects[1].NextTick.After(w.now()) {
				t.Fatal("affect deadline did not resume")
			}
			if (kind == 22 || kind == 23) && mob.HP >= mobHP || kind == 17 && playerCurHP(p.Char) <= 500 || kind == 20 && playerCurMP(p.Char) >= playerCurMP(&snapshot) {
				t.Fatal("real periodic effect did not resume its mutation")
			}
		})
	}
}

func TestCriticalEffectOwnerPausesBeforeSourceValidation(t *testing.T) {
	for _, scenario := range []string{"player-dot", "player-expired", "mob-dot", "mob-expired", "disconnected-mob-owner"} {
		t.Run(scenario, func(t *testing.T) {
			w, owner, target, mob := manaControlCombatFixture(t, true)
			affect := model.Affect{Type: 20, Level: 20, Value: 10, OwnerID: owner.ID, OwnerCharacterUID: owner.Char.UID, ExpiresAt: w.now().Add(time.Minute), NextTick: w.now().Add(-time.Second)}
			if scenario == "player-expired" || scenario == "mob-expired" {
				affect.ExpiresAt = w.now().Add(-time.Second)
			}
			playerEffect := scenario == "player-dot" || scenario == "player-expired"
			if playerEffect {
				target.Char.Affects[1] = affect
			} else {
				mob.Affects[0] = affect
				w.activeMobs[mob.ID] = mob
			}
			ownerSnapshot, targetSnapshot := cloneCharacterState(owner.Char), cloneCharacterState(target.Char)
			mobHP, mobAffects := mob.HP, mob.Affects
			release, queue := holdCriticalCombatParticipant(t, w, owner)
			if scenario == "disconnected-mob-owner" {
				owner.Session.Close()
			}
			tick := func() {
				if playerEffect {
					w.tickPlayerAffects(w.now())
				} else {
					w.tickMobAffects(w.now(), 0, 1)
				}
			}
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				tick()
				assertCriticalCombatUnchanged(t, owner, ownerSnapshot, 0, time.Time{})
				assertCriticalCombatUnchanged(t, target, targetSnapshot, 0, time.Time{})
				if mob.HP != mobHP || !reflect.DeepEqual(mob.Affects, mobAffects) {
					t.Fatalf("%s changed the reserved owner's mob effect", phase)
				}
			}
			w.processCriticalCompletions(1)
			tick()
			if playerEffect {
				if reflect.DeepEqual(*target.Char, targetSnapshot) {
					t.Fatal("player effect did not resume after owner acknowledgement")
				}
			} else if mob.HP == mobHP && reflect.DeepEqual(mob.Affects, mobAffects) {
				t.Fatal("mob effect did not resume damage/expiry/source cleanup")
			}
		})
	}
}

func TestCriticalPlayerRegenPreservesDeadline(t *testing.T) {
	for _, scenario := range []string{"due", "full-vitals", "dead"} {
		t.Run(scenario, func(t *testing.T) {
			w, _, p, _ := manaControlCombatFixture(t, false)
			if scenario == "due" {
				setPlayerCurHP(p.Char, 500)
				setPlayerCurMP(p.Char, 500)
			} else if scenario == "dead" {
				setPlayerCurHP(p.Char, 0)
			}
			p.NextRegen = w.now().Add(-time.Second)
			beforeDeadline, snapshot := p.NextRegen, cloneCharacterState(p.Char)
			release, queue := holdCriticalCombatParticipant(t, w, p)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				w.tickPlayerRegen(w.now())
				assertCriticalCombatUnchanged(t, p, snapshot, 0, time.Time{})
				if !p.NextRegen.Equal(beforeDeadline) {
					t.Fatal("reserved regeneration consumed its deadline")
				}
			}
			w.processCriticalCompletions(1)
			w.tickPlayerRegen(w.now())
			if !p.NextRegen.Equal(w.now().Add(playerRegenInterval)) || scenario == "due" && (playerCurHP(p.Char) <= 500 || playerCurMP(p.Char) <= 500) || scenario == "dead" && playerCurHP(p.Char) != 0 {
				t.Fatal("regeneration did not resume with its original lifecycle semantics")
			}
		})
	}
}

func TestCriticalAreaEffectDirectEntryRejectsReservedCaster(t *testing.T) {
	w, caster, target, mob := manaControlCombatFixture(t, false)
	snapshot, mobHP := cloneCharacterState(caster.Char), mob.HP
	release, queue := holdCriticalCombatParticipant(t, w, caster)
	affect := &model.Affect{Level: 20, Value: 10}
	for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
		if phase == "completed-unacknowledged" {
			release()
			awaitReadyCriticalResults(t, queue, 1)
		}
		w.tickAreaDamageAffect(caster, affect, 37)
		assertCriticalCombatUnchanged(t, caster, snapshot, 0, time.Time{})
		if mob.HP != mobHP || target.Session.QueuedPacketsForTest() != 0 {
			t.Fatal("direct area effect mutated or published while reserved")
		}
	}
	w.processCriticalCompletions(1)
	w.tickAreaDamageAffect(caster, affect, 37)
	if mob.HP >= mobHP {
		t.Fatal("direct area effect did not resume")
	}
}
