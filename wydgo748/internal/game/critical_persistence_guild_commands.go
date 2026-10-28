package game

import (
	"fmt"
	"log"
	"reflect"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Create a private registry/account proposal. Guild commands still on the
// legacy path are fenced before mutation while this aggregate is retained.
func (w *World) queueGuildCreate(p *Player, name string) error {
	if w.criticalWrites == nil || w.guilds == nil || p == nil || p.Account == nil || p.Char == nil ||
		!p.InWorld || p.Session == nil || p.Session.IsClosed() || p.CharSlot < 0 ||
		p.CharSlot >= len(p.Account.Chars) || p.Char != &p.Account.Chars[p.CharSlot] || !bindCriticalPlayer(p).current(w) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalGuildMutationReserved(p) {
		return errCriticalPersistenceBusy
	}
	q := w.criticalWrites.queue
	if w.shuttingDown || q.stopping {
		return errCriticalPersistenceStopping
	}
	if len(q.pending) >= q.capacity {
		return errCriticalPersistenceFull
	}
	if _, ok := q.store.(criticalGameStateStore); !ok {
		return errCriticalPersistenceUnsupported
	}
	if err := model.ValidateGuildName(name); err != nil {
		return err
	}
	if guild, _ := w.guildOf(p.Char); guild != nil || w.guilds.FindByName(name) != nil {
		return fmt.Errorf("guild creation conflicts with existing membership or name")
	}
	id, err := w.guilds.NextGuildID()
	if err != nil {
		return err
	}
	registry := w.guilds
	retained, proposedGuilds := w.criticalGuildSnapshot(), w.criticalGuildSnapshot()
	before, proposed := accountStateSnapshot(p.Account), accountStateSnapshot(p.Account)
	participants, accounts := []*Player{p}, []*model.Account{proposed}
	trade := p.Trade
	var peer *Player
	var peerTrade *TradeState
	var peerBefore *model.Account
	var offer, peerOffer TradeState
	if trade != nil {
		peer = w.playerByID(trade.OpponentID)
		if peer == nil || peer == p || !peer.InWorld || peer.Account == nil || peer.Session == nil || peer.Session.IsClosed() ||
			peer.Trade == nil || peer.Trade.OpponentID != p.ID || !bindCriticalPlayer(peer).current(w) ||
			accountSessionKey(peer.Account.Name) == accountSessionKey(p.Account.Name) {
			return errCriticalPersistenceIdentity
		}
		peerTrade, peerBefore = peer.Trade, accountStateSnapshot(peer.Account)
		offer, peerOffer = *trade, *peerTrade
		participants, accounts = append(participants, peer), append(accounts, peerBefore)
	}
	now := w.now().UTC()
	proposedGuilds.Guilds = append(proposedGuilds.Guilds, model.Guild{
		Citizenship: p.Char.Citizenship,
		ID: id, Name: name, Kingdom: characterKingdom(p.Char), CreatedAt: now,
		Members: []model.GuildMember{{Character: p.Char.Name, Account: p.Account.Name,
			Rank: model.GuildRankLeader, JoinedAt: now}},
	})
	proposed.Chars[p.CharSlot].GuildID, proposed.Chars[p.CharSlot].GuildRank = id, model.GuildRankLeader
	// Pruning is pure registry work; never run it against the retained live state.
	(&World{guilds: proposedGuilds}).pruneGuildWarReferences()
	checkRetained := func() error {
		if w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
			!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Trade != trade ||
			(trade != nil && (*trade != offer || peer.Trade != peerTrade || *peerTrade != peerOffer ||
				!reflect.DeepEqual(accountStateSnapshot(peer.Account), peerBefore))) {
			return errCriticalPersistenceIdentity
		}
		return nil
	}
	_, err = w.submitCriticalWriteWithRejection(criticalWrite{guilds: proposedGuilds, accounts: accounts}, participants, false,
		func(criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			*w.guilds = *proposedGuilds
			p.Char.GuildID, p.Char.GuildRank = id, model.GuildRankLeader
			return nil
		}, func(criticalPersistenceResult) error {
			if !p.Session.IsClosed() {
				p.Session.Send(wire.MessagePanel(fmt.Sprintf("Guild %q created.", name)))
			}
			// Visibility publication follows commit even if the initiating transport
			// closed; Session.Send suppresses packets to closed recipients.
			w.publishGuildIdentity(p)
			log.Printf("[#%d] GUILD created id=%d %q by %q", p.Session.ID, id, name, p.Char.Name)
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			if !p.Session.IsClosed() {
				p.Session.Send(wire.MessagePanel("Save failed. The guild was not created."))
			}
			log.Printf("[#%d] ERROR /create persistence %q: %v", p.Session.ID, name, result.err)
			return nil
		})
	return err
}

// queueGuildJoin receives an already domain-validated acceptance or native
// recruitment. The actor owns admission and settlement; the worker only sees
// detached registry/accounts. Reciprocal trades extend the retained account set.
func (w *World) queueGuildJoin(actor, target *Player, guild *model.Guild, cost, inviteType uint32, acceptance bool) error {
	if w.criticalWrites == nil || w.guilds == nil || guild == nil || w.guilds.FindByID(guild.ID) != guild ||
		actor == nil || target == nil || (acceptance && (actor != target || cost != 0)) ||
		(!acceptance && actor == target) {
		return errCriticalPersistenceIdentity
	}
	if w.criticalGuildMutationReserved(actor, target) {
		return errCriticalPersistenceBusy
	}
	q := w.criticalWrites.queue
	if w.shuttingDown || q.stopping {
		return errCriticalPersistenceStopping
	}
	if len(q.pending) >= q.capacity {
		return errCriticalPersistenceFull
	}
	if _, ok := q.store.(criticalGameStateStore); !ok {
		return errCriticalPersistenceUnsupported
	}
	participants := []*Player{actor}
	if target != actor {
		participants = append(participants, target)
	}
	type retainedParticipant struct {
		player  *Player
		account *model.Account
		trade   *TradeState
		offer   TradeState
	}
	retainedPlayers := make([]retainedParticipant, 0, 4)
	accounts := make([]*model.Account, 0, 4)
	seenAccounts := make(map[string]bool)
	for i := 0; i < len(participants); i++ {
		p := participants[i]
		if p == nil || p.Account == nil || p.Char == nil || !p.InWorld || p.Session == nil || p.Session.IsClosed() ||
			!bindCriticalPlayer(p).current(w) || seenAccounts[accountSessionKey(p.Account.Name)] ||
			len(participants) > criticalPersistenceMaxAccounts {
			return errCriticalPersistenceIdentity
		}
		seenAccounts[accountSessionKey(p.Account.Name)] = true
		before := retainedParticipant{player: p, account: accountStateSnapshot(p.Account), trade: p.Trade}
		if p.Trade != nil {
			peer := w.playerByID(p.Trade.OpponentID)
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
		retainedPlayers = append(retainedPlayers, before)
		accounts = append(accounts, accountStateSnapshot(p.Account))
	}
	if actor.Char.Gold < cost || (!acceptance && target.Char.GuildID != 0) {
		return errCriticalPersistenceIdentity
	}
	registry, retained := w.guilds, w.criticalGuildSnapshot()
	proposedGuilds := w.criticalGuildSnapshot()
	id, name := guild.ID, guild.Name
	proposedGuild := proposedGuilds.FindByID(id)
	rank, ok := guildInviteRank(proposedGuild, inviteType)
	if !ok {
		return errCriticalPersistenceIdentity
	}
	proposedGuild.Members = append(proposedGuild.Members, model.GuildMember{
		Character: target.Char.Name, Account: target.Account.Name,
		Rank: rank, JoinedAt: w.now().UTC(),
	})
	accounts[0].Chars[actor.CharSlot].Gold -= cost
	targetIndex := 0
	if !acceptance {
		targetIndex = 1
	}
	accounts[targetIndex].Chars[target.CharSlot].GuildID = id
	accounts[targetIndex].Chars[target.CharSlot].GuildRank = rank
	(&World{guilds: proposedGuilds}).pruneGuildWarReferences()
	inviteFrom, inviteUntil, cooldown := target.GuildInviteFrom, target.GuildInviteUntil, actor.NextGuildInvite
	checkRetained := func() error {
		if w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
			target.GuildInviteFrom != inviteFrom || target.GuildInviteUntil != inviteUntil || actor.NextGuildInvite != cooldown {
			return errCriticalPersistenceIdentity
		}
		for _, before := range retainedPlayers {
			p := before.player
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before.account) || p.Trade != before.trade ||
				(p.Trade != nil && *p.Trade != before.offer) {
				return errCriticalPersistenceIdentity
			}
		}
		return nil
	}
	_, err := w.submitCriticalWriteWithRejection(criticalWrite{guilds: proposedGuilds, accounts: accounts}, participants, false,
		func(criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			*w.guilds = *proposedGuilds
			actor.Char.Gold -= cost
			target.Char.GuildID, target.Char.GuildRank = id, rank
			if acceptance {
				target.GuildInviteFrom = 0
			}
			return nil
		}, func(criticalPersistenceResult) error {
			w.publishGuildIdentity(target)
			target.Session.Send(wire.MessagePanel(fmt.Sprintf("You joined the guild %s.", name)))
			if !acceptance {
				actor.Session.Send(wire.UpdateEtc(actor.ID, *actor.Char))
				actor.Session.Send(wire.MessagePanel(fmt.Sprintf("%s joined the guild.", target.Char.Name)))
			}
			w.announceToGuild(id, fmt.Sprintf("%s joined the guild.", target.Char.Name), target)
			log.Printf("[#%d] GUILD join guild=%d character=%q acceptance=%t type=%d cost=%d",
				actor.Session.ID, id, target.Char.Name, acceptance, inviteType, cost)
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := checkRetained(); err != nil {
				return err
			}
			message := "Save failed. The recruitment was not applied."
			if acceptance {
				message = "Save failed. You did not join the guild."
			}
			actor.Session.Send(wire.MessagePanel(message))
			log.Printf("[#%d] ERROR guild join persistence: %v", actor.Session.ID, result.err)
			return nil
		})
	return err
}
