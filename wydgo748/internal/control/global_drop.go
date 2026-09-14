package control

import (
	"context"
	"errors"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"
)

const GlobalDropCapability = "game.event.global-drop"

// GlobalDropCommand is versioned separately from the legacy game wire. Actor is
// supplied by the authenticated local adapter, never by a JSON field.
type GlobalDropCommand struct {
	Version         int    `json:"version"`
	OperationID     string `json:"operationId"`
	Epoch           string `json:"epoch"`
	ExpectedEventID string `json:"expectedEventId"`
	Action          string `json:"action"`
	Reason          string `json:"reason"`
	ItemID          uint16 `json:"itemId"`
	RatePercent     int    `json:"ratePercent"`
	MaxDrops        int    `json:"maxDrops"`
	DurationSeconds int    `json:"durationSeconds"`
}

func (c GlobalDropCommand) Validate() error {
	if c.Version != 1 || len(c.OperationID) < 16 || len(c.OperationID) > 64 || len(c.Epoch) != 32 || len(c.ExpectedEventID) > 64 ||
		utf8.RuneCountInString(strings.TrimSpace(c.Reason)) < 3 || utf8.RuneCountInString(c.Reason) > 200 || strings.IndexFunc(c.Reason, unicode.IsControl) >= 0 {
		return errors.New("invalid_command")
	}
	if strings.IndexFunc(c.OperationID, func(r rune) bool {
		return !(r >= 'a' && r <= 'z' || r >= 'A' && r <= 'Z' || r >= '0' && r <= '9' || r == '-')
	}) >= 0 {
		return errors.New("invalid_command")
	}
	switch c.Action {
	case "start":
		if c.ItemID == 0 || c.RatePercent < 1 || c.RatePercent > 100 || c.MaxDrops < 0 || c.MaxDrops > 10000000 || c.DurationSeconds < 0 || c.DurationSeconds > 604800 || c.MaxDrops == 0 && c.DurationSeconds == 0 {
			return errors.New("invalid_command")
		}
	case "stop":
		if c.ExpectedEventID == "" || c.ItemID != 0 || c.RatePercent != 0 || c.MaxDrops != 0 || c.DurationSeconds != 0 {
			return errors.New("invalid_command")
		}
	default:
		return errors.New("invalid_command")
	}
	return nil
}

type GlobalDropStatus struct {
	Version     int        `json:"version"`
	Epoch       string     `json:"epoch"`
	AsOf        time.Time  `json:"asOf"`
	EventID     string     `json:"eventId"`
	Active      bool       `json:"active"`
	ItemID      uint16     `json:"itemId"`
	ItemName    string     `json:"itemName"`
	RatePercent int        `json:"ratePercent"`
	MaxDrops    int        `json:"maxDrops"`
	Dropped     int        `json:"dropped"`
	StartedAt   *time.Time `json:"startedAt"`
	EndsAt      *time.Time `json:"endsAt"`
	EndedBy     string     `json:"endedBy"`
	Actor       string     `json:"actor"`
}

type GlobalDropResult struct {
	OperationID string           `json:"operationId"`
	Code        string           `json:"code"`
	Replayed    bool             `json:"replayed"`
	Status      GlobalDropStatus `json:"status"`
}

// Only the embedded panel supports writes. The separate HTTP control service
// remains read-only. authorize must recheck the staff session at execution time.
type GlobalDropSource interface {
	GlobalDropStatus(context.Context) (GlobalDropStatus, error)
	GlobalDropCommand(context.Context, string, GlobalDropCommand, func() bool) (GlobalDropResult, error)
}
