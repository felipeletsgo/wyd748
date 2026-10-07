package game

import (
	"log"

	"wydgo/internal/net"
)

const (
	// criticalDeferredCommandLimit bounds one session's backlog behind its own
	// persistence reservation. It equals two seconds of the default inbound
	// packet limit; a longer stall closes the transport instead of growing RAM.
	criticalDeferredCommandLimit = 512
	// criticalDeferredReplayLimit bounds the work one resume pass performs on
	// the actor between other completions, commands and ticks.
	criticalDeferredReplayLimit = 64
)

// deferCriticalCommand retains a validated packet while its player is
// reserved, or while older packets of the same session are still waiting.
// It returns false when the packet may be dispatched immediately.
func (w *World) deferCriticalCommand(cmd command) bool {
	if w.criticalWrites == nil || cmd.s == nil || cmd.pkt == nil {
		return false
	}
	queued := w.criticalDeferred[cmd.s]
	if len(queued) == 0 && !w.criticalPlayerReserved(w.players[cmd.s]) {
		return false
	}
	if len(queued) >= criticalDeferredCommandLimit {
		log.Printf("[#%d] persistence backlog exceeded %d packets; closing session",
			cmd.s.ID, criticalDeferredCommandLimit)
		w.dropCriticalDeferred(cmd.s)
		cmd.s.Close()
		return true
	}
	if w.criticalDeferred == nil {
		w.criticalDeferred = make(map[*net.Session][]command)
	}
	w.criticalDeferred[cmd.s] = append(queued, cmd)
	w.criticalDeferredCount++
	metricCriticalDeferredCommands.Set(int64(w.criticalDeferredCount))
	return true
}

func (w *World) dropCriticalDeferred(s *net.Session) {
	queued := w.criticalDeferred[s]
	if queued == nil {
		return
	}
	w.criticalDeferredCount -= len(queued)
	clear(queued)
	delete(w.criticalDeferred, s)
	metricCriticalDeferredCommands.Set(int64(w.criticalDeferredCount))
}

// resumeCriticalDeferredCommands replays waiting packets in arrival order for
// players whose reservation has been acknowledged. A replayed packet may start
// a new reservation; the rest of that session's backlog then waits again.
// Closed transports never replay: their packets cannot mutate a retained player.
func (w *World) resumeCriticalDeferredCommands(limit int) {
	if w.criticalDeferredCount == 0 {
		return
	}
	for s, queued := range w.criticalDeferred {
		if w.shuttingDown || s.IsClosed() || w.players[s] == nil {
			w.dropCriticalDeferred(s)
			continue
		}
		for len(queued) > 0 {
			if limit <= 0 {
				return
			}
			if w.criticalPlayerReserved(w.players[s]) {
				break
			}
			cmd := queued[0]
			queued[0] = command{}
			queued = queued[1:]
			w.criticalDeferredCount--
			if len(queued) == 0 {
				delete(w.criticalDeferred, s)
			} else {
				w.criticalDeferred[s] = queued
			}
			cmd.replayDeferred = true
			limit--
			w.safeHandle(cmd)
			// The handler may close the session or enter maintenance.
			queued = w.criticalDeferred[s]
			if w.shuttingDown || s.IsClosed() {
				w.dropCriticalDeferred(s)
				break
			}
		}
	}
	metricCriticalDeferredCommands.Set(int64(w.criticalDeferredCount))
}
