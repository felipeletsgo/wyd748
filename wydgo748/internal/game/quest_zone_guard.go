package game

import (
	"log"
	"time"

	"wydgo/internal/model"
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
	if p == nil {
		return
	}
	if zone := w.questZoneAt(x, y); zone != nil {
		p.QuestZoneEntry = zone.Name
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
