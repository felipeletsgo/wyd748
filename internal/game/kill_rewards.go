package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// killRewardPlan is an unpublished mutation. The World may aggregate several
// plans (multi-target skills/DoT), persist all affected accounts once, and only
// then emit death/reward packets.
type killRewardPlan struct {
	killer                      *Player
	mob                         *Mob
	calculatedDamage            uint32
	appliedDamage               uint32
	shares                      []partyExpShare
	accounts                    []*model.Account
	snapshots                   []killPlayerSnapshot
	expByPlayer                 map[*Player]uint32
	leveledUp                   map[*Player]bool
	cytheraChanged              map[*Player]bool
	mountUpdates                []mountHuntUpdate
	drops                       []plannedDrop
	gold                        uint32
	killerLevels                int
	killerReward                uint32
	bossState                   *bossSpawnState
	generatorSlot               int
	generatorCounterShouldLower bool
	mobHPBeforeDeath            uint32
}

type killPlayerSnapshot struct {
	player *Player
	char   model.Char
}

func (w *World) killMobState(p *Player, m *Mob, calculatedDamage, appliedDamage uint32) {
	if p == nil || p.Char == nil {
		w.killMobWithoutPlayer(m)
		return
	}
	plan := w.planMobKill(p, m, calculatedDamage, appliedDamage)
	if plan == nil {
		return
	}
	w.commitKillRewardBatch(p, []*killRewardPlan{plan}, plan.accounts, "morte de mob")
}

func (w *World) killMobWithoutPlayer(m *Mob) {
	if m == nil || m.Dead {
		return
	}
	m.Dead = true
	w.publishMobDeath(m, 0, 0, nil)
	w.notifyBossAddDied(m.ID)
	w.finishBossMobKilled(m, false)
	w.UnregisterBoss(m.ID)
	w.onItemInstanceMobKilled(m, w.now())
	if m.InstanceID == "" && m.GenerIndex >= 0 {
		if slot, ok := w.generatorByIndex[m.GenerIndex]; ok && slot >= 0 &&
			slot < len(w.generators) && w.generators[slot].current > 0 {
			w.generators[slot].current--
		}
	}
	w.removeMobInstance(m)
}

func (w *World) planMobKill(p *Player, m *Mob, calculatedDamage, appliedDamage uint32) *killRewardPlan {
	if p == nil || p.Char == nil || m == nil || m.Dead || m.Def == nil {
		return nil
	}
	baseReward := scaledMobExperience(m.Def.ExpReward, w.gameplay)
	shares := w.partyExpShares(p, baseReward, w.gameplay.PartyEXPBonusPercent)
	plan := &killRewardPlan{
		killer: p, mob: m, calculatedDamage: calculatedDamage, appliedDamage: appliedDamage,
		shares: shares, accounts: uniqueKillAccounts(p, shares),
		expByPlayer:    make(map[*Player]uint32, len(shares)),
		leveledUp:      make(map[*Player]bool, len(shares)),
		cytheraChanged: make(map[*Player]bool, len(shares)), generatorSlot: -1,
	}
	plan.mobHPBeforeDeath = m.HP
	if plan.mobHPBeforeDeath == 0 {
		// Normal callers reach this function after applying the lethal hit. Keep
		// the exact amount removed so a failed durable commit can restore the
		// encounter instead of advancing an instance without its reward state.
		plan.mobHPBeforeDeath = appliedDamage
	}
	seen := make(map[*Player]struct{}, len(shares)+1)
	for _, player := range append([]*Player{p}, playersFromShares(shares)...) {
		if player == nil || player.Char == nil {
			continue
		}
		if _, duplicate := seen[player]; duplicate {
			continue
		}
		seen[player] = struct{}{}
		plan.snapshots = append(plan.snapshots, killPlayerSnapshot{
			player: player, char: cloneCharacterState(player.Char),
		})
	}

	m.Dead = true
	for _, share := range shares {
		receiver := share.player
		oldHP, oldMP := playerCurHP(receiver.Char), playerCurMP(receiver.Char)
		combatReward := w.mobKillExperienceForReceiver(p.Char, receiver.Char, share.reward)
		levels, appliedEXP := grantExp(receiver.Char, combatReward)
		if levels > 0 && updateCelestialCythera(receiver.Char) {
			plan.cytheraChanged[receiver] = true
		}
		w.recalcPlayer(receiver.Char)
		if oldHP > 0 {
			setPlayerCurHP(receiver.Char, minU32(oldHP, playerMaxHP(receiver.Char)))
		}
		if oldMP > 0 {
			setPlayerCurMP(receiver.Char, minU32(oldMP, playerMaxMP(receiver.Char)))
		}
		plan.expByPlayer[receiver] = receiver.Char.Exp
		plan.leveledUp[receiver] = levels > 0
		plan.mountUpdates = append(plan.mountUpdates, w.applyMountHuntExp(receiver, m))
		if receiver == p {
			plan.killerLevels, plan.killerReward = levels, appliedEXP
		}
	}
	if p.Party != nil {
		for _, member := range p.Party.Members {
			if member != nil && member.Char != nil {
				plan.expByPlayer[member] = member.Char.Exp
			}
		}
	}
	plan.drops = append(plan.drops, w.planMobDrops(p, m)...)
	plan.gold = w.rollMobGold(p, m)
	plan.bossState = w.bossSpawnStateForMob(m.ID)
	if plan.bossState != nil {
		plan.drops = append(plan.drops, w.planBossDrops(p, m, plan.bossState)...)
	}
	if m.InstanceID == "" && m.GenerIndex >= 0 {
		if slot, ok := w.generatorByIndex[m.GenerIndex]; ok &&
			slot >= 0 && slot < len(w.generators) && w.generators[slot].current > 0 {
			plan.generatorSlot = slot
			plan.generatorCounterShouldLower = true
		}
	}
	return plan
}

