package game

import (
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCriticalRewardRecipientsPauseEveryDamageProducer(t *testing.T) {
	for _, scenario := range []string{"physical", "iron-spear", "iron-spear-direct", "skill", "poison", "area-22", "area-23", "area-direct"} {
		t.Run(scenario, func(t *testing.T) {
			w, owner, mob, store := killRewardTestWorld(t, nil)
			recipient, _ := networkedTestPlayer(2, "Recipient", 2102, 2100)
			w.players[recipient.Session], w.playersByID[recipient.ID] = recipient, recipient
			w.indexPlayerCharacter(recipient)
			w.updatePlayerSpatial(recipient)
			party := &Party{Members: []*Player{owner, recipient}}
			owner.Party, recipient.Party = party, party
			mob.HP = 100
			secondary := &Mob{ID: 1401, X: 2102, Y: 2100, HP: 100,
				Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
			if scenario == "iron-spear" || scenario == "iron-spear-direct" {
				owner.Char.Class, owner.Char.LearnedSkill = 3, 0x40
				w.appendMobInstance(secondary)
				w.registerMobSpatial(secondary)
			} else if scenario == "skill" {
				owner.Char.Class, owner.Char.LearnedSkill = 1, 1
				w.skills = map[int]model.SkillDef{24: {Index: 24, InstanceType: 2, InstanceValue: 100,
					Range: 6, MaxTarget: 1, Aggressive: 1}}
			}
			affect := model.Affect{Type: 20, Level: 100, Value: 100, OwnerID: owner.ID,
				OwnerCharacterUID: owner.Char.UID, ExpiresAt: w.now().Add(time.Minute), NextTick: w.now().Add(-time.Second)}
			if scenario == "poison" {
				mob.Affects[0] = affect
				w.activeMobs[mob.ID] = mob
			} else if scenario == "area-22" || scenario == "area-23" {
				affect.Type = 22
				if scenario == "area-23" {
					affect.Type = 23
				}
				owner.Char.Affects[1] = affect
			}
			beforeOwner, beforeRecipient := cloneCharacterState(owner.Char), cloneCharacterState(recipient.Char)
			beforeMob, beforeSecondary := *mob, *secondary
			drainPlayerPackets(owner)
			drainPlayerPackets(recipient)
			hit := func() {
				switch scenario {
				case "physical":
					w.onAttack(owner.Session, physicalAttackPacket(1000, mob.ID, mob.X, mob.Y))
				case "iron-spear":
					w.onAttack(owner.Session, physicalAttackTwoPacket(1000, mob.ID, mob.X, mob.Y, secondary.ID))
				case "iron-spear-direct":
					result := w.applyHuntressIronSpearSecondary(owner, skillCastRequest{SecondaryTargetID: secondary.ID},
						mob.ID, mob.X, mob.Y, false, false, w.now())
					w.finalizeIronSpearSecondary(owner, result)
				case "skill":
					w.onSkillAttack(owner, skillCastRequest{Skill: 24, TargetID: mob.ID})
				case "poison":
					w.tickMobAffects(w.now(), 0, 1)
				case "area-22", "area-23":
					w.tickPlayerAffects(w.now())
				case "area-direct":
					w.tickAreaDamageAffect(owner, &affect, 37)
				}
			}
			claimCriticalTestPlayers(t, w, owner)
			release, queue := holdCriticalCombatParticipant(t, w, recipient)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				hit()
				assertCriticalCombatUnchanged(t, owner, beforeOwner, 0, time.Time{})
				assertCriticalCombatUnchanged(t, recipient, beforeRecipient, 0, time.Time{})
				if !reflect.DeepEqual(*mob, beforeMob) || !reflect.DeepEqual(*secondary, beforeSecondary) ||
					!owner.LastAttackAt.IsZero() || owner.LastAttackTick != 0 || owner.SkillReady != nil || store.saves != 0 {
					t.Fatalf("%s consumed damage, cooldown, affect deadline or persistence before acknowledgement", phase)
				}
			}
			w.processCriticalCompletions(1)
			hit()
			if mob.HP == beforeMob.HP && secondary.HP == beforeSecondary.HP {
				t.Fatal("damage did not resume after recipient acknowledgement")
			}
			// The resumed lethal hit submits its own detached reward write.
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if store.saves != 0 || recipient.Char.Exp <= beforeRecipient.Exp || len(queue.pending) != 0 {
				t.Fatal("resumed lethal damage did not persist and award the party recipient")
			}
		})
	}
}
