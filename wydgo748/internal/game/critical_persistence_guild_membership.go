package game

import (
	"fmt"
	"log"
	"reflect"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type criticalGuildMemberUpdate struct {
	player *Player
	id     uint16
	rank   byte
}

// Extra shared objects are retained by the actor, never sent to the worker.
// Rejection effects run only after an adapter has certified non-commit.
type criticalGuildChangeHooks struct {
	check  func() error
	reject func()
}

// Registry changes retain the complete account/trade closure even when an
// initiating account itself is unchanged. Only membership fields are applied;
// existing account/character addresses remain valid for lifecycle bindings.
func (w *World) queueGuildRegistryChange(actor *Player, seeds []*Player, proposed *model.GuildRegistry,
	updates []criticalGuildMemberUpdate, failure string, publish func()) error {
	if proposed == nil || proposed == w.guilds {
		return errCriticalPersistenceIdentity
	}
	(&World{guilds: proposed}).pruneGuildWarReferences()
	changes := make(map[*Player]*model.Account, len(updates))
	for _, update := range updates {
		if update.player == nil || update.player.Account == nil || !bindCriticalPlayer(update.player).current(w) {
			return errCriticalPersistenceIdentity
		}
		account := changes[update.player]
		if account == nil {
			account = accountStateSnapshot(update.player.Account)
			changes[update.player] = account
		}
		account.Chars[update.player.CharSlot].GuildID = update.id
		account.Chars[update.player.CharSlot].GuildRank = update.rank
	}
	return w.queueGuildAccountChange(actor, seeds, proposed, changes, failure, func() {
		for _, update := range updates {
			update.player.Char.GuildID, update.player.Char.GuildRank = update.id, update.rank
		}
	}, publish)
}

// Account changes and the guild registry form one detached transaction. Every
// participant, including reciprocal trade peers, remains reserved until ack.
// Apply installs only the producer's changed fields, preserving live addresses.
func (w *World) queueGuildAccountChange(actor *Player, seeds []*Player, proposed *model.GuildRegistry,
	changes map[*Player]*model.Account, failure string, apply, publish func(), hooks ...criticalGuildChangeHooks) error {
	if w.criticalWrites == nil || w.guilds == nil || proposed == nil || proposed == w.guilds || actor == nil {
		return errCriticalPersistenceIdentity
	}
	if w.criticalGuildMutationReserved(seeds...) {
		return errCriticalPersistenceBusy
	}
	type retainedMember struct {
		player  *Player
		account *model.Account
		trade   *TradeState
		offer   TradeState
	}
	participants := append([]*Player(nil), seeds...)
	retainedMembers := make([]retainedMember, 0, len(seeds)*2)
	accounts := make([]*model.Account, 0, len(seeds)*2)
	byPlayer := make(map[*Player]*model.Account)
	seenKeys := make(map[string]bool)
	for i := 0; i < len(participants); i++ {
		p := participants[i]
		if p == nil || p.Account == nil || p.Char == nil || !p.InWorld || p.Session == nil || p.Session.IsClosed() ||
			!bindCriticalPlayer(p).current(w) || seenKeys[accountSessionKey(p.Account.Name)] || len(participants) > criticalPersistenceMaxAccounts {
			return errCriticalPersistenceIdentity
		}
		seenKeys[accountSessionKey(p.Account.Name)] = true
		before := retainedMember{player: p, account: accountStateSnapshot(p.Account), trade: p.Trade}
		if p.Trade != nil {
			owner := w.playerWorld(p)
			if owner == nil {
				return errCriticalPersistenceIdentity
			}
			peer := owner.playerByID(p.Trade.OpponentID)
			if peer == nil || peer == p || peer.Trade == nil || peer.Trade.OpponentID != p.ID {
				return errCriticalPersistenceIdentity
			}
			before.offer = *p.Trade
			found := false
			for _, participant := range participants {
				found = found || participant == peer
			}
			if !found {
				participants = append(participants, peer)
			}
		}
		retainedMembers = append(retainedMembers, before)
		account := changes[p]
		if account == nil {
			account = accountStateSnapshot(p.Account)
		} else if account == p.Account || accountSessionKey(account.Name) != accountSessionKey(p.Account.Name) {
			return errCriticalPersistenceIdentity
		}
		accounts, byPlayer[p] = append(accounts, account), account
	}
	if byPlayer[actor] == nil {
		return errCriticalPersistenceIdentity
	}
	for player := range changes {
		if byPlayer[player] == nil {
			return errCriticalPersistenceIdentity
		}
	}
	registry, retained := w.guilds, w.criticalGuildSnapshot()
	checkRetained := func() error {
		if w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) {
			return errCriticalPersistenceIdentity
		}
		for _, before := range retainedMembers {
			p := before.player
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before.account) || p.Trade != before.trade ||
				(p.Trade != nil && *p.Trade != before.offer) {
				return errCriticalPersistenceIdentity
			}
		}
		for _, hook := range hooks {
			if hook.check != nil {
				if err := hook.check(); err != nil {
					return err
				}
			}
		}
		return nil
	}
	if err := checkRetained(); err != nil {
		return err
	}
	_, err := w.submitCriticalWriteWithRejection(criticalWrite{guilds: proposed, accounts: accounts}, participants, false,
		func(criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			*w.guilds = *proposed
			if apply != nil {
				apply()
			}
			return nil
		}, func(criticalPersistenceResult) error {
			if publish != nil {
				publish()
			}
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			actor.Session.Send(wire.MessagePanel(failure))
			for _, hook := range hooks {
				if hook.reject != nil {
					hook.reject()
				}
			}
			log.Printf("[#%d] ERROR guild registry persistence: %v", actor.Session.ID, result.err)
			return nil
		})
	return err
}

