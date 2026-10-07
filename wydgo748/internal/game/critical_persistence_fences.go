package game

import "wydgo/internal/model"

// These queries belong to the World actor, just like coordinator admission.
// They protect persistence/readmission boundaries, not all gameplay consumers.
// Production activation still requires combat, resource and lifecycle coverage.
func (w *World) criticalAccountReserved(name string) bool {
	return w.criticalWrites != nil && w.criticalWrites.queue.accounts[accountSessionKey(name)] != 0
}

func (w *World) criticalCharacterReserved(uid string) bool {
	return w.criticalWrites != nil && w.criticalWrites.queue.characters[uid] != 0
}

func (w *World) criticalGuildReserved() bool {
	return w.criticalWrites != nil && w.criticalWrites.queue.guilds != 0
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
	if w.criticalGuildReserved() {
		return true
	}
	for _, p := range players {
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

func (w *World) criticalPlayerReserved(p *Player) bool {
	if p != nil && !w.shuttingDown {
		if retained := w.criticalDisconnects[p.Session]; retained != nil && retained.player == p {
			return true
		}
	}
	return w.criticalPlayerWriteReserved(p)
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
