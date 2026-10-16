package game

import (
	"errors"
	"maps"
	"testing"

	"wydgo/internal/model"
)

// Warrior's Seal gives a level-355 Mortal the Elite cape of its kingdom, or the
// white Elite of Adventurer without a kingdom (W2PP Warrior's Seal).
func TestWarriorSealGrantsEliteCape(t *testing.T) {
	for _, tc := range []struct {
		name      string
		evolution string
		level     uint32
		cape      uint16
		want      uint16
	}{
		{"no cape", "", 354, 0, 3193},
		{"white basic cape", "", 354, 548, 3193},
		{"white knight cape", "", 354, 549, 3193},
		{"blue kingdom", "", 400, 545, 3191},
		{"red kingdom", "", 354, 544, 3192},
		{"already elite", "", 354, 3192, 3192},
		{"hero kept", "", 354, 3194, 3194},
		{"below 355", "", 353, 545, 545},
		{"arch", "arch", 400, 545, 545},
	} {
		t.Run(tc.name, func(t *testing.T) {
			rule := model.VolatileRule{Action: "grant_counter", Consume: true, Counters: map[string]uint32{fameCounter: 10}}
			w, p, _ := useItemWorld(rule)
			w.items[warriorSealItem] = model.ItemDef{Index: warriorSealItem}
			w.volatiles.Items[warriorSealItem], w.volatiles.ItemCodes[warriorSealItem] = rule, 199
			p.Char.Evolution = tc.evolution
			p.Char.Score.Level = tc.level
			p.Char.Equip[model.CapeSlot] = model.Item{}
			if tc.cape != 0 {
				p.Char.Equip[model.CapeSlot] = model.Item{Index: tc.cape, UID: "cape-uid", Eff: [6]byte{43, 9}}
			}
			p.Char.Inv[0] = model.Item{Index: warriorSealItem}
			w.onUseItem(p.Session, useItemPacket(0, 0))
			cape := p.Char.Equip[model.CapeSlot]
			if cape.Index != tc.want || counterBalance(p, fameCounter) != 10 || p.Char.Inv[0].Index != 0 {
				t.Fatalf("cape=%+v fame=%d seal=%+v", cape, counterBalance(p, fameCounter), p.Char.Inv[0])
			}
			if tc.want != tc.cape && tc.cape != 0 && (cape.UID != "cape-uid" || cape.Eff != [6]byte{}) {
				t.Fatalf("elite cape lost identity or kept effects: %+v", cape)
			}
			if cape.Index != 0 && cape.UID == "" {
				t.Fatal("the granted cape has no server identity")
			}
			w.onUseItem(p.Session, useItemPacket(0, 0))
			if p.Char.Equip[model.CapeSlot] != cape || counterBalance(p, fameCounter) != 10 {
				t.Fatal("replaying an exhausted seal granted another reward")
			}
		})
	}
}

func TestWarriorSealRollsBackFailedSave(t *testing.T) {
	rule := model.VolatileRule{Action: "grant_counter", Consume: true, Counters: map[string]uint32{fameCounter: 10}}
	w, p, backend := useItemWorld(rule)
	w.items[warriorSealItem] = model.ItemDef{Index: warriorSealItem}
	w.volatiles.Items[warriorSealItem], w.volatiles.ItemCodes[warriorSealItem] = rule, 199
	p.Char.Score.Level = 354
	p.Char.Inv[0] = model.Item{Index: warriorSealItem, UID: "seal"}
	p.Char.Equip[model.CapeSlot] = model.Item{Index: 544, UID: "red-cape", Eff: [6]byte{43, 9}}
	beforeItem, beforeCape, beforeCounters := p.Char.Inv[0], p.Char.Equip[model.CapeSlot], copyCounters(p)
	backend.err = errors.New("save unavailable")
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0] != beforeItem || p.Char.Equip[model.CapeSlot] != beforeCape || !maps.Equal(p.SpecialCoins, beforeCounters) {
		t.Fatal("failed seal save consumed the seal, granted fame or replaced the cape")
	}
}
