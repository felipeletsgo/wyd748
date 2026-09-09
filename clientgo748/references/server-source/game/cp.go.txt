package game

import (
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// tickChaosRecovery porta o contador de 450 segundos do TMSrv. Só CP negativo
// é recuperado; o ponto neutro/positivo não sobe além de zero por passagem de
// tempo. A alteração passa pelo mesmo save-before-confirm dos demais estados.
func (w *World) tickChaosRecovery(now time.Time) {
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
			continue
		}
		if p.NextCPRecovery.IsZero() {
			p.NextCPRecovery = now.Add(chaosRecoveryInterval)
			continue
		}
		if now.Before(p.NextCPRecovery) {
			continue
		}
		p.NextCPRecovery = now.Add(chaosRecoveryInterval)
		if p.Char.CP >= 0 {
			continue
		}
		oldCP := p.Char.CP
		p.Char.CP = model.ClampCP(int(p.Char.CP) + 1)
		if err := w.saveAccount(p.Account); err != nil {
			p.Char.CP = oldCP
			var sessionID int64
			if p.Session != nil {
				sessionID = p.Session.ID
			}
			log.Printf("[#%d] recuperacao de CP nao persistida: %v", sessionID, err)
			continue
		}
		if p.Session != nil {
			p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
			w.syncPlayerChaos(p)
			p.Session.Send(wire.MessagePanel(fmt.Sprintf("Chaos Point increased to %d.", p.Char.CP)))
		}
	}
}

// nativeCPKillDelta porta o calculo basico de MobKilled.cpp. A source trabalha
// com o byte bruto CP+75 (75 = neutro); matar um personagem neutro custa 3 CP,
// enquanto matar um PK profundamente negativo pode nao gerar penalidade.
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

// applyPvPKills atualiza killer e vitimas num unico commit. Wars ainda nao
// entram aqui; quando seus estados forem implementados, devem chamar este
// servico somente para mortes que realmente penalizam CP.
func (w *World) applyPvPKills(killer *Player, victims ...*Player) {
	if killer == nil || killer.Char == nil || len(victims) == 0 {
		return
	}
	changed := make([]*Player, 0, len(victims)+1)
	snapshots := make(map[*Player]model.Char, len(victims)+1)
	snapshots[killer] = cloneCharacterState(killer.Char)
	changed = append(changed, killer)

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
		if _, duplicate := snapshots[victim]; duplicate {
			continue
		}
		snapshots[victim] = cloneCharacterState(victim.Char)
		changed = append(changed, victim)
		addAccount(victim)

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
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		for player, snapshot := range snapshots {
			restored := cloneCharacterState(&snapshot)
			*player.Char = restored
		}
		log.Printf("CP PvP nao persistido: %v", err)
		return
	}
	for index, player := range changed {
		if player.Session != nil {
			player.Session.Send(wire.UpdateEtc(player.ID, *player.Char))
			w.syncPlayerChaos(player)
			if index > 0 {
				player.Session.Send(playerScorePacket(player))
			}
		}
	}
}
