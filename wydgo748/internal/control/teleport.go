package control

import (
	"context"
	"errors"
	"strings"
	"unicode"
	"unicode/utf8"
)

const TeleportCapability = "moderation.player.teleport"

// TeleportCommand targets a connection observed in a specific World process.
// It reuses the existing teleport wire and accepts only public city destinations.
type TeleportCommand struct {
	Version           int    `json:"version"`
	OperationID       string `json:"operationId"`
	Epoch             string `json:"epoch"`
	UID               string `json:"uid"`
	Account           string `json:"account"`
	ExpectedSessionID int64  `json:"expectedSessionId"`
	Reason            string `json:"reason"`
	Destination       string `json:"destination"`
}

func (c TeleportCommand) Validate() error {
	if (c.Destination != "armia" && c.Destination != "azran" && c.Destination != "erion" && c.Destination != "nippleheim") || c.Version != 1 || len(c.Epoch) != 32 || strings.IndexFunc(c.Epoch, func(r rune) bool { return !(r >= '0' && r <= '9' || r >= 'a' && r <= 'f') }) >= 0 ||
		len(c.OperationID) < 16 || len(c.OperationID) > 64 || strings.IndexFunc(c.OperationID, func(r rune) bool {
		return !(r >= 'a' && r <= 'z' || r >= 'A' && r <= 'Z' || r >= '0' && r <= '9' || r == '-')
	}) >= 0 ||
		c.UID == "" || len(c.UID) > 128 || strings.TrimSpace(c.UID) != c.UID || !utf8.ValidString(c.UID) ||
		len(c.Account) < 1 || len(c.Account) > 12 || strings.IndexFunc(c.Account, func(r rune) bool { return !(r >= 'a' && r <= 'z' || r >= '0' && r <= '9') }) >= 0 ||
		c.ExpectedSessionID < 1 || c.ExpectedSessionID > 9007199254740991 ||
		!utf8.ValidString(c.Reason) || utf8.RuneCountInString(strings.TrimSpace(c.Reason)) < 3 || utf8.RuneCountInString(c.Reason) > 200 || strings.IndexFunc(c.Reason+c.UID, unicode.IsControl) >= 0 {
		return errors.New("invalid_command")
	}
	return nil
}

type TeleportResult struct {
	OperationID string `json:"operationId"`
	Code        string `json:"code"`
	Replayed    bool   `json:"replayed"`
	X           uint16 `json:"x"`
	Y           uint16 `json:"y"`
}

type TeleportSource interface {
	TeleportCommand(context.Context, string, TeleportCommand, func() bool) (TeleportResult, error)
}
