package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
)

func volatileTransformWorld() (*World, *Player, *craftStore) {
	p, _ := networkedTestPlayer(1, "Indexed", 2100, 2100)
	st := &craftStore{}
	w := worldWithNetworkedPlayers(p)
	w.store = st
	w.volatiles = model.VolatileCatalog{
		Default: model.VolatileRule{Action: "generic"},
		Items:   make(map[uint16]model.VolatileRule), ItemCodes: make(map[uint16]int),
	}
	w.items = map[uint16]model.ItemDef{}
	return w, p, st
}

func registerTransformRule(w *World, item uint16, code int, rule model.VolatileRule) {
	w.volatiles.ItemCodes[item] = code
	w.volatiles.Items[item] = rule
}

func indexedUsePacket(source, target uint32, targetType uint32) []byte {
	pkt := useItemPacket(source, target)
	binary.LittleEndian.PutUint32(pkt[20:24], targetType)
	return pkt
}

func TestEquipmentGemsApplyAllFourVariants(t *testing.T) {
	for variant := 0; variant < 4; variant++ {
		t.Run([]string{"diamond", "emerald", "coral", "garnet"}[variant], func(t *testing.T) {
			w, p, st := volatileTransformWorld()
			gem := uint16(3386 + variant)
			registerTransformRule(w, gem, 180+variant,
				model.VolatileRule{Action: "equipment_gem", Consume: true, Variant: variant})
			w.items[gem] = model.ItemDef{Index: gem}
			w.items[1000] = model.ItemDef{Index: 1000, Pos: 2}
			p.Char.Inv[0] = model.Item{Index: gem, UID: "gem"}
			p.Char.Equip[1] = model.Item{Index: 1000, UID: "gear", Eff: [6]byte{43, 238}}

			w.onUseItem(p.Session, indexedUsePacket(0, 1, placeEquip))
			raw, _ := sancRaw(p.Char.Equip[1])
			if p.Char.Inv[0].Index != 0 || p.Char.Equip[1].Index != 1000 ||
				p.Char.Equip[1].UID != "gear" || raw != byte(238+variant) || st.saves != 1 {
				t.Fatalf("gema %d: source=%+v target=%+v raw=%d saves=%d",
					variant, p.Char.Inv[0], p.Char.Equip[1], raw, st.saves)
			}
		})
	}
}

func TestEquipmentGemMigratesWeaponFamilyAndPreservesUID(t *testing.T) {
	w, p, _ := volatileTransformWorld()
	registerTransformRule(w, 3389, 183,
		model.VolatileRule{Action: "equipment_gem", Consume: true, Variant: 3})
	w.items[3389] = model.ItemDef{Index: 3389}
	w.items[2000] = model.ItemDef{Index: 2000, Pos: 64, Grade: 5}
	w.items[2003] = model.ItemDef{Index: 2003, Pos: 64, Grade: 8}
	p.Char.Inv[0] = model.Item{Index: 3389}
	p.Char.Equip[6] = model.Item{Index: 2000, UID: "weapon-uid", Eff: [6]byte{43, 250}}

	w.onUseItem(p.Session, indexedUsePacket(0, 6, placeEquip))
	raw, _ := sancRaw(p.Char.Equip[6])
	if p.Char.Equip[6].Index != 2003 || p.Char.Equip[6].UID != "weapon-uid" || raw != 253 {
		t.Fatalf("arma nao migrou corretamente: %+v raw=%d", p.Char.Equip[6], raw)
	}
}

func TestEquipmentGemAcceptsAncientWeaponBelowPlusTen(t *testing.T) {
	w, p, st := volatileTransformWorld()
	registerTransformRule(w, 3387, 181,
		model.VolatileRule{Action: "equipment_gem", Consume: true, Variant: 1})
	w.items[3387] = model.ItemDef{Index: 3387}
	w.items[2100] = model.ItemDef{Index: 2100, Pos: 192, Grade: 5}
	w.items[2101] = model.ItemDef{Index: 2101, Pos: 192, Grade: 6}
	p.Char.Inv[0] = model.Item{Index: 3387}
	p.Char.Equip[6] = model.Item{Index: 2100, UID: "ancient", Eff: [6]byte{43, 7}}

	w.onUseItem(p.Session, indexedUsePacket(0, 6, placeEquip))
	if p.Char.Inv[0].Index != 0 || p.Char.Equip[6].Index != 2101 ||
		p.Char.Equip[6].UID != "ancient" || p.Char.Equip[6].Eff[1] != 7 || st.saves != 1 {
		t.Fatalf("arma Ancient +7 nao recebeu gema: source=%+v target=%+v saves=%d",
			p.Char.Inv[0], p.Char.Equip[6], st.saves)
	}
}

