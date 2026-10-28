package data

import (
	"testing"

	"wydgo/internal/model"
)

func TestClientMerchantValidation(t *testing.T) {
	for _, tc := range []struct {
		name string
		kind string
		ui   byte
		want bool
	}{
		{"explicit-neutral", model.TipoNPC, 0, true},
		{"maximum-selector", model.TipoNPC, 15, true},
		{"direction-bits-rejected", model.TipoNPC, 16, false},
		{"monster-override-rejected", model.TipoMonstro, 11, false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			npcs := []model.NPCDef{{Name: "Selector", Tipo: tc.kind, ClientMerchant: &tc.ui,
				Score: &model.Score{Version: model.ScoreVersion}}}
			if err := validateNPCs("test", npcs); (err == nil) != tc.want {
				t.Fatalf("validateNPCs = %v, want valid=%v", err, tc.want)
			}
		})
	}
}
