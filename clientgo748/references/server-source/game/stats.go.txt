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
var archHPPerLevel = [4]int{4, 1, 1, 2}
var archMPPerLevel = [4]int{1, 3, 4, 2}

func celestialBaseDefense(crystals byte) uint32 {
	switch crystals {
	case 2, 3:
		return 984
	case 4:
		return 1004
	default:
		return 954
	}
}

// celestialHPMPBonus porta o bloco ClassMaster >= CELESTIAL do W2PP normal
// (isHardCore=0). O bonus e inicial/flat; crescimento por nivel usa a mesma
// coluna nao-Mortal do Arch, sem fabricar 399 niveis de HP/MP.
func celestialHPMPBonus(class int, crystals byte) (int64, int64) {
	if class < 0 || class >= len(baseClassStats) {
		return 0, 0
	}
	hp := [4]int64{1600, 800, 800, 800}[class]
	mp := [4]int64{300, 1700, 1300, 700}[class]
	switch crystals {
	case 1, 2:
		mp += 80
	case 3:
		hp += 80
		mp += 80
	case 4:
		hp += 140
		mp += 140
	}
	return hp, mp
}

// naturalStats devolve os quatro atributos que nao foram distribuídos pelo
// jogador. O reset do Skill Master e o orçamento de pontos precisam usar a
// mesma fonte; caso contrário, um Celestial poderia receber ou perder pontos
// ao alternar entre os dois fluxos.
func naturalStats(ch *model.Char) ([4]uint16, bool) {
	if ch == nil {
		return [4]uint16{}, false
	}
	class := int(ch.Class)
	if class < 0 || class >= len(baseClassStats) {
		return [4]uint16{}, false
	}
	return baseClassStats[class], true
}

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

// archPointsPerMortalLevel e quanto cada nivel do Mortal ACIMA de 299 rende ao
// Arch. E o `MortalLevel * 5` do nativo, onde MortalLevel ja e (nivel - 299).
const (
	archPointsPerLevel       = 6
	archPointsPerMortalLevel = 5
	archMortalLevelOffset    = 299
	archBaseBonus            = 28
)

// archStatusPointBudget porta o ramo ARCH de BASE_GetCurrentScore
// (Basedef.cpp:1043-1056):
//
//	leveluse  = lvl * 6
//	leveluse += MortalLevel * 5      // MortalLevel = nivelDoMortal - 299
//	leveluse += 28
//	if lvl >= 354: leveluse += (lvl - 354) * 6
//
// O termo do Mortal e o que faz um Arch nascido de um Mortal 400 ser mais forte
// que um de 370 -- e, como o nativo recalcula isso a cada login, continuar
// subindo o Mortal segue fortalecendo o Arch.
//
// Mortal abaixo de 299 renderia bonus negativo; o piso em zero evita que um
// dado torto tire pontos ja distribuidos.
func archStatusPointBudget(level, mortalLevel int) int {
	total := level*archPointsPerLevel + archBaseBonus
	if bonus := mortalLevel - archMortalLevelOffset; bonus > 0 {
		total += bonus * archPointsPerMortalLevel
	}
	if level >= 354 {
		total += (level - 354) * archPointsPerLevel
	}
	return maxInt(0, total)
}

// statusPointBudget escolhe a formula pela evolucao do personagem.
func statusPointBudget(ch *model.Char) int {
	if isCelestialEvolution(ch) {
		return celestialStatusPointBudget(ch)
	}
	if isArch(ch) {
		return archStatusPointBudget(int(ch.Score.Level), int(ch.ArchMortalLevel))
	}
	return mortalStatusPointBudget(int(ch.Score.Level))
}

func isCelestialEvolution(ch *model.Char) bool {
	if ch == nil {
		return false
	}
	return advancedEvolution(ch, "celestial", "subcelestial")
}

func celestialArchTierBonus(tier byte) int {
	switch tier {
	case 1:
		return 100
	case 2:
		return 300
	case 3:
		return 600
	case 4:
		return 900
	case 5:
		return 1200
	default:
		return 0
	}
}

