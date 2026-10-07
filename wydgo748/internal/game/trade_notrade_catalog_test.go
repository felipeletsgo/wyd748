package game

import (
	"path/filepath"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

// Items the client marks untradeable (effect 111, EF_NOTRADE) must be refused
// by the authoritative trade validation with the real server catalog.
func TestTradeRefusesNativeNoTradeItemsFromCatalog(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	w := &World{items: catalog.Items}
	var items [maxTradeItems]model.Item
	for _, index := range []uint16{3980, 4016, 4106} { // timed horse, Repliction, quest crystal
		items[0] = model.Item{Index: index}
		if err := w.validateTradableItems(items); err == nil || err.Error() != "This item cannot be traded." {
			t.Errorf("item %d: expected the non-tradable refusal, got %v", index, err)
		}
	}
	items[0] = model.Item{Index: 1101} // ordinary armor stays tradable
	if err := w.validateTradableItems(items); err != nil {
		t.Fatalf("tradable item was refused: %v", err)
	}
}
