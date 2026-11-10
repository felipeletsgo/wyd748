package game

import (
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type summonAttackVisual struct {
	skill          int16
	motion, effect byte
}

type bmSummonAttackProfile struct {
	special  [4]uint16
	skillBar [4]byte
}

var noBMSummonSpells = [4]byte{255, 255, 255, 255}

// Native NPC templates supply near skill, near motion, far skill, and far motion.
var bmSummonAttackProfiles = map[uint16]bmSummonAttackProfile{
	206: {[4]uint16{255, 7, 255, 7}, noBMSummonSpells},           // Condor
	225: {[4]uint16{255, 7, 255, 7}, noBMSummonSpells},           // Boar
	226: {[4]uint16{255, 7, 255, 7}, noBMSummonSpells},           // Wolf
	227: {[4]uint16{255, 7, 255, 7}, noBMSummonSpells},           // Bear
	244: {[4]uint16{255, 3, 106, 4}, [4]byte{255, 255, 21, 255}}, // Tiger
	245: {[4]uint16{255, 1, 122, 2}, [4]byte{255, 255, 34, 255}}, // Gorilla
	307: {[4]uint16{255, 7, 108, 2}, [4]byte{255, 255, 0, 255}},  // Black Dragon
	396: {[4]uint16{255, 7, 108, 2}, [4]byte{39, 34, 255, 255}},  // Succubus
}

var bmSummonDistanceTable = [7][7]byte{
	{0, 1, 2, 3, 4, 5, 6},
	{1, 1, 2, 3, 4, 5, 6},
	{2, 2, 3, 4, 4, 5, 6},
	{3, 3, 4, 4, 5, 5, 6},
	{4, 4, 4, 5, 5, 5, 6},
	{5, 5, 5, 5, 5, 6, 6},
	{6, 6, 6, 6, 6, 6, 6},
}

// TMSRVIN 004C2050 uses this table for both attack range and the near/far split.
// The largest configured range is six. Beyond the table it returns max+1.
func nativeSummonDistance(x, y, tx, ty uint16) int {
	dx, dy := absInt(int(x)-int(tx)), absInt(int(y)-int(ty))
	if dx > 6 || dy > 6 {
		return max(dx, dy) + 1
	}
	return int(bmSummonDistanceTable[dy][dx])
}

func (w *World) summonAttackVisual(summon *Mob, distance int) summonAttackVisual {
	if summon.SummonKind != summonKindBM {
		return summonAttackVisual{skill: -1}
	}
	profile, ok := bmSummonAttackProfiles[summon.Def.Equip.Rosto.Index]
	if !ok {
		return summonAttackVisual{skill: -1}
	}
	slot := 0
	if distance >= 3 {
		slot = 2
	}
	visual := summonAttackVisual{skill: int16(profile.special[slot]), motion: 4}
	switch visual.skill {
	case 255:
		visual.skill = -1
	case 122:
		visual.skill, visual.effect = 105, 1
	}
	switch profile.special[slot+1] {
	case 3:
		if w.intn(4) < 2 {
			visual.motion = 5
		}
	case 7:
		visual.motion = 6
		if w.intn(4) == 1 {
			visual.motion = 5
		}
	}
	if profile.skillBar != noBMSummonSpells {
		index := historicalMonsterAttackSkill(profile.skillBar, w.intn(100))
		if _, present := w.skills[index]; present {
			visual.skill = int16(index)
		}
	}
	return visual
}

func bmSummonSpellMastery(summon *Mob, spell model.SkillDef) int {
	profile := bmSummonAttackProfiles[summon.Def.Equip.Rosto.Index]
	return int(profile.special[(spell.Index%24)/8+1])
}

func (w *World) applyBMSummonPlayerSpell(owner *Player, summon *Mob, target *Player, spell model.SkillDef, now time.Time) {
	if hasActiveAffectAt(target.Char, 19, now) {
		return
	}
	mastery := bmSummonSpellMastery(summon, spell)
	changed := false
	if spell.AffectType > 0 {
		changed = setOwnedAffectForPlayerAt(target.Char, owner, byte(spell.AffectType), spell.AffectValue, mastery, spell.AffectTime, now)
	}
	if spell.TickType > 0 {
		changed = setOwnedAffectForPlayerAt(target.Char, owner, byte(spell.TickType), spell.TickValue, mastery, spell.AffectTime, now) || changed
	}
	if changed {
		w.recalcPlayer(target.Char)
		w.publishPlayerAffects(target)
	}
}

func (visual summonAttackVisual) packet(summon *Mob, target summonCombatTarget, damage, maxHP uint32, miss bool) []byte {
	if visual.motion == 0 && visual.skill < 0 {
		return wire.AttackHitWideResult(summon.ID, target.id, summon.X, summon.Y, target.x, target.y,
			damage, maxHP, 0, summon.Def.Score.MaxMP, 0, miss)
	}
	return wire.SkillHits(summon.ID, summon.X, summon.Y, target.x, target.y, 0, summon.Def.Score.MaxMP,
		visual.skill, visual.motion, visual.effect, 1, []wire.SkillTarget{
			{ID: target.id, Damage: damage, MaxHP: maxHP, Miss: miss},
		})
}
