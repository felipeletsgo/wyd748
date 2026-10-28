package game

import (
	"path/filepath"
	"testing"

	"wydgo/internal/data"
)

func TestGeneralGeneratorsExcludeRemovedAndUnscheduledEvents(t *testing.T) {
	general, err := data.LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	instancePositions := map[[2]uint16]string{}
	for _, file := range []string{"RuneTrack0.txt", "RuneTrack1.txt", "RuneTrack2.txt", "RuneTrack3.txt", "RuneTrack4.txt", "RuneTrack5.txt", "RuneTrack6.txt", "CubeN.txt", "CubeM.txt", "CubeA.txt"} {
		generators, err := data.LoadNPCGener(filepath.Join("../../data/spawns/instances", file))
		if err != nil {
			t.Fatal(err)
		}
		for _, g := range generators {
			instancePositions[[2]uint16{g.Segments[0].X, g.Segments[0].Y}] = file
		}
	}
	removed := map[string]bool{
		"Kerdan_Wanted": true, "Legend_Verid": true,
		"LichCruntKing": true, "Darksorcerer": true, "DarkTemplar": true,
		"DarkShaman": true, "DarkRanger": true, "Santa_Follower": true,
		"Inv_Gremlin": true, "Inv_Gremilin": true, "Inv_Lobo": true,
	}
	rvr := map[string]bool{
		"BlazingRanger": true, "BlazingShaman": true, "BlazingSorcerer": true, "BlazingTemplar": true,
		"ArqGelo": true, "MasterGelo": true, "MagoGelo": true, "TemplarioGelo": true,
	}
	for _, g := range general {
		for _, name := range []string{g.Leader, g.Follower} {
			if removed[name] || rvr[name] {
				t.Errorf("general population retains removed/event actor %q at label %d", name, g.Label)
			}
		}
		if file := instancePositions[[2]uint16{g.Segments[0].X, g.Segments[0].Y}]; file != "" {
			t.Errorf("general population duplicates %s label %d", file, g.Label)
		}
	}
	reserved, err := data.LoadNPCGener("../../data/spawns/events/RvR.txt")
	if err != nil {
		t.Fatal(err)
	}
	if len(reserved) != 16 {
		t.Fatalf("RvR source groups changed: got %d, want 16", len(reserved))
	}
	seen := map[string]bool{}
	for _, g := range reserved {
		for _, name := range []string{g.Leader, g.Follower} {
			if !rvr[name] {
				t.Errorf("unexpected RvR actor %q", name)
			}
			seen[name] = true
		}
	}
	for name := range rvr {
		if !seen[name] {
			t.Errorf("RvR source lost %q", name)
		}
	}
}

// Event-owned populations belong in their dedicated files or lifecycle, rather
// than the general population that every channel reads during startup.
func TestGeneralGeneratorsExcludeEventOwnedPopulations(t *testing.T) {
	fixtures, err := eventFixtures()
	if err != nil {
		t.Fatal(err)
	}
	w := newZoneTestWorld()
	w.volatiles = fixtures.volatiles
	for _, name := range []string{"NPCGener.txt"} {
		t.Run(name, func(t *testing.T) {
			geners, err := data.LoadChannelNPCGener(filepath.Join("..", "..", "data", "spawns", name))
			if err != nil {
				t.Fatal(err)
			}
			if len(geners) == 0 {
				t.Fatal("general population is empty")
			}
			for _, g := range geners {
				owner := ""
				switch {
				case w.generatorReservedForItemInstance(g):
					owner = "item_instance"
				case generatorReservedForColosseum(g):
					owner = "colosseum"
				case generatorReservedForGuildWar(g):
					owner = "guild_war"
				}
				if owner != "" {
					t.Errorf("reserved general generator file=%s index=%d label=%d owner=%s leader=%q", name, g.Index, g.Label, owner, g.Leader)
				}
			}
		})
	}
}
