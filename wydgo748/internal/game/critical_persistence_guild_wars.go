package game

import (
	"fmt"
	"log"
	"reflect"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// Calendar transitions have no account participant. The global registry fence
// serializes them with economic writes; no notices or arena effects precede ack.
func (w *World) queueScheduledGuildChange(change func(*model.GuildRegistry) error, publish func(), hooks ...criticalGuildChangeHooks) error {
	if w.criticalWrites == nil || w.guilds == nil || change == nil {
		return errCriticalPersistenceIdentity
	}
	if w.shuttingDown {
		return errCriticalPersistenceStopping
	}
	if w.criticalGuildReserved() {
		return errCriticalPersistenceBusy
	}
	registry, retained, proposed := w.guilds, w.criticalGuildSnapshot(), w.criticalGuildSnapshot()
	if err := change(proposed); err != nil {
		return err
	}
	// Calendar changes edit the active Wars view. Match the worker's detached
	// snapshot, including ChannelWars, before retaining the post-ack identity.
	proposed = proposed.Clone()
	check := func() error {
		if w.shutdownErr != nil {
			return fmt.Errorf("critical persistence: maintenance prevents guild publication")
		}
		if w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) {
			return errCriticalPersistenceIdentity
		}
		for _, hook := range hooks {
			if hook.check != nil {
				if err := hook.check(); err != nil {
					return err
				}
			}
		}
		return nil
	}
	if err := check(); err != nil {
		return err
	}
	_, err := w.criticalWrites.submitWithRejection(criticalWrite{guilds: proposed},
		func(result criticalPersistenceResult) error {
			if result.err != nil {
				return result.err
			}
			if err := check(); err != nil {
				return err
			}
			*w.guilds = *proposed
			return nil
		}, func(criticalPersistenceResult) error {
			if w.shutdownErr != nil || w.guilds != registry || !reflect.DeepEqual(w.criticalGuildSnapshot(), proposed) {
				return errCriticalPersistenceIdentity
			}
			if publish != nil {
				publish()
			}
			return nil
		}, func(result criticalPersistenceResult) error {
			if err := check(); err != nil {
				return err
			}
			log.Printf("guild war persistence rejected: %v", result.err)
			for _, hook := range hooks {
				if hook.reject != nil {
					hook.reject()
				}
			}
			return nil
		})
	return err
}

func (w *World) queueTowerCapture(p *Player, m *Mob, applied uint32) error {
	if p == nil || p.Account == nil || m == nil || m.Def == nil || m.Def.Score == nil ||
		w.mobsByID[m.ID] != m || !w.towerTargetAllowed(p, m) {
		return errCriticalPersistenceIdentity
	}
	guild, _ := w.guildOf(p.Char)
	if guild == nil {
		return errCriticalPersistenceIdentity
	}
	owner, guildName, characterName := guild.ID, guild.Name, p.Char.Name
	proposed := w.criticalGuildSnapshot()
	if err := proposed.CaptureTower(owner); err != nil {
		return err
	}
	w.addTowerMedals(proposed, "capture")
	retained := *m
	publishHP := func() {
		w.sendToMobViewProtocol(m, func(_ *Player) []byte {
			return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP, 0, 0)
		})
	}
	return w.queueGuildAccountChange(p, []*Player{p}, proposed, nil,
		"Save failed. The tower was not captured.", func() {
			m.HP, m.Affects, m.TargetID = m.Def.Score.MaxHP, [16]model.Affect{}, 0
		}, func() {
			w.warNotice(fmt.Sprintf("%s of guild %s brought the tower down! New defender: %s.", characterName, guildName, guildName))
			w.ejectTowerOpponents(w.guilds.TowerKingdom())
			publishHP()
		}, criticalGuildChangeHooks{
			check: func() error {
				if w.mobsByID[m.ID] != m || *m != retained {
					return errCriticalPersistenceIdentity
				}
				return nil
			},
			reject: func() {
				m.HP = minU32(m.Def.Score.MaxHP, max(1, applied))
				publishHP()
			},
		})
}

