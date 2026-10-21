package game

import (
	"encoding/binary"
	"errors"
	"path/filepath"
	"sync"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// eventFixtures loads the catalog, InitItem fixtures, terrain, volatiles and
// Colosseum generators once for every event test; spawnInitItems copies the
// height map, so worlds never share collision state.
var eventFixtures = sync.OnceValues(func() (*eventFixtureData, error) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		return nil, err
	}
	d := &eventFixtureData{items: catalog.Items}
	if d.initItems, err = data.LoadInitItems(filepath.Join(root, "init_items.csv"), catalog.Items); err != nil {
		return nil, err
	}
	if d.terrain, err = data.LoadTerrain(filepath.Join(root, "maps", "HeightMap.dat"), filepath.Join(root, "maps", "AttributeMap.dat")); err != nil {
		return nil, err
	}
	if d.volatiles, err = data.LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.json"), catalog.Items, catalog.Skills); err != nil {
		return nil, err
	}
	d.colosseum, err = data.LoadColosseum(root)
	return d, err
})

type eventFixtureData struct {
	items     map[uint16]model.ItemDef
	initItems []model.InitItem
	terrain   model.TerrainMap
	volatiles model.VolatileCatalog
	colosseum model.ColosseumGenerators
}

func colosseumWorld(t *testing.T, start time.Time) (*World, *fakeClock) {
	t.Helper()
	fixtures, err := eventFixtures()
	if err != nil {
		t.Fatal(err)
	}
	w := newZoneTestWorld()
	w.questZones = nil
	clock := newFakeClock(start)
	w.items, w.initItems, w.terrain, w.volatiles = fixtures.items, fixtures.initItems, fixtures.terrain, fixtures.volatiles
	w.clock, w.nextItemID = clock, 10000
	if err := w.spawnInitItems(); err != nil {
		t.Fatal(err)
	}
	w.warConfig.Timezone = "UTC"
	WithColosseum(model.DefaultColosseumConfig(), fixtures.colosseum)(w)
	templates := map[string]*model.NPCDef{}
	for _, gens := range fixtures.colosseum {
		for _, g := range gens {
			for _, name := range []string{g.Leader, g.Follower} {
				templates[name] = &model.NPCDef{Name: name, Score: &model.Score{MaxHP: 1000}}
			}
		}
	}
	if err := w.bindColosseumTemplates(templates); err != nil {
		t.Fatal(err)
	}
	return w, clock
}

// sentRemoveMob drains p's queue and reports a RemoveMob of id with the type.
func sentRemoveMob(p *Player, id uint16, removeType uint32) bool {
	found := false
	for {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok {
			return found
		}
		if wire.Decrypt(packet) && wire.ParseHeader(packet).Type == wire.OpRemoveMob &&
			wire.ParseHeader(packet).ID == id && binary.LittleEndian.Uint32(packet[12:16]) == removeType {
			found = true
		}
	}
}

func colosseumMobs(w *World) int {
	n := 0
	for _, m := range w.mobs {
		if m != nil && w.colosseumGenerator(m.generator) {
			n++
		}
	}
	return n
}

func colosseumFixturesState(t *testing.T, w *World, item uint16, want byte) {
	t.Helper()
	found := 0
	for _, g := range w.groundItems {
		if g.Permanent && g.Item.Index == item && colosseumFixtureArea.contains(g.X, g.Y) {
			found++
			if g.State != want {
				t.Fatalf("Colosseum fixture %d at (%d,%d) state=%d; want %d", item, g.X, g.Y, g.State, want)
			}
		}
	}
	if found == 0 {
		t.Fatalf("no Colosseum fixture %d", item)
	}
}

func runColosseumUntil(w *World, clock *fakeClock, minute int) {
	for clock.Now().Minute() != minute {
		clock.Advance(time.Second)
		w.tickColosseum(clock.Now())
	}
	// One transition runs per tick; let the minute's transition happen.
	clock.Advance(time.Second)
	w.tickColosseum(clock.Now())
}