func TestEquipmentGemRejectsInvalidTargetAndRollsBackSave(t *testing.T) {
	t.Run("below +10", func(t *testing.T) {
		w, p, st := volatileTransformWorld()
		registerTransformRule(w, 3386, 180,
			model.VolatileRule{Action: "equipment_gem", Consume: true})
		w.items[3386] = model.ItemDef{Index: 3386}
		w.items[1000] = model.ItemDef{Index: 1000, Pos: 2}
		p.Char.Inv[0] = model.Item{Index: 3386}
		p.Char.Equip[1] = model.Item{Index: 1000, Eff: [6]byte{43, 9}}
		w.onUseItem(p.Session, indexedUsePacket(0, 1, placeEquip))
		if p.Char.Inv[0].Index != 3386 || st.saves != 0 {
			t.Fatal("gema invalida foi consumida")
		}
	})

	t.Run("save failure", func(t *testing.T) {
		w, p, st := volatileTransformWorld()
		registerTransformRule(w, 3387, 181,
			model.VolatileRule{Action: "equipment_gem", Consume: true, Variant: 1})
		w.items[3387] = model.ItemDef{Index: 3387}
		w.items[1000] = model.ItemDef{Index: 1000, Pos: 2}
		p.Char.Inv[0] = model.Item{Index: 3387, UID: "gem"}
		p.Char.Equip[1] = model.Item{Index: 1000, UID: "gear", Eff: [6]byte{43, 230}}
		st.err = errors.New("database unavailable")
		w.onUseItem(p.Session, indexedUsePacket(0, 1, placeEquip))
		if p.Char.Inv[0].Index != 3387 || p.Char.Equip[1].Eff[1] != 230 ||
			p.Char.Equip[1].UID != "gear" {
			t.Fatalf("rollback incompleto: source=%+v target=%+v", p.Char.Inv[0], p.Char.Equip[1])
		}
	})
}

func TestAdamantiteSuccessFailureAndValidation(t *testing.T) {
	setup := func(rng int) (*World, *Player, *craftStore) {
		w, p, st := volatileTransformWorld()
		w.rng = fixedRNG{value: rng}
		registerTransformRule(w, 578, 9, model.VolatileRule{
			Action: "ore_upgrade", Consume: true, Variant: 3, SuccessPercent: 50,
		})
		w.items[578] = model.ItemDef{Index: 578}
		w.items[3000] = model.ItemDef{Index: 3000, Pos: 2, Grade: 3, Unique: 8, Extra: 3001}
		w.items[3001] = model.ItemDef{Index: 3001, Pos: 2, Grade: 4, Unique: 8}
		p.Char.Inv[0] = model.Item{Index: 578, UID: "ore"}
		p.Char.Inv[1] = model.Item{Index: 3000, UID: "same-item", Eff: [6]byte{43, 9}}
		return w, p, st
	}

	t.Run("success", func(t *testing.T) {
		w, p, st := setup(0)
		w.onUseItem(p.Session, indexedUsePacket(0, 1, placeInv))
		if p.Char.Inv[0].Index != 0 || p.Char.Inv[1].Index != 3001 ||
			p.Char.Inv[1].UID != "same-item" || p.Char.Inv[1].Eff[1] != 9 || st.saves != 1 {
			t.Fatalf("Adamantita sucesso: source=%+v target=%+v saves=%d",
				p.Char.Inv[0], p.Char.Inv[1], st.saves)
		}
	})

	t.Run("failure consumes only ore", func(t *testing.T) {
		w, p, st := setup(99)
		w.onUseItem(p.Session, indexedUsePacket(0, 1, placeInv))
		if p.Char.Inv[0].Index != 0 || p.Char.Inv[1].Index != 3000 || st.saves != 1 {
			t.Fatalf("Adamantita falha: source=%+v target=%+v saves=%d",
				p.Char.Inv[0], p.Char.Inv[1], st.saves)
		}
	})

	t.Run("invalid unique does not consume", func(t *testing.T) {
		w, p, st := setup(0)
		def := w.items[3000]
		def.Unique = 7
		w.items[3000] = def
		w.onUseItem(p.Session, indexedUsePacket(0, 1, placeInv))
		if p.Char.Inv[0].Index != 578 || p.Char.Inv[1].Index != 3000 || st.saves != 0 {
			t.Fatal("Adamantita aceitou familia incompatível")
		}
	})

	t.Run("Extra must point to matching Legend variant", func(t *testing.T) {
		w, p, st := setup(0)
		legend := w.items[3001]
		legend.Pos = 4
		w.items[3001] = legend
		w.onUseItem(p.Session, indexedUsePacket(0, 1, placeInv))
		if p.Char.Inv[0].Index != 578 || p.Char.Inv[1].Index != 3000 || st.saves != 0 {
			t.Fatal("Adamantita aceitou Extra de outra familia de equipamento")
		}
	})

	t.Run("save failure rolls back both items", func(t *testing.T) {
		w, p, st := setup(0)
		st.err = errors.New("database unavailable")
		w.onUseItem(p.Session, indexedUsePacket(0, 1, placeInv))
		if p.Char.Inv[0].Index != 578 || p.Char.Inv[1].Index != 3000 {
			t.Fatalf("rollback incompleto: source=%+v target=%+v", p.Char.Inv[0], p.Char.Inv[1])
		}
	})
}
