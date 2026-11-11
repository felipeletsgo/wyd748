package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestCriticalCityWarCancelAcknowledgementAndRejection(t *testing.T) {
	for _, reject := range []bool{false, true} {
		t.Run(map[bool]string{false: "commit", true: "rejection"}[reject], func(t *testing.T) {
			w, p, _, _, base, q, release := criticalTreasuryFixture(t, 0)
			w.guilds.FindByID(1).Fame = 100
			w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 2, Name: "Defenders", Fame: 200})
			w.guilds.Wars.Cities = model.CityWarState{
				Day: "2026-09-13", Phase: "battle", FameBids: true,
				Entries: []model.CityWarEntry{{Guild: 1, City: 0, Bid: 50}},
			}
			w.guilds.Wars.Cities.Territories[0] = model.CityTerritory{Owner: 2, Challenger: 1, Bid: 50}
			w.cityWarRunning = true
			w.cityFighters = map[*Player]cityFighter{p: {city: 0, guild: 1}}
			w.cityWarScores = [4]map[uint16]uint64{{1: 10}, {}, {}, {}}
			before := w.criticalGuildSnapshot()
			if reject {
				base.failure = criticalCertifiedRejection(t)
			}

			if err := w.queueCityWarCancel(); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, base.entered)
			city := proposal.guilds.Wars.Cities
			if city.Phase != "finished" || city.Territories[0].Owner != 2 ||
				city.Territories[0].Challenger != 0 || city.Territories[0].Bid != 0 ||
				len(city.Entries) != 0 || proposal.guilds.FindByID(1).Fame != 150 {
				t.Fatal("city-war cancellation proposal lost the owner, bid refund, or clearing semantics")
			}
			if w.cityWarRunning != true || len(w.cityFighters) != 1 ||
				w.guilds.Wars.Cities.Territories[0].Bid != 50 {
				t.Fatal("city-war cancellation became visible before durable acknowledgement")
			}

			release()
			w.applyCriticalCompletion(awaitCriticalResult(t, q))
			if reject {
				if w.shuttingDown || !w.cityWarRunning || len(w.cityFighters) != 1 ||
					w.guilds.Wars.Cities.Territories[0].Bid != 50 {
					t.Fatal("certified rejection changed or contained the city war")
				}
				return
			}
			if w.shuttingDown || w.cityWarRunning || w.cityFighters != nil || w.cityWarScores[0] != nil ||
				w.cityWarScores[1] != nil || w.cityWarScores[2] != nil || w.cityWarScores[3] != nil ||
				w.guilds.Wars.Cities.Phase != "finished" || w.guilds.Wars.Cities.Territories[0].Owner != 2 ||
				w.guilds.Wars.Cities.Territories[0].Bid != 0 || w.guilds.FindByID(1).Fame != before.FindByID(1).Fame+50 {
				t.Fatal("committed city-war cancellation did not publish its complete domain result")
			}
		})
	}
}

func TestCriticalTowerCaptureAcknowledgementAndRejection(t *testing.T) {
	for _, reject := range []bool{false, true} {
		t.Run(map[bool]string{false: "commit", true: "rejection"}[reject], func(t *testing.T) {
			w, p, _, _, base, q, release := criticalTreasuryFixture(t, 0)
			w.warConfig.Enabled = true
			w.guilds.FindByID(1).Kingdom = 7
			w.guilds.Guilds = append(w.guilds.Guilds, model.Guild{ID: 2, Name: "Defenders", Kingdom: 8})
			w.guilds.Wars.Tower = model.TowerWarState{
				Day: "2026-09-13", StartsAt: w.now().Add(-time.Minute), EndsAt: w.now().Add(time.Minute),
				Owner: 2, Kingdom: 8, InitialKingdom: 8, DefenderCaptures: 1,
			}
			p.Char.Citizenship = w.channel
			p.X, p.Y = towerWarSpawnX-1, towerWarSpawnY
			w.updatePlayerSpatial(p)
			mob := &Mob{
				ID: 990, GuildWarTower: true, HP: 1, TargetID: 44,
				Def: &model.NPCDef{Score: &model.Score{MaxHP: 9000}},
			}
			w.mobsByID[mob.ID] = mob
			if reject {
				base.failure = criticalCertifiedRejection(t)
			}

			if err := w.queueTowerCapture(p, mob, 345); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.guilds.Wars.Tower.Owner != 1 || proposal.guilds.Wars.Tower.Kingdom != 7 ||
				proposal.guilds.Wars.Tower.Captures != 1 || mob.HP != 1 || mob.TargetID != 44 {
				t.Fatal("tower capture escaped before acknowledgement or built an invalid guild proposal")
			}

			release()
			w.applyCriticalCompletion(awaitCriticalResult(t, q))
			if reject {
				if w.shuttingDown || w.guilds.Wars.Tower.Owner != 2 || w.guilds.Wars.Tower.Captures != 0 ||
					mob.HP != 345 || mob.TargetID != 44 {
					t.Fatal("certified rejection did not restore the tower's applied HP and retained defender")
				}
				return
			}
			if w.shuttingDown || w.guilds.Wars.Tower.Owner != 1 || w.guilds.Wars.Tower.Kingdom != 7 ||
				w.guilds.Wars.Tower.Captures != 1 || mob.HP != 9000 || mob.TargetID != 0 ||
				w.guilds.Wars.Tower.DefenderCaptures != 1 {
				t.Fatal("committed tower capture did not reset the tower and publish the new defender")
			}
		})
	}
}

