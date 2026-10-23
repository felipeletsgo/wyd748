package game

import (
	"encoding/binary"
	"path/filepath"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

// Lactolerium 100% (item 3022) allows exactly what Lac allows and succeeds
// on every one of those steps, including the earring branch, which therefore
// never breaks.
var perfectLac = model.VolatileRule{Action: "refine", Consume: true, RefineMax: 9, RefineTenChance: 100, RefineChance: 100}

func TestPerfectLactoleriumChanceCoversEveryLacStep(t *testing.T) {
	sword := model.ItemDef{Index: 811, Pos: 64}
	for level := 0; level <= 11; level++ {
		item := model.Item{Index: 811, Eff: [6]byte{43, byte(level)}}
		if level >= 10 {
			item.Eff[1] = byte(230 + (level-10)*4)
		}
		_, lacAllowed := equipmentPowderChance(item, sword, placeEquip, 6, 5,
			model.VolatileRule{Action: "refine", RefineMax: 9, RefineTenChance: 5})
		chance, allowed := equipmentPowderChance(item, sword, placeEquip, 6, 5, perfectLac)
		if allowed != lacAllowed {
			t.Fatalf("+%d: allowed=%t, Lac allows %t", level, allowed, lacAllowed)
		}
		if allowed && chance != 100 {
			t.Fatalf("+%d: chance=%d, want 100", level, chance)
		}
	}
	earring := model.ItemDef{Index: 591, Pos: 1 << 8}
	for _, level := range []int{9, 10} {
		item := model.Item{Index: 591, Eff: [6]byte{43, byte(level)}}
		if level == 10 {
			item.Eff[1] = 230
		}
		if chance, allowed := equipmentPowderChance(item, earring, placeEquip, 8, 5, perfectLac); !allowed || chance != 100 {
			t.Fatalf("earring +%d: chance=%d allowed=%t, want 100", level, chance, allowed)
		}
	}
}

func TestPerfectLactoleriumNeverFailsOrBreaksTheEarring(t *testing.T) {
	w, p, st := useItemWorld(perfectLac)
	w.rng = fixedRNG{value: 99} // the worst roll still succeeds
	w.items[3022] = model.ItemDef{Index: 3022}
	w.volatiles.Items[3022], w.volatiles.ItemCodes[3022] = perfectLac, 5
	w.items[591] = model.ItemDef{Index: 591, Pos: 1 << 8}
	p.Char.Inv[0] = model.Item{Index: 3022, UID: "perfect-lac"}
	p.Char.Equip[8] = model.Item{Index: 591, UID: "earring", Eff: [6]byte{43, 9}}
	packet := useItemPacket(0, 8)
	binary.LittleEndian.PutUint32(packet[20:24], placeEquip)
	w.onUseItem(p.Session, packet)
	if after := p.Char.Equip[8]; after.Index != 591 || itemSanc(after) != 10 || p.Char.Inv[0].Index != 0 || st.saves != 1 {
		t.Fatalf("earring=%+v powder=%+v saves=%d, want +10 and the stone consumed", after, p.Char.Inv[0], st.saves)
	}
}

func TestPerfectLactoleriumCatalogEntry(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"), filepath.Join(root, "Itemname.csv"),
		filepath.Join(root, "ItemEffect.h"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances(filepath.Join(root, "volatiles.json"),
		filepath.Join(root, "instances.json"), catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	rule, code, ok := volatiles.Rule(3022)
	if !ok || code != 5 || rule.Action != "refine" || !rule.Consume || rule.RefineMax != 9 ||
		rule.RefineTenChance != 100 || rule.RefineChance != 100 {
		t.Fatalf("3022 rule=%+v code=%d ok=%t", rule, code, ok)
	}
	def := catalog.Items[3022]
	fury := catalog.Items[3020]
	if def.Name != "Lactolerium_100%" || def.Mesh != fury.Mesh {
		t.Fatalf("3022=%+v, want Lactolerium_100%% with the Fury Stone mesh %+v", def, fury)
	}
	if lac, _, _ := volatiles.Rule(413); lac.RefineChance != 0 {
		t.Fatal("ordinary Lac must keep the native rates")
	}
}
