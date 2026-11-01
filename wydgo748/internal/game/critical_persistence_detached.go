package game

import (
	"log"
	"time"

	"wydgo/internal/wire"
)

// Detached proposals (DOCS/server/server-correction-plan.md, W1/D1). These
// producers plan the write on a private account copy and install it only after
// COMMIT. A certified rejection therefore leaves live state untouched: the
// continuation resends the authoritative view, tells only the affected players
// and keeps every session open. The owner already checked their bindings.

// Timer-driven writes wait this long after a rejection, so a storage outage
// does not resubmit them on every tick.
const backgroundPersistenceRetryDelay = accountAutoSaveInterval

// rejectDetachedProposal returns a rejection continuation. refresh resends one
// player's view; message is English catalog text. Silent background work
// passes no operation, refresh or message.
func (w *World) rejectDetachedProposal(operation, message string, refresh func(*Player), notify ...*Player) func(criticalPersistenceResult) error {
	return func(result criticalPersistenceResult) error {
		for _, p := range notify {
			if p == nil || p.Session == nil || p.Session.IsClosed() {
				continue
			}
			if refresh != nil {
				refresh(p)
			}
			if message != "" {
				p.Session.Send(wire.MessagePanel(w.trFor(p, message)))
			}
		}
		if operation != "" {
			log.Printf("%s canceled after certified rejection: %v", operation, result.err)
		}
		return nil
	}
}

// rejectBackgroundProposal keeps live state and delays the next timer attempt.
func (w *World) rejectBackgroundProposal(p *Player) func(criticalPersistenceResult) error {
	return func(criticalPersistenceResult) error {
		p.persistenceRetryAt = w.now().Add(backgroundPersistenceRetryDelay)
		return nil
	}
}

func (p *Player) backgroundPersistenceDeferred(now time.Time) bool {
	return now.Before(p.persistenceRetryAt)
}

// resendSlots restores the client view of two swapped slots from live state.
func resendSlots(p *Player, st, sp, dt, dp byte) {
	for _, target := range [][2]byte{{st, sp}, {dt, dp}} {
		if item := playerSlotOf(p, target[0], target[1]); item != nil {
			p.Session.Send(wire.SendItem(p.ID, target[0], target[1], *item))
		}
	}
}
