package game

import "wydgo/internal/wire"

// EXTENSAO_COORDENADA: the existing chat packet dismisses only owned BM summons.
func (w *World) dismissBMSummons(p *Player) {
	if p == nil || p.Char == nil || p.Session == nil || !p.InWorld || w.players[p.Session] != p {
		return
	}
	if w.shuttingDown || w.criticalPlayerReserved(p) {
		p.Session.Send(wire.MessageIndexed(453 - 1000))
		return
	}
	var owned []*Mob
	for _, m := range w.summons {
		if m != nil && m.SummonerID == p.ID && m.SummonKind == summonKindBM {
			owned = append(owned, m)
		}
	}
	for _, m := range owned {
		for _, viewer := range w.players {
			w.hideMob(viewer, m, 0)
		}
		m.Dead = true
		w.removeMobInstance(m)
	}
	p.Session.Send(wire.MessageIndexed(452 - 1000))
}
