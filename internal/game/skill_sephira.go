package game

import (
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// A Vinha nativa nasce como BaseSummon RouteType 3, WaitSec 40. O processor de
// paz decrementa seis por visita e a remove apos cerca de sete ciclos; no timer
// de 500 ms sharded por seis isso equivale a aproximadamente 21 segundos.
const thornWallLifetime = 21 * time.Second

func (w *World) groundCannonAt(x, y uint16) *GroundItem {
	for _, g := range w.groundItems {
		if g != nil && g.Item.Index == 746 && g.ID >= 15001 && g.ID <= 15100 && g.X == x && g.Y == y {
			return g
		}
	}
	return nil
}

func (w *World) canCastThornWall(p *Player, req skillCastRequest, skill model.SkillDef) bool {
	return p != nil && req.TargetX > 0 && req.TargetY > 0 &&
		chebyshev(p.X, p.Y, req.TargetX, req.TargetY) <= maxInt(1, skill.Range) &&
		w.terrain.Walkable(req.TargetX, req.TargetY) &&
		w.combatLineOfSight(p.X, p.Y, req.TargetX, req.TargetY) &&
		!w.positionOccupied(req.TargetX, req.TargetY, nil)
}

func (w *World) castThornWall(p *Player, req skillCastRequest, skill model.SkillDef, mastery int, motion byte) bool {
	if !w.canCastThornWall(p, req, skill) {
		return false
	}
	var source *model.NPCDef
	for i := range w.npcs {
		if w.npcs[i].Name == "Vinha" {
			source = &w.npcs[i]
			break
		}
	}
	if source == nil || source.Extended == nil {
		return false
	}
	def := *source
	ext := *source.Extended
	def.Extended = &ext
	def.Extended.CurHP = def.Extended.MaxHP
	mobID := w.allocMobID()
	if mobID == 0 {
		return false
	}
	m := &Mob{ID: mobID, Def: &def, X: req.TargetX, Y: req.TargetY,
		HP: def.Extended.MaxHP, GenerIndex: -1, SummonerID: p.ID,
		SummonKind: summonKindThornWall, ExpiresAt: time.Now().Add(thornWallLifetime)}
	w.mobs = append(w.mobs, m)
	w.publishMobSpawn(m)
	w.sendToPlayerView(p, func() []byte {
		return spectralPacket(p.Char, wire.SkillHits(p.ID, p.X, p.Y, m.X, m.Y, p.Char.Exp, playerCombatMP(p.Char),
			int16(skill.Index), motion, skillVisualLevel(mastery), 1,
			[]wire.SkillTarget{{ID: m.ID}}))
	})
	return true
}

func (w *World) tickSephiraObjects(now time.Time) {
	var expired []*Mob
	for _, m := range w.sephiraObjects {
		if m != nil && m.SummonKind == summonKindThornWall && !m.ExpiresAt.IsZero() && !m.ExpiresAt.After(now) {
			expired = append(expired, m)
		}
	}
	for _, m := range expired {
		for _, viewer := range w.players {
			w.hideMob(viewer, m, 0)
		}
		m.Dead = true
		w.removeMobInstance(m)
	}
}
