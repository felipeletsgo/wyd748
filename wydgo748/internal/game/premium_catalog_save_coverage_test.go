package game

import (
	"context"
	"errors"
	"testing"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

type premiumCatalogSaveCoverageStore struct {
	*premiumTestStore
	loaded    store.PremiumCatalog
	loadErr   error
	saved     store.PremiumCatalog
	saveErr   error
	saveCalls int
	entries   []store.PremiumCatalogEntry
	actor     string
}

func (s *premiumCatalogSaveCoverageStore) LoadPremiumCatalog(context.Context) (store.PremiumCatalog, error) {
	return s.loaded, s.loadErr
}

func (s *premiumCatalogSaveCoverageStore) SavePremiumCatalog(_ context.Context, _ uint32,
	entries []store.PremiumCatalogEntry, actor string) (store.PremiumCatalog, error) {
	s.saveCalls++
	s.entries = append([]store.PremiumCatalogEntry(nil), entries...)
	s.actor = actor
	return s.saved, s.saveErr
}

func premiumCatalogSaveCoverageFixture() (*World, *premiumCatalogSaveCoverageStore) {
	base := &premiumTestStore{criticalTestStore: &criticalTestStore{}}
	backend := &premiumCatalogSaveCoverageStore{premiumTestStore: base}
	w := &World{store: backend, commands: make(chan command, 2), items: map[uint16]model.ItemDef{
		400: {Index: 400, Name: "Premium sword"},
	}}
	return w, backend
}

func premiumCatalogSaveCoverageCommand(itemID uint16) control.PremiumCatalogCommand {
	return control.PremiumCatalogCommand{Version: 1, ExpectedCatalogVersion: 3, Reason: "Update premium catalog",
		Items: []control.PremiumCatalogItem{{Slot: 2, ItemID: itemID, Effects: [6]byte{1, 2}, Price: 500, Category: 1}},
	}
}

func TestSavePremiumCatalogCoversValidationAuthorizationPersistenceAndStaleReload(t *testing.T) {
	t.Run("missing backend", func(t *testing.T) {
		result, err := (&World{}).SavePremiumCatalog(context.Background(), "operator",
			premiumCatalogSaveCoverageCommand(400), func() bool { return true })
		if err != nil || result.Code != "unavailable" {
			t.Fatalf("missing backend returned result=%+v err=%v", result, err)
		}
	})

	w, backend := premiumCatalogSaveCoverageFixture()
	if result, err := w.SavePremiumCatalog(context.Background(), "operator", control.PremiumCatalogCommand{}, func() bool { return true }); err != nil || result.Code != "invalid_command" || backend.saveCalls != 0 {
		t.Fatalf("invalid command returned result=%+v err=%v calls=%d", result, err, backend.saveCalls)
	}
	if result, err := w.SavePremiumCatalog(context.Background(), "operator", premiumCatalogSaveCoverageCommand(401), func() bool { return true }); err != nil || result.Code != "unknown_item" || backend.saveCalls != 0 {
		t.Fatalf("unknown item returned result=%+v err=%v calls=%d", result, err, backend.saveCalls)
	}
	command := premiumCatalogSaveCoverageCommand(400)
	if result, err := w.SavePremiumCatalog(context.Background(), "", command, func() bool { return true }); err != nil || result.Code != "capability_required" || backend.saveCalls != 0 {
		t.Fatalf("unauthorized save returned result=%+v err=%v calls=%d", result, err, backend.saveCalls)
	}
	if result, err := w.SavePremiumCatalog(context.Background(), "operator", command, nil); err != nil || result.Code != "capability_required" || backend.saveCalls != 0 {
		t.Fatalf("missing capability returned result=%+v err=%v calls=%d", result, err, backend.saveCalls)
	}

	backend.saveErr = store.ErrPremiumCatalogStale
	backend.loaded = store.PremiumCatalog{Version: 8, UpdatedBy: "another-operator", Entries: []store.PremiumCatalogEntry{{
		Slot: 1, Item: model.Item{Index: 400}, Price: 700, Category: 2,
	}}}
	result, err := w.SavePremiumCatalog(context.Background(), "operator", command, func() bool { return true })
	if err != nil || result.Code != "stale_catalog" || result.Catalog.CatalogVersion != 8 ||
		len(result.Catalog.Items) != 1 || result.Catalog.Items[0].ItemName != "Premium sword" {
		t.Fatalf("stale save did not return the current catalog: result=%+v err=%v", result, err)
	}

	backend.loadErr = errors.New("catalog reload failed")
	if _, err := w.SavePremiumCatalog(context.Background(), "operator", command, func() bool { return true }); err == nil {
		t.Fatal("stale reload error was hidden")
	}
	backend.saveErr = errors.New("catalog save failed")
	backend.loadErr = nil
	if _, err := w.SavePremiumCatalog(context.Background(), "operator", command, func() bool { return true }); err == nil {
		t.Fatal("catalog save error was hidden")
	}

	backend.saveErr = nil
	backend.saved = store.PremiumCatalog{Version: 4, UpdatedBy: "operator", Entries: backend.entries}
	result, err = w.SavePremiumCatalog(context.Background(), "operator", command, func() bool { return true })
	if err != nil || result.Code != "ok" || result.Catalog.CatalogVersion != 4 || len(backend.entries) != 1 ||
		backend.entries[0].Slot != 2 || backend.entries[0].Item.Index != 400 || backend.entries[0].Item.Eff != [6]byte{1, 2} ||
		backend.entries[0].Price != 500 || backend.entries[0].Category != 1 || backend.actor != "operator" {
		t.Fatalf("successful catalog save mismatch: result=%+v err=%v backend=%+v", result, err, backend)
	}
	select {
	case event := <-w.commands:
		if event.premium == nil || event.premium.catalog == nil || event.premium.catalog.Version != 4 {
			t.Fatalf("successful catalog save queued wrong event: %+v", event)
		}
	default:
		t.Fatal("successful catalog save did not notify the world")
	}
}
