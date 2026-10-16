package game

import (
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// newbieZoneMessage tells a character why it cannot stay in the training field.
const newbieZoneMessage = "The training field is only for mortals below level 35."

// newbieZoneEligible reports whether a character may be in the training field
// (AttributeMap bit 0x80): mortals below displayed level 35. Score.Level is
// zero-based, so displayed level 35 is Score.Level 34. Reaching level 35 or
// evolving removes access, as the native FREEEXP rule does.
func newbieZoneEligible(ch *model.Char) bool {
	return ch != nil && ch.Score != nil && ch.Score.Level+1 < mortalBeginnerLevelLimit &&
		(ch.Evolution == "" || strings.EqualFold(ch.Evolution, "mortal"))
}

// newbieZoneDestinationAllowed blocks walking or teleporting into the training
// field once the character no longer qualifies.
func (w *World) newbieZoneDestinationAllowed(p *Player, x, y uint16) bool {
	return p == nil || !w.terrain.NewbieZone(x, y) || newbieZoneEligible(p.Char)
}

// tickNewbieZone returns to its home city any character that is inside the
// training field without qualifying, for example after reaching level 35
// there. It runs on the per-second zone sweep and waits for pending durable
// writes instead of queueing a second recall.
func (w *World) tickNewbieZone(now time.Time) {
	_ = now
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || p.Session == nil ||
			w.gameplaySpaceForPlayer(p) != "" || w.criticalPlayerReserved(p) ||
			!w.terrain.NewbieZone(p.X, p.Y) || newbieZoneEligible(p.Char) {
			continue
		}
		log.Printf("TRAINING FIELD: %q (level %d) returned to town", p.Char.Name, p.Char.Score.Level+1)
		if w.recallPlayer(p, "training field level limit") {
			p.Session.Send(wire.MessagePanel(newbieZoneMessage))
		}
	}
}
