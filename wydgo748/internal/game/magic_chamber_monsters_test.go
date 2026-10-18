package game

import (
	"fmt"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/data"
)

// All four real rooms and their delayed bosses must use the dedicated data,
// including private runtime IDs and the final damage/loot policy.
func TestMagicChamberRoomsSpawnDedicatedLevelingMonsters(t *testing.T) {
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
	for item, tier := range map[uint16]string{3172: "normal", 3171: "mystic", 1731: "arcane"} {
		rule := catalog.Items[item]
		if rule.Instance == nil || len(rule.Instance.Stages) != 4 {
			t.Fatalf("%s is missing its four rooms", tier)
		}
		for stageIndex, stage := range rule.Instance.Stages {
			t.Run(fmt.Sprintf("%s/room-%d", tier, stageIndex+1), func(t *testing.T) {
				w, player, outsider, _, clock := instanceTestWorld()
				w.terrain, w.npcs = terrain, npcs
				cfg := *rule.Instance
				inst := &ItemInstance{RuntimeID: cfg.ID + "#2", Config: cfg, MemberIDs: []uint16{player.ID}}
				w.itemInstances[inst.RuntimeID] = inst
				w.indexInstanceMembers(inst)
				want := map[string][2]uint32{"normal": {1000, 200000}, "mystic": {4000, 300000}, "arcane": {10000, 500000}}[tier]
				population := 0
				for _, spawn := range stage.Spawns {
					population += spawn.Count
				}
				check := func(count int) {
					t.Helper()
					if len(inst.MobIDs) != count {
						t.Fatalf("spawned %d monsters, want %d", len(inst.MobIDs), count)
					}
					seen := make(map[uint32]bool)
					for id := range inst.MobIDs {
						mob := w.mobsByID[id]
						if mob == nil || mob.Def == nil || mob.Def.Score == nil {
							t.Fatal("missing spawned Magic Chamber monster")
						}
						score := mob.Def.Score
						if mob.HP != want[0] || score.MaxHP != want[0] || score.CurHP != want[0] ||
							mob.Def.ExpReward != want[1] || score.Attack != 1 || score.MagicAttack != 1 || score.Defense != 1 ||
							len(mob.Def.Carry) != 0 || mob.Def.Gold != 0 || !strings.HasPrefix(mob.Def.Name, "MC") {
							t.Fatalf("incorrect dedicated Magic Chamber monster: %+v", mob.Def)
						}
						tile := uint32(mob.X)<<16 | uint32(mob.Y)
						if seen[tile] || !terrain.Walkable(mob.X, mob.Y) || w.instanceForMob(mob) != inst ||
							!expOnlyInstance(inst) || w.levelingInstanceDamage(mob, 10000) != 1 {
							t.Fatal("blocked/overlapping spawn, incorrect ownership or missing leveling policy")
						}
						seen[tile] = true
						if !w.instanceMobInsideArea(mob) {
							t.Fatalf("room %d spawned mob %d outside its combat area at %d,%d", stageIndex+1, id, mob.X, mob.Y)
						}
						// Every spawned tile must remain visible and targetable to a
						// member, including tiles beyond the old entrance radius.
						player.X, player.Y = mob.X, mob.Y
						player.Char.X, player.Char.Y = player.X, player.Y
						if !instanceMemberInStage(inst, player) || !w.mobVisibleToPlayer(player, mob) ||
							!w.playerCanInteractWithMob(player, mob) {
							t.Fatalf("member cannot see/target room mob %d at %d,%d", id, mob.X, mob.Y)
						}
						outsider.X, outsider.Y = player.X, player.Y
						if w.mobVisibleToPlayer(outsider, mob) || w.playerCanInteractWithMob(outsider, mob) {
							t.Fatal("room mob leaked to a non-member")
						}
					}
					player.X, player.Y = stage.X, stage.Y
					player.Char.X, player.Char.Y = player.X, player.Y
					if !instanceMemberInStage(inst, player) {
						t.Fatal("the entrance is outside the combat area")
					}
					for otherIndex, other := range cfg.Stages {
						if otherIndex != stageIndex && instanceStageContains(stage, other.X, other.Y) {
							t.Fatalf("room %d overlaps room %d's entrance", stageIndex+1, otherIndex+1)
						}
					}
				}
				if ok, _ := w.spawnItemInstanceStage(inst, stageIndex, clock.Now(), false, false); !ok {
					t.Fatal("room spawn failed")
				}
				check(population)
				deadline := instanceCombatDeadline(inst)
				ids := make([]uint16, 0, len(inst.MobIDs))
				for id := range inst.MobIDs {
					ids = append(ids, id)
				}
				for _, id := range ids {
					mob := w.mobsByID[id]
					mob.Dead, mob.HP = true, 0
					w.onItemInstanceMobKilled(mob, clock.Now())
					w.removeMobInstance(mob)
				}
				if stageIndex == 3 {
					if !inst.BossSpawned || inst.Remaining != 1 ||
						!instanceCombatDeadline(inst).Equal(deadline) || inst.CurrentStage != stageIndex {
						t.Fatal("boss spawn failed or reset the fourth-room deadline/stage")
					}
					check(1)
					for id := range inst.MobIDs {
						boss := w.mobsByID[id]
						boss.Dead, boss.HP = true, 0
						w.onItemInstanceMobKilled(boss, clock.Now())
						w.removeMobInstance(boss)
					}
					if inst.Remaining != 0 || !inst.RewardGranted || inst.ExitAt.IsZero() {
						t.Fatal("defeating the final boss did not complete the card")
					}
				} else if inst.Remaining != 0 || len(inst.MobIDs) != 0 || inst.TransitionAt.IsZero() {
					t.Fatal("clearing the room did not schedule the next room")
				}
			})
		}
	}
}
