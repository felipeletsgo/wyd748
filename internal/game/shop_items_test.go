package game

import (
	"encoding/binary"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func amountItemDef(index uint16, amount int) model.ItemDef {
	return model.ItemDef{
		Index: index,
		StaticEffects: []model.StaticEffect{
			{Name: "EF_GRID", Value: 1},
			{Name: "EF_VOLATILE", Value: 11},
			{Name: "EF_AMOUNT", Value: amount},
		},
	}
}

func TestInitShopItemDefaultsSeedsAmountWithoutCopyingStaticStats(t *testing.T) {
	w := &World{
		items: map[uint16]model.ItemDef{
			411: {
				Index: 411,
				StaticEffects: []model.StaticEffect{
					{Name: "EF_DAMAGE", Value: 50},
					{Name: "EF_AMOUNT", Value: 10},
				},
			},
		},
		npcs: []model.NPCDef{{Name: "Aki", Vende: []model.Item{{Index: 411}}}},
	}
	if err := w.initShopItemDefaults(); err != nil {
		t.Fatalf("initShopItemDefaults: %v", err)
	}
	item := w.npcs[0].Vende[0]
	if amount, ok := itemEffectValue(item, effectAmount); !ok || amount != 10 {
		t.Fatalf("Recall Scroll sem EF_AMOUNT=10: item=%+v", item)
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] != 0 && item.Eff[i*2] != effectAmount {
			t.Fatalf("efeito estatico foi duplicado no item da loja: %+v", item.Eff)
		}
	}
}

func TestInitShopItemDefaultsPreservesExplicitBundleOverride(t *testing.T) {
	w := &World{
		items: map[uint16]model.ItemDef{685: amountItemDef(685, 20)},
		npcs: []model.NPCDef{{
			Name:  "Aki",
			Vende: []model.Item{{Index: 685, Eff: [6]byte{effectAmount, 100}}},
		}},
	}
	if err := w.initShopItemDefaults(); err != nil {
		t.Fatalf("initShopItemDefaults: %v", err)
	}
	if amount, _ := itemEffectValue(w.npcs[0].Vende[0], effectAmount); amount != 100 {
		t.Fatalf("pacote explicito da loja foi sobrescrito: %d", amount)
	}
}

func TestInitShopItemDefaultsRejectsInvalidAmountStock(t *testing.T) {
	tests := []struct {
		name  string
		items map[uint16]model.ItemDef
		item  model.Item
		want  string
	}{
		{
			name:  "amount outside wire",
			items: map[uint16]model.ItemDef{411: amountItemDef(411, 256)},
			item:  model.Item{Index: 411},
			want:  "fora do wire",
		},
		{
			name:  "no dynamic effect slot",
			items: map[uint16]model.ItemDef{411: amountItemDef(411, 10)},
			item:  model.Item{Index: 411, Eff: [6]byte{1, 1, 2, 2, 3, 3}},
			want:  "sem par de efeito livre",
		},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			w := &World{items: tt.items, npcs: []model.NPCDef{{Name: "TestShop", Vende: []model.Item{tt.item}}}}
			if err := w.initShopItemDefaults(); err == nil || !strings.Contains(err.Error(), tt.want) {
				t.Fatalf("erro=%v, esperado conter %q", err, tt.want)
			}
		})
	}
}

func TestInitShopItemDefaultsIgnoresSyntheticSkillStock(t *testing.T) {
	w := &World{npcs: []model.NPCDef{{
		Name:  "SkillMaster",
		Vende: []model.Item{{Index: 5000, Eff: [6]byte{1, 2}}},
	}}}
	want := w.npcs[0].Vende[0]
	if err := w.initShopItemDefaults(); err != nil {
		t.Fatalf("estoque sintetico nao deveria derrubar boot: %v", err)
	}
	if got := w.npcs[0].Vende[0]; got != want {
		t.Fatalf("estoque sintetico foi alterado: got=%+v want=%+v", got, want)
	}
}

func TestBuyItemKeepsSeededAmountAndAssignsUID(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	def := amountItemDef(411, 10)
	def.Price = 15_000
	w.items[411] = def
	w.npcs = []model.NPCDef{{
		Name: "Aki", Tipo: model.TipoNPC,
		Extended: &model.ExtendedScore{Merchant: nativeShopMerchant},
		Vende:    []model.Item{{Index: 411}},
	}}
	if err := w.initShopItemDefaults(); err != nil {
		t.Fatalf("initShopItemDefaults: %v", err)
	}
	shop := &Mob{ID: 1100, X: 2101, Y: 2100, Def: &w.npcs[0]}
	w.registerMobSpatial(shop)
	p.show(shop.ID)
	p.ShopNPC = shop.ID
	p.Char.Gold = 20_000

	buy := make([]byte, 24)
	binary.LittleEndian.PutUint16(buy[14:16], 0)
	w.onBuyItem(p.Session, buy)

	item := p.Char.Inv[0]
	if item.Index != 411 || item.UID == "" {
		t.Fatalf("compra nao materializou item/UID: %+v", item)
	}
	if amount, ok := itemEffectValue(item, effectAmount); !ok || amount != 10 {
		t.Fatalf("compra perdeu EF_AMOUNT=10: %+v", item.Eff)
	}
	if p.Char.Gold != 5_000 || st.saves != 1 {
		t.Fatalf("persistencia/gold incorretos: gold=%d saves=%d", p.Char.Gold, st.saves)
	}
}

func TestAuthoritativeNPCShopsResolveMissingItemListAmounts(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(
		filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"),
		filepath.Join(root, "SkillData.csv"),
	)
	if err != nil {
		t.Fatalf("LoadCatalog: %v", err)
	}
	npcs, err := data.LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatalf("LoadNPCs: %v", err)
	}
	w := &World{items: catalog.Items, npcs: npcs}
	if err := w.initShopItemDefaults(); err != nil {
		t.Fatalf("estoque real invalido: %v", err)
	}

	foundRecall := false
	checked := 0
	for _, npc := range w.npcs {
		for _, item := range npc.Vende {
			def, ok := w.items[item.Index]
			if !ok {
				continue
			}
			want := staticEffect(def, "EF_AMOUNT")
			if want == 0 {
				continue
			}
			got, present := itemEffectValue(item, effectAmount)
			if !present || got == 0 {
				t.Fatalf("NPC %q vende item %d sem EF_AMOUNT materializado", npc.Name, item.Index)
			}
			checked++
			if item.Index == 411 && got == 10 {
				foundRecall = true
			}
		}
	}
	if checked == 0 || !foundRecall {
		t.Fatalf("cobertura real insuficiente: itens=%d recall411=%v", checked, foundRecall)
	}
}
