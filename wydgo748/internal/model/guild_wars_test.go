package model

import (
	"encoding/json"
	"math"
	"testing"
	"time"
)

func TestCityRegistrationFameBidReplayAndRejections(t *testing.T) {
	s := CityWarState{Phase: "registration"}
	a, b, c := Guild{ID: 1, Fame: 300}, Guild{ID: 2, Fame: 300}, Guild{ID: 3, Fame: 400}
	for _, g := range []*Guild{&a, &b, &c} {
		if err := s.Register(g, 0, 100); err != nil {
			t.Fatal(err)
		}
	}
	if s.Territories[0].Challenger != 1 || s.Territories[0].Bid != 100 || a.Fame != 200 || b.Fame != 200 || c.Fame != 300 {
		t.Fatal("incorrect bid/fee")
	}
	if err := s.Register(&c, 1, 200); err != nil || c.Fame != 100 {
		t.Fatal("replay charged")
	}
	s.Territories[1].Owner = 4
	for _, tc := range []struct {
		g    Guild
		city int
	}{{Guild{ID: 4, Fame: 500}, 1}, {Guild{ID: 5, Fame: 99}, 0}, {Guild{ID: 5, Fame: 100}, 4}} {
		g := tc.g
		if err := s.Register(&g, tc.city, 100); err == nil || g.Fame != tc.g.Fame {
			t.Fatal("invalid registration mutated fame")
		}
	}
	s.Phase = "waiting"
	g := Guild{ID: 6, Fame: 100}
	if err := s.Register(&g, 0, 100); err == nil || g.Fame != 100 {
		t.Fatal("closed registration")
	}
	tie := CityWarState{Phase: "registration"}
	a.Fame, b.Fame = 100, 100
	_ = tie.Register(&a, 0, 100)
	_ = tie.Register(&b, 0, 100)
	if tie.Territories[0].Challenger != 1 {
		t.Fatal("tie replaced first challenger")
	}
}

func TestCityBidIncreasesReplayMultipleCitiesAndZeroBalance(t *testing.T) {
	s := CityWarState{Phase: "registration"}
	g := Guild{ID: 1, Fame: 75}
	s.Territories[3].Owner = g.ID
	for _, bid := range []uint32{20, 20, 50} {
		if err := s.Register(&g, 0, bid); err != nil {
			t.Fatal(err)
		}
	}
	if err := s.Register(&g, 1, 25); err != nil {
		t.Fatal(err)
	}
	if g.Fame != 0 || len(s.Entries) != 2 || s.Entries[0].Bid != 50 {
		t.Fatal("incorrect cumulative spending")
	}
	if err := s.Register(&g, 0, 49); err == nil {
		t.Fatal("bid reduction accepted")
	}
	if err := s.Register(&g, 1, 26); err == nil {
		t.Fatal("overspend accepted")
	}
	if err := s.Register(&g, 2, 0); err != nil || g.Fame != 0 {
		t.Fatal("zero threshold not honored", err)
	}
}

func TestWeeklyGuildFameResetPreservesBidsAndDoesNotReplay(t *testing.T) {
	r := GuildRegistry{Guilds: []Guild{{ID: 1, Fame: 80}, {ID: 2, Fame: 20}}}
	loc, _ := time.LoadLocation("America/Sao_Paulo")
	saturday := time.Date(2026, 10, 24, 23, 59, 59, 0, loc)
	if !r.ResetWeeklyGuildFame(saturday) || r.Guilds[0].Fame != 80 {
		t.Fatal("migration baseline destroyed fame")
	}
	r.Wars.Cities = CityWarState{Phase: "registration", Day: "2026-10-25"}
	if err := r.Wars.Cities.Register(&r.Guilds[0], 0, 30); err != nil {
		t.Fatal(err)
	}
	if !r.ResetWeeklyGuildFame(saturday.Add(time.Second)) || r.Guilds[0].Fame != 0 || r.Guilds[1].Fame != 0 || r.Wars.Cities.Phase != "waiting" || r.Wars.Cities.Territories[0].Bid != 30 {
		t.Fatal("midnight reset lost bids or left balances")
	}
	r.Guilds[0].Fame = 10
	if r.ResetWeeklyGuildFame(saturday.Add(time.Hour)) || r.Guilds[0].Fame != 10 {
		t.Fatal("reset replay")
	}
	if err := r.CancelCityBids(); err != nil || r.Guilds[0].Fame != 10 {
		t.Fatal("cancel carried previous week's bid across reset", err)
	}
}

