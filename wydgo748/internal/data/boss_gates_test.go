package data

import "testing"

// The four world bosses live in their own areas, and an Armia NPC per boss
// teleports players to that area's entrance through a free repeatable quest.
func TestBossGatesTeleportToTheBossEntrances(t *testing.T) {
	terrain, err := LoadTerrain("../../data/maps/HeightMap.dat", "../../data/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	bosses, err := LoadBossCatalog("../../data/boss")
	if err != nil {
		t.Fatal(err)
	}
	quests, err := LoadQuests("../../data/quests.json")
	if err != nil {
		t.Fatal(err)
	}
	geners, err := LoadNPCGener("../../data/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	known := map[string]bool{}
	for _, npc := range npcs {
		known[npc.Name] = true
	}
	want := []struct {
		boss, gate                 string
		bossX, bossY, gateX, gateY uint16
	}{
		{"Black Archmage", "ArchmageGate", 3910, 3418, 3893, 3418},
		{"Guardian of the Ruins", "GuardianGate", 3806, 4019, 3792, 4006},
		{"Demon Lord", "DemonGate", 731, 3173, 711, 3169},
		{"Ancient Necromancer", "NecroGate", 2083, 347, 2087, 328},
	}
	for _, w := range want {
		found := false
		for _, boss := range bosses.Bosses {
			if boss.Name == w.boss {
				found = boss.Spawn.X == w.bossX && boss.Spawn.Y == w.bossY
			}
		}
		if !found || !terrain.Walkable(w.bossX, w.bossY) {
			t.Fatalf("%s is not spawned at walkable (%d,%d)", w.boss, w.bossX, w.bossY)
		}
		var quest bool
		for _, q := range quests.Quests {
			if q.NPC == w.gate {
				quest = q.Repeatable && q.Rewards.Teleport != nil &&
					q.Rewards.Teleport.X == w.gateX && q.Rewards.Teleport.Y == w.gateY &&
					q.Requires.MinLevel == 0 && q.Requires.Gold == 0 && len(q.Consumes) == 0
			}
		}
		if !quest || !terrain.Walkable(w.gateX, w.gateY) || !known[w.gate] {
			t.Fatalf("%s does not teleport freely to walkable (%d,%d)", w.gate, w.gateX, w.gateY)
		}
		placed := false
		for _, g := range geners {
			if g.Enabled && g.Leader == w.gate {
				x, y := g.Segments[0].X, g.Segments[0].Y
				placed = x >= 2085 && x <= 2115 && y >= 2085 && y <= 2115 && terrain.Walkable(x, y)
			}
		}
		if !placed {
			t.Fatalf("%s is not placed on a walkable Armia cell", w.gate)
		}
	}
}
