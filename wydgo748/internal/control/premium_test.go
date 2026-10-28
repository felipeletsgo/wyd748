package control

import "testing"

func TestPremiumCatalogCategoryValidation(t *testing.T) {
	command := PremiumCatalogCommand{Version: 1, Reason: "Classify store items", Items: []PremiumCatalogItem{{Slot: 2, ItemID: 400, Price: 1000}}}
	for category := uint8(0); category < 4; category++ {
		command.Items[0].Category = category
		if err := command.Validate(); err != nil {
			t.Fatalf("category %d rejected: %v", category, err)
		}
	}
	for _, category := range []uint8{4, 255} {
		command.Items[0].Category = category
		if command.Validate() == nil {
			t.Fatalf("unknown category %d accepted", category)
		}
	}
}