// Keep the boss indexed until the defeat is durable. No death, removal, guard
// cleanup or generator decrement may escape an unacknowledged transaction.
func (w *World) queueKefraDeathWithoutPlayer(m *Mob) error {
	if !isKefraBoss(m) || w.guilds == nil || w.mobsByID[m.ID] != m || m.Dead {
		return errCriticalPersistenceIdentity
	}
	retained, defeatedAt := *m, w.now()
	return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
		guilds.Wars.Kefra = model.KefraState{Defeated: true, DefeatedAt: defeatedAt}
		return nil
	}, func() {
		w.killMobWithoutPlayer(m)
		w.announceKefraDefeat(nil, nil)
	}, criticalGuildChangeHooks{
		check: func() error {
			if w.mobsByID[m.ID] != m || *m != retained {
				return errCriticalPersistenceIdentity
			}
			return nil
		},
		reject: func() {
			m.HP = max(1, retained.HP)
			if m.Def.Score != nil {
				w.sendToMobViewProtocol(m, func(_ *Player) []byte {
					return wire.MobHpMp(m.ID, m.HP, m.Def.Score.MaxHP, m.Def.Score.MaxMP, m.Def.Score.MaxMP)
				})
			}
		},
	})
}

func (w *World) queueTowerWarFinish(_ uint16) error {
	if w.guilds.Wars.Tower.Finished {
		return nil
	}
	return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
		if err := guilds.ResolveTowerWar(); err != nil {
			return err
		}
		w.addTowerMedals(guilds, "finish")
		return nil
	}, w.publishTowerFinish)
}

func (w *World) tickCriticalTowerWar(now time.Time) error {
	if w.criticalGuildReserved() {
		return errCriticalPersistenceBusy
	}
	s := &w.guilds.Wars.Tower
	if s.Day != "" && !s.Finished && !now.Before(s.EndsAt) {
		return w.queueTowerWarFinish(s.Owner)
	}
	day := now.Format("2006-01-02")
	start := time.Date(now.Year(), now.Month(), now.Day(), int(w.warConfig.TowerHour), 0, 0, 0, now.Location())
	if now.Weekday() >= time.Monday && now.Weekday() <= time.Friday &&
		!now.Before(start.Add(-5*time.Minute)) && now.Before(start.Add(time.Hour)) && s.Day != day {
		return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
			startTowerRound(guilds, day, start)
			return nil
		}, nil)
	}
	if s.Day == day && !s.Finished {
		battle := s.EndsAt.Add(-time.Hour)
		for _, warning := range []struct {
			end     time.Time
			bit     uint16
			minutes int
			text    string
		}{
			{battle, 1, 5, "Tower war starts within 5 minutes! Enter with /tower in Erion."},
			{battle, 2, 1, "Tower war starts in 1 minute!"},
			{s.EndsAt, 4, 5, "Tower war ends in 5 minutes! Guild fame is awarded only on a kingdom change."},
			{s.EndsAt, 8, 1, "Tower war ends in 1 minute!"},
		} {
			if err := w.warCountdown(now, warning.end, &s.Notices, warning.bit, warning.minutes, warning.text); err != nil {
				return err
			}
			if w.criticalGuildReserved() {
				return nil
			}
		}
	}
	if w.towerBattleActive(now) && !s.Started {
		return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
			guilds.Wars.Tower.Started = true
			return nil
		}, func() {
			// Spawn failure is a confirmed post-ack failure, never a rollback of
			// the committed calendar. Contain it through the owner continuation.
			if w.warTower() == nil {
				if err := w.spawnWarTower(); err != nil {
					panic(fmt.Sprintf("committed tower start cannot spawn: %v", err))
				}
			}
			w.ejectTowerOpponents(w.guilds.TowerKingdom())
			w.warNotice("Tower war started! Use /tower in Erion. Duration: 60 minutes.")
		})
	}
	if w.towerBattleActive(now) && w.warTower() == nil {
		return w.spawnWarTower()
	}
	return nil
}

func (w *World) queueCityWarCancel() error {
	return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
		return guilds.CancelCityBids()
	}, func() {
		w.cityWarRunning = false
		for p := range w.cityFighters {
			p.Session.Send(wire.WarInfo())
		}
		w.cityFighters = nil
		w.cityWarScores = [4]map[uint16]uint64{}
		w.warNotice("City war interrupted; owners kept. Only bids from the current fame week can be refunded.")
	})
}