// 7.59 TMSRV sub_567780: Colosseum N (EventHour+1) locks the windows and
// recalls non-mortals at minute 5, spawns waves from minute 6, opens the inner
// gates at minute 7 and empties and resets the arena at minute 17.
func TestColosseumNFollowsTheOfficialSchedule(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 21, 20, 4, 58, 0, time.UTC))
	mortal := addZonePlayer(w, 1, 2630, 1726, 500)
	arch := addZonePlayer(w, 2, 2630, 1720, 500)
	arch.Char.Evolution = archEvolution
	fallen := addZonePlayer(w, 3, 2632, 1730, 500)
	observer := addZonePlayer(w, 4, 2600, 1726, 500)
	observer.Session = net.NewTestSession(4, 1<<14) // room for every wave and removal
	w.players = map[*net.Session]*Player{mortal.Session: mortal, arch.Session: arch, fallen.Session: fallen, observer.Session: observer}
	runColosseumUntil(w, clock, 5)
	colosseumFixturesState(t, w, steelWindowItem, gateClosed)
	colosseumFixturesState(t, w, arenaGateItem, gateClosed)
	if colosseumArea.contains(arch.X, arch.Y) {
		t.Fatal("an Arch stayed inside Colosseum N")
	}
	if !colosseumArea.contains(mortal.X, mortal.Y) {
		t.Fatal("a mortal was removed from Colosseum N")
	}
	runColosseumUntil(w, clock, 6)
	if n := colosseumMobs(w); n == 0 {
		t.Fatal("wave 1 spawned no Colosseum mobs")
	}
	runColosseumUntil(w, clock, 7)
	colosseumFixturesState(t, w, arenaGateItem, gateOpen)
	// An Arch walking in while N runs is recalled by the movement filter.
	arch.X, arch.Y = 2630, 1720
	w.updatePlayerSpatial(arch)
	runColosseumUntil(w, clock, 8)
	if colosseumArea.contains(arch.X, arch.Y) {
		t.Fatal("the movement filter let an Arch stay inside Colosseum N")
	}
	runColosseumUntil(w, clock, 15)
	for _, gens := range w.colosseum.generators[model.ColosseumTierN] {
		if gens.current > gens.def.MaxNumMob {
			t.Fatalf("generator %d exceeded MaxNumMob", gens.def.Label)
		}
	}
	setPlayerCurHP(fallen.Char, 0)
	fallen.DeadAt = clock.Now()
	observer.Visible[fallen.ID] = struct{}{}
	runColosseumUntil(w, clock, 17)
	if playerCurHP(fallen.Char) != 1 || chebyshev(fallen.X, fallen.Y, colosseumExitX, colosseumExitY) > 3 {
		t.Fatalf("the fallen player was not revived and moved out: hp=%d (%d,%d)", playerCurHP(fallen.Char), fallen.X, fallen.Y)
	}
	if !sentRemoveMob(observer, fallen.ID, 3) {
		t.Fatal("observers kept the fallen player's corpse")
	}
	colosseumFixturesState(t, w, steelWindowItem, gateOpen)
	colosseumFixturesState(t, w, arenaGateItem, gateClosed)
	if n := colosseumMobs(w); n != 0 {
		t.Fatalf("%d Colosseum mobs survived the end of the event", n)
	}
	if colosseumExitArea.contains(mortal.X, mortal.Y) || chebyshev(mortal.X, mortal.Y, colosseumExitX, colosseumExitY) > 3 {
		t.Fatalf("the mortal was not moved out to (2615,1727): (%d,%d)", mortal.X, mortal.Y)
	}
	if w.colosseum.stage != 8 || w.colosseum.filter {
		t.Fatalf("stage=%d filter=%v after the event", w.colosseum.stage, w.colosseum.filter)
	}
	// The next hour (M) resets during minutes 0..4 and runs again.
	clock.Advance(44 * time.Minute)
	runColosseumUntil(w, clock, 5)
	if w.colosseum.stage != 1 || !w.colosseum.filter {
		t.Fatalf("Colosseum M did not start: stage=%d", w.colosseum.stage)
	}
}

func TestColosseumResetsBeforeTheNextDay(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 22, 20, 2, 0, 0, time.UTC))
	w.colosseum.stage = 8
	w.colosseum.filter = true
	w.colosseum.unpaid = map[*Player]bool{}

	w.tickColosseum(clock.Now())
	if w.colosseum.stage != 0 || w.colosseum.filter || w.colosseum.unpaid != nil {
		t.Fatalf("stage 8 was not reset in the next event window: stage=%d filter=%v unpaid=%v",
			w.colosseum.stage, w.colosseum.filter, w.colosseum.unpaid)
	}
	runColosseumUntil(w, clock, 5)
	if w.colosseum.stage != 1 {
		t.Fatalf("the next Colosseum event did not start: stage=%d", w.colosseum.stage)
	}
}

func TestColosseumArenaExitRollsBackRevivalWhenPersistenceFails(t *testing.T) {
	w := newZoneTestWorld()
	w.terrain = model.TerrainMap{
		Height:    make([]byte, model.TerrainCells),
		Attribute: make([]byte, model.AttributeCells),
	}
	w.store = &craftStore{err: errors.New("storage unavailable")}
	p := addZonePlayer(w, 1, 2630, 1726, 0)
	p.DeadAt = time.Now().Add(-time.Second)
	observer := addZonePlayer(w, 2, 2631, 1726, 500)
	w.showPlayerPair(observer, p)
	oldX, oldY, oldDeadAt := p.X, p.Y, p.DeadAt

	if w.moveOutOfArena(p, colosseumExitX, colosseumExitY) {
		t.Fatal("the arena exit succeeded despite failed persistence")
	}
	if playerCurHP(p.Char) != 0 || p.X != oldX || p.Y != oldY || !p.DeadAt.Equal(oldDeadAt) {
		t.Fatalf("failed exit changed player state: hp=%d pos=(%d,%d) deadAt=%v",
			playerCurHP(p.Char), p.X, p.Y, p.DeadAt)
	}
	if !observer.hasVisible(p.ID) || !p.hasVisible(observer.ID) {
		t.Fatal("failed exit left the corpse hidden from the observer")
	}
}

