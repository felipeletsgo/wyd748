package control

import (
	"context"
	"errors"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"
)

// Premium shop staff contract (DOCS/protocol/premium-shop.md).
const (
	PremiumCatalogCapability = "economy.premium.catalog"
	DonateCreditCapability   = "economy.donate.credit"
	PremiumCatalogSlots      = 27
	MaxDonateAmount          = 2_000_000_000
)

type PremiumCatalogItem struct {
	Slot     int      `json:"slot"`
	ItemID   uint16   `json:"itemId"`
	Effects  [6]uint8 `json:"effects"`
	Price    uint32   `json:"price"`
	ItemName string   `json:"itemName,omitempty"`
	Category uint8    `json:"category"`
}

type PremiumCatalogView struct {
	Version        int                  `json:"version"`
	Available      bool                 `json:"available"`
	CatalogVersion uint32               `json:"catalogVersion"`
	Items          []PremiumCatalogItem `json:"items"`
	UpdatedBy      string               `json:"updatedBy"`
	UpdatedAt      time.Time            `json:"updatedAt"`
}

// PremiumCatalogCommand replaces the whole catalog. ExpectedCatalogVersion
// refuses a save based on an outdated read.
type PremiumCatalogCommand struct {
	Version                int                  `json:"version"`
	ExpectedCatalogVersion uint32               `json:"expectedCatalogVersion"`
	Reason                 string               `json:"reason"`
	Items                  []PremiumCatalogItem `json:"items"`
}

func validReason(reason string) bool {
	trimmed := strings.TrimSpace(reason)
	return utf8.ValidString(reason) && utf8.RuneCountInString(trimmed) >= 3 &&
		utf8.RuneCountInString(reason) <= 200 && strings.IndexFunc(reason, unicode.IsControl) < 0
}

func (c PremiumCatalogCommand) Validate() error {
	if c.Version != 1 || !validReason(c.Reason) || len(c.Items) > PremiumCatalogSlots {
		return errors.New("invalid_command")
	}
	seen := make(map[int]bool, len(c.Items))
	for _, item := range c.Items {
		if item.Slot < 0 || item.Slot >= PremiumCatalogSlots || seen[item.Slot] ||
			item.ItemID == 0 || item.ItemID >= 6500 || item.Price == 0 || item.Price > MaxDonateAmount || item.Category > 3 {
			return errors.New("invalid_command")
		}
		seen[item.Slot] = true
	}
	return nil
}

type PremiumCatalogResult struct {
	Code    string             `json:"code"`
	Catalog PremiumCatalogView `json:"catalog"`
}

// DonateCreditCommand credits an account. RequestID makes a retry idempotent.
type DonateCreditCommand struct {
	Version   int    `json:"version"`
	RequestID string `json:"requestId"`
	Account   string `json:"account"`
	Amount    uint32 `json:"amount"`
	Reason    string `json:"reason"`
}

func (c DonateCreditCommand) Validate() error {
	if c.Version != 1 || len(c.RequestID) < 16 || len(c.RequestID) > 64 ||
		strings.IndexFunc(c.RequestID, func(r rune) bool {
			return !(r >= 'a' && r <= 'z' || r >= 'A' && r <= 'Z' || r >= '0' && r <= '9' || r == '-')
		}) >= 0 ||
		len(c.Account) < 1 || len(c.Account) > 12 ||
		strings.IndexFunc(c.Account, func(r rune) bool { return !(r >= 'a' && r <= 'z' || r >= '0' && r <= '9') }) >= 0 ||
		c.Amount == 0 || c.Amount > MaxDonateAmount || !validReason(c.Reason) {
		return errors.New("invalid_command")
	}
	return nil
}

type DonateCreditResult struct {
	Code     string `json:"code"`
	CreditID int64  `json:"creditId"`
	Replayed bool   `json:"replayed"`
}

type DonatePendingView struct {
	ID        int64     `json:"id"`
	Amount    uint32    `json:"amount"`
	Reason    string    `json:"reason"`
	Source    string    `json:"source"`
	Actor     string    `json:"actor"`
	CreatedAt time.Time `json:"createdAt"`
}

type DonateLedgerView struct {
	ID           int64     `json:"id"`
	Delta        int64     `json:"delta"`
	BalanceAfter uint32    `json:"balanceAfter"`
	Kind         string    `json:"kind"`
	ItemID       uint16    `json:"itemId"`
	ItemName     string    `json:"itemName,omitempty"`
	Character    string    `json:"character"`
	CreatedAt    time.Time `json:"createdAt"`
}

type DonateAccountView struct {
	Version int                 `json:"version"`
	Account string              `json:"account"`
	Balance uint32              `json:"balance"`
	Pending []DonatePendingView `json:"pending"`
	Ledger  []DonateLedgerView  `json:"ledger"`
}

// ErrUnknownAccount reports a donate query for an account that does not exist.
var ErrUnknownAccount = errors.New("control: unknown account")

// ErrPremiumUnavailable reports a server without the premium shop backend.
var ErrPremiumUnavailable = errors.New("control: premium shop unavailable")

type PremiumSource interface {
	PremiumCatalog(context.Context) (PremiumCatalogView, error)
	SavePremiumCatalog(context.Context, string, PremiumCatalogCommand, func() bool) (PremiumCatalogResult, error)
	CreditDonate(context.Context, string, DonateCreditCommand, func() bool) (DonateCreditResult, error)
	DonateAccount(context.Context, string) (DonateAccountView, error)
}