// celestialStatusPointBudget porta BASE_GetBonusScorePoint da 7.54:
//
//	1001 + cristais*100 + faixaArch + nivelAtivo*10
//	     + nivelDaOutraForma*6 + 290 a partir do nivel interno 190.
//
// O nivel da outra forma contribui para o ORCAMENTO da ativa. Nao existe uma
// carteira compartilhada: EXP e atributos ja distribuidos continuam separados.
func celestialStatusPointBudget(ch *model.Char) int {
	if ch == nil || ch.Score == nil {
		return 0
	}
	total := 1001 + int(ch.ArchCrystals)*100 +
		celestialArchTierBonus(ch.CelestialArchTier) +
		int(ch.Score.Level)*10
	if ch.AlternateCelestial != nil && ch.AlternateCelestial.Score != nil {
		total += int(ch.AlternateCelestial.Score.Level) * 6
	}
	if ch.Score.Level > 189 {
		total += 290
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

func skillPointBudget(ch *model.Char) int {
	if ch == nil || ch.Score == nil {
		return 0
	}
	if isCelestialEvolution(ch) {
		// BASE_GetBonusSkillPoint do W2PP: Celestial/Sub recebe uma base de
		// 1600 e preserva a curva 3/4 pontos por nivel.
		return 1600 + mortalSkillPointBudget(int(ch.Score.Level)) + int(ch.SkillPointBonus)
	}
	if isArch(ch) {
		// BASE_GetBonusSkillPoint: todo Arch nasce com 168 pontos de
		// habilidade alem do mesmo ganho por nivel do Mortal.
		return mortalSkillPointBudget(int(ch.Score.Level)) + 168 + int(ch.SkillPointBonus)
	}
	return mortalSkillPointBudget(int(ch.Score.Level)) + int(ch.SkillPointBonus)
}

func masteryPointLimit(ch *model.Char, detail int) int {
	limit := 200
	if detail >= 1 && detail <= 3 && ch.LearnedSkill&(1<<uint(detail*8-1)) != 0 {
		limit = 255
	}
	if ch.SecondaryLearnedSkill&(1<<0|1<<4|1<<8) != 0 {
		limit = 320
	}
	if isCelestialEvolution(ch) {
		return limit
	}
	levelLimit := 3 * (int(playerLevel(ch)) + 1) / 2
	return minInt(limit, levelLimit)
}

func syncSkillPoints(ch *model.Char) {
	if ch == nil {
		return
	}
	ensureScore(ch)
	ch.Score.SkillPts = uint32(skillPointBudget(ch))
}

func syncMasteryPoints(ch *model.Char) {
	if ch == nil {
		return
	}
	ensureScore(ch)
	var spent uint32
	for _, value := range ch.Score.Mastery {
		spent += value
	}
	total := ch.Score.Level * masteryPointsPerLevel
	if isCelestialEvolution(ch) {
		total = 855
	} else if isArch(ch) {
		// CFileDB cria o Arch com SpecialBonus=112; cada nivel acrescenta 2.
		total += 112
	}
	if spent >= total {
		ch.Score.MasterPts = 0
		return
	}
	ch.Score.MasterPts = total - spent
}

// O saldo e sempre recalculado pelo nivel e atributos gastos. Um JSON
// adulterado nao pode criar pontos e nenhum atributo pode cair abaixo da base.
func syncStatusPoints(ch *model.Char) {
	if ch == nil {
		return
	}
	ensureScore(ch)
	natural, ok := naturalStats(ch)
	if !ok {
		ch.Score.StatusPts = 0
		return
	}
	stats := [4]*uint32{
		&ch.Score.Str, &ch.Score.Int,
		&ch.Score.Dex, &ch.Score.Con,
	}
	var spent uint32
	for i, value := range stats {
		if *value > uint32(natural[i]) {
			spent += *value - uint32(natural[i])
		}
	}
	total := uint32(statusPointBudget(ch))
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
	ch.Score.StatusPts = total - spent
}

func applyBonus(ch *model.Char, bonusType, detail int) bool {
	if ch == nil || detail < 0 || detail > 3 {
		return false
	}
	ensureScore(ch)
	e := ch.Score
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
		*target = clampScoreValue(*target + points)
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
	ch.RuntimeScore = nil
	return true
}
