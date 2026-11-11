package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func TestOnUseNPCRejectsMissingPlayerAndShortPacket(t *testing.T) {
	t.Run("missing player", func(t *testing.T) {
		w, p, _ := handlerTestWorld(t)
		delete(w.players, p.Session)
		before := p.Session.QueuedPacketsForTest()

		w.onUseNPC(p.Session, useNPCPacket(1234))

		if got := p.Session.QueuedPacketsForTest(); got != before {
			t.Fatalf("missing player queued %d packets, want %d", got, before)
		}
	})

	t.Run("short packet", func(t *testing.T) {
		w, p, _ := handlerTestWorld(t)
		p.ShopNPC = 1234
		before := p.Session.QueuedPacketsForTest()

		w.onUseNPC(p.Session, useNPCPacket(1234)[:13])

		if p.ShopNPC != 1234 {
			t.Fatalf("malformed request changed shop context to %d", p.ShopNPC)
		}
		if got := p.Session.QueuedPacketsForTest(); got != before {
			t.Fatalf("short packet queued %d packets, want %d", got, before)
		}
	})
}

func TestOnUseNPCOpensOnlyCraftOrCargoContext(t *testing.T) {
	for _, tc := range []struct {
		name      string
		merchant  uint32
		wantCraft bool
		wantCargo bool
	}{
		{name: "crafting merchant", merchant: craftingMerchant, wantCraft: true},
		{name: "cargo merchant", merchant: 0x12, wantCargo: true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			mob := &Mob{
				ID: 1200, X: p.X + 1, Y: p.Y,
				Def: &model.NPCDef{Name: tc.name, Tipo: model.TipoNPC, Score: &model.Score{Merchant: tc.merchant}},
			}
			w.registerMobSpatial(mob)
			p.show(mob.ID)
			p.ShopNPC, p.ShopTax = 31, 17
			p.CraftNPC, p.CargoNPC = 32, 33
			p.PremiumNPC, p.PremiumVersion = 34, 35
			p.CityWarNPC, p.CityWarGuild, p.CityWarCity = 36, 37, 2
			p.CityWarUntil = w.now().Add(cityWarInteractionTTL)

			w.onUseNPC(p.Session, useNPCPacket(mob.ID))

			if p.CraftNPC != map[bool]uint16{true: mob.ID}[tc.wantCraft] {
				t.Fatalf("craft context=%d, want enabled=%v", p.CraftNPC, tc.wantCraft)
			}
			if p.CargoNPC != map[bool]uint16{true: mob.ID}[tc.wantCargo] {
				t.Fatalf("cargo context=%d, want enabled=%v", p.CargoNPC, tc.wantCargo)
			}
			if p.ShopNPC != 0 || p.ShopTax != 0 || p.PremiumNPC != 0 || p.PremiumVersion != 0 {
				t.Fatalf("stale merchant context survived: shop=%d tax=%d premium=%d version=%d",
					p.ShopNPC, p.ShopTax, p.PremiumNPC, p.PremiumVersion)
			}
			if p.CityWarNPC != 0 || p.CityWarGuild != 0 || p.CityWarCity != 0 || !p.CityWarUntil.IsZero() {
				t.Fatalf("stale city context survived: npc=%d guild=%d city=%d until=%v",
					p.CityWarNPC, p.CityWarGuild, p.CityWarCity, p.CityWarUntil)
			}
		})
	}
}

func TestOnUseNPCOpensCityCollectorContext(t *testing.T) {
	t.Run("guild owner", func(t *testing.T) {
		w, _, p, collector, _ := cityWarChallengeFixture(t)
		w.onUseNPC(p.Session, useNPCPacket(collector.ID))

		if p.CityWarNPC != collector.ID || p.CityWarCity != 0 || p.CityWarGuild != 1 || p.CityWarUntil.IsZero() {
			t.Fatalf("collector context mismatch: npc=%d city=%d guild=%d until=%v",
				p.CityWarNPC, p.CityWarCity, p.CityWarGuild, p.CityWarUntil)
		}
	})

	t.Run("no guild store", func(t *testing.T) {
		w, p, _ := handlerTestWorld(t)
		collector := &Mob{
			ID: 1201, X: p.X + 1, Y: p.Y,
			Def: &model.NPCDef{Name: "Collector", Tipo: model.TipoNPC, Score: &model.Score{Merchant: 6, Level: 0}},
		}
		w.registerMobSpatial(collector)
		p.show(collector.ID)

		w.onUseNPC(p.Session, useNPCPacket(collector.ID))

		if p.CityWarNPC != collector.ID || p.CityWarCity != 0 || p.CityWarGuild != 0 || p.CityWarUntil.IsZero() {
			t.Fatalf("collector context without guilds mismatch: npc=%d city=%d guild=%d until=%v",
				p.CityWarNPC, p.CityWarCity, p.CityWarGuild, p.CityWarUntil)
		}
	})
}