func (w *World) tickCriticalCityWar(now time.Time) error {
	if w.criticalGuildReserved() {
		return errCriticalPersistenceBusy
	}
	c := &w.guilds.Wars.Cities
	if c.Day != "" && c.Phase != "finished" && !now.Before(c.StartsAt.Add(31*time.Minute)) {
		if !w.cityWarRunning {
			return w.queueCityWarCancel()
		}
		return w.finishCityWar()
	}
	if now.Weekday() == time.Saturday {
		sunday := now.AddDate(0, 0, 1)
		day := sunday.Format("2006-01-02")
		if c.Day != day {
			return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
				territories := guilds.Wars.Cities.Territories
				for i := range territories {
					territories[i].Challenger, territories[i].Bid = 0, 0
				}
				guilds.Wars.Cities = model.CityWarState{Day: day, Phase: "registration", FameBids: true, Territories: territories,
					StartsAt: time.Date(sunday.Year(), sunday.Month(), sunday.Day(), int(w.warConfig.CityHour), 0, 0, 0, now.Location())}
				return nil
			}, func() {
				w.warNotice("Saturday bids open until 23:59:59! /citywar <1..4> <total guild fame>. No entry fee.")
			})
		}
	}
	if c.Day == "" || c.Phase == "finished" {
		return nil
	}
	localStart := c.StartsAt.In(now.Location())
	registrationEnd := time.Date(localStart.Year(), localStart.Month(), localStart.Day(), 0, 0, 0, 0, now.Location())
	if c.Phase == "registration" && !now.Before(registrationEnd) {
		return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
			guilds.Wars.Cities.Phase = "waiting"
			return nil
		}, func() { w.warNotice("City war applications closed. Check owners and challengers with /citywar.") })
	}
	for _, warning := range []struct {
		end     time.Time
		bit     uint16
		minutes int
		text    string
	}{
		{c.StartsAt, 1, 5, "City war starts in 5 minutes! Gather in the contested city; summons 3 minutes after the start."},
		{c.StartsAt, 2, 1, "City war starts in 1 minute! Limit: 26 per guild. Erion: mortals only."},
		{c.StartsAt.Add(6 * time.Minute), 8, 1, "City war combat opens in 1 minute!"},
		{c.StartsAt.Add(31 * time.Minute), 16, 5, "City wars end in 5 minutes! The surviving forces win."},
		{c.StartsAt.Add(31 * time.Minute), 32, 1, "City wars end in 1 minute!"},
	} {
		if err := w.warCountdown(now, warning.end, &c.Notices, warning.bit, warning.minutes, warning.text); err != nil {
			return err
		}
		if w.criticalGuildReserved() {
			return nil
		}
	}
	if !now.Before(c.StartsAt) && c.Notices&4 == 0 {
		return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
			guilds.Wars.Cities.Notices |= 4
			return nil
		}, func() { w.warNotice("City war started! Summons at +3 minutes; combat at +6; result at +31.") })
	}
	if c.Phase == "waiting" && !now.Before(c.StartsAt.Add(3*time.Minute)) {
		if !now.Before(c.StartsAt.Add(6 * time.Minute)) {
			return w.queueCityWarCancel()
		}
		return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
			guilds.Wars.Cities.Phase = "preparing"
			return nil
		}, func() {
			w.cityWarRunning = true
			w.summonCityFighters()
			w.warNotice("Guilds summoned! 3 minutes of preparation; combat still locked.")
		})
	}
	if c.Phase == "preparing" && !now.Before(c.StartsAt.Add(6*time.Minute)) {
		return w.queueScheduledGuildChange(func(guilds *model.GuildRegistry) error {
			guilds.Wars.Cities.Phase = "battle"
			return nil
		}, func() {
			w.warNotice("Combat open in the four cities! Duration: 25 minutes; a tie favors the defender.")
			for p, fighter := range w.cityFighters {
				territory := w.guilds.Wars.Cities.Territories[fighter.city]
				enemy := territory.Challenger
				if fighter.guild == territory.Challenger {
					enemy = territory.Owner
				}
				p.Session.Send(wire.GuildWarInfo(enemy))
			}
		})
	}
	return nil
}
