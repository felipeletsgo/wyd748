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

// expWithDoubleBuff dobra a recompensa quando o personagem informado tem o
// bau de EXP ativo (affect 39). O chamador define a origem: recompensas
// individuais usam o receptor; mortes de mobs usam o matador para que o mesmo
// bonus seja compartilhado com todos os membros elegiveis da party.
// O clamp evita overflow no teto de uint32.
func expWithDoubleBuff(ch *model.Char, reward uint32) uint32 {
	if activePlayerAffect(ch, affectDoubleExp) == nil {
		return reward
	}
	if reward > ^uint32(0)/2 {
		return ^uint32(0)
	}
	return reward * 2
}

// mobKillExperienceForReceiver aplica os bonus de itens do MATADOR a uma
// parcela de EXP de combate e depois respeita as quedas da evolucao de quem a
// recebe. Esse e o contrato nativo observado em MobKilled.cpp do W2PP:
// pMob[conn].ExpBonus e reutilizado dentro do loop de todos os membros.
//
// A ordem conserva o comportamento anterior do emulador: Coral/fada entram
// antes das quedas de Celestial e o bau de EXP dobra o resultado final. O que
// muda e somente a origem autoritativa desses bonus, que deixa de ser cada
// receptor e passa a ser o personagem responsavel pelo abate.
func (w *World) mobKillExperienceForReceiver(killer, receiver *model.Char, reward uint32) uint32 {
	if killer == nil || receiver == nil {
		return 0
	}
	reward = applyPercentReward(reward, w.equipmentGemBonuses(killer).expPercent)
	reward = celestialCombatExperience(receiver, reward)
	return expWithDoubleBuff(killer, reward)
}
