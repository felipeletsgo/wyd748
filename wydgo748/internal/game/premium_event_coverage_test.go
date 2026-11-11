package game

import (
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

func TestHandlePremiumEventClearsPollAndInstallsOnlyChangedCatalog(t *testing.T) {
	const availableItem = uint16(501)
	w := &World{
		items: map[uint16]model.ItemDef{availableItem: {Index: availableItem}},
		premiumCatalog: store.PremiumCatalog{Version: 1, Entries: []store.PremiumCatalogEntry{
			{Slot: 1, Item: model.Item{Index: availableItem}, Price: 5},
		}},
	}
	w.premiumPolling.Store(true)
	changed := store.PremiumCatalog{Version: 2, Entries: []store.PremiumCatalogEntry{
		{Slot: 1, Item: model.Item{Index: availableItem}, Price: 7},
		{Slot: 2, Item: model.Item{Index: availableItem + 1}, Price: 9},
	}}
	w.handlePremiumEvent(&premiumEvent{catalog: &changed})
	if w.premiumPolling.Load() || w.premiumCatalog.Version != 2 || len(w.premiumCatalog.Entries) != 1 ||
		w.premiumCatalog.Entries[0].Price != 7 {
		t.Fatalf("changed premium catalog was not installed and filtered: polling=%t catalog=%+v",
			w.premiumPolling.Load(), w.premiumCatalog)
	}

	sameVersion := store.PremiumCatalog{Version: 2, Entries: []store.PremiumCatalogEntry{
		{Slot: 3, Item: model.Item{Index: availableItem}, Price: 99},
	}}
	w.premiumPolling.Store(true)
	w.handlePremiumEvent(&premiumEvent{catalog: &sameVersion})
	if w.premiumPolling.Load() || w.premiumCatalog.Entries[0].Price != 7 {
		t.Fatal("same-version poll replaced the installed catalog or left polling active")
	}

	w.premiumPolling.Store(true)
	w.handlePremiumEvent(&premiumEvent{credits: []store.DonateCredit{{ID: 17, Account: "offline", Amount: 2}}})
	if w.premiumPolling.Load() || w.premiumInFlight == nil || len(w.premiumInFlight) != 0 {
		t.Fatal("offline credit event failed to initialize pending-credit tracking safely")
	}
	w.premiumPolling.Store(true)
	w.handlePremiumEvent(&premiumEvent{pollFailed: true})
	if w.premiumPolling.Load() {
		t.Fatal("failed poll left the world marked as polling")
	}
	w.premiumPolling.Store(true)
	w.handlePremiumEvent(&premiumEvent{})
	if !w.premiumPolling.Load() {
		t.Fatal("empty event cleared a poll that had no result")
	}
}
