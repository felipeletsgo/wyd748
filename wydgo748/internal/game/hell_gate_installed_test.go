package game

import (
	"path/filepath"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func installedHellGate(t *testing.T) (*World, model.VolatileRule) {
	t.Helper()
	root := filepath.Join("..", "..", "data")
	terrain, err := data.LoadTerrain(filepath.Join(root, "world", "maps", "HeightMap.dat"), filepath.Join(root, "world", "maps", "AttributeMap.dat"))
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := data.LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	items, err := data.LoadCatalog(filepath.Join(root, "catalog", "itemlist.csv"), filepath.Join(root, "catalog", "Itemname.csv"), filepath.Join(root, "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadVolatilesWithInstances(filepath.Join(root, "content", "volatiles.json"), filepath.Join(root, "content", "instances.json"), filepath.Join(root, "spawns", "instances"), items.Items, items.Skills)
	if err != nil {
		t.Fatal(err)
	}
	rule, ok := catalog.Items[3328]
	if !ok || rule.Instance == nil || rule.Instance.HellGate == nil {
		t.Fatal("Hell Gate ticket 3328 has no encounter configuration")
	}
	w, _, _, _, _ := instanceTestWorld()
	w.terrain, w.npcs = terrain, npcs
	return w, rule
}

func TestHellGateInstalledTicketPublishesAttackableEntry(t *testing.T) {
	w, rule := installedHellGate(t)
	var leader *Player
	for _, p := range w.players {
		if p.Char.Name == "Leader" {
			leader = p
		}
	}
	if leader == nil {
		t.Fatal("entry fixture has no leader")
	}
	w.clock = newFakeClock(time.Date(2026, 10, 17, 12, 0, 30, 0, time.UTC))
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	runtimeID := w.playerRuntimeInstanceID(leader.ID)
	if runtimeID == "" {
		t.Fatal("installed Hell Gate ticket did not attach the player")
	}
	inst := w.itemInstances[runtimeID]
	if inst == nil || inst.State != "preparing" || len(inst.MobIDs) != 0 ||
		!inst.ScheduleEnd.Equal(time.Date(2026, 10, 17, 12, 2, 0, 0, time.UTC)) ||
		!inst.HardDeadline.Equal(time.Date(2026, 10, 17, 12, 20, 0, 0, time.UTC)) {
		t.Fatalf("installed Hell Gate preparation or absolute deadline is wrong: %+v", inst)
	}
	w.clock = newFakeClock(inst.ScheduleEnd)
	w.tickItemInstances(w.now())
	if inst.State != "entry" || len(inst.MobIDs) != 1 {
		t.Fatal("Hell Gate did not publish its controller after two-minute admission")
	}
	for id := range inst.MobIDs {
		mob := w.mobsByID[id]
		if mob == nil || !mob.Awake || !leader.hasVisible(id) || mob.InstanceID != runtimeID {
			t.Fatalf("entry monster is not active and visible in the member's gameplay space: %+v", mob)
		}
		if !w.playerCanInteractWithMob(leader, mob) {
			t.Fatal("entry monster is rejected by authoritative combat targeting")
		}
	}
}

func TestHellGateInstalledEntryAndQuadrants(t *testing.T) {
	w, rule := installedHellGate(t)
	inst := &ItemInstance{Config: cfgCopy(*rule.Instance), RuntimeID: "hell-gate-installed", State: "entry"}
	w.itemInstances[inst.RuntimeID] = inst
	if ok, _ := w.spawnItemInstanceStage(inst, 0, w.now(), false, false); !ok {
		t.Fatal("installed Hell Gate first population could not be created")
	}
	assertPopulation := func() {
		t.Helper()
		if len(inst.MobIDs) == 0 {
			t.Fatal("Hell Gate population is empty")
		}
		for id := range inst.MobIDs {
			mob := w.mobsByID[id]
			if mob == nil || !w.terrain.Walkable(mob.X, mob.Y) {
				t.Fatalf("monster %d is absent or on blocked terrain: %+v", id, mob)
			}
		}
	}
	assertPopulation()
	for _, q := range inst.Config.HellGate.Quadrants {
		t.Logf("quadrant %d: (%d,%d), walkable=%v", q.ID, q.X, q.Y, w.terrain.Walkable(q.X, q.Y))
	}
	if !w.spawnHellGateQuadrants(inst, w.now()) {
		t.Fatal("installed Hell Gate quadrants could not be created")
	}
	assertPopulation()
	for _, q := range inst.Config.HellGate.Quadrants {
		if !w.spawnHellGateLich(inst, uint8(q.ID)) {
			t.Fatalf("installed Hell Gate Lich %d could not be created", q.ID)
		}
	}
	assertPopulation()
	if !w.spawnHellGateFinal(inst, w.now()) {
		t.Fatal("installed Hell Gate final population could not be created")
	}
	assertPopulation()
}
