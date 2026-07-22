package game

import "wydgo/internal/model"

const statusPointsPerLevel = 5
const masteryPointsPerLevel = 2
const skillPointsPerLevel = 3

// Atributos naturais das quatro classes (BaseSIDCHM do Basedef.cpp).
var baseClassStats = [4][4]uint16{
	{8, 4, 7, 6},  // TK
	{5, 8, 5, 5},  // FM
	{6, 6, 9, 5},  // BM
	{8, 9, 13, 6}, // HT
}

var baseClassHPMP = [4][2]int{
	{80, 45},
	{60, 65},
	{70, 55},
	{75, 60},
}

var mortalHPPerLevel = [4]int{3, 1, 1, 2}
var mortalMPPerLevel = [4]int{1, 3, 2, 1}

func mortalStatusPointBudget(level int) int {
	total := level * statusPointsPerLevel
	if level >= 254 {
		total += (level - 254) * 5
	}
	if level >= 299 {
		total += (level - 299) * 10
	}
	if level >= 354 {
		total -= (level - 354) * 8
	}
	return maxInt(0, total)
}

func mortalSkillPointBudget(level int) int {
	total := level * skillPointsPerLevel
	if level > 199 {
		total += level - 199
	}
	return maxInt(0, total)
}

func masteryPointLimit(ch *model.Char, detail int) int {
	limit := 200
	if detail >= 1 && detail <= 3 && ch.LearnedSkill&(1<<uint(detail*8-1)) != 0 {
		limit = 255
	}
	if ch.SecondaryLearnedSkill&(1<<0|1<<4|1<<8) != 0 {
		limit = 320
	}
	levelLimit := 3 * (int(playerLevel(ch)) + 1) / 2
	return minInt(limit, levelLimit)
}

func syncSkillPoints(ch *model.Char) {
	if ch == nil {
		return
	}
	ensureExtendedScore(ch)
	ch.Extended.SkillPts = uint32(mortalSkillPointBudget(int(ch.Extended.Level)))
}

func syncMasteryPoints(ch *model.Char) {
	if ch == nil {
		return
	}
	ensureExtendedScore(ch)
	var spent uint32
	for _, value := range ch.Extended.Mastery {
		spent += value
	}
	total := ch.Extended.Level * masteryPointsPerLevel
	if spent >= total {
		ch.Extended.MasterPts = 0
		return
	}
	ch.Extended.MasterPts = total - spent
}

// O saldo e sempre recalculado pelo nivel e atributos gastos. Um JSON
// adulterado nao pode criar pontos e nenhum atributo pode cair abaixo da base.
func syncStatusPoints(ch *model.Char) {
	if ch == nil {
		return
	}
	ensureExtendedScore(ch)
	class := int(ch.Class)
	if class < 0 || class >= len(baseClassStats) {
		ch.Extended.StatusPts = 0
		return
	}
	natural := baseClassStats[class]
	stats := [4]*uint32{
		&ch.Extended.Str, &ch.Extended.Int,
		&ch.Extended.Dex, &ch.Extended.Con,
	}
	var spent uint32
	for i, value := range stats {
		if *value > uint32(natural[i]) {
			spent += *value - uint32(natural[i])
		}
	}
	total := uint32(mortalStatusPointBudget(int(ch.Extended.Level)))
	if spent > total {
		overflow := spent - total
		for i, value := range stats {
			base := uint32(natural[i])
			if *value <= base {
				continue
			}
			remove := *value - base
			if remove > overflow {
				remove = overflow
			}
			*value -= remove
			overflow -= remove
			if overflow == 0 {
				break
			}
		}
		spent = total
	}
	ch.Extended.StatusPts = total - spent
}

func applyBonus(ch *model.Char, bonusType, detail int) bool {
	if ch == nil || detail < 0 || detail > 3 {
		return false
	}
	ensureExtendedScore(ch)
	e := ch.Extended
	switch bonusType {
	case 0:
		if e.StatusPts == 0 {
			return false
		}
		points := uint32(1)
		if e.StatusPts >= 300 {
			points = 100
		}
		if points > e.StatusPts {
			points = e.StatusPts
		}
		target := []*uint32{&e.Str, &e.Int, &e.Dex, &e.Con}[detail]
		*target = clampExtended(*target + points)
		e.StatusPts -= points
	case 1:
		if e.MasterPts == 0 || int(e.Mastery[detail]) >= masteryPointLimit(ch, detail) {
			return false
		}
		e.Mastery[detail]++
		e.MasterPts--
	default:
		return false
	}
	ch.ExtendedRuntime = nil
	return true
}
