package game

import (
	"fmt"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// MODERNIZACAO_COMPATIVEL: reuse the existing notice/chat and 0x3A1 timer
// contracts. Announcements do not change admission, wave timing or rewards.
type colosseumNoticeState struct {
	day  string
	mask uint8
}

func (w *World) colosseumNotice(message string) {
	w.broadcast(func() []byte { return wire.MessagePanel(message) })
	w.broadcast(func() []byte { return wire.MessageWhisper(0, "[Colosseum]", message, 7) })
}

func (w *World) tickColosseumNotices(local time.Time) {
	c := w.colosseum
	if (c.stage != 0 && c.stage != 8) || len(c.exits) != 0 {
		return
	}
	day := local.Format("2006-01-02")
	for tier := 0; tier < model.ColosseumTiers; tier++ {
		if c.claimed[tier] == day {
			continue
		}
		start := time.Date(local.Year(), local.Month(), local.Day(), int(c.config.EventHour)+tier+1, 5, 0, 0, local.Location())
		left := start.Sub(local)
		if left <= 0 || left > 5*time.Minute {
			continue
		}
		notice := &c.notices[tier]
		if notice.day != day {
			*notice = colosseumNoticeState{day: day}
		}
		bit := uint8(1)
		if left <= time.Minute {
			// Do not replay the five-minute notice after a late startup/tick.
			bit = 3
		}
		if notice.mask&bit == bit {
			continue
		}
		notice.mask |= bit
		minutes := int((left + time.Minute - 1) / time.Minute)
		unit := "minutes"
		if minutes == 1 {
			unit = "minute"
		}
		w.colosseumNotice(fmt.Sprintf("Colosseum %c starts in %d %s. Entrance gates close at %s.",
			"NMA"[tier], minutes, unit, start.Format("15:04")))
	}
}

// The next wave follows the actual stage, including the gate-opening stage.
// An overdue wave has no positive countdown until the scheduler spawns it;
// pending Elixir writes must never produce a fictitious future wave.
func (w *World) nextColosseumWave() (int, time.Time) {
	c := w.colosseum
	if c == nil || !c.config.Enabled || c.startedAt.IsZero() {
		return 0, time.Time{}
	}
	var wave, minutes int
	switch c.stage {
	case 1:
		wave, minutes = 1, 1
	case 2, 3:
		wave, minutes = 2, 4
	case 4:
		wave, minutes = 3, 6
	case 5:
		wave, minutes = 4, 8
	case 6:
		wave, minutes = 5, 10
	default:
		return 0, time.Time{}
	}
	return wave, c.startedAt.Add(time.Duration(minutes) * time.Minute)
}

func (w *World) syncColosseumCountdowns(now time.Time) {
	c := w.colosseum
	wave, deadline := w.nextColosseumWave()
	if !deadline.After(now) {
		w.clearColosseumCountdowns(now)
		return
	}
	if c.countdowns == nil {
		c.countdowns = make(map[*Player]time.Time)
	}
	for p := range c.countdowns {
		if !w.colosseumCountdownPlayer(p) {
			w.clearColosseumCountdown(p, now)
		}
	}
	seconds := uint32((deadline.Sub(now) + time.Second - 1) / time.Second)
	for _, p := range w.colosseumPlayers(colosseumArea) {
		if !w.colosseumCountdownPlayer(p) || c.countdowns[p].Equal(deadline) {
			continue
		}
		// Retire a previous public-area timer before taking ownership of the
		// shared HUD. The quest guard must not clear this newer countdown.
		w.hideQuestZoneTimer(p)
		p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, seconds))
		p.Session.Send(wire.MessageWhisper(0, "[Colosseum]",
			fmt.Sprintf("Next wave: %d/5. Countdown: %d seconds.", wave, seconds), 7))
		c.countdowns[p] = deadline
	}
}

func (w *World) colosseumCountdownPlayer(p *Player) bool {
	return p != nil && p.InWorld && p.Char != nil && p.Session != nil && !p.Session.IsClosed() &&
		w.players[p.Session] == p && w.gameplaySpaceForPlayer(p) == "" && colosseumArea.contains(p.X, p.Y)
}

func (w *World) clearColosseumCountdown(p *Player, now time.Time) {
	delete(w.colosseum.countdowns, p)
	// A new runtime or public-area timer may already own the same client HUD.
	// Never clear that newer timer or an unrelated instance question.
	if p == nil || p.Session == nil || p.Session.IsClosed() || !p.InWorld || w.players[p.Session] != p ||
		w.gameplaySpaceForPlayer(p) != "" || w.areaTimerDeadline(p, now).After(now) {
		return
	}
	p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, 0))
}

func (w *World) clearColosseumCountdowns(now time.Time) {
	if w.colosseum == nil {
		return
	}
	for p := range w.colosseum.countdowns {
		w.clearColosseumCountdown(p, now)
	}
}
