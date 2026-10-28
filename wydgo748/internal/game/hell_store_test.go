package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func hellStoreCatalog(t *testing.T) (map[uint16]model.ItemDef, model.VolatileCatalog) {
	t.Helper()
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances("../../data/content/volatiles.json", "../../data/content/instances.json", "../../data/spawns/instances", catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	volatiles.Repliction, err = data.LoadRepliction("../../data/content/repliction.json", catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	return catalog.Items, volatiles
}

func hellStoreWorld(t *testing.T) (*World, *Player, *craftStore, *Mob) {
	t.Helper()
	items, volatiles := hellStoreCatalog(t)
	w, p, st := handlerTestWorld(t)
	w.items, w.volatiles = items, volatiles
	store := &Mob{ID: 1200, X: p.X + 1, Y: p.Y, Def: &model.NPCDef{
		Name: hellStoreNPCName, Tipo: model.TipoNPC, Score: &model.Score{},
	}}
	w.registerMobSpatial(store)
	p.show(store.ID)
	p.Char.Gold = 20_000_000
	return w, p, st, store
}

func hellStorePacket(playerID, target uint16, option, dst int16) []byte {
	pkt := inboundPacket(wire.OpHellStoreBuy, hellStorePacketSize)
	binary.LittleEndian.PutUint16(pkt[6:8], playerID)
	binary.LittleEndian.PutUint16(pkt[12:14], target)
	binary.LittleEndian.PutUint16(pkt[14:16], uint16(option))
	binary.LittleEndian.PutUint16(pkt[16:18], uint16(dst))
	return pkt
}

func containsIndex(pool []uint16, index uint16) bool {
	for _, candidate := range pool {
		if candidate == index {
			return true
		}
	}
	return false
}

func TestHellStorePoolsFollowTheNativeTiers(t *testing.T) {
	items, _ := hellStoreCatalog(t)
	cases := []struct {
		category int
		size     int
		include  []uint16
		exclude  []uint16
	}{
		// Gram (9th) and Bhopal Sword (10th) are the native text's examples;
		// Soul Blade (8th), Thron of Shard and Gram(Ancient) are not sold.
		{hellStoreWeapon, 20, []uint16{869, 870, 809, 911}, []uint16{868, 871, 2723}},
		// Dwarven Plate (7th) and Golden Embersed (8th) for every class and
		// part; Golden Plate (6th) and the arch Initiate armor are not sold.
		{hellStoreArmor, 120, []uint16{1194, 1209, 1344, 1494, 1644}, []uint16{1179, 1221, 2182}},
		// Silver, Gold and Mystic amulets; Crystal and Arcane are not sold.
		{hellStoreAmulet, 12, []uint16{551, 555, 559, 562}, []uint16{563, 567}},
	}
	for _, tc := range cases {
		pool := hellStorePool(items, tc.category)
		if len(pool) != tc.size {
			t.Fatalf("category %d has %d items; want %d: %v", tc.category, len(pool), tc.size, pool)
		}
		for _, index := range tc.include {
			if !containsIndex(pool, index) {
				t.Errorf("category %d is missing item %d", tc.category, index)
			}
		}
		for _, index := range tc.exclude {
			if containsIndex(pool, index) {
				t.Errorf("category %d must not sell item %d", tc.category, index)
			}
		}
	}
}

func TestHellStoreWeaponOptionsUseTheNativeTable(t *testing.T) {
	if got := hellStoreWeaponChoice(1, false, false); got != (hellStoreWeaponOption{2, 9, -1}) {
		t.Fatalf("physical roll 1 = %+v; want EF_DAMAGE x9", got)
	}
	if got := hellStoreWeaponChoice(2, false, true); got != (hellStoreWeaponOption{74, 3, 1}) {
		t.Fatalf("second physical roll 2 = %+v; want EF_SPECIALALL x3 +1", got)
	}
	if got := hellStoreWeaponChoice(0, true, false); got != (hellStoreWeaponOption{60, 4, -1}) {
		t.Fatalf("magic roll 0 = %+v; want EF_MAGIC x4", got)
	}
	for roll, want := range map[int]int{0: 4, 1: 4, 2: 3, 5: 3, 6: 2, 23: 2, 24: 1, 54: 1, 55: 0, 99: 0} {
		if got := hellStoreWeaponGrade(roll); got != want {
			t.Fatalf("grade(%d) = %d; want %d", roll, got, want)
		}
	}
}

func TestHellStoreSellsARolledItemOfTheChosenCategory(t *testing.T) {
	for option, price := range hellStorePrices {
		w, p, st, store := hellStoreWorld(t)
		packet := hellStorePacket(p.ID, store.ID, int16(option), 7)
		if !w.validateInboundCommand(p.Session, packet) {
			t.Fatalf("option %d: native 24-byte request rejected at ingress", option)
		}
		w.handle(command{s: p.Session, pkt: packet})
		item := p.Char.Inv[7]
		if p.Char.Gold != uint32(20_000_000-price) || st.saves != 1 {
			t.Fatalf("option %d: gold=%d saves=%d", option, p.Char.Gold, st.saves)
		}
		inPool := false
		for category := hellStoreWeapon; category <= hellStoreAmulet; category++ {
			if (option == hellStoreRandom || option == category) && containsIndex(hellStorePool(w.items, category), item.Index) {
				inPool = true
			}
		}
		if !inPool || item.UID == "" {
			t.Fatalf("option %d produced item %+v outside its pool", option, item)
		}
		def := w.items[item.Index]
		if hellStoreCategoryAccepts(def, hellStoreAmulet) {
			if item.Eff != ([6]byte{}) {
				t.Fatalf("amulet received options: %v", item.Eff)
			}
		} else if item.Eff[0] != hellStoreSancEffect || item.Eff[1] != 0 {
			t.Fatalf("option %d item %d is refined or lost EF_SANC: %v", option, item.Index, item.Eff)
		}
	}
}

func TestHellStoreRejectsInvalidPurchasesWithoutCharging(t *testing.T) {
	cases := []struct {
		name  string
		setup func(*World, *Player, *Mob) []byte
	}{
		{"other merchant", func(w *World, p *Player, m *Mob) []byte {
			m.Def.Name = "Merchant"
			return hellStorePacket(p.ID, m.ID, hellStoreWeapon, 3)
		}},
		{"no category", func(w *World, p *Player, m *Mob) []byte { return hellStorePacket(p.ID, m.ID, 0, 3) }},
		{"unknown category", func(w *World, p *Player, m *Mob) []byte { return hellStorePacket(p.ID, m.ID, 5, 3) }},
		{"occupied slot", func(w *World, p *Player, m *Mob) []byte {
			p.Char.Inv[3] = model.Item{Index: 400}
			return hellStorePacket(p.ID, m.ID, hellStoreArmor, 3)
		}},
		{"slot outside carry", func(w *World, p *Player, m *Mob) []byte {
			return hellStorePacket(p.ID, m.ID, hellStoreArmor, model.PlayerCarrySlots)
		}},
		{"not enough gold", func(w *World, p *Player, m *Mob) []byte {
			p.Char.Gold = 8_999_999
			return hellStorePacket(p.ID, m.ID, hellStoreWeapon, 3)
		}},
		{"too far", func(w *World, p *Player, m *Mob) []byte {
			m.X = p.X + npcInteractionRange + 1
			return hellStorePacket(p.ID, m.ID, hellStoreWeapon, 3)
		}},
	}
	for _, tc := range cases {
		w, p, st, store := hellStoreWorld(t)
		packet := tc.setup(w, p, store)
		gold, inv := p.Char.Gold, p.Char.Inv
		w.onHellStoreBuy(p.Session, packet)
		if p.Char.Gold != gold || p.Char.Inv != inv || st.saves != 0 {
			t.Fatalf("%s: purchase changed the character", tc.name)
		}
		reply, ok := p.Session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(reply) || wire.ParseHeader(reply).Type != wire.OpMessagePanel {
			t.Fatalf("%s: no rejection message", tc.name)
		}
	}

	w, p, _, store := hellStoreWorld(t)
	w.onHellStoreBuy(p.Session, hellStorePacket(p.ID+1, store.ID, hellStoreWeapon, 3))
	if p.Session.QueuedPacketsForTest() != 0 || violationsFor(w, p) != 1 || p.Char.Gold != 20_000_000 {
		t.Fatal("request with another session id was not refused as a violation")
	}
}

func TestHellStoreRollsBackWhenTheAccountCannotBeSaved(t *testing.T) {
	w, p, st, store := hellStoreWorld(t)
	st.err = errors.New("disk full")
	w.onHellStoreBuy(p.Session, hellStorePacket(p.ID, store.ID, hellStoreAmulet, 4))
	if p.Char.Gold != 20_000_000 || p.Char.Inv[4].Index != 0 {
		t.Fatalf("failed save kept the purchase: gold=%d item=%+v", p.Char.Gold, p.Char.Inv[4])
	}
}
