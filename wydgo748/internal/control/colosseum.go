package control

import (
	"context"
	"errors"
	"time"
)

const ColosseumCapability = "game.event.colosseum"

type ColosseumCommand struct {
	Version          int    `json:"version"`
	OperationID      string `json:"operationId"`
	Epoch            string `json:"epoch"`
	ExpectedRevision uint64 `json:"expectedRevision"`
	Tier             string `json:"tier"`
	Reason           string `json:"reason"`
}

func (c ColosseumCommand) Validate() error {
	if (OXEventCommand{Version: c.Version, OperationID: c.OperationID, Action: "mode_on", Reason: c.Reason}).Validate() != nil ||
		len(c.Epoch) != 32 || c.ExpectedRevision == 0 || (c.Tier != "N" && c.Tier != "M" && c.Tier != "A") {
		return errors.New("invalid_command")
	}
	return nil
}

type ColosseumSchedule struct {
	Tier     string    `json:"tier"`
	StartsAt time.Time `json:"startsAt"`
	Claimed  bool      `json:"claimed"`
}

type ColosseumStatus struct {
	Version      int                 `json:"version"`
	AsOf         time.Time           `json:"asOf"`
	Epoch        string              `json:"epoch"`
	Revision     uint64              `json:"revision"`
	Enabled      bool                `json:"enabled"`
	Active       bool                `json:"active"`
	Tier         string              `json:"tier"`
	Stage        int                 `json:"stage"`
	EndsAt       *time.Time          `json:"endsAt"`
	PendingExits int                 `json:"pendingExits"`
	Timezone     string              `json:"timezone"`
	Schedule     []ColosseumSchedule `json:"schedule"`
}

type ColosseumResult struct {
	OperationID string          `json:"operationId"`
	Code        string          `json:"code"`
	Replayed    bool            `json:"replayed"`
	Status      ColosseumStatus `json:"status"`
}

type ColosseumSource interface {
	ColosseumStatus(context.Context) (ColosseumStatus, error)
	ColosseumCommand(context.Context, string, ColosseumCommand, func() bool) (ColosseumResult, error)
}
