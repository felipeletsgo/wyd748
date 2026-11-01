package control

import (
	"context"
	"errors"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"
)

// PersistenceRecoveryCapability lets staff read quarantined critical writes and
// request an immediate exact-outcome check (W4). The check releases a fence
// only when PostgreSQL reports a final status; staff never unlock directly.
const PersistenceRecoveryCapability = "game.persistence.recovery"

// QuarantinedOperation is one isolated critical write. Accounts are the
// affected account keys, so support staff can answer the affected players.
type QuarantinedOperation struct {
	ID      uint64    `json:"id"`
	Channel byte      `json:"channel"`
	Since   time.Time `json:"since"`
	// Accounts is bounded by the coordinator's participant limit.
	Accounts []string `json:"accounts"`
	// Shared marks ground objects, mobs, guilds or instances. These stay
	// fenced until a restart reloads them from the database.
	Shared bool `json:"shared"`
	// Checkable means the exact transaction is known: a final status from
	// PostgreSQL releases the accounts without a restart.
	Checkable  bool   `json:"checkable"`
	LastStatus string `json:"lastStatus,omitempty"`
}

type PersistenceRecoveryStatus struct {
	Version     int                    `json:"version"`
	AsOf        time.Time              `json:"asOf"`
	Quarantined []QuarantinedOperation `json:"quarantined"`
	// Released counts quarantines released since startup after a final status.
	Released int64 `json:"released"`
}

type PersistenceRecheckCommand struct {
	Version     int    `json:"version"`
	OperationID string `json:"operationId"`
	Reason      string `json:"reason"`
}

func (c PersistenceRecheckCommand) Validate() error {
	if c.Version != 1 || len(c.OperationID) < 16 || len(c.OperationID) > 64 ||
		!utf8.ValidString(c.Reason) || utf8.RuneCountInString(strings.TrimSpace(c.Reason)) < 3 ||
		utf8.RuneCountInString(c.Reason) > 200 || strings.IndexFunc(c.Reason, unicode.IsControl) >= 0 ||
		strings.IndexFunc(c.OperationID, func(r rune) bool {
			return !(r >= 'a' && r <= 'z' || r >= 'A' && r <= 'Z' || r >= '0' && r <= '9' || r == '-')
		}) >= 0 {
		return errors.New("invalid_command")
	}
	return nil
}

// PersistenceRecheckResult reports that a check started, not its outcome.
// Read the status again to see released operations.
type PersistenceRecheckResult struct {
	OperationID string                    `json:"operationId"`
	Code        string                    `json:"code"`
	Started     int                       `json:"started"`
	Status      PersistenceRecoveryStatus `json:"status"`
}

type PersistenceRecoverySource interface {
	PersistenceRecoveryStatus(context.Context) (PersistenceRecoveryStatus, error)
	PersistenceRecheck(context.Context, string, PersistenceRecheckCommand, func() bool) (PersistenceRecheckResult, error)
}
