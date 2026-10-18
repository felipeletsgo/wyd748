package game

import (
	"path/filepath"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

// TestNightmareSoloOpensWithInstalledData drives ticket 3390 through the
// production critical-persistence entry with the shipped terrain and monsters.
// Admission reserves member destinations; the scheduled combat start must
// then materialize the complete first wave on distinct walkable tiles.
func TestNightmareSoloOpensWithInstalledData(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	terrain, err := data.LoadTerrain(filepath.Join(root, "maps", "HeightMap.dat"),
		filepath.Join(root, "maps", "AttributeMap.dat"))
	if err != nil {
		t.Skipf("installed maps unavailable: %v", err)
	}
	npcs, err := data.LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	items, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.json"), items.Items, items.Skills)
	if err != nil {
		t.Fatal(err)
	}
	rule, ok := catalog.Items[3390]
	if !ok || rule.Instance == nil {
		t.Fatal("ticket 3390 is not an instance ticket")
	}
	configs := make(map[string]model.VolatileInstance)
	for _, rule := range catalog.Items {
		if rule.Instance != nil && rule.Instance.NightmareTier != "" {
			configs[rule.Instance.ID] = *rule.Instance
		}
	}
	if len(configs) != 6 {
		t.Fatalf("found %d Nightmare configurations, want all six party/solo tiers", len(configs))
	}
	for _, config := range configs {
		if config.NightmareTier == "" {
			continue
		}
		if config.NightmareTier == "arcane" {
			boss := config.NightmareBoss
			if boss == nil || boss.Spawn.NPC != "NMA_Lugefer" || boss.SecondsRemaining != 300 || boss.Spawn.Count != 1 {
				t.Fatalf("%s lost the delayed one-shot Lugefer configuration", config.ID)
			}
			families := map[string]bool{"NMA_Tauron": false, "NMA_ArmTaur": false, "NMA_Spider": false, "NMA_Manticor": false}
			for _, spawn := range config.Spawns {
				if _, exists := families[spawn.NPC]; !exists {
					t.Fatalf("%s has an unexpected ordinary spawn: %s", config.ID, spawn.NPC)
				}
				families[spawn.NPC] = true
			}
			for family, present := range families {
				if !present {
					t.Fatalf("%s is missing %s", config.ID, family)
				}
			}
		}
		t.Run(config.ID+"/protectors", func(t *testing.T) {
			world, _, _, _, _ := instanceTestWorld()
			world.terrain, world.npcs = terrain, npcs
			instance := &ItemInstance{Config: cfgCopy(config), MobIDs: make(map[uint16]struct{})}
			if !world.spawnNightmareProtectors(instance) {
				t.Fatal("installed protected NPCs could not be placed")
			}
			want := 8
			if config.NightmareTier == "normal" {
				want = 5
			}
			if len(instance.NPCIDs) != want {
				t.Fatalf("protected NPCs=%d, want=%d", len(instance.NPCIDs), want)
			}
			for id := range instance.NPCIDs {
				npc := world.mobsByID[id]
				if npc == nil || !terrain.Walkable(npc.X, npc.Y) || npc.Def.IsMonster() {
					t.Fatal("installed protected NPC is blocked or hostile")
				}
			}
			clock := world.clock.(*fakeClock)
			instance.State = "combat"
			instance.ScheduleEnd = clock.Now()
			instance.HardDeadline = clock.Now().Add(nightmareCombatDuration)
			world.itemInstances[config.ID] = instance
			population := 0
			for _, spawn := range config.Spawns {
				population += spawn.Count
			}
			if ok, _ := world.spawnItemInstanceStage(instance, 0, clock.Now(), false, false); !ok || len(instance.MobIDs) != population {
				t.Fatal("installed Nightmare population could not be placed without the delayed boss")
			}
			if config.NightmareTier == "arcane" {
				clock.Advance(10 * time.Minute)
				if !world.spawnNightmareBoss(instance, clock.Now(), false) || instance.NightmareBossID == 0 || len(instance.MobIDs) != 81 {
					t.Fatal("installed Arcane boss could not be placed at five minutes remaining")
				}
				assertNightmareLevelingBalance(t, world.mobsByID[instance.NightmareBossID], "arcane")
			}
			for id := range instance.MobIDs {
				if id == instance.NightmareBossID {
					continue
				}
				world.killMobWithoutPlayer(world.mobsByID[id])
				break
			}
			seen := make(map[uint32]struct{})
			for _, mob := range world.mobsByID {
				tile := uint32(mob.X)<<16 | uint32(mob.Y)
				if _, overlap := seen[tile]; overlap || !terrain.Walkable(mob.X, mob.Y) {
					t.Fatal("installed Nightmare boss, monster or protector overlaps or is blocked")
				}
				seen[tile] = struct{}{}
				if mob.Def.IsMonster() {
					assertNightmareLevelingBalance(t, mob, config.NightmareTier)
				}
			}
		})
	}

	w, leader, member, peer, _, base, q, _, release := criticalInstanceEntryFixture(t, "water", "commit")
	leader.Party, member.Party = nil, nil
	leader.Trade, peer.Trade = nil, nil
	w.terrain = terrain
	w.npcs = npcs
	w.volatiles.Items[100] = rule
	// Inside the minute-40 window and the Erion entry area.
	w.clock = newFakeClock(time.Date(2026, 10, 13, 20, 41, 0, 0, time.UTC))
	leader.X, leader.Y, leader.Char.X, leader.Char.Y = 2457, 2016, 2457, 2016
	w.updatePlayerSpatial(leader)
	leader.Char.Score.Level = 300
	leader.Char.RuntimeScore = nil
	applyScore(leader.Char)

	if err := w.queueCriticalInstanceTicket(leader, 0, rule, ""); err != nil {
		t.Fatalf("Nightmare Normal entry rejected: %v", err)
	}
	awaitCriticalWrite(t, base.entered)
	release()
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)

	var inst *ItemInstance
	for _, candidate := range w.itemInstances {
		if candidate != nil && candidate.Config.ID == rule.Instance.ID {
			inst = candidate
		}
	}
	want := 0
	for _, spawn := range rule.Instance.Spawns {
		want += spawn.Count
	}
	if inst == nil || !itemInstanceHasMember(inst, leader.ID) || len(inst.MobIDs) != 0 || !nightmareWaiting(inst) {
		t.Fatalf("Nightmare Normal did not open in admission: %+v", inst)
	}
	if len(inst.NPCIDs) != 5 || instanceDisplayedMobCount(inst) != 5 {
		t.Fatal("Nightmare Normal did not publish its five protected NPCs")
	}
	clock := w.clock.(*fakeClock)
	clock.Advance(3 * time.Minute)
	w.tickItemInstances(clock.Now())
	if len(inst.MobIDs) != want || remainingInstanceSeconds(instanceCombatDeadline(inst), clock.Now()) != 900 {
		t.Fatalf("Nightmare Normal did not start a 900-second combat with %d monsters: %+v", want, inst)
	}
	for id := range inst.MobIDs {
		w.killMobWithoutPlayer(w.mobsByID[id])
		break
	}
	if len(inst.MobIDs) != want || len(inst.NightmareSpawns) != want || len(w.mobsByID) != want+len(inst.NPCIDs) {
		t.Fatal("individual death did not immediately restore the installed population")
	}
	seen := make(map[uint32]struct{}, want)
	for id := range inst.MobIDs {
		mob := w.mobsByID[id]
		if mob == nil || !w.terrain.Walkable(mob.X, mob.Y) {
			t.Fatalf("monster %d was not placed on walkable terrain", id)
		}
		assertNightmareLevelingBalance(t, mob, "normal")
		tile := uint32(mob.X)<<16 | uint32(mob.Y)
		if _, duplicate := seen[tile]; duplicate {
			t.Fatalf("two monsters share tile (%d,%d)", mob.X, mob.Y)
		}
		seen[tile] = struct{}{}
	}
}
