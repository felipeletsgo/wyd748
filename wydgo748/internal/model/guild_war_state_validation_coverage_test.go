package model

import (
	"testing"
	"time"
)

func TestGuildWarStateValidationRejectsInvalidBranches(t *testing.T) {
	now := time.Date(2026, 10, 18, 12, 0, 0, 0, time.UTC)
	registry := GuildRegistry{Guilds: []Guild{{ID: 1}, {ID: 2}}}
	tests := []struct {
		name  string
		edit  func(*GuildWarState)
		valid bool
	}{
		{name: "empty legacy state is valid", valid: true},
		{name: "valid populated state", valid: true, edit: func(s *GuildWarState) {
			s.Tower = TowerWarState{Day: "2026-10-18", StartsAt: now, EndsAt: now.Add(time.Hour), Owner: 1, Kingdom: 7, InitialKingdom: 8}
			s.FameResetDay = "2026-10-18"
			s.Cities = CityWarState{Phase: "waiting", Day: "2026-10-18", StartsAt: now,
				Territories: [4]CityTerritory{{Owner: 1, Challenger: 2, Victories: 4, Treasury: GuildWarTreasuryCap}},
				Entries:     []CityWarEntry{{Guild: 2, City: 0}}}
			s.Kefra = KefraState{Defeated: true, Guild: 1, DefeatedAt: now}
			s.Castle = CastleWarState{Day: "2026-10-18", StartsAt: now, Phase: CastlePreparing, Owner: 1}
		}},
		{name: "invalid castle calendar", edit: func(s *GuildWarState) { s.Castle.Day = "bad" }},
		{name: "castle phase without round", edit: func(s *GuildWarState) { s.Castle.Phase = CastlePreparing }},
		{name: "captured castle before finish", edit: func(s *GuildWarState) {
			s.Castle = CastleWarState{Day: "2026-10-18", StartsAt: now, Phase: CastlePreparing, Captured: true}
		}},
		{name: "invalid tower kingdom", edit: func(s *GuildWarState) { s.Tower.Kingdom = 6 }},
		{name: "invalid initial kingdom", edit: func(s *GuildWarState) { s.Tower.InitialKingdom = 9 }},
		{name: "invalid fame reset weekday", edit: func(s *GuildWarState) { s.FameResetDay = "2026-10-19" }},
		{name: "missing tower owner", edit: func(s *GuildWarState) { s.Tower.Owner = 9 }},
		{name: "missing castle owner", edit: func(s *GuildWarState) { s.Castle.Owner = 9 }},
		{name: "invalid tower day", edit: func(s *GuildWarState) { s.Tower.Day, s.Tower.EndsAt = "bad", now }},
		{name: "missing tower end", edit: func(s *GuildWarState) { s.Tower.Day = "2026-10-18" }},
		{name: "invalid tower window", edit: func(s *GuildWarState) {
			s.Tower.Day, s.Tower.StartsAt, s.Tower.EndsAt = "2026-10-18", now, now
		}},
		{name: "empty city phase carries data", edit: func(s *GuildWarState) { s.Cities.Day = "2026-10-18" }},
		{name: "invalid city phase", edit: func(s *GuildWarState) { s.Cities.Phase = "unknown" }},
		{name: "invalid city calendar", edit: func(s *GuildWarState) { s.Cities.Phase, s.Cities.Day = "waiting", "bad" }},
		{name: "city phase missing start", edit: func(s *GuildWarState) { s.Cities.Phase, s.Cities.Day = "battle", "2026-10-18" }},
		{name: "missing territory owner", edit: func(s *GuildWarState) { s.Cities.Territories[0].Owner = 9 }},
		{name: "missing territory challenger", edit: func(s *GuildWarState) { s.Cities.Territories[0].Challenger = 9 }},
		{name: "too many territory victories", edit: func(s *GuildWarState) { s.Cities.Territories[0].Victories = 5 }},
		{name: "guild challenges own territory", edit: func(s *GuildWarState) {
			s.Cities.Territories[0] = CityTerritory{Owner: 1, Challenger: 1}
		}},
		{name: "territory treasury over cap", edit: func(s *GuildWarState) {
			s.Cities.Territories[0].Treasury = GuildWarTreasuryCap + 1
		}},
		{name: "city entry has no guild", edit: func(s *GuildWarState) { s.Cities.Entries = []CityWarEntry{{City: 0}} }},
		{name: "city entry guild missing", edit: func(s *GuildWarState) { s.Cities.Entries = []CityWarEntry{{Guild: 9, City: 0}} }},
		{name: "city entry negative city", edit: func(s *GuildWarState) { s.Cities.Entries = []CityWarEntry{{Guild: 1, City: -1}} }},
		{name: "city entry city out of range", edit: func(s *GuildWarState) { s.Cities.Entries = []CityWarEntry{{Guild: 1, City: 4}} }},
		{name: "duplicate city entry", edit: func(s *GuildWarState) {
			s.Cities.Entries = []CityWarEntry{{Guild: 1, City: 0}, {Guild: 1, City: 0}}
		}},
		{name: "defeated kefra needs timestamp", edit: func(s *GuildWarState) { s.Kefra.Defeated = true }},
		{name: "living kefra cannot have defeat timestamp", edit: func(s *GuildWarState) { s.Kefra.DefeatedAt = now }},
		{name: "living kefra cannot have a guild", edit: func(s *GuildWarState) { s.Kefra.Guild = 1 }},
		{name: "missing defeated kefra guild", edit: func(s *GuildWarState) {
			s.Kefra = KefraState{Defeated: true, Guild: 9, DefeatedAt: now}
		}},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			state := GuildWarState{}
			if test.edit != nil {
				test.edit(&state)
			}
			err := state.Validate(&registry)
			if test.valid && err != nil {
				t.Fatalf("valid state rejected: %v", err)
			}
			if !test.valid && err == nil {
				t.Fatal("invalid state was accepted")
			}
		})
	}
}