func TestCriticalCityWarSchedulerAcknowledgesPhaseTransitions(t *testing.T) {
	tests := []struct {
		name  string
		phase string
		start time.Time
		at    time.Time
		want  string
	}{
		{
			name: "registration closes at midnight", phase: "registration",
			start: time.Date(2026, 9, 13, 20, 0, 0, 0, time.UTC),
			at:    time.Date(2026, 9, 13, 0, 0, 0, 0, time.UTC), want: "waiting",
		},
		{
			name: "waiting enters preparation", phase: "waiting",
			start: time.Date(2026, 9, 13, 12, 0, 0, 0, time.UTC),
			at:    time.Date(2026, 9, 13, 12, 3, 0, 0, time.UTC), want: "preparing",
		},
		{
			name: "preparation opens battle", phase: "preparing",
			start: time.Date(2026, 9, 13, 12, 0, 0, 0, time.UTC),
			at:    time.Date(2026, 9, 13, 12, 6, 0, 0, time.UTC), want: "battle",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			w, _, _, _, base, q, release := criticalTreasuryFixture(t, 0)
			w.guilds.Wars.Cities = model.CityWarState{
				Day: tt.start.Format("2006-01-02"), Phase: tt.phase, StartsAt: tt.start,
				Notices: 63, // These cases exercise the phase transition after all countdowns.
			}
			if err := w.tickCriticalCityWar(tt.at); err != nil {
				t.Fatal(err)
			}
			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.guilds.Wars.Cities.Phase != tt.want || w.guilds.Wars.Cities.Phase != tt.phase {
				t.Fatal("phase transition was not staged behind persistence acknowledgement")
			}
			release()
			w.applyCriticalCompletion(awaitCriticalResult(t, q))
			if w.shuttingDown || w.guilds.Wars.Cities.Phase != tt.want {
				t.Fatalf("phase after commit = %q, want %q", w.guilds.Wars.Cities.Phase, tt.want)
			}
			if tt.want == "preparing" && !w.cityWarRunning {
				t.Fatal("preparation acknowledgement did not start city-war admission")
			}
		})
	}
}

func TestCriticalCityWarSaturdayOpensNextRegistrationAfterAcknowledgement(t *testing.T) {
	w, _, _, _, base, q, release := criticalTreasuryFixture(t, 0)
	now := time.Date(2026, 9, 12, 12, 0, 0, 0, time.UTC)
	w.warConfig.CityHour = 20
	w.guilds.Wars.Cities = model.CityWarState{Territories: [4]model.CityTerritory{
		{Owner: 2, Challenger: 1, Bid: 75},
		{Owner: 3, Challenger: 1, Bid: 100},
	}}

	if err := w.tickCriticalCityWar(now); err != nil {
		t.Fatalf("Saturday registration transition failed: %v", err)
	}
	proposal := awaitCriticalWrite(t, base.entered)
	city := proposal.guilds.Wars.Cities
	if city.Day != "2026-09-13" || city.Phase != "registration" || !city.FameBids ||
		!city.StartsAt.Equal(time.Date(2026, 9, 13, 20, 0, 0, 0, time.UTC)) ||
		city.Territories[0] != (model.CityTerritory{Owner: 2}) ||
		city.Territories[1] != (model.CityTerritory{Owner: 3}) {
		t.Fatalf("Saturday proposal lost date, start time, or defender ownership: %+v", city)
	}
	if w.guilds.Wars.Cities.Day != "" || len(w.warNotices) != 0 {
		t.Fatal("Saturday registration became visible before durable acknowledgement")
	}

	release()
	w.applyCriticalCompletion(awaitCriticalResult(t, q))
	if w.shuttingDown || w.guilds.Wars.Cities.Day != city.Day || w.guilds.Wars.Cities.Phase != "registration" ||
		len(w.warNotices) != 1 {
		t.Fatal("committed Saturday registration did not publish the next schedule")
	}
}

