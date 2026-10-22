package control

import (
	"context"
	"errors"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"
)

// Account investigation contract (DOCS/server/staff-operations.md).
const (
	// AccountIPCapability reveals client IP addresses in login history.
	AccountIPCapability = "moderation.account.ip"
	// LogsCapability reads the allowlisted server logs and backup receipts.
	LogsCapability = "operations.logs.read"
	// LoginHistoryRetention bounds how long login rows are kept.
	LoginHistoryRetention = 90 * 24 * time.Hour
)

type AccountLogin struct {
	At       time.Time `json:"at"`
	ClientIP string    `json:"clientIp"`
	Channel  int       `json:"channel"`
}

type AccountLogins struct {
	Version   int            `json:"version"`
	AsOf      time.Time      `json:"asOf"`
	Account   string         `json:"account"`
	IPVisible bool           `json:"ipVisible"`
	Logins    []AccountLogin `json:"logins"`
}

type AccountActions struct {
	Version int          `json:"version"`
	AsOf    time.Time    `json:"asOf"`
	Account string       `json:"account"`
	Entries []AuditEntry `json:"entries"`
}

// AccountHistoryStore serves the account page from the game writer store:
// the panel database role may not read these tables.
type AccountHistoryStore interface {
	ListAccountLogins(context.Context, string, int) ([]AccountLogin, time.Time, error)
	ListStaffAuditForAccount(context.Context, string, int) ([]AuditEntry, time.Time, error)
	PruneAccountLogins(context.Context, time.Time, int) (int64, error)
}

// StoredItem is one occupied cell of a persisted character snapshot.
type StoredItem struct {
	Slot    int      `json:"slot"`
	ItemID  uint16   `json:"itemId"`
	Name    string   `json:"name"`
	Effects [6]uint8 `json:"effects"`
}

// CharacterStorage is a read-only staff view of the saved equipment, carry
// inventory and account Cargo. Live state may differ until the next save.
type CharacterStorage struct {
	Version   int          `json:"version"`
	AsOf      time.Time    `json:"asOf"`
	Account   string       `json:"account"`
	UID       string       `json:"uid"`
	Name      string       `json:"name"`
	Gold      uint32       `json:"gold"`
	CargoGold uint64       `json:"cargoGold"`
	Equipment []StoredItem `json:"equipment"`
	Inventory []StoredItem `json:"inventory"`
	Cargo     []StoredItem `json:"cargo"`
}

// AccountNote is one append-only staff case note. BugRefs link entries of the
// gameplay bug register (BUG-NNN).
type AccountNote struct {
	ID      int64     `json:"id"`
	At      time.Time `json:"at"`
	Actor   string    `json:"actor"`
	Body    string    `json:"body"`
	BugRefs []string  `json:"bugRefs"`
}

type AccountNotes struct {
	Version int           `json:"version"`
	AsOf    time.Time     `json:"asOf"`
	Account string        `json:"account"`
	Notes   []AccountNote `json:"notes"`
}

type AccountNoteCommand struct {
	Version int      `json:"version"`
	Body    string   `json:"body"`
	BugRefs []string `json:"bugRefs"`
}

// Validate accepts 3–2,000 characters without control characters other than
// newline, and up to 10 distinct BUG-NNN references.
func (c AccountNoteCommand) Validate() error {
	trimmed := strings.TrimSpace(c.Body)
	if c.Version != 1 || !utf8.ValidString(c.Body) || utf8.RuneCountInString(trimmed) < 3 || utf8.RuneCountInString(c.Body) > 2000 ||
		strings.IndexFunc(c.Body, func(r rune) bool { return r != '\n' && unicode.IsControl(r) }) >= 0 || len(c.BugRefs) > 10 {
		return errors.New("invalid_command")
	}
	seen := map[string]bool{}
	for _, ref := range c.BugRefs {
		digits := strings.TrimPrefix(ref, "BUG-")
		if digits == ref || len(digits) < 3 || len(digits) > 6 || strings.IndexFunc(digits, func(r rune) bool { return r < '0' || r > '9' }) >= 0 || seen[ref] {
			return errors.New("invalid_command")
		}
		seen[ref] = true
	}
	return nil
}

type AccountNotesStore interface {
	ListAccountNotes(context.Context, string, int) ([]AccountNote, time.Time, error)
	AddAccountNote(context.Context, string, string, AccountNoteCommand) (AccountNote, error)
}
