package game

import (
	"log"
	"reflect"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// killRewardParticipants maps every account in a kill batch to its live,
// authenticated player. Persistence ownership is per account, so a batch that
// names an account without a current player cannot be admitted.
func (w *World) killRewardParticipants(plans []*killRewardPlan, accounts []*model.Account) ([]*Player, bool) {
	candidates := make(map[*model.Account]*Player, len(accounts))
	add := func(player *Player) {
		if player != nil && player.Account != nil && candidates[player.Account] == nil {
			candidates[player.Account] = player
		}
	}
	for _, plan := range plans {
		if plan == nil {
			continue
		}
		add(plan.killer)
		for _, share := range plan.shares {
			add(share.player)
		}
		for _, snapshot := range plan.snapshots {
			add(snapshot.player)
		}
	}
	participants := make([]*Player, 0, len(accounts))
	for _, account := range accounts {
		player := candidates[account]
		if player == nil {
			for _, live := range w.players {
				if live != nil && live.Account == account {
					player = live
					break
				}
			}
		}
		if player == nil || player.Char == nil {
			return nil, false
		}
		participants = append(participants, player)
	}
	return participants, true
}

// kefraDefeatProposal applies a player Kefra defeat to a detached registry.
// The live registry changes only after the combined transaction commits.
func (w *World) kefraDefeatProposal(plans []*killRewardPlan) (*killRewardPlan, *model.GuildRegistry, uint16) {
	if w.guilds == nil || w.kefraDefeated() {
		return nil, nil, 0
	}
	for _, plan := range plans {
		if plan == nil || !isKefraBoss(plan.mob) {
			continue
		}
		proposed := w.criticalGuildSnapshot()
		state := model.KefraState{Defeated: true, DefeatedAt: w.now()}
		var guildID uint16
		if plan.killer != nil && plan.killer.Char != nil {
			if guild, _ := w.guildOf(plan.killer.Char); guild != nil {
				guildID = guild.ID
				state.Guild = guild.ID
				if detached := proposed.FindByID(guild.ID); detached != nil {
					if err := detached.AddFame(kefraGuildFame); err != nil {
						log.Printf("kefra: guild %d fame not credited: %v", guild.ID, err)
					}
				}
			}
		}
		proposed.Wars.Kefra = state
		return plan, proposed, guildID
	}
	return nil, nil, 0
}

// resurrectPendingKillMobs reverses unpublished deaths after a kill batch was
// not admitted or its transaction was certified as rejected.
func (w *World) resurrectPendingKillMobs(plans []*killRewardPlan) {
	for i := len(plans) - 1; i >= 0; i-- {
		plan := plans[i]
		if plan == nil {
			continue
		}
		for _, drop := range plan.drops {
			w.settleGlobalDrop(drop.globalDrop, false)
		}
		m := plan.mob
		if m == nil {
			continue
		}
		m.Dead = false
		m.HP = plan.mobHPBeforeDeath
		if m.Def != nil && m.Def.Score != nil {
			if m.HP > m.Def.Score.MaxHP {
				m.HP = m.Def.Score.MaxHP
			}
			w.sendToMobViewProtocol(m, func(*Player) []byte {
				return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP, m.Def.Score.MaxMP, m.Def.Score.MaxMP)
			})
		}
	}
}

// restoreKillCharacters returns every planned receiver to its pre-kill
// character. Plans snapshot before their own mutation, so reverse order ends
// at the earliest image even when one player appears in several plans.
func restoreKillCharacters(plans []*killRewardPlan) {
	for i := len(plans) - 1; i >= 0; i-- {
		if plans[i] == nil {
			continue
		}
		for _, snapshot := range plans[i].snapshots {
			if snapshot.player != nil && snapshot.player.Char != nil {
				*snapshot.player.Char = cloneCharacterState(&snapshot.char)
			}
		}
	}
}

// queueCriticalKillRewards persists a planned kill batch without blocking
// World. planMobKill has already mutated live receivers; this function takes
// those post-kill accounts as the detached proposal, restores live state, and
// keeps each mob dead but unpublished. Rewards, deaths, drops, generator and
// boss/instance progression become visible only after acknowledgement.
func (w *World) queueCriticalKillRewards(p *Player, plans []*killRewardPlan, accounts []*model.Account, operation string) bool {
	participants, ok := w.killRewardParticipants(plans, accounts)
	var proposals []*model.Account
	if ok {
		proposals = make([]*model.Account, len(participants))
		for i, participant := range participants {
			proposals[i] = accountStateSnapshot(participant.Account)
		}
	}
	kefraPlan, guildProposal, kefraGuildID := w.kefraDefeatProposal(plans)
	restoreKillCharacters(plans)
	reject := func(err error) bool {
		w.resurrectPendingKillMobs(plans)
		log.Printf("persist %s: admission rejected: %v", operation, err)
		return false
	}
	if !ok {
		return reject(errCriticalPersistenceIdentity)
	}
	before := make([]*model.Account, len(participants))
	for i, participant := range participants {
		before[i] = accountStateSnapshot(participant.Account)
	}
	registry := w.guilds
	var guildsBefore *model.GuildRegistry
	if guildProposal != nil {
		guildsBefore = w.criticalGuildSnapshot()
	}
	check := func() error {
		for i, participant := range participants {
			if !reflect.DeepEqual(accountStateSnapshot(participant.Account), before[i]) {
				return errCriticalPersistenceIdentity
			}
		}
		if guildProposal != nil && (w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), guildsBefore)) {
			return errCriticalPersistenceIdentity
		}
		for _, plan := range plans {
			if plan != nil && plan.mob != nil && (!plan.mob.Dead || w.mobsByID[plan.mob.ID] != plan.mob) {
				return errCriticalPersistenceIdentity
			}
		}
		return nil
	}
	write := criticalWrite{accounts: proposals, guilds: guildProposal}
	_, err := w.submitCriticalSharedWriteWithRejection(write, participants,
		func(criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			for i, participant := range participants {
				participant.Account.Chars[participant.CharSlot] = proposals[i].Chars[participant.CharSlot]
			}
			if guildProposal != nil {
				*w.guilds = *guildProposal
			}
			return nil
		}, func(criticalPersistenceResult) error {
			for _, plan := range plans {
				w.finalizeKillRewardWithState(plan, true, len(plans) == 1)
				w.respawnKefraGuard(plan.mob)
			}
			if kefraPlan != nil {
				w.announceKefraDefeat(kefraPlan.killer, w.guilds.FindByID(kefraGuildID))
			}
			if len(plans) > 1 {
				w.publishKillBatchPlayerState(plans)
			}
			if len(plans) > 1 && p != nil && p.Session != nil {
				w.gameplayLogf("batch", "[#%d] multi-target batch saved (%d deaths, %d account(s))",
					p.Session.ID, len(plans), len(participants))
			}
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			w.resurrectPendingKillMobs(plans)
			log.Printf("persist %s: certified rejection: %v", operation, result.err)
			return nil
		})
	if err != nil {
		return reject(err)
	}
	return true
}
