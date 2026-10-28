package store

import (
	"context"
	"encoding/json"
	"errors"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestValidatePremiumEntries(t *testing.T) {
	valid := []PremiumCatalogEntry{{Slot: 0, Item: model.Item{Index: 3394}, Price: 1000}, {Slot: 26, Item: model.Item{Index: 412}, Price: 5}}
	if err := ValidatePremiumEntries(valid); err != nil {
		t.Fatal(err)
	}
	for name, entries := range map[string][]PremiumCatalogEntry{
		"category":  {{Slot: 1, Item: model.Item{Index: 1}, Price: 1, Category: 4}},
		"slot":      {{Slot: 27, Item: model.Item{Index: 1}, Price: 1}},
		"repeat":    {{Slot: 1, Item: model.Item{Index: 1}, Price: 1}, {Slot: 1, Item: model.Item{Index: 2}, Price: 1}},
		"item":      {{Slot: 1, Item: model.Item{Index: 0}, Price: 1}},
		"big item":  {{Slot: 1, Item: model.Item{Index: model.ItemListSize}, Price: 1}},
		"uid":       {{Slot: 1, Item: model.Item{Index: 1, UID: "x"}, Price: 1}},
		"price":     {{Slot: 1, Item: model.Item{Index: 1}, Price: 0}},
		"big price": {{Slot: 1, Item: model.Item{Index: 1}, Price: model.MaxDonateBalance + 1}},
	} {
		if ValidatePremiumEntries(entries) == nil {
			t.Fatalf("%s accepted", name)
		}
	}
}

func TestPremiumCategoryJSONCompatibility(t *testing.T) {
	var old PremiumCatalogEntry
	if err := json.Unmarshal([]byte(`{"slot":2,"item":{"index":400},"price":1000}`), &old); err != nil || old.Category != 0 {
		t.Fatalf("old catalogs must default to Featured: %+v %v", old, err)
	}
	for category := uint8(0); category < 4; category++ {
		entry := PremiumCatalogEntry{Slot: 2, Item: model.Item{Index: 400}, Price: 1000, Category: category}
		encoded, err := json.Marshal(entry)
		if err != nil {
			t.Fatal(err)
		}
		var decoded PremiumCatalogEntry
		if err := json.Unmarshal(encoded, &decoded); err != nil || decoded != entry || ValidatePremiumEntries([]PremiumCatalogEntry{decoded}) != nil {
			t.Fatalf("category %d failed JSON storage round trip: %+v %v", category, decoded, err)
		}
	}
}

func TestPostgresPremiumCatalogAndDonateCredits(t *testing.T) {
	st, _ := criticalAccountsPostgresFixture(t)
	owner := postgresTestAccount("dn"+time.Now().UTC().Format("150405.00")[:9], "DonateOwner", model.Item{Index: 4011})
	owner.Name = strings.ReplaceAll(owner.Name, ".", "")
	if err := st.CreateAccount(owner); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(owner.Name))
	})
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	previous, err := st.LoadPremiumCatalog(ctx)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		if previous.Version == 0 {
			_, _ = st.pool.Exec(context.Background(), `DELETE FROM premium_shop`)
		} else {
			current, _ := st.LoadPremiumCatalog(context.Background())
			_, _ = st.SavePremiumCatalog(context.Background(), current.Version, previous.Entries, previous.UpdatedBy)
		}
	})

	entries := []PremiumCatalogEntry{{Slot: 3, Item: model.Item{Index: 3394, Eff: [6]byte{61, 2}}, Price: 1000, Category: 3}}
	saved, err := st.SavePremiumCatalog(ctx, previous.Version, entries, "staffer")
	if err != nil || saved.Version == 0 || saved.Version <= previous.Version {
		t.Fatalf("save catalog: %+v %v", saved, err)
	}
	loaded, err := st.LoadPremiumCatalog(ctx)
	if err != nil || loaded.Version != saved.Version || len(loaded.Entries) != 1 ||
		loaded.Entries[0] != entries[0] || loaded.UpdatedBy != "staffer" {
		t.Fatalf("load catalog: %+v %v", loaded, err)
	}
	again, err := st.SavePremiumCatalog(ctx, saved.Version, entries, "staffer")
	if err != nil || again.Version != saved.Version+1 {
		t.Fatalf("catalog version must grow: %d -> %d (%v)", saved.Version, again.Version, err)
	}
	if _, err := st.SavePremiumCatalog(ctx, saved.Version, entries, "late"); !errors.Is(err, ErrPremiumCatalogStale) {
		t.Fatalf("a save from an outdated version must be refused: %v", err)
	}
	if _, err := st.SavePremiumCatalog(ctx, again.Version, []PremiumCatalogEntry{{Slot: 99, Item: model.Item{Index: 1}, Price: 1}}, "x"); err == nil {
		t.Fatal("invalid catalog saved")
	}

	requestID := "req-" + strings.ToLower(owner.Name)
	credit, duplicate, err := st.AddDonateCredit(ctx, DonateCredit{Account: owner.Name, Amount: 700,
		Reason: "test credit", Source: DonateSourceStaff, Actor: "staffer", RequestID: requestID})
	if err != nil || duplicate || credit.ID == 0 {
		t.Fatalf("add credit: %+v %v %v", credit, duplicate, err)
	}
	replay, duplicate, err := st.AddDonateCredit(ctx, DonateCredit{Account: owner.Name, Amount: 700,
		Reason: "test credit", Source: DonateSourceWeb, RequestID: requestID})
	if err != nil || !duplicate || replay.ID != credit.ID {
		t.Fatalf("request ID replay must return the same credit: %+v %v %v", replay, duplicate, err)
	}
	if _, _, err := st.AddDonateCredit(ctx, DonateCredit{Account: owner.Name, Amount: 701,
		Reason: "test credit", Source: DonateSourceWeb, RequestID: requestID}); !errors.Is(err, ErrDonateCreditConflict) {
		t.Fatalf("reused request ID with another amount: %v", err)
	}
	if _, _, err := st.AddDonateCredit(ctx, DonateCredit{Account: "nobody" + strings.ToLower(owner.Name[:4]),
		Amount: 1, Reason: "test credit", Source: DonateSourceStaff}); !IsAccountMissing(err) {
		t.Fatalf("credit for a missing account: %v", err)
	}
	pending, err := st.PendingDonateCredits(ctx, []string{owner.Name}, 10)
	if err != nil || len(pending) != 1 || pending[0].ID != credit.ID || pending[0].Amount != 700 {
		t.Fatalf("pending credits: %+v %v", pending, err)
	}

	owner.Donate = 700
	ledger := []DonateLedgerEntry{{Account: owner.Name, Delta: 700, BalanceAfter: 700, Kind: DonateKindCredit, CreditID: credit.ID}}
	if err := st.SaveDonateAccount(owner, []int64{credit.ID}, ledger); err != nil {
		t.Fatal(err)
	}
	loadedAccount, err := st.LoadAccount(owner.Name)
	if err != nil || loadedAccount.Donate != 700 {
		t.Fatalf("balance not persisted: %v %+v", err, loadedAccount)
	}
	owner.Donate = 1400
	err = st.SaveDonateAccount(owner, []int64{credit.ID}, ledger)
	if !errors.Is(err, ErrDonateCreditApplied) || !IsWriteRejected(err) {
		t.Fatalf("a credit must apply once: %v", err)
	}
	loadedAccount, _ = st.LoadAccount(owner.Name)
	if loadedAccount.Donate != 700 {
		t.Fatalf("rejected credit changed the balance: %d", loadedAccount.Donate)
	}
	summary, err := st.DonateSummary(ctx, owner.Name, 10)
	if err != nil || summary.Balance != 700 || len(summary.Pending) != 0 || len(summary.Ledger) != 1 ||
		summary.Ledger[0].CreditID != credit.ID || summary.Ledger[0].Kind != DonateKindCredit {
		t.Fatalf("summary: %+v %v", summary, err)
	}
	if err := st.SaveDonateAccount(owner, nil, []DonateLedgerEntry{{Account: "other", Delta: 1, Kind: DonateKindCredit}}); err == nil {
		t.Fatal("ledger for another account accepted")
	}
}
