package game

import (
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// questZoneGuardInterval matches the native per-second user sweep
// (TMSrv ProcessSecMinTimer: QuestFlag check for each quest area).
const questZoneGuardInterval = time.Second

// questZoneInterior reports whether a tile is strictly inside the zone. The
// native guard uses strict comparisons, so the border tiles (walls) never
// expel a player standing on them; the periodic reset keeps the inclusive
// ClearAreaQuest rectangle.
func questZoneInterior(z model.QuestZone, x, y uint16) bool {
	return x > z.X1 && x < z.X2 && y > z.Y1 && y < z.Y2
}

// questZoneAt returns the quest zone containing the tile, if any.
func (w *World) questZoneAt(x, y uint16) *model.QuestZone {
	for i := range w.questZones {
		if w.questZones[i].Contains(x, y) {
			return &w.questZones[i]
		}
	}
	return nil
}

// grantQuestZoneEntry records the zone a legitimate teleport placed the
// player in. It is the native QuestFlag: runtime only, never persisted, so a
// relogin inside the area has no entry and is returned to town.
func (w *World) grantQuestZoneEntry(p *Player, x, y uint16) {
	if p == nil || w.gameplaySpaceForPlayer(p) != "" {
		return
	}
	if zone := w.questZoneAt(x, y); zone != nil {
		p.QuestZoneEntry = zone.Name
		w.showQuestZoneTimer(p)
	}
}

// Quest-zone countdown (EXTENSAO_COORDENADA). Native 7.48 shows no timer in
// quest areas. The ClearAreaQuest cycle is one global deadline, so the client
// receives the seconds left until the next reset through the InstanceTime
// counter (0x3A1) it already draws for instances.
func (w *World) showQuestZoneTimer(p *Player) {
	if p == nil || p.Session == nil || p.Session.IsClosed() || w.nextQuestZoneReset.IsZero() {
		return
	}
	remaining := w.nextQuestZoneReset.Sub(w.now())
	if remaining <= 0 {
		return
	}
	seconds := uint32((remaining + time.Second - 1) / time.Second)
	p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, seconds))
	p.QuestZoneTimer = true
}

// hideQuestZoneTimer clears a countdown this server started for a quest zone.
// It never touches a counter shown for an instance.
func (w *World) hideQuestZoneTimer(p *Player) {
	if p == nil || !p.QuestZoneTimer {
		return
	}
	p.QuestZoneTimer = false
	// An instance may already have replaced the shared HUD. Retire the quest
	// ownership without sending zero over the instance's newer countdown.
	if w.gameplaySpaceForPlayer(p) != "" {
		return
	}
	if p.Session != nil && !p.Session.IsClosed() {
		p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, 0))
	}
}

// questZoneQuest returns the entry quest whose NPC owns the zone.
func (w *World) questZoneQuest(zone *model.QuestZone) *model.QuestDef {
	if zone == nil || len(w.questsByNPC) == 0 {
		return nil
	}
	return w.questsByNPC[model.NormalizeQuestNPC(zone.Name)]
}

// questZoneViolation explains why a player may not stay in the zone, or
// returns "" when the stay is legitimate. A player must have entered through
// the zone's NPC (no wall jumping or warping in) and must still satisfy the
// entry level range: reaching the cap ends the stay.
func (w *World) questZoneViolation(p *Player, zone *model.QuestZone) string {
	if p.QuestZoneEntry != zone.Name {
		return "no entry"
	}
	quest := w.questZoneQuest(zone)
	if quest == nil || p.Char == nil || p.Char.Score == nil {
		return ""
	}
	level := p.Char.Score.Level
	if quest.Requires.MinLevel != 0 && level < quest.Requires.MinLevel {
		return "below level"
	}
	if quest.Requires.MaxLevel != 0 && level > quest.Requires.MaxLevel {
		return "level limit reached"
	}
	return ""
}

// tickQuestZoneGuard returns to town every player inside a quest zone without
// a valid stay.
func (w *World) tickQuestZoneGuard(now time.Time) {
	if len(w.questZones) == 0 {
		return
	}
	for _, p := range w.players {
		// A player whose teleport or save is still committing is re-checked
		// on a later sweep instead of queueing a second recall.
		if p == nil || !p.InWorld || p.Char == nil || w.criticalPlayerReserved(p) {
			continue
		}
		if w.gameplaySpaceForPlayer(p) != "" {
			p.QuestZoneEntry = ""
			w.hideQuestZoneTimer(p)
			continue
		}
		if p.QuestZoneTimer && w.questZoneAt(p.X, p.Y) == nil {
			w.hideQuestZoneTimer(p) // left the zone by any route
		}
		for i := range w.questZones {
			zone := &w.questZones[i]
			if !questZoneInterior(*zone, p.X, p.Y) {
				continue
			}
			if reason := w.questZoneViolation(p, zone); reason != "" {
				log.Printf("QUEST ZONE %s: %q returned to town (%s)", zone.Name, p.Char.Name, reason)
				p.QuestZoneEntry = ""
				w.recallPlayer(p, "quest zone "+zone.Name+": "+reason)
			}
			break
		}
	}
}
