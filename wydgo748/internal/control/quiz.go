package control

import (
	"context"
	"errors"
	"time"
)

const QuizCapability = "game.event.quiz"

type QuizCommand struct {
	Version         int    `json:"version"`
	OperationID     string `json:"operationId"`
	Epoch           string `json:"epoch"`
	ExpectedEventID string `json:"expectedEventId"`
	Action          string `json:"action"`
	Reason          string `json:"reason"`
	ItemID          uint16 `json:"itemId"`
	RewardQuantity  int    `json:"rewardQuantity"`
	DurationSeconds int    `json:"durationSeconds"`
}

func (c QuizCommand) Validate() error {
	// Reuse the command envelope's strict version, reason and operation-id rules.
	envelope := GlobalDropCommand{Version: c.Version, OperationID: c.OperationID, Epoch: c.Epoch,
		ExpectedEventID: c.ExpectedEventID, Action: c.Action, Reason: c.Reason,
		ItemID: c.ItemID, DurationSeconds: c.DurationSeconds}
	if c.Action == "start" {
		envelope.RatePercent = 100
	}
	if envelope.Validate() != nil {
		return errors.New("invalid_command")
	}
	if c.Action == "start" && (c.RewardQuantity < 1 || c.RewardQuantity > 60 || c.DurationSeconds < 60 || c.DurationSeconds > 86400 || c.DurationSeconds%60 != 0) ||
		c.Action == "stop" && c.RewardQuantity != 0 {
		return errors.New("invalid_command")
	}
	return nil
}

type QuizStatus struct {
	Version          int        `json:"version"`
	Epoch            string     `json:"epoch"`
	AsOf             time.Time  `json:"asOf"`
	EventID          string     `json:"eventId"`
	Active           bool       `json:"active"`
	ItemID           uint16     `json:"itemId"`
	ItemName         string     `json:"itemName"`
	RewardQuantity   int        `json:"rewardQuantity"`
	Rounds           int        `json:"rounds"`
	Correct          int        `json:"correct"`
	Rewarded         int        `json:"rewarded"`
	DeliveryFailures int        `json:"deliveryFailures"`
	StartedAt        *time.Time `json:"startedAt"`
	EndsAt           *time.Time `json:"endsAt"`
	NextRoundAt      *time.Time `json:"nextRoundAt"`
	RoundClosesAt    *time.Time `json:"roundClosesAt"`
	EndedBy          string     `json:"endedBy"`
	Actor            string     `json:"actor"`
}
type QuizResult struct {
	OperationID string     `json:"operationId"`
	Code        string     `json:"code"`
	Replayed    bool       `json:"replayed"`
	Status      QuizStatus `json:"status"`
}
type QuizSource interface {
	QuizStatus(context.Context) (QuizStatus, error)
	QuizCommand(context.Context, string, QuizCommand, func() bool) (QuizResult, error)
}
