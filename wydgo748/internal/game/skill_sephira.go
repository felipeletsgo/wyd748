package game

import (
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// The native thorn wall starts as BaseSummon RouteType 3, WaitSec 40. The peace
// processor decrements six per visit and removes it after about seven cycles;
// the 500 ms timer sharded six ways yields approximately 21 seconds.
const thornWallLifetime = 21 * time.Second

// The native countdown runs only in peace mode. A damaged wall holds its
// remaining lifetime until one peace visit (about 6 s) passes without damage.
const thornWallCombatHold = 6 * time.Second

func isThornWall(m *Mob) bool {
	return m != nil && m.SummonKind == summonKindThornWall
}

// thornWallAt reports a live thorn wall on the tile in the gameplay space.
// Player routes may cross occupied tiles, but not a wall.
func (w *World) thornWallAt(x, y uint16, space string) bool {
	for _, m := range w.sephiraObjects {
		if isThornWall(m) && !m.Dead && m.HP > 0 && m.X == x && m.Y == y && mobGameplaySpace(m) == space {
			return true
		}
	}
	return false
}

// destroyPlayerSummon removes a thorn wall or combat summon killed by damage.
// It grants no reward.
func (w *World) destroyPlayerSummon(m *Mob, killerID uint16) {
	if m == nil || m.Dead {
		return
	}
	m.HP = 0
	m.Dead = true
	w.publishMobDeath(m, killerID, nil)
	w.removeMobInstance(m)
}

func (w *World) groundCannonAt(x, y uint16) *GroundItem {
	for _, g := range w.nearbyGroundItems(x, y, 0) {
		if g != nil && !g.criticalReserved() && g.Item.Index == 746 && g.ID >= 15001 && g.ID <= 15100 && g.X == x && g.Y == y {
			return g
		}
	}
	return nil
}

func (w *World) canCastThornWall(p *Player, req skillCastRequest, skill model.SkillDef) bool {
	if p == nil || req.TargetX == 0 || req.TargetY == 0 {
		return false
	}
	runtimeID := w.playerRuntimeInstanceID(p.ID)
	occupied := w.positionOccupiedExcept(req.TargetX, req.TargetY, nil, nil)
	if runtimeID != "" {
		occupied = w.positionOccupiedInGameplaySpace(req.TargetX, req.TargetY, runtimeID, nil, nil, nil)
	}
	return !occupied &&
		chebyshev(p.X, p.Y, req.TargetX, req.TargetY) <= maxInt(1, skill.Range) &&
		w.terrain.Walkable(req.TargetX, req.TargetY) &&
		w.combatLineOfSight(p.X, p.Y, req.TargetX, req.TargetY)
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
	if source == nil || source.Score == nil {
		return false
	}
	def := *source
	ext := *source.Score
	def.Score = &ext
	def.Score.CurHP = def.Score.MaxHP
	mobID := w.allocMobID()
	if mobID == 0 {
		return false
	}
	m := &Mob{ID: mobID, Def: &def, X: req.TargetX, Y: req.TargetY,
		InstanceID: w.playerRuntimeInstanceID(p.ID),
		HP:         def.Score.MaxHP, GenerIndex: -1, SummonerID: p.ID,
		SummonKind: summonKindThornWall, ExpiresAt: w.now().Add(thornWallLifetime),
		wallHP: def.Score.MaxHP}
	w.appendMobInstance(m)
	w.publishMobSpawn(m)
	w.sendToPlayerView(p, func() []byte {
		return playerAttackPacket(p.Char, wire.SkillHits(p.ID, p.X, p.Y, m.X, m.Y, p.Char.Exp, playerCombatMP(p.Char),
			int16(skill.Index), motion, skillVisualLevel(mastery), 1,
			[]wire.SkillTarget{{ID: m.ID}}))
	})
	return true
}

func (w *World) tickSephiraObjects(now time.Time) {
	elapsed := time.Duration(0)
	if !w.sephiraTickAt.IsZero() && now.After(w.sephiraTickAt) {
		elapsed = now.Sub(w.sephiraTickAt)
	}
	w.sephiraTickAt = now
	var expired []*Mob
	for _, m := range w.sephiraObjects {
		if !isThornWall(m) || m.Dead {
			continue
		}
		if m.HP < m.wallHP {
			m.wallCombatUntil = now.Add(thornWallCombatHold)
		}
		m.wallHP = m.HP
		if !m.ExpiresAt.IsZero() && now.Before(m.wallCombatUntil) {
			m.ExpiresAt = m.ExpiresAt.Add(elapsed)
		}
		if !m.ExpiresAt.IsZero() && !m.ExpiresAt.After(now) {
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
