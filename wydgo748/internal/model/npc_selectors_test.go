package model

import "testing"

func TestNPCSourceSelectorsPreserveStaffContent(t *testing.T) {
	selector := byte(11)
	source := NPCDef{Name: "Treinador_II", Tipo: TipoNPC, ClientMerchant: &selector, Score: &Score{Merchant: 40, MaxHP: 100}}
	override := source
	override.ClientMerchant = nil // Override saved before the client selector existed.
	override.Score = &Score{Merchant: 43, MaxHP: 250}
	override.Vende = []Item{{Index: 411}}
	merged := override.WithSourceSelectors(source)
	if merged.ClientMerchant == nil || *merged.ClientMerchant != 11 || merged.Score.Merchant != 40 || merged.Score.MaxHP != 250 || merged.Vende[0].Index != 411 {
		t.Fatalf("invalid rebase: %+v", merged)
	}
	*merged.ClientMerchant = 12
	merged.Score.MaxHP = 300
	if selector != 11 || override.Score.MaxHP != 250 || source.Score.MaxHP != 100 {
		t.Fatal("rebase mutated its inputs")
	}
	source.ClientMerchant = nil
	if got := merged.WithSourceSelectors(source); got.ClientMerchant != nil {
		t.Fatal("removed source selector survived rebase")
	}
	source.Equip.Rosto.Index = 57
	if got := merged.WithSourceSelectors(source); got.Equip.Rosto.Index != 57 {
		t.Fatal("premium service identity did not follow source")
	}
}

func TestNPCServiceFacesFollowSourceWithoutLosingStaffEdits(t *testing.T) {
	for _, tc := range []struct {
		name                  string
		merchant              uint32
		sourceFace, staffFace uint16
		locked                bool
	}{
		{"air transport removed", 7, 63, 60, true},
		{"air transport introduced", 7, 60, 63, true},
		{"air transport with high merchant bits", 23, 63, 60, true},
		{"premium removed", 0, 57, 60, true},
		{"premium introduced", 0, 60, 57, true},
		{"cosmetic face", 1, 60, 61, false},
		{"face 63 without air transport", 1, 63, 60, false},
		{"unchanged air transport", 7, 63, 63, false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			source := NPCDef{Name: "Service", Tipo: TipoNPC, Score: &Score{Merchant: tc.merchant, MaxHP: 100}}
			source.Equip.Rosto.Index = tc.sourceFace
			override := source
			override.Equip.Rosto.Index = tc.staffFace
			override.DisplayName = "Staff label"
			override.Score = &Score{Merchant: tc.merchant, MaxHP: 250}
			override.Vende = []Item{{Index: 411}}
			if source.ServiceFaceChanged(override) != tc.locked {
				t.Fatal("incorrect live service selector protection")
			}
			merged := override.WithSourceSelectors(source)
			wantFace := tc.staffFace
			if tc.locked {
				wantFace = tc.sourceFace
			}
			if merged.Equip.Rosto.Index != wantFace || merged.DisplayName != "Staff label" || merged.Score.MaxHP != 250 || merged.Vende[0].Index != 411 {
				t.Fatalf("service repair lost staff content: %+v", merged)
			}
			if source.Equip.Rosto.Index != tc.sourceFace || override.Equip.Rosto.Index != tc.staffFace {
				t.Fatal("rebase mutated source or override")
			}
		})
	}
}