func TestGuildFameOverflowAndLegacyRegistry(t *testing.T) {
	g := Guild{Fame: math.MaxUint32 - 99}
	if err := g.AddFame(100); err == nil || g.Fame != math.MaxUint32-99 {
		t.Fatal("overflow not atomic")
	}
	var r GuildRegistry
	if err := json.Unmarshal([]byte(`{"version":1,"guilds":[]}`), &r); err != nil {
		t.Fatal(err)
	}
	if err := r.Wars.Validate(&r); err != nil {
		t.Fatal(err)
	}
	if err := DefaultGuildWarConfig().Validate(); err != nil {
		t.Fatal(err)
	}
}

func TestCityConquestReferenceFameAndDefenderRules(t *testing.T) {
	for _, tc := range []struct {
		name                                 string
		owner, attack                        uint16
		defenseScore, attackScore            uint64
		defenseFame, wantDefense, wantAttack uint32
		wantOwner                            uint16
		wantDefenses                         byte
	}{
		{"conquest", 1, 2, 100, 101, 100, 50, 250, 2, 0},
		{"loss floor", 1, 2, 100, 101, 20, 0, 250, 2, 0},
		{"empty city", 0, 2, 0, 1, 100, 100, 250, 2, 0},
		{"tie", 1, 2, 100, 100, 100, 100, 200, 1, 3},
		{"defense", 1, 2, 101, 100, 100, 100, 200, 1, 3},
		{"no challenger", 1, 0, 0, 0, 100, 100, 200, 1, 3},
	} {
		t.Run(tc.name, func(t *testing.T) {
			r := GuildRegistry{Guilds: []Guild{{ID: 1, Fame: tc.defenseFame}, {ID: 2, Fame: 200}}}
			r.Wars.Cities.Phase = "battle"
			r.Wars.Cities.Territories[0] = CityTerritory{Owner: tc.owner, Challenger: tc.attack, Victories: 2, Bid: 200, Treasury: 900}
			var scores [4]map[uint16]uint64
			scores[0] = map[uint16]uint64{1: tc.defenseScore, 2: tc.attackScore}
			if err := r.ResolveCityWars(scores); err != nil {
				t.Fatal(err)
			}
			got := r.Wars.Cities.Territories[0]
			if got.Owner != tc.wantOwner || got.Victories != tc.wantDefenses || got.Challenger != 0 || got.Bid != 0 || got.Treasury != 900 ||
				r.FindByID(1).Fame != tc.wantDefense || r.FindByID(2).Fame != tc.wantAttack {
				t.Fatalf("incorrect result: territory=%+v guilds=%+v", got, r.Guilds)
			}
			if err := r.ResolveCityWars(scores); err != nil || r.FindByID(2).Fame != tc.wantAttack || r.Wars.Cities.Territories[0] != got {
				t.Fatal("settled city war was replayed")
			}
		})
	}
}

func TestCastleCityOwnerReconciliationReferenceOrder(t *testing.T) {
	for _, tc := range []struct {
		name   string
		owner  uint16
		cities [4]uint16
		want   uint16
	}{
		{"retained in another city", 2, [4]uint16{1, 2, 1, 1}, 2},
		{"lost final city", 3, [4]uint16{2, 1, 1, 1}, 2},
		{"first nonempty entry", 3, [4]uint16{0, 2, 1, 1}, 2},
		{"initial fully owned cities", 0, [4]uint16{2, 1, 1, 1}, 2},
		{"native zero match", 0, [4]uint16{2, 0, 1, 1}, 0},
		{"no city owner", 3, [4]uint16{}, 3},
	} {
		t.Run(tc.name, func(t *testing.T) {
			s := GuildWarState{Castle: CastleWarState{Owner: tc.owner}}
			for i, owner := range tc.cities {
				s.Cities.Territories[i].Owner = owner
			}
			s.ReconcileCastleOwner()
			if s.Castle.Owner != tc.want {
				t.Fatalf("castle owner = %d, want %d", s.Castle.Owner, tc.want)
			}
		})
	}
}

func TestCastleOwnershipPersistenceAndValidation(t *testing.T) {
	r := GuildRegistry{Version: GuildRegistryVersion, Guilds: []Guild{{ID: 1}}}
	r.Wars.Castle.Owner = 1
	b, err := json.Marshal(r)
	if err != nil {
		t.Fatal(err)
	}
	var loaded GuildRegistry
	if err := json.Unmarshal(b, &loaded); err != nil || loaded.Wars.Castle.Owner != 1 {
		t.Fatalf("castle owner was not restored: %v", err)
	}
	if err := loaded.Wars.Validate(&loaded); err != nil {
		t.Fatal(err)
	}
	loaded.Wars.Castle.Owner = 2
	if loaded.Wars.Validate(&loaded) == nil {
		t.Fatal("dangling castle owner was accepted")
	}
}