func playersFromShares(shares []partyExpShare) []*Player {
	players := make([]*Player, 0, len(shares))
	for _, share := range shares {
		players = append(players, share.player)
	}
	return players
}

func (w *World) bossSpawnStateForMob(id uint16) *bossSpawnState {
	for _, state := range w.bossSpawns {
		if state != nil && state.mobID == id {
			return state
		}
	}
	return nil
}

func uniqueAccounts(accounts ...*model.Account) []*model.Account {
	result := make([]*model.Account, 0, len(accounts))
	seen := make(map[*model.Account]struct{}, len(accounts))
	for _, account := range accounts {
		if account == nil {
			continue
		}
		if _, duplicate := seen[account]; duplicate {
			continue
		}
		seen[account] = struct{}{}
		result = append(result, account)
	}
	return result
}

func killPlanAccounts(plans []*killRewardPlan, supplied []*model.Account) []*model.Account {
	accounts := append([]*model.Account(nil), supplied...)
	for _, plan := range plans {
		if plan != nil {
			accounts = append(accounts, plan.accounts...)
		}
	}
	return uniqueAccounts(accounts...)
}

func (w *World) commitKillRewardBatch(p *Player, plans []*killRewardPlan,
	accounts []*model.Account, operation string) bool {
	if len(plans) == 0 {
		return true
	}
	accounts = killPlanAccounts(plans, accounts)
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		for i := len(plans) - 1; i >= 0; i-- {
			plans[i].rollback()
		}
		for _, plan := range plans {
			if plan == nil || plan.mob == nil || plan.mob.Def == nil || plan.mob.Def.Extended == nil {
				continue
			}
			m := plan.mob
			w.sendToMobView(m, func() []byte {
				return wire.SetMobHpMp(m.ID, m.HP, m.Def.Extended.MaxHP,
					m.Def.Extended.MaxMP, m.Def.Extended.MaxMP)
			})
		}
		log.Printf("persistir %s: %v", operation, err)
		w.poisonAccountsAfterPersistenceFailure(accounts, operation, err)
		return false
	}
	for _, plan := range plans {
		// CNFMobKill/loot remain one packet per mob, but the character state is
		// an aggregate update. Sending UpdateEtc + party + appearance once per
		// target made a 13-target AoE fan out dozens of redundant packets to
		// every member of a four-player party.
		w.finalizeKillRewardWithState(plan, true, len(plans) == 1)
	}
	if len(plans) > 1 {
		w.publishKillBatchPlayerState(plans)
	}
	if len(plans) > 1 && p != nil && p.Session != nil {
		w.gameplayLogf("batch", "[#%d] lote multi-alvo salvo (%d mortes, %d conta(s))",
			p.Session.ID, len(plans), len(accounts))
	}
	return true
}

func (plan *killRewardPlan) rollback() {
	if plan == nil {
		return
	}
	for _, snapshot := range plan.snapshots {
		if snapshot.player != nil && snapshot.player.Char != nil {
			*snapshot.player.Char = cloneCharacterState(&snapshot.char)
		}
	}
	if plan.mob != nil {
		plan.mob.Dead = false
		plan.mob.HP = plan.mobHPBeforeDeath
		if plan.mob.Def != nil && plan.mob.Def.Extended != nil &&
			plan.mob.HP > plan.mob.Def.Extended.MaxHP {
			plan.mob.HP = plan.mob.Def.Extended.MaxHP
		}
	}
}

