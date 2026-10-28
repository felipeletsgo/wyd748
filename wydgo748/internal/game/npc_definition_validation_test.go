package game

import (
	"encoding/json"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/npccontent"
)

func TestNPCDefinitionValidationBoundaries(t *testing.T) {
	defs, err := data.LoadNPCs("../../data/npcs/Aki.json")
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	defs[0].Vende = []model.Item{{Index: 411}}
	for _, tc := range []struct {
		name  string
		edit  func(*model.NPCDef)
		valid bool
	}{
		{"27 visible", func(d *model.NPCDef) {
			d.Vende = make([]model.Item, 27)
			for i := range d.Vende {
				d.Vende[i].Index = 411
			}
		}, true},
		{"28 visible", func(d *model.NPCDef) {
			d.Vende = make([]model.Item, 28)
			for i := range d.Vende {
				d.Vende[i].Index = 411
			}
		}, false},
		{"64 sparse", func(d *model.NPCDef) { d.Vende = make([]model.Item, 64); d.Vende[63].Index = 411 }, true},
		{"65 slots", func(d *model.NPCDef) { d.Vende = make([]model.Item, 65) }, false},
		{"identity", func(d *model.NPCDef) { d.Name = "Other" }, false},
		{"merchant", func(d *model.NPCDef) { d.Score.Merchant++ }, false},
		{"client selector", func(d *model.NPCDef) { v := byte(11); d.ClientMerchant = &v }, false},
		{"no score", func(d *model.NPCDef) { d.Score = nil }, false},
		{"unknown item", func(d *model.NPCDef) { d.Vende[0].Index = 6499; delete(catalog.Items, 6499) }, false},
		{"instance UID", func(d *model.NPCDef) { d.Vende[0].UID = "instance" }, false},
		{"activated instance", func(d *model.NPCDef) { d.Vende[0].ActivatedUnix = 1 }, false},
		{"expiring instance", func(d *model.NPCDef) { d.Vende[0].ExpiresUnix = 1 }, false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			d := data.CloneNPCs(defs)[0]
			tc.edit(&d)
			raw, err := json.Marshal(d)
			if err != nil {
				t.Fatal(err)
			}
			_, err = npccontent.ValidateEditable(raw, defs[0], catalog.Items)
			if (err == nil) != tc.valid {
				t.Fatalf("valid=%t error=%v", tc.valid, err)
			}
		})
	}
	raw, _ := json.Marshal(defs[0])
	for _, invalid := range [][]byte{append(append([]byte(nil), raw...), []byte(" {}")...), []byte(`{"name":"Aki","unrecognized":true}`), []byte(`null`)} {
		if _, err := npccontent.ValidateEditable(invalid, defs[0], catalog.Items); err == nil {
			t.Fatalf("accepted malformed or unknown content: %s", invalid)
		}
	}
}
