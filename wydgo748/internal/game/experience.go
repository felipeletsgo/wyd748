package game

import (
	"time"

	"wydgo/internal/model"
)

// affectDoubleExp is the experience box affect (type 39 in WYD 7.48). It doubles
// received EXP without changing stats, so only the EXP pipeline consumes it.
const affectDoubleExp = 39

// scaledMobExperience applies the reward floor before the global rate.
// partyExpShares applies the party bonus to the resulting reward.
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

// scaledQuestExperience applies only the global rate. Quest rewards do not
// receive the mob floor, party bonus or Kefra combat bonus. expWithDoubleBuff
// applies the character's individual EXP affect separately.
func scaledQuestExperience(base uint32, config model.GameplayConfig) uint32 {
	scaled := uint64(base) * uint64(config.EXPRatePercent) / 100
	if scaled > uint64(^uint32(0)) {
		return ^uint32(0)
	}
	return uint32(scaled)
}

// expWithDoubleBuff doubles the reward while the supplied character has an
// active experience box (affect 39). Individual rewards use the receiver;
// mob kills use the killer so every eligible party member shares the bonus.
// Saturation prevents uint32 overflow.
func expWithDoubleBuff(ch *model.Char, reward uint32) uint32 {
	return expWithDoubleBuffAt(ch, reward, time.Now())
}

func expWithDoubleBuffAt(ch *model.Char, reward uint32, now time.Time) uint32 {
	if activePlayerAffectAt(ch, affectDoubleExp, now) == nil {
		return reward
	}
	if reward > ^uint32(0)/2 {
		return ^uint32(0)
	}
	return reward * 2
}

// mobKillExperienceForReceiver preserves each receiver's evolution/level curve
// while applying the killer's item bonuses to every eligible party share.
// MobKilled.cpp reuses pMob[conn].ExpBonus across the receiver loop: GetExpApply
// reduces first, then Coral/fairy and the experience box amplify the reward.
// The owner-defined Kefra bonus doubles the result while the durable world
// defeat flag is set; it does not penalize EXP while Kefra is alive.
func (w *World) mobKillExperienceForReceiver(killer, receiver *model.Char, reward uint32) uint32 {
	if killer == nil || receiver == nil {
		return 0
	}
	reward = combatExperienceByEvolution(receiver, reward)
	reward = applyPercentReward(reward, w.equipmentGemBonuses(killer).expPercent)
	reward = expWithDoubleBuffAt(killer, reward, w.now())
	if w.kefraDefeated() {
		if reward > ^uint32(0)/2 {
			return ^uint32(0)
		}
		reward *= 2
	}
	return reward
}
