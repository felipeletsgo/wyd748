package game

import (
	"fmt"
	"log"
	"time"

	"wydgo/internal/model"
)

// World fixture controllers (InitItem gates). W2PP ProcessMinTimer and the
// 7.59 TMSRV relock an opened keyed gate shortly after it opens; the owner
// fixed the open window at ten seconds. City-war arena gates follow W2PP
// SetArenaDoor: locked outside combat and open while the battle runs.
const (
	keyedGateOpenDuration = 10 * time.Second
	gateRelockRetry       = time.Second
	arenaGateItem         = 471
	pvpDoorItem           = 4143
	pvpDoorX, pvpDoorY    = 2123, 2143
)

// eventGateKeyID marks the arena, Colosseum and OX gates: no key item exists
// for it, so only events and staff drive those gates.
const eventGateKeyID = 9

func playerGateKeyID(keyID int) bool {
	return keyID > 0 && keyID < 15 && keyID != eventGateKeyID
}

// keyedGate reports a gate that a key item opens (EF_KEYID 1..14 except 9).
func (w *World) keyedGate(g *GroundItem) bool {
	if g == nil || !g.Permanent {
		return false
	}
	def, ok := w.items[g.Item.Index]
	key := staticEffect(def, "EF_KEYID")
	return ok && staticEffect(def, "EF_GROUND") > 0 && playerGateKeyID(key)
}

// setFixtureState changes a permanent fixture, its collision and every
// viewer's copy. It never touches a gate whose key write is still pending.
func (w *World) setFixtureState(g *GroundItem, state byte) bool {
	if g == nil || !g.Permanent || g.State == state || g.criticalGate || w.groundItems[g.ID] != g {
		return false
	}
	g.State = state
	w.applyGroundItemHeight(g)
	for _, observer := range w.nearbyWorldPlayers(g.X, g.Y, viewHalfX) {
		if observer.hasVisible(g.ID) && observer.Session != nil && !observer.Session.IsClosed() {
			observer.Session.Send(groundItemUpdatePacket(g))
		}
	}
	return true
}

// scheduleGateRelock closes a key-opened gate after the open window.
func (w *World) scheduleGateRelock(g *GroundItem) {
	if !w.keyedGate(g) {
		return
	}
	if w.gateRelockAt == nil {
		w.gateRelockAt = make(map[uint16]time.Time)
	}
	w.gateRelockAt[g.ID] = w.now().Add(keyedGateOpenDuration)
}

// gateOccupied reports a player standing on a cell the closed gate would
// block, so relocking never traps anyone inside the collision mask.
func (w *World) gateOccupied(g *GroundItem) bool {
	def, ok := w.items[g.Item.Index]
	mask := staticEffect(def, "EF_GROUND")
	if !ok || mask <= 0 || mask >= len(groundMaskBits) || g.Rotate >= 4 {
		return false
	}
	bits := groundMaskBits[mask][g.Rotate]
	for _, p := range w.nearbyWorldPlayers(g.X, g.Y, 3) {
		dx, dy := int(p.X)-int(g.X)+2, int(p.Y)-int(g.Y)+2
		if dx >= 0 && dx < 6 && dy >= 0 && dy < 6 && bits&(uint64(1)<<(dy*6+dx)) != 0 {
			return true
		}
	}
	return false
}

func (w *World) tickGateRelock(now time.Time) {
	for id, at := range w.gateRelockAt {
		if now.Before(at) {
			continue
		}
		g := w.groundItems[id]
		if g == nil || g.State != gateOpen || !w.keyedGate(g) {
			delete(w.gateRelockAt, id)
			continue
		}
		if g.criticalGate || w.gateOccupied(g) {
			w.gateRelockAt[id] = now.Add(gateRelockRetry)
			continue
		}
		if w.setFixtureState(g, gateClosed) {
			log.Printf("gate %d (item %d) relocked", g.ID, g.Item.Index)
		}
		delete(w.gateRelockAt, id)
	}
}

// cityArenaGates returns the arena gates inside each city's war arena, in
// city order (W2PP SetArenaDoor gates 3*city+1..3*city+3).
func (w *World) cityArenaGates() []*GroundItem {
	var gates []*GroundItem
	for _, g := range w.groundItems {
		if g == nil || !g.Permanent || g.Item.Index != arenaGateItem {
			continue
		}
		for _, zone := range cityWarZones {
			if zone.arena.contains(g.X, g.Y) {
				gates = append(gates, g)
				break
			}
		}
	}
	return gates
}

// syncCityArenaGates opens the arena gates only while city-war combat runs
// and locks them at every other time, including after a restart.
func (w *World) syncCityArenaGates(now time.Time) {
	open := false
	if w.guilds != nil && w.cityWarRunning {
		c := w.guilds.Wars.Cities
		open = c.Phase == "battle" && !now.Before(c.StartsAt.Add(6*time.Minute)) &&
			now.Before(c.StartsAt.Add(31*time.Minute))
	}
	state := gateClosed
	if open {
		state = gateOpen
	}
	changed := 0
	for _, g := range w.cityArenaGates() {
		if w.setFixtureState(g, state) {
			changed++
		}
	}
	if changed > 0 {
		log.Printf("city-war arena gates open=%v (%d changed)", open, changed)
	}
}

// serverFixtures are world objects that the 7.48 InitItem.bin lacks. They are
// spawned after the client-mirrored init_items.csv so its IDs stay unchanged.
// 7.59 TMSRV InitItem index 97 ("pvpdoor") is the Armia PvP door.
var serverFixtures = []model.InitItem{{Index: pvpDoorItem, X: pvpDoorX, Y: pvpDoorY, Rotate: 1}}

func (w *World) spawnServerFixtures() error {
	for _, obj := range serverFixtures {
		if _, known := w.items[obj.Index]; !known || len(w.initItems) == 0 {
			continue // isolated test worlds load neither the catalog nor the fixtures
		}
		id, ok := w.allocGroundItemID(obj.Index)
		if !ok {
			return fmt.Errorf("no free ID for fixture %d at (%d,%d)", obj.Index, obj.X, obj.Y)
		}
		item := &GroundItem{ID: id, Item: model.Item{Index: obj.Index}, X: obj.X, Y: obj.Y,
			Rotate: obj.Rotate, Permanent: true, State: initialFixtureState(w.items[obj.Index])}
		w.registerGroundItem(item)
		if item.State == gateClosed {
			w.applyGroundItemHeight(item)
		}
	}
	return nil
}

// pvpDoor returns the 7.59 Armia PvP door fixture, if loaded.
func (w *World) pvpDoor() *GroundItem {
	for _, g := range w.nearbyGroundItems(pvpDoorX, pvpDoorY, 0) {
		if g != nil && g.Permanent && g.Item.Index == pvpDoorItem && g.X == pvpDoorX && g.Y == pvpDoorY {
			return g
		}
	}
	return nil
}

// setPvPDoorLocked is the staff control behind the 7.59 "pkgate" command.
func (w *World) setPvPDoorLocked(locked bool) (bool, bool) {
	door := w.pvpDoor()
	if door == nil {
		return false, false
	}
	state := gateOpen
	if locked {
		state = gateClosed
	}
	return w.setFixtureState(door, state), true
}

// openFixtureAtStartup lists fixtures that W2PP opens after the InitItem pass:
// the Colosseum Steel_Window pair (SetColoseumDoor(1)).
func openFixtureAtStartup(def model.ItemDef) bool {
	return def.Index == 472
}
