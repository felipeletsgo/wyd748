package game

import (
	"errors"
	"fmt"
)

// npcInteractionRange is the maximum Chebyshev interaction distance.
// Captured 7.48 clicks reached at least six tiles; eight accommodates those
// clicks while remaining well below the 32-tile visibility half-window.
// The former four-tile estimate rejected legitimate interactions.
const npcInteractionRange = 8

var (
	errNPCNotFound   = errors.New("NPC not found")
	errNPCNotVisible = errors.New("NPC not visible")
	errNPCTooFar     = errors.New("NPC out of range")
)

// resolveNPCInteraction validates existence, current gameplay-space visibility,
// prior materialization and distance for shops, masters, storage and quests.
// Coordinates alone cannot authorize interaction with another runtime.
func (w *World) resolveNPCInteraction(p *Player, npcID uint16) (*Mob, error) {
	if p == nil || p.Char == nil {
		return nil, errNPCNotFound
	}
	m := w.mobByID(npcID)
	if m == nil || m.Def == nil || m.Dead {
		return nil, errNPCNotFound
	}
	if !p.hasVisible(m.ID) || !w.mobVisibleToPlayer(p, m) {
		return nil, errNPCNotVisible
	}
	if distance := chebyshev(p.X, p.Y, m.X, m.Y); distance > npcInteractionRange {
		return nil, fmt.Errorf("%w (%d tiles)", errNPCTooFar, distance)
	}
	return m, nil
}

// npcInteractionMessage maps a rejection to the client message panel.
func npcInteractionMessage(err error) string {
	switch {
	case errors.Is(err, errNPCTooFar):
		return "Move closer to interact."
	case errors.Is(err, errNPCNotVisible):
		return "That character is not visible."
	default:
		return "That character is not available."
	}
}
