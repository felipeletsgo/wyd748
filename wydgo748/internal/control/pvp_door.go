package control

import (
	"context"
	"errors"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"
)

// PvPDoorCapability lets staff lock and open the Armia PvP door, the 7.59
// TMSRV "pkgate" command. Locking is idempotent, so no receipt is kept.
const PvPDoorCapability = "game.world.pvp-door"

type PvPDoorCommand struct {
	Version     int    `json:"version"`
	OperationID string `json:"operationId"`
	Action      string `json:"action"`
	Reason      string `json:"reason"`
}

func (c PvPDoorCommand) Validate() error {
	if c.Version != 1 || len(c.OperationID) < 16 || len(c.OperationID) > 64 ||
		c.Action != "lock" && c.Action != "open" ||
		!utf8.ValidString(c.Reason) || utf8.RuneCountInString(strings.TrimSpace(c.Reason)) < 3 ||
		utf8.RuneCountInString(c.Reason) > 200 || strings.IndexFunc(c.Reason, unicode.IsControl) >= 0 ||
		strings.IndexFunc(c.OperationID, func(r rune) bool {
			return !(r >= 'a' && r <= 'z' || r >= 'A' && r <= 'Z' || r >= '0' && r <= '9' || r == '-')
		}) >= 0 {
		return errors.New("invalid_command")
	}
	return nil
}

type PvPDoorStatus struct {
	Version   int       `json:"version"`
	AsOf      time.Time `json:"asOf"`
	Available bool      `json:"available"`
	Locked    bool      `json:"locked"`
	X         uint16    `json:"x"`
	Y         uint16    `json:"y"`
}

type PvPDoorResult struct {
	OperationID string        `json:"operationId"`
	Code        string        `json:"code"`
	Status      PvPDoorStatus `json:"status"`
}

type PvPDoorSource interface {
	PvPDoorStatus(context.Context) (PvPDoorStatus, error)
	PvPDoorCommand(context.Context, string, PvPDoorCommand, func() bool) (PvPDoorResult, error)
}
