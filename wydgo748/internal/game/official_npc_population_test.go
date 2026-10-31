package game

import (
	"encoding/binary"
	"math/rand"
	"testing"

	"wydgo/internal/data"
)

func TestOfficialMonsterPopulationBootstraps(t *testing.T) {
	npcs, geners := officialNPCDataset(t)
	fixtures, err := eventFixtures()
	if err != nil {
		t.Fatal(err)
	}
	w := newZoneTestWorld()
	w.terrain, w.volatiles = fixtures.terrain, fixtures.volatiles
	w.rng = rand.New(rand.NewSource(759))
	checked := 0
	blocked := 0
	for _, def := range geners {
		if !def.Enabled || !npcs[def.Leader].IsMonster() ||
			w.generatorReservedForItemInstance(def) || generatorReservedForColosseum(def) || generatorReservedForGuildWar(def) {
			continue
		}
		checked++
		for _, name := range []string{def.Leader, def.Follower} {
			npc := npcs[name]
			if npc == nil || !npc.IsMonster() || npc.Score == nil || npc.Score.MaxHP == 0 {
				t.Fatalf("generator %d has an unusable monster template %q", def.Label, name)
			}
			items := npc.Equip.Items()
			for _, item := range append(items[:], npc.Carry...) {
				if item.Index != 0 {
					if _, ok := fixtures.items[item.Index]; !ok {
						t.Fatalf("generator %d template %q references unsupported item %d", def.Label, name, item.Index)
					}
				}
			}
		}
		g := &generState{def: def, leader: npcs[def.Leader], follower: npcs[def.Follower]}
		start := len(w.mobs)
		w.spawnGroup(g)
		minimum, maximum := min(1+def.MinGroup, def.MaxNumMob), min(1+def.MaxGroup, def.MaxNumMob)
		if g.current < minimum || g.current > maximum || len(w.mobs)-start != g.current {
			t.Fatalf("generator %d spawned %d; expected %d..%d", def.Label, g.current, minimum, maximum)
		}
		leaderID := w.mobs[start].ID
		for i, mob := range w.mobs[start:] {
			want := g.leader
			if i > 0 {
				want = g.follower
				if mob.LeaderID != leaderID {
					t.Fatalf("generator %d follower lost its group leader", def.Label)
				}
			}
			if mob.Def != want || mob.HP != want.Score.MaxHP || mob.GenerIndex != def.Index || mob.InstanceID != "" || w.mobsByID[mob.ID] != mob {
				t.Fatalf("generator %d did not register its authoritative monster state", def.Label)
			}
			if !w.terrain.Walkable(mob.X, mob.Y) {
				blocked++
				if blocked <= 5 {
					t.Logf("blocked reference generator %d (%s) at (%d,%d)", def.Label, mob.Def.Name, mob.X, mob.Y)
				}
			}
		}
	}
	if checked == 0 {
		t.Fatal("no field monster generators were exercised")
	}
	if blocked != 0 {
		t.Fatalf("reference population spawned %d monsters outside usable terrain", blocked)
	}
	t.Logf("bootstrapped %d reference monster groups with %d registered mobs", checked, len(w.mobs))
}

func TestOfficialMixedMonsterGroupRespectsPopulationCap(t *testing.T) {
	npcs, geners := officialNPCDataset(t)
	fixtures, err := eventFixtures()
	if err != nil {
		t.Fatal(err)
	}
	for _, def := range geners {
		if !def.Enabled || def.Leader != "FrenzyDemonLord" || def.Follower != "GargulaFogo___" {
			continue
		}
		if def.MaxNumMob != 7 || def.MinGroup != 6 || def.MaxGroup != 6 {
			t.Fatal("reference mixed group changed without compatibility review")
		}
		w := newZoneTestWorld()
		w.terrain = fixtures.terrain
		w.rng = rand.New(rand.NewSource(759))
		g := &generState{def: def, leader: npcs[def.Leader], follower: npcs[def.Follower]}
		w.spawnGroup(g)
		if len(w.mobs) != 7 || g.current != 7 {
			t.Fatal("reference group did not spawn its leader and six followers")
		}
		w.spawnGroup(g)
		if len(w.mobs) != 7 || g.current != 7 {
			t.Fatal("repeated generation exceeded the reference population cap")
		}
		for _, mob := range w.mobs {
			if !w.terrain.Walkable(mob.X, mob.Y) {
				t.Fatalf("mixed group spawned outside usable terrain at (%d,%d)", mob.X, mob.Y)
			}
		}
		return
	}
	t.Fatal("reference mixed monster group is missing")
}