func (w *World) guildChangeAdmission(actor *Player, err error) {
	if err == nil {
		return
	}
	actor.Session.Send(wire.MessagePanel("Guild persistence is unavailable. Try again later."))
	log.Printf("[#%d] ERROR guild registry admission: %v", actor.Session.ID, err)
}

func (w *World) queueGuildDeparture(actor, victim *Player, guild *model.Guild, leaving bool) error {
	id, name, victimName := guild.ID, guild.Name, victim.Char.Name
	proposed := w.criticalGuildSnapshot()
	privateWorld := &World{guilds: proposed}
	dissolved, promoted := privateWorld.removeGuildMember(proposed.FindByID(id), victimName)
	seeds := []*Player{actor}
	if victim != actor {
		seeds = append(seeds, victim)
	}
	updates := []criticalGuildMemberUpdate{{victim, 0, 0}}
	var successor *Player
	if promoted != "" {
		successor = w.guildPlayerByName(promoted)
		if successor != nil {
			if successor == actor || successor == victim {
				return errCriticalPersistenceIdentity
			}
			seeds = append(seeds, successor)
			updates = append(updates, criticalGuildMemberUpdate{successor, id, model.GuildRankLeader})
		}
	}
	failure := "Save failed. Nobody was expelled."
	if leaving {
		failure = "Save failed. You are still in the guild."
	}
	return w.queueGuildRegistryChange(actor, seeds, proposed, updates, failure, func() {
		if leaving {
			actor.Session.Send(wire.MessagePanel(fmt.Sprintf("You left the guild %s.", name)))
			w.publishGuildIdentity(victim)
			if promoted != "" {
				w.publishGuildIdentity(successor)
				w.announceToGuild(id, fmt.Sprintf("%s now leads the guild.", promoted), nil)
			}
		} else {
			actor.Session.Send(wire.MessagePanel(fmt.Sprintf("%s was expelled from the guild.", victimName)))
			victim.Session.Send(wire.MessagePanel("You were expelled from the guild."))
			w.publishGuildIdentity(victim)
			w.publishGuildIdentity(successor)
		}
		log.Printf("[#%d] GUILD departure guild=%d character=%q leaving=%t dissolved=%t", actor.Session.ID, id, victimName, leaving, dissolved)
	})
}

func (w *World) queueGuildPromotion(actor, target *Player, guild *model.Guild, rank byte) error {
	proposed := w.criticalGuildSnapshot()
	proposed.FindByID(guild.ID).Member(target.Char.Name).Rank = rank
	return w.queueGuildRegistryChange(actor, []*Player{actor, target}, proposed,
		[]criticalGuildMemberUpdate{{target, guild.ID, rank}}, "Save failed. The promotion was not applied.", func() {
			actor.Session.Send(wire.MessagePanel(fmt.Sprintf("%s is now a sub-leader.", target.Char.Name)))
			target.Session.Send(wire.MessagePanel("You were promoted to sub-leader."))
			w.publishGuildIdentity(target)
			log.Printf("[#%d] GUILD promotion character=%q rank=%d", actor.Session.ID, target.Char.Name, rank)
		})
}

func (w *World) queueGuildAlliance(actor *Player, guild *model.Guild, allyID uint16) error {
	id := guild.ID
	proposed := w.criticalGuildSnapshot()
	proposed.FindByID(id).Ally = allyID
	return w.queueGuildRegistryChange(actor, []*Player{actor}, proposed, nil, "Save failed. The alliance was not changed.", func() {
		message := "Alliance formed."
		if allyID == 0 {
			message = "Alliance broken."
		}
		actor.Session.Send(wire.MessagePanel(message))
		log.Printf("[#%d] GUILD %d alliance -> %d", actor.Session.ID, id, allyID)
	})
}

func (w *World) queueGuildWarDeclaration(actor *Player, guild *model.Guild, targetID uint16) error {
	id, name := guild.ID, guild.Name
	proposed := w.criticalGuildSnapshot()
	privateGuild := proposed.FindByID(id)
	cancel := targetID == 0
	if cancel {
		targetID = privateGuild.WarTarget
	}
	target := proposed.FindByID(targetID)
	reciprocal := target != nil && target.WarTarget == id
	privateGuild.WarTarget = targetID
	failure := "Save failed. The war declaration was not applied."
	if cancel {
		privateGuild.WarTarget = 0
		if reciprocal {
			target.WarTarget = 0
		}
		failure = "Save failed. The war declaration was not canceled."
	}
	targetName := ""
	if target != nil {
		targetName = target.Name
	}
	return w.queueGuildRegistryChange(actor, []*Player{actor}, proposed, nil, failure, func() {
		if cancel {
			message := "War declaration canceled."
			if reciprocal {
				message = "War canceled."
			}
			w.announceToGuild(id, message, nil)
			if target != nil {
				w.announceToGuild(targetID, message, nil)
			}
		} else if reciprocal {
			w.announceToGuild(id, fmt.Sprintf("War started against guild %s.", targetName), nil)
			w.announceToGuild(targetID, fmt.Sprintf("War started against guild %s.", name), nil)
		} else {
			w.announceToGuild(id, fmt.Sprintf("War declared against guild %s.", targetName), nil)
			w.announceToGuild(targetID, fmt.Sprintf("Guild %s declared war on your guild.", name), nil)
		}
		log.Printf("[#%d] GUILD war guild=%d target=%d cancel=%t reciprocal=%t", actor.Session.ID, id, targetID, cancel, reciprocal)
	})
}
