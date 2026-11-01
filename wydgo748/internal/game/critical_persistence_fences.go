package game

import (
	"wydgo/internal/model"
	"wydgo/internal/store"
)

// These queries belong to the World actor, just like coordinator admission.
// They protect persistence/readmission boundaries, not all gameplay consumers.
// Production activation still requires combat, resource and lifecycle coverage.
func (w *World) criticalAccountReserved(name string) bool {
	return w.criticalWrites != nil && (w.criticalWrites.queue.accounts[accountSessionKey(name)] != 0 ||
		w.criticalWrites.queue.failedAccounts[accountSessionKey(name)] != 0)
}

func (w *World) criticalCharacterReserved(uid string) bool {
	return w.criticalWrites != nil && (w.criticalWrites.queue.characters[uid] != 0 ||
		w.criticalWrites.queue.failedCharacters[uid] != 0)
}

// Input variants ignore a background autosave (W7). It carries a detached
// snapshot, and any later write of the same participants parks behind it, so
// gameplay may continue. Persistence and admission boundaries use the
// variants above, which include every pending write.
func (q *criticalPersistence) holdsInput(id uint64) bool {
	if id == 0 {
		return false
	}
	pending := q.pending[id]
	return pending == nil || !pending.background || pending.quarantined
}

func (w *World) criticalAccountInputReserved(name string) bool {
	if w.criticalWrites == nil {
		return false
	}
	q, key := w.criticalWrites.queue, accountSessionKey(name)
	return q.failedAccounts[key] != 0 || q.holdsInput(q.accounts[key])
}

func (w *World) criticalCharacterInputReserved(uid string) bool {
	if w.criticalWrites == nil {
		return false
	}
	q := w.criticalWrites.queue
	return q.failedCharacters[uid] != 0 || q.holdsInput(q.characters[uid])
}

func (w *World) criticalPlayerInputReserved(p *Player) bool {
	if p == nil || w.criticalWrites == nil {
		return false
	}
	if p.Account != nil {
		if w.criticalAccountInputReserved(p.Account.Name) {
			return true
		}
		for _, character := range p.Account.Chars {
			if w.criticalCharacterInputReserved(character.UID) {
				return true
			}
		}
	}
	return p.Char != nil && w.criticalCharacterInputReserved(p.Char.UID)
}

func (w *World) criticalGuildReserved() bool {
	if w.criticalWrites == nil {
		return false
	}
	q := w.criticalWrites.queue
	return q.guilds != 0 || q.failedGuilds != 0 || len(q.guildKeys) > 0 || len(q.failedGuildKeys) > 0
}

// criticalGuildWholeReserved reports a pending or failed whole-registry write.
func (w *World) criticalGuildWholeReserved() bool {
	return w.criticalWrites != nil && (w.criticalWrites.queue.guilds != 0 || w.criticalWrites.queue.failedGuilds != 0)
}

// criticalGuildIDReserved reports a write that owns this guild (W8), through
// its key or the whole registry. ID 0 is the registry participant.
func (w *World) criticalGuildIDReserved(id uint16) bool {
	if w.criticalGuildWholeReserved() {
		return true
	}
	if w.criticalWrites == nil {
		return false
	}
	q, name := w.criticalWrites.queue, guildRowKey(store.GuildParticipant{GuildID: id})
	return q.guildKeys[name] != 0 || q.failedGuildKeys[name] != 0
}

// Tower captures and Kefra deaths retain the mob as well as the registry.
// These public-world entities cannot move, expire, retarget or be replaced
// while the shared registry transaction remains unacknowledged.
func (w *World) criticalGuildMobReserved(m *Mob) bool {
	return m != nil && (m.GuildWarTower || isKefraBoss(m)) && w.criticalGuildReserved()
}

// Check before any guild, invitation or account mutation. A save-time check is
// too late: even a rolled-back mutation can invalidate another retained plan.
func (w *World) criticalGuildMutationReserved(players ...*Player) bool {
	// A pending write of the registry header or of a player's own guild holds
	// the command (W8); writes of unrelated guilds do not. The queue still
	// refuses any proposal whose participants are reserved.
	if w.criticalGuildIDReserved(0) {
		return true
	}
	for _, p := range players {
		if p != nil && p.Char != nil && p.Char.GuildID != 0 && w.criticalGuildIDReserved(p.Char.GuildID) {
			return true
		}
		if w.criticalTradeReserved(p) {
			return true
		}
	}
	return false
}

func (w *World) criticalAccountStateReserved(account *model.Account) bool {
	if account != nil && !w.shuttingDown {
		session := w.accountSessions[accountSessionKey(account.Name)]
		if retained := w.criticalDisconnects[session]; retained != nil && retained.player.Account != nil &&
			accountSessionKey(retained.player.Account.Name) == accountSessionKey(account.Name) {
			return true
		}
	}
	return w.criticalAccountWriteReserved(account)
}

func (w *World) criticalAccountWriteReserved(account *model.Account) bool {
	if account == nil || w.criticalWrites == nil {
		return false
	}
	if w.criticalAccountReserved(account.Name) {
		return true
	}
	for _, character := range account.Chars {
		if w.criticalCharacterReserved(character.UID) {
			return true
		}
	}
	return false
}

// criticalPlayerReserved gates gameplay input and interaction with p.
func (w *World) criticalPlayerReserved(p *Player) bool {
	if p != nil && !w.shuttingDown {
		if retained := w.criticalDisconnects[p.Session]; retained != nil && retained.player == p {
			return true
		}
	}
	return w.criticalPlayerInputReserved(p)
}

// Pending disconnect intent also fences gameplay when the bounded queue is
// full. Admission/resumption must query actual writes, not its own intent fence.
func (w *World) criticalPlayerWriteReserved(p *Player) bool {
	return p != nil && (w.criticalAccountWriteReserved(p.Account) ||
		(p.Char != nil && w.criticalCharacterReserved(p.Char.UID)))
}

// criticalKillParticipantsReserved must be checked before damage, not after a
// lethal hit has already been published. Keep reward eligibility unchanged:
// dropping a reserved party member would silently change EXP and party bonuses.
func (w *World) criticalKillParticipantsReserved(killer *Player) bool {
	// Guild fame/Kefra and event progression share retained aggregates. A
	// lethal hit must not mutate membership/rewards behind an instance write.
	if w.criticalGuildReserved() || w.criticalInstanceReserved() || w.criticalPlayerReserved(killer) {
		return true
	}
	if w.criticalWrites == nil {
		return false
	}
	for _, share := range w.partyExpShares(killer, 1, 0) {
		if w.criticalPlayerReserved(share.player) {
			return true
		}
	}
	return false
}
