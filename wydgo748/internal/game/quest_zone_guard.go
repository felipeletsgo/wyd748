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

// clearAreaAt returns the ClearArea rectangle containing the tile, if any.
func (w *World) clearAreaAt(x, y uint16) *model.QuestZone {
	for i := range w.clearAreas {
		if w.clearAreas[i].Contains(x, y) {
			return &w.clearAreas[i]
		}
	}
	return nil
}

// areaTimerDeadline is when the public-map area holding the player ends its
// stay: Hidden Valley access expiry, or the shared reset for a clear area and
// for a quest zone entered through its NPC. Zero means no countdown.
func (w *World) areaTimerDeadline(p *Player, now time.Time) time.Time {
	if inHiddenValley(p.X, p.Y) && hiddenValleyAccessActive(p, now) {
		return time.Unix(int64(p.SpecialCoins[hiddenValleyAccessCounter]), 0)
	}
	if w.clearAreaAt(p.X, p.Y) != nil || (p.QuestZoneEntry != "" && w.questZoneAt(p.X, p.Y) != nil) {
		return w.nextQuestZoneReset
	}
	return time.Time{}
}

// syncAreaTimer shows, refreshes or clears the area countdown so every timed
// stay on the public map displays its remaining time, whatever route entered.
func (w *World) syncAreaTimer(p *Player, now time.Time) {
	deadline := w.areaTimerDeadline(p, now)
	if deadline.IsZero() || !deadline.After(now) {
		w.hideQuestZoneTimer(p)
		return
	}
	if p.QuestZoneTimer && p.AreaTimerDeadline.Equal(deadline) {
		return
	}
	if p.Session == nil || p.Session.IsClosed() {
		return
	}
	seconds := uint32((deadline.Sub(now) + time.Second - 1) / time.Second)
	p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, seconds))
	p.QuestZoneTimer, p.AreaTimerDeadline = true, deadline
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
	p.QuestZoneTimer, p.AreaTimerDeadline = true, w.nextQuestZoneReset
}

// hideQuestZoneTimer clears a countdown this server started for a quest zone.
// It never touches a counter shown for an instance.
func (w *World) hideQuestZoneTimer(p *Player) {
	if p == nil || !p.QuestZoneTimer {
		return
	}
	p.QuestZoneTimer, p.AreaTimerDeadline = false, time.Time{}
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
		w.syncAreaTimer(p, now)
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
