package control

import (
	"strings"
	"testing"
)

func TestQuizCommandValidation(t *testing.T) {
	valid := QuizCommand{Version: 1, OperationID: "quiz-operation-0001", Epoch: strings.Repeat("a", 32), Action: "start", Reason: "math event", ItemID: 500, RewardQuantity: 2, DurationSeconds: 60}
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	for _, change := range []func(*QuizCommand){
		func(c *QuizCommand) { c.RewardQuantity = 0 }, func(c *QuizCommand) { c.RewardQuantity = 61 },
		func(c *QuizCommand) { c.DurationSeconds = 59 }, func(c *QuizCommand) { c.DurationSeconds = 61 }, func(c *QuizCommand) { c.DurationSeconds = 86460 },
		func(c *QuizCommand) { c.Version = 2 }, func(c *QuizCommand) { c.ItemID = 0 }, func(c *QuizCommand) { c.Epoch = "" }, func(c *QuizCommand) { c.Reason = "bad\nlog" },
	} {
		c := valid
		change(&c)
		if c.Validate() == nil {
			t.Fatal(c)
		}
	}
	valid.Action = "stop"
	valid.ExpectedEventID = "event-1"
	valid.ItemID = 0
	valid.RewardQuantity = 0
	valid.DurationSeconds = 0
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	valid.RewardQuantity = 1
	if valid.Validate() == nil {
		t.Fatal("hidden stop settings")
	}
}
