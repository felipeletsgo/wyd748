package game

import (
	"context"
	"path/filepath"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/data"
	"wydgo/internal/model"
)

func fixtureWorld(t *testing.T) (*World, *fakeClock) {
	t.Helper()
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "catalog", "itemlist.csv"),
		filepath.Join(root, "catalog", "Itemname.csv"), filepath.Join(root, "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	objects, err := data.LoadInitItems(filepath.Join(root, "world", "init_items.csv"), catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	terrain, err := data.LoadTerrain(filepath.Join(root, "world", "maps", "HeightMap.dat"), filepath.Join(root, "world", "maps", "AttributeMap.dat"))
	if err != nil {
		t.Fatal(err)
	}
	clock := newFakeClock(time.Date(2026, 10, 21, 12, 0, 0, 0, time.UTC))
	w := &World{groundItems: map[uint16]*GroundItem{}, items: catalog.Items, nextItemID: 10000,
		initItems: objects, terrain: terrain, clock: clock}
	if err := w.spawnInitItems(); err != nil {
		t.Fatal(err)
	}
	if err := w.spawnServerFixtures(); err != nil {
		t.Fatal(err)
	}
	return w, clock
}

func fixtureAt(w *World, index, x, y uint16) *GroundItem {
	for _, g := range w.groundItems {
		if g.Item.Index == index && g.X == x && g.Y == y {
			return g
		}
	}
	return nil
}

// Owner rule: a key-opened gate closes again ten seconds later, but never on
// a player standing in its collision mask.
func TestKeyOpenedGateRelocksAfterTenSeconds(t *testing.T) {
	w, clock := fixtureWorld(t)
	gate := fixtureAt(w, 458, 2075, 2015) // training field First_Gate (KEYID 2)
	if gate == nil || gate.State != gateClosed {
		t.Fatal("the training field gate is not locked at startup")
	}
	gate.State = gateOpen
	w.applyGroundItemHeight(gate)
	w.scheduleGateRelock(gate)
	clock.Advance(9 * time.Second)
	w.tickGateRelock(clock.Now())
	if gate.State != gateOpen {
		t.Fatal("the gate closed before ten seconds")
	}
	blocker := &Player{ID: 3, X: gate.X, Y: gate.Y, InWorld: true, Char: &model.Char{}}
	w.updatePlayerSpatial(blocker)
	clock.Advance(time.Second)
	w.tickGateRelock(clock.Now())
	if gate.State != gateOpen {
		t.Fatal("the gate closed on a player standing in it")
	}
	blocker.X, blocker.Y = gate.X+10, gate.Y+10
	w.updatePlayerSpatial(blocker)
	clock.Advance(time.Second)
	w.tickGateRelock(clock.Now())
	if gate.State != gateClosed {
		t.Fatal("the gate did not relock ten seconds after opening")
	}
	if h := w.terrain.Height[int(gate.Y)*model.TerrainWidth+int(gate.X)]; h != clientGateHeight {
		t.Fatalf("relocked gate left collision height %d", h)
	}
	if _, pending := w.gateRelockAt[gate.ID]; pending {
		t.Fatal("relock schedule was not cleared")
	}
	// Event and staff gates are never put on the key relock schedule.
	arena := fixtureAt(w, 471, 217, 215)
	w.scheduleGateRelock(arena)
	if _, pending := w.gateRelockAt[arena.ID]; pending {
		t.Fatal("an arena gate was scheduled for key relock")
	}
}

// City-war arena gates stay locked outside combat and open only while the
// battle runs (W2PP SetArenaDoor); the Colosseum windows start open and the
// Armia PvP door is open until staff locks it.
func TestArenaGatesFollowCityWarCombat(t *testing.T) {
	w, clock := fixtureWorld(t)
	gates := w.cityArenaGates()
	if len(gates) != 12 {
		t.Fatalf("found %d city-war arena gates; want 12", len(gates))
	}
	start := clock.Now()
	w.guilds = &model.GuildRegistry{}
	w.guilds.Wars.Cities.StartsAt = start
	w.guilds.Wars.Cities.Phase = "battle"
	w.cityWarRunning = true
	for _, at := range []struct {
		offset time.Duration
		open   bool
	}{{5 * time.Minute, false}, {6 * time.Minute, true}, {30 * time.Minute, true}, {31 * time.Minute, false}} {
		w.syncCityArenaGates(start.Add(at.offset))
		for _, g := range gates {
			if (g.State == gateOpen) != at.open {
				t.Fatalf("arena gate at (%d,%d) open=%v at +%v; want %v", g.X, g.Y, g.State == gateOpen, at.offset, at.open)
			}
		}
	}
	w.syncCityArenaGates(start.Add(10 * time.Minute))
	w.cityWarRunning = false
	w.syncCityArenaGates(start.Add(10 * time.Minute))
	for _, g := range gates {
		if g.State != gateClosed {
			t.Fatal("a stopped city war left an arena gate open")
		}
	}
	for _, xy := range [][2]uint16{{2603, 1717}, {2603, 1733}} {
		if window := fixtureAt(w, 472, xy[0], xy[1]); window == nil || window.State != gateOpen {
			t.Fatalf("Colosseum Steel_Window at %v is not open at startup", xy)
		}
	}
	door := w.pvpDoor()
	if door == nil || door.State != gateOpen || door.Rotate != 1 {
		t.Fatal("the Armia PvP door is missing or not open by default")
	}
	if changed, found := w.setPvPDoorLocked(true); !changed || !found || door.State != gateClosed {
		t.Fatal("staff could not lock the PvP door")
	}
	if changed, _ := w.setPvPDoorLocked(true); changed {
		t.Fatal("locking a locked PvP door reported a change")
	}
	if changed, _ := w.setPvPDoorLocked(false); !changed || door.State != gateOpen {
		t.Fatal("staff could not reopen the PvP door")
	}
	w.scheduleGateRelock(door)
	if _, pending := w.gateRelockAt[door.ID]; pending {
		t.Fatal("the PvP door was scheduled for key relock")
	}
}

func TestPvPDoorStaffCommand(t *testing.T) {
	w, _ := fixtureWorld(t)
	run := func(action string, allowed bool) control.PvPDoorResult {
		cmd := control.PvPDoorCommand{Version: 1, OperationID: "operation-" + action + "-000001", Action: action, Reason: "pvp event"}
		req := &pvpDoorRequest{ctx: context.Background(), actor: "operator", command: &cmd,
			authorize: func() bool { return allowed }, reply: make(chan control.PvPDoorResult, 1)}
		w.handlePvPDoor(req)
		return <-req.reply
	}
	if r := run("lock", false); r.Code != "capability_required" || r.Status.Locked {
		t.Fatalf("unauthorized lock = %+v", r)
	}
	if r := run("lock", true); r.Code != "ok" || !r.Status.Locked || !r.Status.Available {
		t.Fatalf("lock = %+v", r)
	}
	if r := run("lock", true); r.Code != "unchanged" || !r.Status.Locked {
		t.Fatalf("repeated lock = %+v", r)
	}
	if r := run("open", true); r.Code != "ok" || r.Status.Locked {
		t.Fatalf("open = %+v", r)
	}
}
