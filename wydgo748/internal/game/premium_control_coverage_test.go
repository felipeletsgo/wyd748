package game

import (
	"context"
	"errors"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

type premiumControlCoverageStore struct {
	*premiumTestStore
	catalog    store.PremiumCatalog
	catalogErr error
	credit     store.DonateCredit
	duplicate  bool
	creditErr  error
	summary    store.DonateSummary
	summaryErr error
}

func (s *premiumControlCoverageStore) LoadPremiumCatalog(context.Context) (store.PremiumCatalog, error) {
	return s.catalog, s.catalogErr
}

func (s *premiumControlCoverageStore) AddDonateCredit(_ context.Context, credit store.DonateCredit) (store.DonateCredit, bool, error) {
	if s.credit.ID != 0 {
		return s.credit, s.duplicate, s.creditErr
	}
	credit.ID = 91
	return credit, s.duplicate, s.creditErr
}

func (s *premiumControlCoverageStore) DonateSummary(context.Context, string, int) (store.DonateSummary, error) {
	return s.summary, s.summaryErr
}

func premiumControlCoverageWorld() (*World, *premiumControlCoverageStore) {
	base := &premiumTestStore{criticalTestStore: &criticalTestStore{}}
	backend := &premiumControlCoverageStore{premiumTestStore: base}
	w := &World{store: backend, commands: make(chan command, 2), items: map[uint16]model.ItemDef{
		400: {Index: 400, Name: "Premium sword"},
	}}
	return w, backend
}

func TestPremiumControlCatalogReadsStoreAndHandlesUnavailableBackend(t *testing.T) {
	w, backend := premiumControlCoverageWorld()
	backend.catalog = store.PremiumCatalog{Version: 3, UpdatedBy: "operator", Entries: []store.PremiumCatalogEntry{{
		Slot: 2, Item: model.Item{Index: 400, Eff: [6]uint8{1, 2}}, Price: 123, Category: 3,
	}}}
	view, err := w.PremiumCatalog(context.Background())
	if err != nil || !view.Available || view.CatalogVersion != 3 || len(view.Items) != 1 || view.Items[0].ItemName != "Premium sword" || view.Items[0].Effects[1] != 2 {
		t.Fatalf("catalog=%+v err=%v", view, err)
	}
	backend.catalogErr = errors.New("catalog unavailable")
	if _, err := w.PremiumCatalog(context.Background()); err == nil {
		t.Fatal("catalog read error was hidden")
	}
	unavailable, err := (&World{}).PremiumCatalog(context.Background())
	if err != nil || unavailable.Available || unavailable.Items == nil {
		t.Fatalf("missing backend view=%+v err=%v", unavailable, err)
	}
}

func TestCreditDonateValidatesAuthorizesPersistsAndNotifies(t *testing.T) {
	w, backend := premiumControlCoverageWorld()
	cmd := control.DonateCreditCommand{Version: 1, RequestID: "request-0000000001", Account: "validacct", Amount: 500,
		Reason: "support grant"}
	if got, err := w.CreditDonate(context.Background(), "operator", control.DonateCreditCommand{}, func() bool { return true }); err != nil || got.Code != "invalid_command" {
		t.Fatalf("invalid command result=%+v err=%v", got, err)
	}
	if got, err := w.CreditDonate(context.Background(), "", cmd, func() bool { return true }); err != nil || got.Code != "capability_required" {
		t.Fatalf("unauthorized result=%+v err=%v", got, err)
	}
	got, err := w.CreditDonate(context.Background(), "operator", cmd, func() bool { return true })
	if err != nil || got.Code != "ok" || got.CreditID != 91 || got.Replayed {
		t.Fatalf("credit result=%+v err=%v", got, err)
	}
	queued := <-w.commands
	if queued.premium == nil || len(queued.premium.credits) != 1 || queued.premium.credits[0].Account != cmd.Account {
		t.Fatalf("credit notification=%+v", queued.premium)
	}

	backend.duplicate = true
	if replay, err := w.CreditDonate(context.Background(), "operator", cmd, func() bool { return true }); err != nil || replay.Code != "ok" || !replay.Replayed {
		t.Fatalf("replayed credit result=%+v err=%v", replay, err)
	}
	if len(w.commands) != 0 {
		t.Fatal("replayed credit emitted a duplicate notification")
	}
	backend.creditErr = errors.New("database unavailable")
	if _, err := w.CreditDonate(context.Background(), "operator", cmd, func() bool { return true }); err == nil {
		t.Fatal("credit store error was hidden")
	}
	if got, err := (&World{}).CreditDonate(context.Background(), "operator", cmd, nil); err != nil || got.Code != "unavailable" {
		t.Fatalf("missing backend result=%+v err=%v", got, err)
	}
}

func TestDonateAccountMapsPendingAndLedger(t *testing.T) {
	w, backend := premiumControlCoverageWorld()
	created := time.Date(2026, 11, 1, 12, 0, 0, 0, time.UTC)
	backend.summary = store.DonateSummary{Account: "validacct", Balance: 1200,
		Pending: []store.DonateCredit{{ID: 8, Amount: 300, Reason: "grant", Source: "staff", Actor: "operator", CreatedAt: created}},
		Ledger:  []store.DonateLedgerEntry{{ID: 9, Delta: -100, BalanceAfter: 1200, Kind: "purchase", ItemIndex: 400, Character: "Knight", CreatedAt: created}},
	}
	view, err := w.DonateAccount(context.Background(), "validacct")
	if err != nil || view.Account != "validacct" || view.Balance != 1200 || len(view.Pending) != 1 || len(view.Ledger) != 1 || view.Ledger[0].ItemName != "Premium sword" {
		t.Fatalf("donate view=%+v err=%v", view, err)
	}
	backend.summaryErr = errors.New("database unavailable")
	if _, err := w.DonateAccount(context.Background(), "validacct"); err == nil {
		t.Fatal("donate summary error was hidden")
	}
	if _, err := (&World{}).DonateAccount(context.Background(), "validacct"); !errors.Is(err, control.ErrPremiumUnavailable) {
		t.Fatalf("missing backend error=%v", err)
	}
}

func TestItemNameExposesOnlyKnownCatalogNames(t *testing.T) {
	w, _ := premiumControlCoverageWorld()
	if w.ItemName(400) != "Premium sword" || w.ItemName(401) != "" {
		t.Fatalf("ItemName() returned known=%q unknown=%q", w.ItemName(400), w.ItemName(401))
	}
}
