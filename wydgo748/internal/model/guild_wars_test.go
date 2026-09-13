package model

import (
	"encoding/json"
	"math"
	"testing"
)

func TestCityRegistrationFameBidReplayAndRejections(t *testing.T) {
	s := CityWarState{Phase: "registration"}
	a, b, c := Guild{ID: 1, Fame: 300}, Guild{ID: 2, Fame: 300}, Guild{ID: 3, Fame: 400}
	for _, g := range []*Guild{&a, &b, &c} {
		if err := s.Register(g, 0); err != nil {
			t.Fatal(err)
		}
	}
	if s.Territories[0].Challenger != 3 || s.Territories[0].Bid != 400 || a.Fame != 200 || b.Fame != 200 || c.Fame != 300 {
		t.Fatal("incorrect bid/fee")
	}
	if err := s.Register(&c, 1); err == nil || c.Fame != 300 {
		t.Fatal("replay charged")
	}
	s.Territories[1].Owner = 4
	for _, tc := range []struct {
		g    Guild
		city int
	}{{Guild{ID: 4, Fame: 500}, 0}, {Guild{ID: 5, Fame: 99}, 0}, {Guild{ID: 5, Fame: 100}, 4}} {
		g := tc.g
		if err := s.Register(&g, tc.city); err == nil || g.Fame != tc.g.Fame {
			t.Fatal("invalid registration mutated fame")
		}
	}
	s.Phase = "waiting"
	g := Guild{ID: 6, Fame: 100}
	if err := s.Register(&g, 0); err == nil || g.Fame != 100 {
		t.Fatal("closed registration")
	}
	tie := CityWarState{Phase: "registration"}
	a.Fame, b.Fame = 100, 100
	_ = tie.Register(&a, 0)
	_ = tie.Register(&b, 0)
	if tie.Territories[0].Challenger != 1 {
		t.Fatal("tie replaced first challenger")
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
