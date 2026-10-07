package store

import (
	"errors"
	"fmt"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"

	"wydgo/internal/model"
)

// Premium shop persistence types (DOCS/protocol/premium-shop.md).

// PremiumCatalogSlots matches the 27 cells of the native 7.48 shop window.
const PremiumCatalogSlots = 27

// PremiumCatalogEntry sells one item blueprint (no UID, no expiry) for Price
// donate at a fixed shop cell.
type PremiumCatalogEntry struct {
	Slot  int        `json:"slot"`
	Item  model.Item `json:"item"`
	Price uint32     `json:"price"`
}

// PremiumCatalog is the staff-edited catalog. Version grows on every save so a
// purchase from a window opened with an older catalog is refused.
type PremiumCatalog struct {
	Version   uint32                `json:"version"`
	Entries   []PremiumCatalogEntry `json:"entries"`
	UpdatedBy string                `json:"updatedBy"`
	UpdatedAt time.Time             `json:"updatedAt"`
}

// Donate credit sources.
const (
	DonateSourceStaff = "staff"
	DonateSourceWeb   = "web"
)

// Donate ledger kinds.
const (
	DonateKindCredit   = "credit"
	DonateKindPurchase = "purchase"
	DonateKindCoinUse  = "coin_use"
)

// DonateCredit is one inbox row written by the staff panel or the web
// platform and applied once by the game server.
type DonateCredit struct {
	ID        int64      `json:"id"`
	Account   string     `json:"account"`
	Amount    uint32     `json:"amount"`
	Reason    string     `json:"reason"`
	Source    string     `json:"source"`
	Actor     string     `json:"actor"`
	RequestID string     `json:"requestId,omitempty"`
	CreatedAt time.Time  `json:"createdAt"`
	AppliedAt *time.Time `json:"appliedAt,omitempty"`
}

// DonateLedgerEntry records one balance change, written in the same
// transaction as the account.
type DonateLedgerEntry struct {
	ID           int64     `json:"id"`
	Account      string    `json:"account"`
	Delta        int64     `json:"delta"`
	BalanceAfter uint32    `json:"balanceAfter"`
	Kind         string    `json:"kind"`
	ItemIndex    uint16    `json:"itemIndex,omitempty"`
	ItemUID      string    `json:"itemUid,omitempty"`
	CreditID     int64     `json:"creditId,omitempty"`
	Character    string    `json:"character,omitempty"`
	CreatedAt    time.Time `json:"createdAt"`
}

// DonateSummary is the staff view of one account.
type DonateSummary struct {
	Account string              `json:"account"`
	Balance uint32              `json:"balance"`
	Pending []DonateCredit      `json:"pending"`
	Ledger  []DonateLedgerEntry `json:"ledger"`
}

// ErrDonateCreditApplied rejects a donate write whose credit was already
// applied (or does not belong to the account); the transaction rolls back.
var ErrDonateCreditApplied = errors.New("store: donate credit already applied")

// ErrPremiumCatalogStale rejects a catalog save based on an outdated version.
var ErrPremiumCatalogStale = errors.New("store: premium catalog changed")

// ErrDonateCreditConflict reports a reused request ID with different content.
var ErrDonateCreditConflict = errors.New("store: donate request ID reused with different content")

// ValidatePremiumEntries checks the catalog structure. Item existence is
// checked by the game server, which owns the item list.
func ValidatePremiumEntries(entries []PremiumCatalogEntry) error {
	if len(entries) > PremiumCatalogSlots {
		return fmt.Errorf("premium catalog has %d entries; maximum %d", len(entries), PremiumCatalogSlots)
	}
	seen := make(map[int]bool, len(entries))
	for _, entry := range entries {
		if entry.Slot < 0 || entry.Slot >= PremiumCatalogSlots {
			return fmt.Errorf("premium catalog slot %d outside 0..%d", entry.Slot, PremiumCatalogSlots-1)
		}
		if seen[entry.Slot] {
			return fmt.Errorf("premium catalog slot %d repeated", entry.Slot)
		}
		seen[entry.Slot] = true
		if entry.Item.Index == 0 || int(entry.Item.Index) >= model.ItemListSize {
			return fmt.Errorf("premium catalog slot %d has invalid item %d", entry.Slot, entry.Item.Index)
		}
		if entry.Item.UID != "" || entry.Item.ActivatedUnix != 0 || entry.Item.ExpiresUnix != 0 {
			return fmt.Errorf("premium catalog slot %d must be an item blueprint", entry.Slot)
		}
		if entry.Price == 0 || entry.Price > model.MaxDonateBalance {
			return fmt.Errorf("premium catalog slot %d has invalid price %d", entry.Slot, entry.Price)
		}
	}
	return nil
}

// ValidateDonateCredit checks an inbox row before insertion.
func ValidateDonateCredit(c DonateCredit) error {
	if strings.TrimSpace(c.Account) == "" || len(c.Account) > 12 {
		return errors.New("donate credit: invalid account")
	}
	if c.Amount == 0 || c.Amount > model.MaxDonateBalance {
		return errors.New("donate credit: invalid amount")
	}
	reason := strings.TrimSpace(c.Reason)
	if !utf8.ValidString(c.Reason) || utf8.RuneCountInString(reason) < 3 || utf8.RuneCountInString(c.Reason) > 200 ||
		strings.IndexFunc(c.Reason, unicode.IsControl) >= 0 {
		return errors.New("donate credit: reason must have 3..200 characters")
	}
	if c.Source != DonateSourceStaff && c.Source != DonateSourceWeb {
		return errors.New("donate credit: invalid source")
	}
	if len(c.Actor) > 64 || len(c.RequestID) > 64 || strings.IndexFunc(c.RequestID, unicode.IsSpace) >= 0 {
		return errors.New("donate credit: invalid actor or request ID")
	}
	return nil
}

// ValidateDonateLedger checks ledger rows produced by the game server.
func ValidateDonateLedger(account *model.Account, ledger []DonateLedgerEntry) error {
	for _, entry := range ledger {
		if entry.Delta == 0 || int64(entry.BalanceAfter) > int64(model.MaxDonateBalance) {
			return errors.New("donate ledger: invalid delta or balance")
		}
		switch entry.Kind {
		case DonateKindCredit, DonateKindPurchase, DonateKindCoinUse:
		default:
			return fmt.Errorf("donate ledger: invalid kind %q", entry.Kind)
		}
		if !strings.EqualFold(entry.Account, account.Name) {
			return errors.New("donate ledger: entry for another account")
		}
	}
	return nil
}
