package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func officialNPCDataset(t *testing.T) (map[string]*model.NPCDef, []model.NPCGener) {
	t.Helper()
	npcs, err := data.LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]*model.NPCDef, len(npcs))
	for i := range npcs {
		byName[npcs[i].Name] = &npcs[i]
	}
	for i := range npcs {
		identity := npcs[i].InteractionName()
		if byName[identity] == nil {
			byName[identity] = &npcs[i]
		}
	}
	geners, err := data.LoadChannelNPCGener("../../data/spawns/NPCGener.txt")
	if err != nil {
		t.Fatal(err)
	}
	return byName, geners
}

func TestOfficialAirTransportDatasetCompletesFlight(t *testing.T) {
	npcs, geners := officialNPCDataset(t)
	var source *model.NPCGener
	for i := range geners {
		g := &geners[i]
		if g.Enabled && g.Leader == "Griphon_Master" {
			if source != nil {
				t.Fatal("duplicate air transport generator")
			}
			source = g
		}
	}
	if source == nil || !isAirMoveNPC(npcs[source.Leader]) {
		t.Fatal("air transport has no usable template/generator")
	}
	for route, destination := range airMoveDestinations {
		w, p, _ := handlerTestWorld(t)
		clock := newFakeClock(time.Unix(100, 0))
		w.clock = clock
		origin := source.Segments[0]
		p.X, p.Y = origin.X-1, origin.Y
		w.updatePlayerSpatial(p)
		m := &Mob{ID: 1100, X: origin.X, Y: origin.Y, Def: npcs[source.Leader]}
		w.registerMobSpatial(m)
		p.show(m.ID)
		w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, uint32(route), 1)})
		if !p.AirMoveActive {
			t.Fatalf("route %d rejected the imported NPC at (%d,%d)", route, m.X, m.Y)
		}
		clock.Advance(airMoveMinimumDuration)
		w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, uint32(route), 2)})
		if p.AirMoveActive || p.X != destination.x || p.Y != destination.y {
			t.Fatalf("route %d did not reach %v: (%d,%d)", route, destination, p.X, p.Y)
		}
	}
}

func TestOfficialArtisansRetainCompatibleWindowAndServerContext(t *testing.T) {
	npcs, geners := officialNPCDataset(t)
	for _, tc := range []struct {
		name   string
		face   uint16
		chunkX uint16
		chunkY uint16
	}{
		{"Compositor", 54, 19, 13},
		{"Aylin", 55, 0, 0},
		{"Agatha", 56, 0, 0},
		{"Ehre", 68, 19, 15},
		{"Lindy", 67, 13, 13},
		{"Odin", 67, 25, 13},
		{"Tiny", 68, 28, 24},
	} {
		t.Run(tc.name, func(t *testing.T) {
			found := 0
			for _, g := range geners {
				if !g.Enabled || npcs[tc.name] == nil || npcs[g.Leader] != npcs[tc.name] {
					continue
				}
				found++
				def := npcs[tc.name]
				if def == nil || def.Equip.Rosto.Index != tc.face || def.MakeScore(1).Merchant&15 != 8 {
					t.Fatal("imported appearance/function cannot select the supported craft window")
				}
				origin := g.Segments[0]
				if tc.chunkX != 0 && (origin.X/128 != tc.chunkX || origin.Y/128 != tc.chunkY) {
					t.Fatalf("position (%d,%d) selects a different client window", origin.X, origin.Y)
				}
				w, p, _ := handlerTestWorld(t)
				p.X, p.Y = origin.X+1, origin.Y
				m := &Mob{ID: 1100, X: origin.X, Y: origin.Y, Def: def}
				w.registerMobSpatial(m)
				p.show(m.ID)
				got, err := w.combineNPC(p, tc.name)
				if err != nil || got != m || p.CraftNPC != m.ID {
					t.Fatalf("client-selected artisan has no authoritative context: mob=%v err=%v", got, err)
				}
			}
			if found != 1 {
				t.Fatalf("enabled artisan generators = %d, want 1", found)
			}
		})
	}
	for _, g := range geners {
		if g.Enabled && (g.Leader == "Compounder" || g.Follower == "Compounder") {
			t.Fatal("incompatible duplicate composition identity is still enabled")
		}
	}
}

func TestOfficialShopStockPurchasesAcrossClientGridBlocks(t *testing.T) {
	npcs, _ := officialNPCDataset(t)
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w, p, st := handlerTestWorld(t)
	w.items = catalog.Items
	w.npcs = []model.NPCDef{*npcs["Galford"]}
	if err := w.initShopItemDefaults(); err != nil {
		t.Fatal(err)
	}
	shop := &Mob{ID: 1100, X: p.X + 1, Y: p.Y, Def: &w.npcs[0]}
	w.registerMobSpatial(shop)
	p.show(shop.ID)
	w.onUseNPC(p.Session, useNPCPacket(shop.ID))
	if p.ShopNPC != shop.ID {
		t.Fatal("imported shop did not bind its purchase context")
	}
	kind, _ := shopTypeForMerchant(shop.Def.Score.Merchant)
	display := shopDisplayList(shop.Def.Vende, kind)
	if len(display) <= 9 {
		t.Fatal("fixture must exercise the second client grid block")
	}
	const slot = 9
	item := display[slot]
	def := w.items[item.Index]
	p.Char.Gold = 2_000_000_000
	before := p.Char.Gold
	buy := make([]byte, 24)
	binary.LittleEndian.PutUint16(buy[12:14], shop.ID)
	binary.LittleEndian.PutUint16(buy[14:16], slot%9+27*(slot/9))
	w.onBuyItem(p.Session, buy)
	if got := p.Char.Inv[0]; got.Index != item.Index || got.Eff != item.Eff || got.UID == "" {
		t.Fatalf("purchase selected a different imported product: got=%+v want=%+v", got, item)
	}
	if st.saves != 1 || p.Char.Gold != before-def.Price {
		t.Fatalf("purchase was not persisted/charged once: saves=%d gold=%d price=%d", st.saves, p.Char.Gold, def.Price)
	}
}
