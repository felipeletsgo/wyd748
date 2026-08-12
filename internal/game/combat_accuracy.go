package game

import "wydgo/internal/model"

const (
	combatBaseAccuracyPercent         = 100
	combatMinimumAccuracyPercent      = 20
	combatConcentrationMinimumPercent = 35
	combatMaximumEvasionPercent       = 80
	combatDexterityForMaximumEvasion  = 4000
	combatBonusPointsPerPercent       = 10
)

// combatAccuracyPercent is the single authoritative hit-chance rule used by
// physical attacks, offensive skills, mobs and summons. Every attacker starts
// at 100% accuracy. Only the defender's evasion lowers that chance: 4,000 DEX
// supplies the full 80% evasion cap. Item/mount/affect bonuses use ten points
// per percentage point; attacker accuracy bonuses counter that evasion.
func combatAccuracyPercent(defenderDex, accuracyBonusPoints, evasionBonusPoints int,
	concentration bool) int {
	chance := combatBaseAccuracyPercent -
		combatEvasionRatingPercent(defenderDex, evasionBonusPoints) +
		maxInt(0, accuracyBonusPoints)/combatBonusPointsPerPercent
	minimum := combatMinimumAccuracyPercent
	if concentration {
		chance += 15
		minimum = combatConcentrationMinimumPercent
	}
	return clampInt(chance, minimum, 100)
}

func combatEvasionRatingPercent(defenderDex, evasionBonusPoints int) int {
	dexEvasion := clampInt(defenderDex, 0, combatDexterityForMaximumEvasion) *
		combatMaximumEvasionPercent / combatDexterityForMaximumEvasion
	bonusEvasion := maxInt(0, evasionBonusPoints) / combatBonusPointsPerPercent
	return clampInt(dexEvasion+bonusEvasion, 0, combatMaximumEvasionPercent)
}

func combatEvasionPercent(defenderDex, defenderEvasionPoints,
	attackerAccuracyPoints int, attackerConcentration bool) int {
	return clampInt(100-combatAccuracyPercent(defenderDex,
		attackerAccuracyPoints, defenderEvasionPoints, attackerConcentration),
		0, combatMaximumEvasionPercent)
}

func combatRollHits(chance int, intn func(int) int) bool {
	if intn == nil {
		return false
	}
	return intn(100) < clampInt(chance, 0, 100)
}

func playerAccuracyBonusPoints(ch *model.Char) int {
	if ch == nil {
		return 0
	}
	return int(effectiveExtended(ch).Accuracy)
}

func playerEvasionBonusPoints(ch *model.Char) int {
	if ch == nil {
		return 0
	}
	return maxInt(0, int(effectiveExtended(ch).Evasion))
}

func playerHasConcentration(ch *model.Char) bool {
	return specialSkillLearned(ch, 100)
}

func playerVersusPlayerAccuracy(attacker, defender *model.Char) int {
	if attacker == nil || defender == nil {
		return 0
	}
	return combatAccuracyPercent(playerDex(defender), playerAccuracyBonusPoints(attacker),
		playerEvasionBonusPoints(defender),
		playerHasConcentration(attacker))
}

func playerVersusMobAccuracy(attacker *model.Char, defender *model.NPCDef) int {
	if attacker == nil || defender == nil {
		return 0
	}
	return combatAccuracyPercent(int(defender.Extended.Dex), playerAccuracyBonusPoints(attacker),
		int(defender.Extended.Evasion),
		playerHasConcentration(attacker))
}

func mobVersusPlayerAccuracy(attacker *model.NPCDef, defender *model.Char) int {
	if attacker == nil || defender == nil {
		return 0
	}
	return combatAccuracyPercent(playerDex(defender), int(attacker.Extended.Accuracy),
		playerEvasionBonusPoints(defender), false)
}