func (w *World) finalizeKillReward(plan *killRewardPlan, rewardCommitted bool) {
	w.finalizeKillRewardWithState(plan, rewardCommitted, true)
}

func (w *World) finalizeKillRewardWithState(plan *killRewardPlan, rewardCommitted, publishState bool) {
	if plan == nil || plan.mob == nil {
		return
	}
	p, m := plan.killer, plan.mob
	expByPlayer := plan.expByPlayer
	killerExp := uint32(0)
	if p != nil && p.Char != nil {
		killerExp = p.Char.Exp
	}
	if !rewardCommitted {
		expByPlayer = make(map[*Player]uint32, len(plan.expByPlayer))
		for player := range plan.expByPlayer {
			if player != nil && player.Char != nil {
				expByPlayer[player] = player.Char.Exp
			}
		}
	}
	w.publishMobDeath(m, p.ID, killerExp, expByPlayer)
	if rewardCommitted {
		for _, share := range plan.shares {
			receiver := share.player
			if !publishState || receiver == nil || receiver.Char == nil || receiver.Session == nil {
				continue
			}
			receiver.Session.Send(wire.UpdateEtc(receiver.ID, *receiver.Char))
			if plan.leveledUp[receiver] {
				receiver.Session.Send(wire.UpdateScore(receiver.ID, *receiver.Char))
			}
			if plan.cytheraChanged[receiver] {
				receiver.Session.Send(wire.SendItem(receiver.ID, placeEquip, 1, receiver.Char.Equip[1]))
				w.refreshAppearance(receiver)
			}
			w.updatePartyMember(receiver)
		}
		for _, update := range plan.mountUpdates {
			w.publishMountHuntUpdate(update)
		}
		w.publishPlannedDrops(plan.drops)
		if plan.gold > 0 && p != nil && p.Session != nil {
			w.gameplayLogf("gold", "[#%d] DROP gold +%d (total=%d) de %q",
				p.Session.ID, plan.gold, p.Char.Gold, m.Def.Name)
		}
	}
	if plan.generatorCounterShouldLower && plan.generatorSlot >= 0 &&
		plan.generatorSlot < len(w.generators) && w.generators[plan.generatorSlot].current > 0 {
		w.generators[plan.generatorSlot].current--
	}
	w.notifyBossAddDied(m.ID)
	w.finishBossMobKilled(m, rewardCommitted)
	w.UnregisterBoss(m.ID)
	w.onItemInstanceMobKilled(m, w.now())
	w.removeMobInstance(m)
	if p != nil && p.Session != nil {
		reward, levels := plan.killerReward, plan.killerLevels
		if !rewardCommitted {
			reward, levels = 0, 0
		}
		w.gameplayLogf("kill", "[#%d] MATOU mob id=%d %q (dmg_calculado=%d aplicado=%d hp_alvo=%d exp=+%d/%d membros level=%d +%d)",
			p.Session.ID, m.ID, m.Def.Name, plan.calculatedDamage, plan.appliedDamage,
			m.Def.Extended.MaxHP, reward, len(plan.shares), playerLevel(p.Char), levels)
	}
}

// publishKillBatchPlayerState sends one final aggregate state per character
// after a multi-target kill commit.  The per-mob death packets are still
// emitted by finalizeKillRewardWithState, so the client keeps every corpse,
// EXP confirmation and damage animation; only redundant score/party redraws
// are coalesced.
func (w *World) publishKillBatchPlayerState(plans []*killRewardPlan) {
	type state struct {
		player  *Player
		leveled bool
		cythera bool
	}
	states := make(map[*Player]*state)
	for _, plan := range plans {
		if plan == nil {
			continue
		}
		for _, share := range plan.shares {
			receiver := share.player
			if receiver == nil || receiver.Char == nil || receiver.Session == nil {
				continue
			}
			current := states[receiver]
			if current == nil {
				current = &state{player: receiver}
				states[receiver] = current
			}
			current.leveled = current.leveled || plan.leveledUp[receiver]
			current.cythera = current.cythera || plan.cytheraChanged[receiver]
		}
	}
	for _, current := range states {
		receiver := current.player
		receiver.Session.Send(wire.UpdateEtc(receiver.ID, *receiver.Char))
		if current.leveled {
			receiver.Session.Send(wire.UpdateScore(receiver.ID, *receiver.Char))
		}
		if current.cythera {
			receiver.Session.Send(wire.SendItem(receiver.ID, placeEquip, 1, receiver.Char.Equip[1]))
			w.refreshAppearance(receiver)
		}
		w.updatePartyMember(receiver)
	}
}
