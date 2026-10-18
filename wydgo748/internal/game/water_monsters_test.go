package game

import (
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/data"
)

// Exercise the shipped definitions through the real private-room spawn path,
// not a balance override or a test-only monster catalog.
func TestWaterRoomsSpawnDedicatedLevelingMonsters(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	terrain, err := data.LoadTerrain(filepath.Join(root, "maps", "HeightMap.dat"), filepath.Join(root, "maps", "AttributeMap.dat"))
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := data.LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	items, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"), filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"), filepath.Join(root, "instances.json"), items.Items, items.Skills)
	if err != nil {
		t.Fatal(err)
	}
	rooms := 0
	for _, rule := range catalog.Items {
		if rule.Instance == nil || !isDurablePrivateWaterConfig(*rule.Instance) {
			continue
		}
		rooms++
		t.Run(rule.Instance.ID, func(t *testing.T) {
			w, _, _, _, clock := instanceTestWorld()
			w.terrain, w.npcs = terrain, npcs
			cfg := *rule.Instance
			inst := &ItemInstance{RuntimeID: cfg.ID + "#2", Config: cfg}
			w.itemInstances[inst.RuntimeID] = inst
			want := map[string][2]uint32{"normal": {1000, 200000}, "mystic": {4000, 300000}, "arcane": {10000, 500000}}[strings.Split(cfg.ID, "-")[1]]
			population := 0
			for _, spawn := range cfg.Spawns {
				population += spawn.Count
			}
			if ok, _ := w.spawnItemInstanceStage(inst, 0, clock.Now(), false, false); !ok || len(inst.MobIDs) != population {
				t.Fatalf("spawned %d monsters, want %d", len(inst.MobIDs), population)
			}
			seen := make(map[uint32]bool)
			for id := range inst.MobIDs {
				mob := w.mobsByID[id]
				if mob == nil || mob.Def == nil || mob.Def.Score == nil {
					t.Fatal("missing spawned Water monster")
				}
				score := mob.Def.Score
				if mob.HP != want[0] || score.MaxHP != want[0] || score.CurHP != want[0] ||
					mob.Def.ExpReward != want[1] || score.Attack != 1 || score.MagicAttack != 1 || score.Defense != 1 ||
					len(mob.Def.Carry) != 0 || mob.Def.Gold != 0 || !strings.HasPrefix(mob.Def.Name, "WT") {
					t.Fatalf("incorrect dedicated Water monster: %+v", mob.Def)
				}
				tile := uint32(mob.X)<<16 | uint32(mob.Y)
				if seen[tile] || !terrain.Walkable(mob.X, mob.Y) || w.instanceForMob(mob) != inst ||
					!expOnlyInstance(inst) || w.levelingInstanceDamage(mob, 10000) != 1 {
					t.Fatal("blocked/overlapping spawn, incorrect private ownership or missing leveling policy")
				}
				seen[tile] = true
			}
		})
	}
	if rooms != 27 {
		t.Fatalf("validated %d Water rooms, want all 27", rooms)
	}
}
