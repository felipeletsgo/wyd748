package game

import "wydgo/internal/model"

// affectDoubleExp e o affect do bau de experiencia (Type 39 no W2PP): enquanto
// ativo, dobra a EXP recebida. Nao altera atributos, entao nao entra no
// applyExtendedAffectStats; e consultado apenas no pipeline de EXP.
const affectDoubleExp = 39

// scaledMobExperience aplica o piso a recompensa-base e depois a taxa global.
// A recompensa resultante ainda recebe o bonus de party em partyExpShares.
func scaledMobExperience(base uint32, config model.GameplayConfig) uint32 {
	if base < config.EXPMinimum {
		base = config.EXPMinimum
	}
	scaled := uint64(base) * uint64(config.EXPRatePercent) / 100
	if scaled > uint64(^uint32(0)) {
		return ^uint32(0)
	}
	return uint32(scaled)
}

// scaledQuestExperience aplica somente a taxa global. Recompensas de quest
// nao recebem o piso dos mobs nem bonus de party; o buff individual de EXP e
// aplicado separadamente por expWithDoubleBuff.
func scaledQuestExperience(base uint32, config model.GameplayConfig) uint32 {
	scaled := uint64(base) * uint64(config.EXPRatePercent) / 100
	if scaled > uint64(^uint32(0)) {
		return ^uint32(0)
	}
	return uint32(scaled)
}

// expWithDoubleBuff dobra a recompensa quando o personagem tem o bau de EXP
// ativo (affect 39). Aplicado por RECEPTOR, logo cada membro do grupo usa o
// proprio buff. O clamp evita overflow no teto de uint32.
func expWithDoubleBuff(ch *model.Char, reward uint32) uint32 {
	if activePlayerAffect(ch, affectDoubleExp) == nil {
		return reward
	}
	if reward > ^uint32(0)/2 {
		return ^uint32(0)
	}
	return reward * 2
}
