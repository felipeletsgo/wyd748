package npccontent

import (
	"encoding/json"
	"testing"

	"wydgo/internal/model"
)

func editableNPCFixture() model.NPCDef {
	return model.NPCDef{Name: "Test_Shop", Tipo: model.TipoNPC,
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100}}
}

func marshalNPC(t *testing.T, def model.NPCDef) []byte {
	t.Helper()
	raw, err := json.Marshal(def)
	if err != nil {
		t.Fatal(err)
	}
	return raw
}

func cloneEditableNPC(def model.NPCDef) model.NPCDef {
	clone := def
	if def.Score != nil {
		score := *def.Score
		clone.Score = &score
	}
	if def.ClientMerchant != nil {
		selector := *def.ClientMerchant
		clone.ClientMerchant = &selector
	}
	return clone
}

func TestDecodeRejectsUnknownAndTrailingContent(t *testing.T) {
	valid := marshalNPC(t, editableNPCFixture())
	if def, err := Decode(valid); err != nil || def.Name != "Test_Shop" {
		t.Fatalf("valid NPC definition decode = %+v, %v", def, err)
	}
	for _, raw := range [][]byte{
		[]byte(`{"name":"Test_Shop","score":{},"unknown":true}`),
		append(append([]byte(nil), valid...), []byte(` {}`)...),
		append(append([]byte(nil), valid...), []byte(` trailing`)...),
		[]byte(`{`),
	} {
		if _, err := Decode(raw); err == nil {
			t.Fatalf("accepted unknown, malformed, or trailing JSON: %s", raw)
		}
	}
}

func TestValidateEditableEnforcesIdentityContractAndSeedsStock(t *testing.T) {
	source := editableNPCFixture()
	items := map[uint16]model.ItemDef{
		1:  {Index: 1, StaticEffects: []model.StaticEffect{{Name: "EF_AMOUNT", Value: 12}}},
		57: {Index: 57},
		58: {Index: 58},
	}
	candidate := cloneEditableNPC(source)
	candidate.Vende = []model.Item{{Index: 1}}
	validated, err := ValidateEditable(marshalNPC(t, candidate), source, items)
	if err != nil {
		t.Fatalf("valid editable shop rejected: %v", err)
	}
	if validated.Vende[0].Eff != [6]byte{ShopAmountEffect, 12} {
		t.Fatalf("omitted stock amount was not seeded: %+v", validated.Vende[0])
	}

	for _, test := range []struct {
		name   string
		mutate func(*model.NPCDef, *model.NPCDef)
	}{
		{name: "identity", mutate: func(_ *model.NPCDef, next *model.NPCDef) { next.Name = "Other" }},
		{name: "shop size", mutate: func(_ *model.NPCDef, next *model.NPCDef) {
			next.Vende = make([]model.Item, MaxShopItems+1)
			for i := range next.Vende {
				next.Vende[i].Index = 1
			}
		}},
		{name: "service identity", mutate: func(_ *model.NPCDef, next *model.NPCDef) { next.ServiceIdentity = "other-service" }},
		{name: "combat role", mutate: func(_ *model.NPCDef, next *model.NPCDef) { next.CombatRole = "city_guard" }},
		{name: "type", mutate: func(_ *model.NPCDef, next *model.NPCDef) { next.Tipo = model.TipoMonstro }},
		{name: "merchant selector", mutate: func(_ *model.NPCDef, next *model.NPCDef) { next.Score.Merchant++ }},
		{name: "introduce client selector", mutate: func(_ *model.NPCDef, next *model.NPCDef) { selector := byte(2); next.ClientMerchant = &selector }},
		{name: "remove client selector", mutate: func(source, _ *model.NPCDef) { selector := byte(2); source.ClientMerchant = &selector }},
		{name: "change client selector", mutate: func(source, next *model.NPCDef) {
			old, changed := byte(1), byte(2)
			source.ClientMerchant, next.ClientMerchant = &old, &changed
		}},
		{name: "service face", mutate: func(source, next *model.NPCDef) { source.Equip.Rosto.Index, next.Equip.Rosto.Index = 57, 58 }},
		{name: "live item blueprint", mutate: func(_ *model.NPCDef, next *model.NPCDef) {
			next.Vende = []model.Item{{Index: 1, ActivatedUnix: 1}}
		}},
	} {
		t.Run(test.name, func(t *testing.T) {
			baseline, edited := cloneEditableNPC(source), cloneEditableNPC(source)
			test.mutate(&baseline, &edited)
			if _, err := ValidateEditable(marshalNPC(t, edited), baseline, items); err == nil {
				t.Fatal("invalid live edit was accepted")
			}
		})
	}
}

func TestSeedShopItemAmountBoundaries(t *testing.T) {
	withAmount := model.ItemDef{Index: 1, StaticEffects: []model.StaticEffect{{Name: "EF_AMOUNT", Value: 10}}}
	withoutAmount := model.ItemDef{Index: 1}
	for _, test := range []struct {
		name    string
		item    model.Item
		def     model.ItemDef
		want    [6]byte
		wantErr bool
	}{
		{name: "nil-like zero item", item: model.Item{}, def: withAmount},
		{name: "no configured amount", item: model.Item{Index: 1}, def: withoutAmount},
		{name: "negative amount", item: model.Item{Index: 1}, def: model.ItemDef{StaticEffects: []model.StaticEffect{{Name: "EF_AMOUNT", Value: -1}}}, wantErr: true},
		{name: "oversized amount", item: model.Item{Index: 1}, def: model.ItemDef{StaticEffects: []model.StaticEffect{{Name: "EF_AMOUNT", Value: 256}}}, wantErr: true},
		{name: "seed first empty pair", item: model.Item{Index: 1, Eff: [6]byte{7, 2}}, def: withAmount, want: [6]byte{7, 2, ShopAmountEffect, 10}},
		{name: "replace empty amount pair", item: model.Item{Index: 1, Eff: [6]byte{ShopAmountEffect, 0}}, def: withAmount, want: [6]byte{ShopAmountEffect, 10}},
		{name: "preserve explicit stack", item: model.Item{Index: 1, Eff: [6]byte{ShopAmountEffect, 4}}, def: withAmount, want: [6]byte{ShopAmountEffect, 4}},
		{name: "no free pair", item: model.Item{Index: 1, Eff: [6]byte{1, 1, 2, 2, 3, 3}}, def: withAmount, want: [6]byte{1, 1, 2, 2, 3, 3}, wantErr: true},
	} {
		t.Run(test.name, func(t *testing.T) {
			item := test.item
			err := SeedShopItemAmount(&item, test.def)
			if (err != nil) != test.wantErr || item.Eff != test.want {
				t.Fatalf("seed result eff=%v err=%v, want eff=%v error=%t", item.Eff, err, test.want, test.wantErr)
			}
		})
	}
	if err := SeedShopItemAmount(nil, withAmount); err != nil {
		t.Fatalf("nil item should be ignored: %v", err)
	}
}
