package game

import (
	"errors"
	"fmt"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestScheduledGuildChangeAcrossChannels(t *testing.T) {
	for _, channel := range []byte{1, 2} {
		for _, outcome := range []string{"commit", "rejection", "identity-change"} {
			t.Run(fmt.Sprintf("channel-%d/%s", channel, outcome), func(t *testing.T) {
				w, _, _, _, base, q, release := criticalTreasuryFixture(t, 0)
				w.channel = channel
				other := newZoneTestWorld()
				other.store, other.channel = w.store, 3-channel
				cluster, err := NewChannelCluster(w.store)
				if err != nil {
					t.Fatal(err)
				}
				cluster.owner = w.criticalWrites
				if err := cluster.attach(w); err != nil {
					t.Fatal(err)
				}
				if err := cluster.attach(other); err != nil {
					t.Fatal(err)
				}
				other.criticalWrites = cluster.owner
				cluster.enter(w)
				before := w.criticalGuildSnapshot()
				published := false
				if outcome == "rejection" {
					base.failure = criticalCertifiedRejection(t)
				}
				change := func(registry *model.GuildRegistry) error {
					registry.Wars.Cities.Notices |= 4
					return nil
				}
				if err := w.queueScheduledGuildChange(change, func() { published = true }); err != nil {
					t.Fatal(err)
				}
				proposal := awaitCriticalWrite(t, base.entered)
				if proposal.guilds.ChannelWars[channel].Cities.Notices != 4 {
					t.Fatal("worker did not receive the canonical channel proposal")
				}
				cluster.enter(other)
				cluster.enter(w)
				if published || !reflect.DeepEqual(w.criticalGuildSnapshot(), before) {
					t.Fatal("scheduled change escaped before acknowledgement")
				}
				if err := w.queueScheduledGuildChange(change, nil); !errors.Is(err, errCriticalPersistenceBusy) {
					t.Fatalf("shared reservation was lost: %v", err)
				}
				if outcome == "identity-change" {
					w.guilds.Wars.Cities.Phase = "unexpected"
				}
				release()
				awaitReadyCriticalResults(t, q, 1)
				cluster.enter(other)
				other.processCriticalCompletions(1)
				if outcome == "identity-change" {
					if !w.shuttingDown || !other.shuttingDown || published {
						t.Fatal("real identity divergence did not contain both channels")
					}
					return
				}
				if w.shuttingDown || other.shuttingDown || len(q.pending) != 0 {
					t.Fatal("scheduled change failed to settle across channel rotation")
				}
				cluster.enter(w)
				if outcome == "rejection" {
					if published || !reflect.DeepEqual(w.criticalGuildSnapshot(), before) {
						t.Fatal("certified rejection changed or published guild state")
					}
				} else if !published || w.guilds.Wars.Cities.Notices != 4 ||
					!reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) {
					t.Fatal("committed guild state and publication differ from the durable proposal")
				}
				cluster.enter(other)
				if w.guilds.Wars.Cities.Notices != before.ChannelWars[other.channel].Cities.Notices {
					t.Fatal("scheduled change leaked into the other channel")
				}
			})
		}
	}
}

func TestScheduledGuildChannelIncidentProducers(t *testing.T) {
	local := time.FixedZone("BRT", -3*60*60)
	for _, channel := range []byte{1, 2} {
		for _, producer := range []string{"five-minute-warning", "start-notice", "weekly-reset"} {
			t.Run(fmt.Sprintf("channel-%d/%s", channel, producer), func(t *testing.T) {
				w, _, _, _, base, q, release := criticalTreasuryFixture(t, 0)
				w.guilds.SelectWarChannel(channel)
				start := time.Date(2026, 10, 25, 20, 0, 0, 0, local)
				w.guilds.Wars.Cities = model.CityWarState{Day: "2026-10-25", Phase: "waiting", FameBids: true, StartsAt: start}
				expectedNotices := uint16(1)
				var err error
				if producer == "weekly-reset" {
					w.guilds.FameResetDay, w.guilds.Wars.FameResetDay = "2026-10-18", "2026-10-18"
					var queued bool
					queued, err = w.tickGuildWarEconomy(start)
					if !queued {
						t.Fatal("weekly reset did not enqueue persistence")
					}
				} else if producer == "start-notice" {
					w.guilds.Wars.Cities.Notices = 1
					expectedNotices = 5
					err = w.tickCriticalCityWar(start.Add(15*time.Minute + 24*time.Second))
				} else {
					err = w.tickCriticalCityWar(start.Add(-5*time.Minute + time.Second))
				}
				if err != nil {
					t.Fatal(err)
				}
				proposal := awaitCriticalWrite(t, base.entered)
				release()
				w.applyCriticalCompletion(awaitCriticalResult(t, q))
				if w.shuttingDown || !reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) {
					t.Fatal("incident producer entered maintenance after its own committed change")
				}
				if producer == "weekly-reset" {
					if w.guilds.FameResetDay != "2026-10-25" {
						t.Fatal("weekly reset date was not committed")
					}
				} else if w.guilds.Wars.Cities.Notices != expectedNotices {
					t.Fatal("city scheduler did not commit the expected announcement")
				}
			})
		}
	}
}
