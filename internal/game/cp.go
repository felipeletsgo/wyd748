package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// nativeCPKillDelta porta o calculo basico de MobKilled.cpp. A source trabalha
// com o byte bruto CP+75; matar um personagem neutro custa 3 CP, enquanto
// matar um PK profundamente negativo pode nao gerar penalidade adicional.
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
	oldCP := make(map[*Player]int16, len(victims)+1)
	oldCP[killer] = killer.Char.CP
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
		if _, duplicate := oldCP[victim]; duplicate {
			continue
		}
		oldCP[victim] = victim.Char.CP
		changed = append(changed, victim)
		addAccount(victim)

		killer.Char.CP = model.ClampCP(int(killer.Char.CP) + nativeCPKillDelta(victim.Char.CP))
		if victim.Char.CP < 0 {
			victim.Char.CP = model.ClampCP(int(victim.Char.CP) + 1)
		}
	}
	if len(changed) == 1 {
		return
	}
	if err := w.saveAccountsAtomic(accounts...); err != nil {
		for player, cp := range oldCP {
			player.Char.CP = cp
		}
		log.Printf("CP PvP nao persistido: %v", err)
		return
	}
	for _, player := range changed {
		if player.Session != nil {
			player.Session.Send(wire.UpdateEtc(player.ID, *player.Char))
		}
	}
}
