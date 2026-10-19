package game

import (
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Historical skill-bar selection uses one percentile: slots 0/1/2 have
// 50/35/15 percent windows. An empty slot means a physical attack, not a reroll.
// Slot 3 is reserved for healing in the reference; these imported infantry
// templates have no healing skill. This does not alter ordinary monster AI.
func hellGateAttackSkill(bar [4]byte, roll int) int {
	slot := 0
	if roll >= 85 {
		slot = 2
	} else if roll >= 50 {
		slot = 1
	}
	if bar[slot] == 255 {
		return -1
	}
	return int(bar[slot])
}

func (w *World) hellGateCombatSkill(m *Mob) (model.SkillDef, bool) {
	inst := w.instanceForMob(m)
	if hellGatePopulation(inst) == nil || m == nil || m.Def == nil || m.Def.Name != "HG_SkelMage" {
		return model.SkillDef{}, false
	}
	if _, owned := inst.MobIDs[m.ID]; !owned {
		return model.SkillDef{}, false
	}
	index := hellGateAttackSkill(m.Def.SkillBar, w.intn(100))
	skill, exists := w.skills[index]
	return skill, exists
}

// The reference's monster spell path uses current physical attack versus
// defense, then compressed elemental resistance, without player spell scaling.
func hellGateSkillDamage(m *Mob, defense int, resist model.ElementalResists, skill model.SkillDef, intn func(int) int, now time.Time) uint32 {
	damage := hitDamageWithRNG(effectiveMobAttackAt(m, now), defense, 0, intn)
	if skill.InstanceType >= 2 && skill.InstanceType <= 5 {
		value := resistanceForInstance(skill.InstanceType, resist)
		switch {
		case value > 300:
			value = (value-300)/20 + 60
		case value > 200:
			value = (value-200)/10 + 50
		default:
			value /= 4
		}
		damage = damage * max(0, 100-value) / 100
	}
	return uint32(clampInt(damage, 0, int(maxScoreValue)))
}

// Melee reach, hit accuracy and attack deadlines retain the existing server
// contract. The client receives the supported skill-wide packet, not a new opcode.
func (w *World) hellGateCastAtPlayer(m *Mob, target *Player, now time.Time) bool {
	skill, ok := w.hellGateCombatSkill(m)
	if !ok {
		return false
	}
	w.castHistoricalSkillAtPlayer(m, target, skill, now)
	return true
}

// Both imported events use the existing authoritative damage and visibility
// paths. Selection and event ownership remain separate from packet construction.
func (w *World) castHistoricalSkillAtPlayer(m *Mob, target *Player, skill model.SkillDef, now time.Time) {
	if !combatRollHits(mobVersusPlayerAccuracy(m.Def, target.Char), w.intn) {
		w.sendToMobView(m, func() []byte {
			return wire.SkillHits(m.ID, m.X, m.Y, target.X, target.Y, 0, m.Def.Score.MaxMP,
				int16(skill.Index), 0, 0, 1, []wire.SkillTarget{{ID: target.ID, Miss: true, MaxHP: playerMaxHP(target.Char)}})
		})
		return
	}
	damage := hellGateSkillDamage(m, playerDefense(target.Char), playerElementalResists(target.Char), skill, w.intn, now)
	w.applyMobDamageToPlayer(m, target, damage, now, func(applied uint32) []byte {
		return wire.SkillHitExtended(m.ID, target.ID, m.X, m.Y, target.X, target.Y,
			applied, playerMaxHP(target.Char), 0, m.Def.Score.MaxMP, int16(skill.Index), 0, 0)
	})
}

func (w *World) hellGateCastAtProtector(m, target *Mob, now time.Time) bool {
	skill, ok := w.hellGateCombatSkill(m)
	if !ok {
		return false
	}
	w.castHistoricalSkillAtProtector(m, target, skill, now)
	return true
}

func (w *World) castHistoricalSkillAtProtector(m, target *Mob, skill model.SkillDef, now time.Time) {
	score := m.Def.Score
	damage := hellGateSkillDamage(m, int(target.Def.Score.Defense), model.ElementalResists{}, skill, w.intn, now)
	damage = min(w.levelingInstanceDamage(m, damage), target.HP)
	target.HP -= damage
	if inst := w.instanceForMob(target); hellGateVersion2(inst) {
		inst.HellGateDionysHP = target.HP
		w.markInstanceStateDirty()
	}
	w.sendToMobView(m, func() []byte {
		return wire.SkillHitExtended(m.ID, target.ID, m.X, m.Y, target.X, target.Y,
			damage, target.Def.Score.MaxHP, 0, score.MaxMP, int16(skill.Index), 0, 0)
	})
	if target.HP == 0 {
		m.TargetID = 0
		w.killMobWithoutPlayer(target)
	}
}