func TestOnUseNPCOpensTaxedAndOverflowShop(t *testing.T) {
	t.Run("taxed city shop", func(t *testing.T) {
		w, _, p, shop := cityEconomyFixture(t, model.ItemDef{Index: 400, Price: 1000})
		w.onUseNPC(p.Session, useNPCPacket(shop.ID))

		if p.ShopNPC != shop.ID || p.ShopTax != cityWarTaxRate {
			t.Fatalf("taxed shop context mismatch: npc=%d tax=%d", p.ShopNPC, p.ShopTax)
		}
	})

	t.Run("shop display limit", func(t *testing.T) {
		w, p, _ := handlerTestWorld(t)
		items := make([]model.Item, clientShopSlots+1)
		for i := range items {
			items[i].Index = uint16(100 + i)
		}
		shop := &Mob{
			ID: 1202, X: p.X + 1, Y: p.Y,
			Def: &model.NPCDef{Name: "Large Merchant", Tipo: model.TipoNPC,
				Score: &model.Score{Merchant: nativeShopMerchant}, Vende: items},
		}
		w.registerMobSpatial(shop)
		p.show(shop.ID)

		w.onUseNPC(p.Session, useNPCPacket(shop.ID))

		if p.ShopNPC != shop.ID {
			t.Fatalf("overflow shop did not open: got merchant %d", p.ShopNPC)
		}
	})
}

func TestOnUseNPCRoutesConfirmedSefirotComposition(t *testing.T) {
	w, p, st := newAscensionWorld(t)
	master := &Mob{
		ID: 1203, X: p.X + 1, Y: p.Y,
		Def: &model.NPCDef{Name: "Knight_Leader", Tipo: model.TipoNPC, Score: &model.Score{Merchant: skillMasterMerchant}},
	}
	w.registerMobSpatial(master)
	p.show(master.ID)
	giveSephiraStones(p.Char)
	p.Char.Gold = sefirotGoldCost
	pkt := useNPCPacket(master.ID)
	binary.LittleEndian.PutUint32(pkt[16:20], 1)

	w.onUseNPC(p.Session, pkt)

	if st.saves != 1 || p.Char.Gold != 0 {
		t.Fatalf("Sefirot composition did not commit once: saves=%d gold=%d", st.saves, p.Char.Gold)
	}
	want := uint16(sefirotFirstItem)
	found := false
	for _, item := range p.Char.Inv {
		if item.Index >= sephiraFirstStone && item.Index < sephiraFirstStone+sephiraStoneCount {
			t.Fatalf("Sephira stone %d was not consumed", item.Index)
		}
		if item.Index == want {
			found = true
			break
		}
	}
	if !found {
		t.Fatalf("class master did not create Sefirot %d", want)
	}
}

func TestOnUseNPCRoutesUxmalEntry(t *testing.T) {
	w, leader, _, store, _ := uxmalTestWorld()
	uxmal := &Mob{
		ID: 1204, X: leader.X + 1, Y: leader.Y,
		Def: &model.NPCDef{Name: "Uxmal", Tipo: model.TipoNPC, Score: &model.Score{Version: model.ScoreVersion}},
	}
	w.registerMobSpatial(uxmal)
	leader.show(uxmal.ID)

	w.onUseNPC(leader.Session, useNPCPacket(uxmal.ID))

	if w.itemInstances["uxmal:0:0"] == nil || leader.Char.Inv[0].Index != 0 || store.gameSaves != 1 {
		t.Fatalf("Uxmal click did not create entry: instance=%v ticket=%d saves=%d",
			w.itemInstances["uxmal:0:0"] != nil, leader.Char.Inv[0].Index, store.gameSaves)
	}
}

func TestOnUseNPCLeavesAirMoveWindowToClient(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	def := &model.NPCDef{Name: "Air Station", Tipo: model.TipoNPC, Score: &model.Score{Merchant: 0x17}}
	def.Equip.Rosto.Index = 63
	mob := &Mob{ID: 1205, X: p.X + 1, Y: p.Y, Def: def}
	w.registerMobSpatial(mob)
	p.show(mob.ID)
	before := p.Session.QueuedPacketsForTest()

	w.onUseNPC(p.Session, useNPCPacket(mob.ID))

	if got := p.Session.QueuedPacketsForTest(); got != before {
		t.Fatalf("air-move NPC should leave its window to the client; queued %d packets, want %d", got, before)
	}
}