// Colosseum A (EventHour+3) consumes one Elixir_Notice per player inside and
// recalls players without it (sub_565F50); M recalls only Celestials.
func TestColosseumEntryRules(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 21, 22, 4, 59, 0, time.UTC))
	paid := addZonePlayer(w, 1, 2630, 1726, 500)
	paid.Char.Evolution = "celestial"
	paid.Char.Inv[4] = model.Item{Index: colosseumElixirItem}
	unpaid := addZonePlayer(w, 2, 2630, 1720, 500)
	runColosseumUntil(w, clock, 5)
	if paid.Char.Inv[4].Index != 0 || !colosseumArea.contains(paid.X, paid.Y) {
		t.Fatal("the paying player lost the entry or kept the elixir")
	}
	if colosseumArea.contains(unpaid.X, unpaid.Y) {
		t.Fatal("a player without an Elixir Notice stayed in Colosseum A")
	}

	// Only the 60 carry slots sub_565F50 scans count; a rejected charge is
	// retried while Colosseum A runs.
	w, clock = colosseumWorld(t, time.Date(2026, 10, 21, 22, 4, 59, 0, time.UTC))
	hidden := addZonePlayer(w, 1, 2630, 1726, 500)
	hidden.Char.Inv[60] = model.Item{Index: colosseumElixirItem}
	retried := addZonePlayer(w, 2, 2630, 1720, 500)
	retried.Char.Inv[3] = model.Item{Index: colosseumElixirItem}
	retried.Char.Inv[5] = model.Item{Index: colosseumElixirItem}
	runColosseumUntil(w, clock, 5)
	if colosseumArea.contains(hidden.X, hidden.Y) || hidden.Char.Inv[60].Index == 0 {
		t.Fatal("an elixir outside the 60 scanned slots paid the entry")
	}
	w.colosseum.unpaid[retried] = true // a rejected critical write marks the player unpaid
	clock.Advance(time.Second)
	w.tickColosseum(clock.Now())
	if retried.Char.Inv[5].Index != 0 || len(w.colosseum.unpaid) != 0 {
		t.Fatal("an unpaid player was not charged again")
	}

	w, clock = colosseumWorld(t, time.Date(2026, 10, 21, 21, 4, 59, 0, time.UTC))
	arch := addZonePlayer(w, 1, 2630, 1726, 500)
	arch.Char.Evolution = archEvolution
	celestial := addZonePlayer(w, 2, 2630, 1720, 500)
	celestial.Char.Evolution = "celestial"
	runColosseumUntil(w, clock, 5)
	if !colosseumArea.contains(arch.X, arch.Y) || colosseumArea.contains(celestial.X, celestial.Y) {
		t.Fatal("Colosseum M must keep Archs and recall Celestials")
	}
}

// A restart that misses minutes 5..6 skips the hour instead of firing every
// missed wave within seconds; the next hour runs normally.
func TestColosseumRestartAfterTheStartSkipsTheHour(t *testing.T) {
	for _, minute := range []int{7, 12, 30} {
		w, clock := colosseumWorld(t, time.Date(2026, 10, 21, 21, minute, 0, 0, time.UTC))
		runColosseumUntil(w, clock, minute+1)
		if w.colosseum.stage != 8 || colosseumMobs(w) != 0 {
			t.Fatalf("restart at :%02d replayed the event: stage=%d mobs=%d", minute, w.colosseum.stage, colosseumMobs(w))
		}
		colosseumFixturesState(t, w, steelWindowItem, gateOpen)
		clock.Advance(time.Duration(61-minute) * time.Minute) // minute 2 of the next hour
		w.tickColosseum(clock.Now())
		runColosseumUntil(w, clock, 5)
		if w.colosseum.stage != 1 {
			t.Fatalf("the next hour did not start after a skipped hour: stage=%d", w.colosseum.stage)
		}
	}
}

// Only the event populates the arena: base NPCGener entries with
// MinuteGenerate -1 inside it are reserved, other generators are not.
func TestColosseumReservesBaseArenaGenerators(t *testing.T) {
	geners, err := data.LoadNPCGener(filepath.Join("..", "..", "data", "NPCGener.txt"))
	if err != nil {
		t.Fatal(err)
	}
	var reserved []int
	for _, g := range geners {
		if generatorReservedForColosseum(g) {
			reserved = append(reserved, g.Label)
		}
	}
	if len(reserved) != 5 {
		t.Fatalf("reserved base Colosseum generators %v; want labels 0, 2, 5, 6, 7", reserved)
	}
}
