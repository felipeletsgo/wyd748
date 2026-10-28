package game

import (
	"time"

	"wydgo/internal/model"
)

// Guild-fame bidding is a project economy on the existing 7.48 command and
// persistence contracts (MODERNIZACAO_COMPATIVEL), not a native fee rule.
func (w *World) tickGuildWarEconomy(local time.Time) (bool, error) {
	proposed := w.criticalGuildSnapshot()
	c := &proposed.Wars.Cities
	migrated := !c.FameBids && len(c.Entries) != 0
	if migrated {
		phase := c.Phase
		if err := proposed.CancelCityBids(); err != nil {
			return false, err
		}
		if phase == "registration" {
			c.Phase = phase
		}
		c.FameBids = true
	}
	before := proposed.Wars.FameResetDay
	phaseBefore := proposed.Wars.Cities.Phase
	reset := proposed.ResetWeeklyGuildFame(local)
	if !reset && !migrated && before == proposed.Wars.FameResetDay && phaseBefore == proposed.Wars.Cities.Phase {
		return false, nil
	}
	publish := func() {
		if migrated {
			w.warNotice("Legacy city applications refunded. Submit an explicit guild-fame bid.")
		}
		if reset && before != "" {
			w.warNotice("Saturday bidding closed. Unspent guild fame reset; accepted city bids retained.")
		}
	}
	if w.criticalWrites != nil {
		err := w.queueScheduledGuildChange(func(registry *model.GuildRegistry) error {
			*registry = *proposed
			return nil
		}, publish)
		return true, err
	}
	if err := w.warCommit(func() error { *w.guilds = *proposed; return nil }); err != nil {
		return false, err
	}
	publish()
	return false, nil
}
