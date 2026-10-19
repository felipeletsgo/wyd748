package game

import (
	"strings"
	"time"

	"wydgo/internal/model"
)

// This is a contract-preserving adaptation of the historical 7.59 monster
// skill bar, not a replacement for public-world or other instance combat.
func (w *World) runeTrackCombatActor(m *Mob, now time.Time) bool {
	inst := w.instanceForMob(m)
	if inst == nil || inst.RuneTrack == nil || inst.State != "combat" || inst.RuneTrack.Settled ||
		now.Before(inst.RuneTrack.Start.Add(31*time.Second)) || !now.Before(inst.RuneTrack.Start.Add(15*time.Minute)) ||
		w.criticalInstanceReserved() || m.Dead || m.HP == 0 || m.Def == nil || m.Def.Score == nil ||
		!m.Def.IsMonster() || !strings.HasPrefix(m.Def.Name, "RT_") || w.mobsByID[m.ID] != m {
		return false
	}
	_, owned := inst.MobIDs[m.ID]
	label, generated := inst.RuneTrack.Generators[m.ID]
	return owned && generated && runeGeneratorRoom(label) == inst.UxmalRoom
}

func (w *World) runeTrackCombatSkill(m *Mob, now time.Time) (model.SkillDef, bool) {
	if !w.runeTrackCombatActor(m, now) || m.Def.SkillBar == [4]byte{255, 255, 255, 255} {
		return model.SkillDef{}, false
	}
	index := hellGateAttackSkill(m.Def.SkillBar, w.intn(100))
	skill, exists := w.skills[index]
	return skill, exists
}

func (w *World) runeTrackCastAtPlayer(m *Mob, target *Player, now time.Time) bool {
	if !w.mobCanTargetPlayer(m, target) {
		return false
	}
	skill, ok := w.runeTrackCombatSkill(m, now)
	if !ok {
		return false
	}
	w.castHistoricalSkillAtPlayer(m, target, skill, now)
	return true
}

func (w *World) runeTrackCastAtTower(m, target *Mob, now time.Time) bool {
	inst := w.instanceForMob(m)
	if inst == nil || inst.RuneTrack == nil || inst.UxmalRoom != 1 || target == nil || target.Dead || target.HP == 0 ||
		target.Def == nil || target.Def.Score == nil || w.instanceForMob(target) != inst || w.mobsByID[target.ID] != target {
		return false
	}
	label := inst.RuneTrack.Generators[target.ID]
	if _, owned := inst.NPCIDs[target.ID]; !owned || label < 5715 || label > 5717 {
		return false
	}
	skill, ok := w.runeTrackCombatSkill(m, now)
	if !ok {
		return false
	}
	w.castHistoricalSkillAtProtector(m, target, skill, now)
	return true
}
