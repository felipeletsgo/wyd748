package data

import (
	"os"
	"path/filepath"
	"testing"
)

func TestLoadTeleports(t *testing.T) {
	path := filepath.Join(t.TempDir(), "teleports.ini")
	content := "# comment\n2118,2102,1044,1724,700,Armia to Noatum\n"
	if err := os.WriteFile(path, []byte(content), 0o600); err != nil {
		t.Fatal(err)
	}
	got, err := LoadTeleports(path)
	if err != nil {
		t.Fatal(err)
	}
	if len(got) != 1 || got[0].SourceX != 2118 || got[0].DestY != 1724 ||
		got[0].Price != 700 || got[0].Name != "Armia to Noatum" {
		t.Fatalf("wrong teleport: %+v", got)
	}
}

func TestLoadTeleportsRejectsInvalidCoordinate(t *testing.T) {
	path := filepath.Join(t.TempDir(), "teleports.ini")
	if err := os.WriteFile(path, []byte("5000,1,2,3,0,invalid\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadTeleports(path); err == nil {
		t.Fatal("an off-map coordinate was accepted")
	}
}

// Every client portal (TMGlobal.h g_TeleportTable) that the shipped table
// serves must resolve in its own 4x4 block; Armia Field used Y 2169 and failed.
func TestShippedTeleportsCoverClientPortalBlocks(t *testing.T) {
	routes, err := LoadTeleports(filepath.Join("..", "..", "data", "teleports.ini"))
	if err != nil {
		t.Fatal(err)
	}
	type block struct{ x, y uint16 }
	bySource := make(map[block]int, len(routes))
	for i, route := range routes {
		key := block{route.SourceX &^ 3, route.SourceY &^ 3}
		if previous, duplicate := bySource[key]; duplicate {
			t.Fatalf("routes %q and %q share source block %v", routes[previous].Name, route.Name, key)
		}
		bySource[key] = i
	}
	for portal, destination := range map[block]block{
		{2116, 2100}: {1044, 1724}, {2140, 2068}: {2589, 2097},
		{1044, 1724}: {2118, 2102}, {1044, 1716}: {2481, 1717}, {1044, 1708}: {2457, 2018},
		{2456, 2016}: {1044, 1709}, {2480, 1716}: {1044, 1717}, {2468, 1716}: {2248, 1558},
		{2548, 1740}: {2281, 3688},
		{2364, 2284}: {145, 3789}, {144, 3788}: {2367, 2284}, {2668, 2156}: {146, 3775},
		{144, 3772}: {2669, 2157}, {144, 3780}: {1006, 4030}, {148, 3780}: {1006, 4030},
		{1004, 4028}: {147, 3781}, {408, 4072}: {1006, 4066}, {1004, 4064}: {410, 4072},
		{744, 3820}: {1006, 3994}, {1004, 3992}: {747, 3817}, {680, 4076}: {917, 3822},
		{916, 3820}: {681, 4077}, {876, 3872}: {933, 3822}, {932, 3820}: {877, 3874},
	} {
		i, ok := bySource[portal]
		if !ok {
			t.Errorf("client portal %v has no server route", portal)
			continue
		}
		if got := (block{routes[i].DestX, routes[i].DestY}); got != destination {
			t.Errorf("portal %v leads to %v, want %v", portal, got, destination)
		}
	}
	// The server refuses a teleport whose destination is not walkable.
	terrain, err := LoadTerrain("../../data/maps/HeightMap.dat", "../../data/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	for _, route := range routes {
		if !terrain.Walkable(route.DestX, route.DestY) {
			t.Errorf("route %q lands on blocked cell (%d,%d)", route.Name, route.DestX, route.DestY)
		}
	}
}
