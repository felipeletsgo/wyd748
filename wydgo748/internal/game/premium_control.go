package game

import (
	"context"
	"errors"
	"log"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

// Staff panel capabilities of the premium shop (control.PremiumSource). They
// run on the HTTP goroutine and touch only the store and the immutable item
// catalog; the World learns about changes through a premiumEvent command.

func (w *World) premiumControlStore() (premiumStore, error) {
	backend, ok := w.store.(premiumStore)
	if !ok {
		return nil, control.ErrPremiumUnavailable
	}
	return backend, nil
}

func (w *World) itemName(index uint16) string {
	if def, ok := w.items[index]; ok {
		return def.Name
	}
	return ""
}

func (w *World) premiumCatalogView(catalog store.PremiumCatalog) control.PremiumCatalogView {
	view := control.PremiumCatalogView{Version: 1, Available: true, CatalogVersion: catalog.Version,
		Items: make([]control.PremiumCatalogItem, 0, len(catalog.Entries)), UpdatedBy: catalog.UpdatedBy, UpdatedAt: catalog.UpdatedAt}
	for _, entry := range catalog.Entries {
		view.Items = append(view.Items, control.PremiumCatalogItem{Slot: entry.Slot, ItemID: entry.Item.Index,
			Effects: entry.Item.Eff, Price: entry.Price, ItemName: w.itemName(entry.Item.Index)})
	}
	return view
}

// notifyPremium hands a catalog or credits to the World without blocking; the
// periodic poll picks them up if the command queue is full.
func (w *World) notifyPremium(event *premiumEvent) {
	select {
	case w.commands <- command{premium: event, queuedAt: time.Now()}:
	default:
	}
}

func (w *World) PremiumCatalog(ctx context.Context) (control.PremiumCatalogView, error) {
	backend, err := w.premiumControlStore()
	if err != nil {
		return control.PremiumCatalogView{Version: 1, Items: []control.PremiumCatalogItem{}}, nil
	}
	catalog, err := backend.LoadPremiumCatalog(ctx)
	if err != nil {
		return control.PremiumCatalogView{}, err
	}
	return w.premiumCatalogView(catalog), nil
}

func (w *World) SavePremiumCatalog(ctx context.Context, actor string, cmd control.PremiumCatalogCommand, authorize func() bool) (control.PremiumCatalogResult, error) {
	backend, err := w.premiumControlStore()
	if err != nil {
		return control.PremiumCatalogResult{Code: "unavailable"}, nil
	}
	if cmd.Validate() != nil {
		return control.PremiumCatalogResult{Code: "invalid_command"}, nil
	}
	entries := make([]store.PremiumCatalogEntry, 0, len(cmd.Items))
	for _, item := range cmd.Items {
		if _, exists := w.items[item.ItemID]; !exists {
			return control.PremiumCatalogResult{Code: "unknown_item"}, nil
		}
		entries = append(entries, store.PremiumCatalogEntry{Slot: item.Slot,
			Item: model.Item{Index: item.ItemID, Eff: item.Effects}, Price: item.Price})
	}
	if actor == "" || authorize == nil || !authorize() {
		return control.PremiumCatalogResult{Code: "capability_required"}, nil
	}
	catalog, err := backend.SavePremiumCatalog(ctx, cmd.ExpectedCatalogVersion, entries, actor)
	if errors.Is(err, store.ErrPremiumCatalogStale) {
		current, loadErr := backend.LoadPremiumCatalog(ctx)
		if loadErr != nil {
			return control.PremiumCatalogResult{}, loadErr
		}
		return control.PremiumCatalogResult{Code: "stale_catalog", Catalog: w.premiumCatalogView(current)}, nil
	}
	if err != nil {
		return control.PremiumCatalogResult{}, err
	}
	log.Printf("[web-admin] premium catalog v%d saved by %q (%d item(s)) reason=%q",
		catalog.Version, actor, len(entries), cmd.Reason)
	w.notifyPremium(&premiumEvent{catalog: &catalog})
	return control.PremiumCatalogResult{Code: "ok", Catalog: w.premiumCatalogView(catalog)}, nil
}

func (w *World) CreditDonate(ctx context.Context, actor string, cmd control.DonateCreditCommand, authorize func() bool) (control.DonateCreditResult, error) {
	backend, err := w.premiumControlStore()
	if err != nil {
		return control.DonateCreditResult{Code: "unavailable"}, nil
	}
	if cmd.Validate() != nil {
		return control.DonateCreditResult{Code: "invalid_command"}, nil
	}
	if actor == "" || authorize == nil || !authorize() {
		return control.DonateCreditResult{Code: "capability_required"}, nil
	}
	credit, duplicate, err := backend.AddDonateCredit(ctx, store.DonateCredit{Account: cmd.Account,
		Amount: cmd.Amount, Reason: cmd.Reason, Source: store.DonateSourceStaff, Actor: actor, RequestID: cmd.RequestID})
	switch {
	case store.IsAccountMissing(err):
		return control.DonateCreditResult{Code: "unknown_account"}, nil
	case errors.Is(err, store.ErrDonateCreditConflict):
		return control.DonateCreditResult{Code: "request_conflict"}, nil
	case err != nil:
		return control.DonateCreditResult{}, err
	}
	if !duplicate {
		log.Printf("[web-admin] donate credit %d: %q +%d by %q reason=%q", credit.ID, credit.Account, credit.Amount, actor, cmd.Reason)
		w.notifyPremium(&premiumEvent{credits: []store.DonateCredit{credit}})
	}
	return control.DonateCreditResult{Code: "ok", CreditID: credit.ID, Replayed: duplicate}, nil
}

func (w *World) DonateAccount(ctx context.Context, account string) (control.DonateAccountView, error) {
	backend, err := w.premiumControlStore()
	if err != nil {
		return control.DonateAccountView{}, err
	}
	summary, err := backend.DonateSummary(ctx, account, 20)
	if store.IsAccountMissing(err) {
		return control.DonateAccountView{}, control.ErrUnknownAccount
	}
	if err != nil {
		return control.DonateAccountView{}, err
	}
	view := control.DonateAccountView{Version: 1, Account: summary.Account, Balance: summary.Balance,
		Pending: make([]control.DonatePendingView, 0, len(summary.Pending)),
		Ledger:  make([]control.DonateLedgerView, 0, len(summary.Ledger))}
	for _, c := range summary.Pending {
		view.Pending = append(view.Pending, control.DonatePendingView{ID: c.ID, Amount: c.Amount,
			Reason: c.Reason, Source: c.Source, Actor: c.Actor, CreatedAt: c.CreatedAt})
	}
	for _, e := range summary.Ledger {
		view.Ledger = append(view.Ledger, control.DonateLedgerView{ID: e.ID, Delta: e.Delta,
			BalanceAfter: e.BalanceAfter, Kind: e.Kind, ItemID: e.ItemIndex, ItemName: w.itemName(e.ItemIndex),
			Character: e.Character, CreatedAt: e.CreatedAt})
	}
	return view, nil
}

var _ control.PremiumSource = (*World)(nil)
