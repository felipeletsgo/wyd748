package game

import (
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// tickChaosRecovery retains the 450-second negative-CP recovery contract and
// save-before-confirm ordering. Process at most one due save per tick instead
// of synchronously saving every account that entered in the same interval.
// Oldest deadline first (then ID) prevents starvation under continuous entry.
func (w *World) tickChaosRecovery(now time.Time) {
	var due *Player
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || p.Account == nil || p.PersistencePoisoned || w.criticalPlayerReserved(p) {
			continue
		}
		if p.NextCPRecovery.IsZero() {
			p.NextCPRecovery = now.Add(chaosRecoveryInterval)
			continue
		}
		if now.Before(p.NextCPRecovery) {
			continue
		}
		if p.Char.CP >= 0 {
			p.NextCPRecovery = now.Add(chaosRecoveryInterval)
			continue
		}
		if due == nil || p.NextCPRecovery.Before(due.NextCPRecovery) ||
			(p.NextCPRecovery.Equal(due.NextCPRecovery) && p.ID < due.ID) {
			due = p
		}
	}
	if due == nil {
		return
	}
	p := due
	if w.criticalWrites != nil {
		w.submitChaosRecovery(p, now)
		return
	}
	p.NextCPRecovery = now.Add(chaosRecoveryInterval)
	oldCP := p.Char.CP
	p.Char.CP = model.ClampCP(int(p.Char.CP) + 1)
	if err := w.saveAccount(p.Account); err != nil {
		p.Char.CP = oldCP
		var sessionID int64
		if p.Session != nil {
			sessionID = p.Session.ID
		}
		log.Printf("[#%d] CP recovery was not persisted: %v", sessionID, err)
		return
	}
	w.publishChaosRecovery(p)
}

// Propose only detached state: a held worker must not expose recovered CP or
// consume its deadline. The actor applies the result before releasing fences.
// NewWorld still leaves the coordinator disabled pending the full migration.
func (w *World) submitChaosRecovery(p *Player, now time.Time) {
	proposal := accountStateSnapshot(p.Account)
	if p.CharSlot < 0 || p.CharSlot >= len(proposal.Chars) || p.Char != &p.Account.Chars[p.CharSlot] {
		log.Printf("CP recovery rejected: %v", errCriticalPersistenceIdentity)
		return
	}
	newCP := model.ClampCP(int(p.Char.CP) + 1)
	proposal.Chars[p.CharSlot].CP = newCP
	_, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposal}}, []*Player{p},
		func(criticalPersistenceResult) error {
			p.Char.CP = newCP
			p.NextCPRecovery = now.Add(chaosRecoveryInterval)
			w.publishChaosRecovery(p)
			return nil
		})
	if err != nil {
		log.Printf("CP recovery admission rejected: %v", err)
	}
}

func (w *World) publishChaosRecovery(p *Player) {
	if p.Session != nil && !p.Session.IsClosed() {
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		w.syncPlayerChaos(p)
		p.Session.Send(wire.MessagePanel(fmt.Sprintf("Chaos Point increased to %d.", p.Char.CP)))
	}
}

// nativeCPKillDelta uses MobKilled.cpp's raw CP+75 byte (75 is neutral).
// Killing a neutral character costs 3 CP; deeply negative PKs can cost zero.
func nativeCPKillDelta(victimCP int16) int {
	raw := int(model.ClampCP(int(victimCP))) + 75
	delta := 3 * raw / -25
	if delta < -3 {
		return -3
	}
	if delta > 0 {
		return 0
	}
	return delta
}

// applyPvPKills commits the killer and victims together. War deaths are excluded
// by guildWarPvP; only deaths that actually charge CP reach this mutation.
func (w *World) applyPvPKills(killer *Player, victims ...*Player) {
	if killer == nil || killer.Char == nil || len(victims) == 0 || w.criticalPlayerReserved(killer) {
		return
	}
	// Preflight the full set before penalties, war cleanup or publication.
	for _, victim := range victims {
		if w.criticalPlayerReserved(victim) {
			return
		}
	}
	changed := make([]*Player, 0, len(victims)+1)
	snapshots := make(map[*Player]model.Char, len(victims)+1)
	snapshots[killer] = cloneCharacterState(killer.Char)
	changed = append(changed, killer)
	images := criticalPreimages(killer)

	accounts := make([]*model.Account, 0, len(victims)+1)
	seenAccounts := make(map[*model.Account]struct{}, len(victims)+1)
	addAccount := func(p *Player) {
		if p == nil || p.Account == nil {
			return
		}
		if _, exists := seenAccounts[p.Account]; exists {
			return
		}
		seenAccounts[p.Account] = struct{}{}
		accounts = append(accounts, p.Account)
	}
	addAccount(killer)

	for _, victim := range victims {
		if victim == nil || victim == killer || victim.Char == nil {
			continue
		}
		// Death has already set HP=0, so authorization cannot be re-evaluated
		// with the living-target predicate here. War areas never charge CP/EXP.
		if _, handled := w.guildWarPvP(killer, victim); handled {
			if _, fighting := w.cityFighters[victim]; fighting && victim.Session != nil {
				victim.Session.Send(wire.WarInfo())
			}
			delete(w.cityFighters, victim)
			continue
		}
		if _, duplicate := snapshots[victim]; duplicate {
			continue
		}
		snapshots[victim] = cloneCharacterState(victim.Char)
		changed = append(changed, victim)
		addAccount(victim)
		images = append(images, criticalPreimages(victim)...)

		killer.Char.CP = model.ClampCP(int(killer.Char.CP) + nativeCPKillDelta(victim.Char.CP))
		if victim.Char.CP < 0 {
			victim.Char.CP = model.ClampCP(int(victim.Char.CP) + 1)
		}
		addHeldExperienceDeathDebt(victim.Char)
		w.recalcPlayer(victim.Char)
	}
	if len(changed) == 1 {
		return
	}
	publish := func() {
		for index, player := range changed {
			if player.Session != nil && !player.Session.IsClosed() {
				player.Session.Send(wire.UpdateEtc(player.ID, *player.Char))
				w.syncPlayerChaos(player)
				if index > 0 {
					player.Session.Send(playerScorePacket(player))
				}
			}
		}
	}
	if w.criticalWrites != nil {
		if err := w.queueCriticalLiveMutation("PvP CP", images, publish, criticalLiveMutationOptions{}); err != nil {
			log.Printf("PvP CP changes were not admitted: %v", err)
		}
		return
	}
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		for player, snapshot := range snapshots {
			restored := cloneCharacterState(&snapshot)
			*player.Char = restored
		}
		log.Printf("PvP CP changes were not persisted: %v", err)
		return
	}
	publish()
}