func TestOfficialServicePopulationReplacesLegacyAliases(t *testing.T) {
	_, geners := officialNPCDataset(t)
	want := map[string][][2]uint16{
		"BlackOracle":  {{1714, 1850}, {1714, 1604}},
		"MercZakum":    {{1832, 1965}, {1832, 1965}, {1707, 1840}},
		"MercCabuncle": {{2112, 2220}, {2112, 2220}, {1707, 1840}},
		"Aki":          {{2144, 2088}},
		"reimers":      {{2468, 1996}},
	}
	removed := map[string]bool{
		"prona": true, "Tera": true, "Hera": true, "Fire_Golem_r": true,
		"Rainy_": true, "Lila": true, "Mercador": true, "Armas_Mortal": true,
		"Set_Mortal": true, "Utilidades": true, "July": true,
		"Trainer4": true, "Trainer5": true,
	}
	for _, g := range geners {
		if !g.Enabled {
			continue
		}
		if removed[g.Leader] {
			t.Errorf("non-reference service remains enabled: %s", g.Leader)
		}
		positions, checked := want[g.Leader]
		if !checked {
			continue
		}
		found := -1
		for i, xy := range positions {
			if g.Segments[0].X == xy[0] && g.Segments[0].Y == xy[1] {
				found = i
				break
			}
		}
		if found == -1 {
			t.Fatalf("unexpected or duplicated %s position: %+v", g.Leader, g.Segments[0])
		}
		want[g.Leader] = append(positions[:found], positions[found+1:]...)
	}
	for name, missing := range want {
		if len(missing) != 0 {
			t.Errorf("missing %s reference positions: %v", name, missing)
		}
	}
}

func TestOfficialAddedShopsBindAndPersistPurchases(t *testing.T) {
	npcs, geners := officialNPCDataset(t)
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	terrain, err := data.LoadTerrain("../../data/world/maps/HeightMap.dat", "../../data/world/maps/AttributeMap.dat")
	if err != nil {
		t.Fatal(err)
	}
	for _, name := range []string{"Arkanaun", "Redmiran", "VictoryAzran", "VictoryErion", "ArchArmor", "ArchWeapons", "MountDealer", "MountFeed", "EvolveShop", "NPC_TOTO", "Rapein", "Neptus", "Theron", "Aylinde", "Martine", "Creta"} {
		t.Run(name, func(t *testing.T) {
			def := npcs[name]
			if def == nil || def.IsMonster() || len(def.Carry) != 0 || def.MakeScore(1).Merchant&15 != 1 {
				t.Fatal("shop lacks its safe NPC role or ordinary-shop client selector")
			}
			for _, item := range def.Vende {
				if item.Index != 0 {
					if _, ok := catalog.Items[item.Index]; !ok {
						t.Fatalf("unsupported stock item %d", item.Index)
					}
				}
			}
			found := 0
			for _, g := range geners {
				if !g.Enabled || g.Leader != name {
					continue
				}
				found++
				origin := g.Segments[0]
				if !terrain.Walkable(origin.X, origin.Y) {
					t.Fatalf("reference spawn is blocked in the current terrain: %v", origin)
				}
				w, p, st := handlerTestWorld(t)
				w.items = catalog.Items
				p.X, p.Y = origin.X-1, origin.Y
				w.updatePlayerSpatial(p)
				m := &Mob{ID: 1100, X: origin.X, Y: origin.Y, Def: def}
				w.registerMobSpatial(m)
				p.show(m.ID)
				w.onUseNPC(p.Session, useNPCPacket(m.ID))
				if p.ShopNPC != m.ID {
					t.Fatal("shop click did not bind the purchase context")
				}
				kind, _ := shopTypeForMerchant(def.Score.Merchant)
				display := shopDisplayList(def.Vende, kind)
				if len(display) == 0 {
					t.Fatal("shop has no visible products")
				}
				// Exercise the final visible product, including sparse native stock
				// and the client's nine-column purchase-slot remapping.
				slot := len(display) - 1
				p.Char.Gold = 2_000_000_000
				packet := make([]byte, 24)
				binary.LittleEndian.PutUint16(packet[12:14], m.ID)
				binary.LittleEndian.PutUint16(packet[14:16], uint16(slot%9+27*(slot/9)))
				w.onBuyItem(p.Session, packet)
				if p.Char.Inv[0].Index != display[slot].Index || p.Char.Inv[0].UID == "" || st.saves != 1 {
					t.Fatalf("reference shop purchase did not persist: item=%+v saves=%d", p.Char.Inv[0], st.saves)
				}
			}
			if found == 0 {
				t.Fatal("shop has no enabled reference spawn")
			}
		})
	}
}

func TestOfficialCargoPopulationBindsStorage(t *testing.T) {
	npcs, geners := officialNPCDataset(t)
	found := 0
	for _, g := range geners {
		if !g.Enabled || (g.Leader != "Guarda_Carga" && g.Leader != "Carthago") {
			continue
		}
		found++
		w, p, _ := handlerTestWorld(t)
		origin := g.Segments[0]
		p.X, p.Y = origin.X-1, origin.Y
		w.updatePlayerSpatial(p)
		m := &Mob{ID: 1100, X: origin.X, Y: origin.Y, Def: npcs[g.Leader]}
		w.registerMobSpatial(m)
		p.show(m.ID)
		w.onUseNPC(p.Session, useNPCPacket(m.ID))
		if p.CargoNPC != m.ID || w.resolveCargoNPC(p, m.ID) != m {
			t.Fatalf("cargo context is unavailable at %s (%d,%d)", g.Leader, m.X, m.Y)
		}
	}
	if found == 0 {
		t.Fatal("no reference cargo population")
	}
}
