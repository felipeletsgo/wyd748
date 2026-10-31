package game

import (
	"log"
	"time"

	"wydgo/internal/model"
)

type generatorRespawn struct {
	mob     *Mob
	readyAt time.Time
}

// Schedule only finalized deaths. Rejected and unsettled kills cannot respawn.
func (w *World) scheduleGeneratorRespawn(g *generState, m *Mob, diedAt time.Time) {
	if g == nil || m == nil || !m.Dead || g.def.RespawnSeconds <= 0 || g.def.MinuteGenerate <= 0 ||
		!g.def.Enabled || m.InstanceID != "" || !w.hostileMob(m) || generatorReservedForGuildWar(g.def) {
		return
	}
	g.respawns = append(g.respawns, generatorRespawn{mob: m,
		readyAt: diedAt.Add(time.Duration(g.def.RespawnSeconds) * time.Second)})
}

func (w *World) tickGeneratorRespawns(g *generState, now time.Time) {
	if !g.def.Enabled || generatorReservedForGuildWar(g.def) || w.generatorBlockedByKefra(g) {
		return
	}
	remaining := g.respawns[:0]
	for _, pending := range g.respawns {
		if now.Before(pending.readyAt) || g.current >= g.def.MaxNumMob {
			remaining = append(remaining, pending)
			continue
		}
		dead := pending.mob
		leaderID := dead.LeaderID
		if leader := w.mobByID(leaderID); leader == nil || leader.Dead || w.mobGenerator(leader) != g {
			leaderID = 0
		}
		m := w.spawnGeneratorMob(g, dead.Def, leaderID)
		if m == nil {
			remaining = append(remaining, pending)
			continue
		}
		// Keep surviving followers and later replacements attached to their leader.
		if dead.LeaderID == 0 {
			for _, follower := range w.mobs {
				if follower.LeaderID == dead.ID && w.mobGenerator(follower) == g {
					follower.LeaderID = m.ID
				}
			}
			for _, queued := range g.respawns {
				if queued.mob.LeaderID == dead.ID {
					queued.mob.LeaderID = m.ID
				}
			}
		}
		w.npcGenerLog.mobs++
	}
	clear(g.respawns[len(remaining):])
	g.respawns = remaining
}

// Share native placement, route scattering, IDs and publication with startup.
func (w *World) spawnGeneratorMob(g *generState, def *model.NPCDef, leaderID uint16) *Mob {
	pos := g.def.Segments[0]
	requestedX, requestedY := w.scatter(pos.X, pos.Y, pos.Range)
	x, y := w.findFreePosition(requestedX, requestedY, pos.Range)
	if x != requestedX || y != requestedY {
		w.npcGenerLog.relocations++
		if w.npcGenerLogMode == npcGenerLogVerbose {
			log.Printf("NPCGener[%d]: spawn %q relocated (%d,%d)->(%d,%d): blocked/occupied terrain",
				g.def.Index, def.Name, requestedX, requestedY, x, y)
		}
	}
	segments := g.def.Segments
	for i := range segments {
		if segments[i].X != 0 && segments[i].Y != 0 {
			segments[i].X, segments[i].Y = w.scatter(segments[i].X, segments[i].Y, segments[i].Range)
			segments[i].X, segments[i].Y = w.findWalkablePosition(segments[i].X, segments[i].Y, segments[i].Range)
		}
	}
	id := w.allocMobID()
	if id == 0 {
		log.Printf("NPCGener[%d]: spawn stopped: mob ID range exhausted", g.def.Index)
		return nil
	}
	m := &Mob{ID: id, Def: def, X: x, Y: y, HP: def.Score.MaxHP, LeaderID: leaderID,
		GenerIndex: g.def.Index, generator: g, Segments: segments, RouteType: g.def.RouteType,
		WaitUntil: w.now().Add(time.Duration(pos.Wait) * time.Second)}
	w.appendMobInstance(m)
	g.current++
	w.publishMobSpawn(m)
	return m
}
