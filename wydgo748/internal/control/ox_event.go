package control

import (
	"context"
	"errors"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"
)

// OXEventCapability lets staff run the OX quiz arena, the 7.59 TMSRV
// "quizmode", "quizdoor", "quizresultx" and "quizresulto" commands. Every
// action is idempotent or reports how many players it moved, so no receipt is
// kept.
const OXEventCapability = "game.event.ox"

var oxEventActions = map[string]bool{
	"mode_on": true, "mode_off": true, "doors_lock": true, "doors_open": true,
	"result_x": true, "result_o": true,
}

type OXEventCommand struct {
	Version     int    `json:"version"`
	OperationID string `json:"operationId"`
	Action      string `json:"action"`
	Reason      string `json:"reason"`
}

func (c OXEventCommand) Validate() error {
	if c.Version != 1 || len(c.OperationID) < 16 || len(c.OperationID) > 64 || !oxEventActions[c.Action] ||
		!utf8.ValidString(c.Reason) || utf8.RuneCountInString(strings.TrimSpace(c.Reason)) < 3 ||
		utf8.RuneCountInString(c.Reason) > 200 || strings.IndexFunc(c.Reason, unicode.IsControl) >= 0 ||
		strings.IndexFunc(c.OperationID, func(r rune) bool {
			return !(r >= 'a' && r <= 'z' || r >= 'A' && r <= 'Z' || r >= '0' && r <= '9' || r == '-')
		}) >= 0 {
		return errors.New("invalid_command")
	}
	return nil
}

type OXEventStatus struct {
	Version     int       `json:"version"`
	AsOf        time.Time `json:"asOf"`
	QuizMode    bool      `json:"quizMode"`
	Doors       int       `json:"doors"`
	DoorsLocked bool      `json:"doorsLocked"`
	PlayersX    int       `json:"playersX"`
	PlayersO    int       `json:"playersO"`
}

type OXEventResult struct {
	OperationID string        `json:"operationId"`
	Code        string        `json:"code"`
	Moved       int           `json:"moved"`
	Status      OXEventStatus `json:"status"`
}

type OXEventSource interface {
	OXEventStatus(context.Context) (OXEventStatus, error)
	OXEventCommand(context.Context, string, OXEventCommand, func() bool) (OXEventResult, error)
}