func TestCriticalCityWarSchedulerHonorsBusyFenceAndMissedEndWindows(t *testing.T) {
	t.Run("reserved registry", func(t *testing.T) {
		w, _, _, _, base, q, release := criticalTreasuryFixture(t, 0)
		q.guilds = 1
		if err := w.tickCriticalCityWar(w.now()); err != errCriticalPersistenceBusy {
			t.Fatalf("reserved scheduler result = %v, want busy", err)
		}
		if len(base.entered) != 0 {
			t.Fatal("reserved scheduler queued another registry write")
		}
		release()
	})

	t.Run("late event cancellation", func(t *testing.T) {
		w, _, _, _, base, q, release := criticalTreasuryFixture(t, 0)
		now := w.now()
		w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", Phase: "battle",
			StartsAt: now.Add(-31 * time.Minute), Entries: []model.CityWarEntry{{Guild: 1, City: 0, Bid: 40}}}
		w.guilds.Wars.Cities.Territories[0] = model.CityTerritory{Owner: 2, Challenger: 1, Bid: 40}
		if err := w.tickCriticalCityWar(now); err != nil {
			t.Fatalf("late city-war cancellation failed: %v", err)
		}
		proposal := awaitCriticalWrite(t, base.entered)
		if proposal.guilds.Wars.Cities.Phase != "finished" || w.guilds.Wars.Cities.Phase != "battle" {
			t.Fatal("late city-war cancellation was not staged behind persistence")
		}
		release()
		w.applyCriticalCompletion(awaitCriticalResult(t, q))
		if w.shuttingDown || w.guilds.Wars.Cities.Phase != "finished" || w.guilds.Wars.Cities.Territories[0].Owner != 2 {
			t.Fatal("committed late cancellation lost the defender or finish state")
		}
	})

	t.Run("waiting phase missed combat opening", func(t *testing.T) {
		w, _, _, _, base, q, release := criticalTreasuryFixture(t, 0)
		now := w.now()
		w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", Phase: "waiting",
			StartsAt: now.Add(-6 * time.Minute), Notices: 63}
		if err := w.tickCriticalCityWar(now); err != nil {
			t.Fatalf("missed-opening cancellation failed: %v", err)
		}
		proposal := awaitCriticalWrite(t, base.entered)
		if proposal.guilds.Wars.Cities.Phase != "finished" || w.guilds.Wars.Cities.Phase != "waiting" {
			t.Fatal("missed city-war opening was not held behind persistence")
		}
		release()
		w.applyCriticalCompletion(awaitCriticalResult(t, q))
		if w.shuttingDown || w.guilds.Wars.Cities.Phase != "finished" {
			t.Fatal("missed city-war opening did not finish the event")
		}
	})
}

func TestCriticalCityWarBattleTransitionNotifiesFightersOfTheirEnemy(t *testing.T) {
	w, player, _, _, base, q, release := criticalTreasuryFixture(t, 0)
	now := w.now()
	w.guilds.Wars.Cities = model.CityWarState{Day: "2026-09-13", Phase: "preparing",
		StartsAt: now.Add(-6 * time.Minute), Notices: 63}
	w.guilds.Wars.Cities.Territories[0] = model.CityTerritory{Owner: 2, Challenger: 1}
	w.cityWarRunning = true
	w.cityFighters = map[*Player]cityFighter{player: {city: 0, guild: 1}}

	if err := w.tickCriticalCityWar(now); err != nil {
		t.Fatalf("battle transition failed: %v", err)
	}
	proposal := awaitCriticalWrite(t, base.entered)
	if proposal.guilds.Wars.Cities.Phase != "battle" || w.guilds.Wars.Cities.Phase != "preparing" {
		t.Fatal("battle phase became visible before durable acknowledgement")
	}
	release()
	w.applyCriticalCompletion(awaitCriticalResult(t, q))
	if w.shuttingDown || w.guilds.Wars.Cities.Phase != "battle" || len(w.warNotices) == 0 {
		t.Fatal("committed battle transition did not publish the phase and notice")
	}
	foundEnemy := false
	for {
		packet, ok := player.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid guild-war notification packet")
		}
		if wire.ParseHeader(packet).Type == wire.OpWarInfo && binary.LittleEndian.Uint32(packet[12:16]) == 2 {
			foundEnemy = true
		}
	}
	if !foundEnemy {
		t.Fatal("challenger did not receive the defending guild ID")
	}
}
