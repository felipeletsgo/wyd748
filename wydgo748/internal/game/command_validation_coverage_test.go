package game

import (
	"context"
	"errors"
	"testing"

	"wydgo/internal/control"
	"wydgo/internal/lang"
)

func TestControlCommandEndpointsRejectInvalidCommandsBeforeEnqueue(t *testing.T) {
	w := &World{commands: make(chan command, 1)}
	ctx := context.Background()
	allow := func() bool { return true }
	checks := []struct {
		name string
		call func() error
	}{
		{name: "Colosseum", call: func() error {
			_, err := w.ColosseumCommand(ctx, "operator", control.ColosseumCommand{}, allow)
			return err
		}},
		{name: "GlobalDrop", call: func() error {
			_, err := w.GlobalDropCommand(ctx, "operator", control.GlobalDropCommand{}, allow)
			return err
		}},
		{name: "Quiz", call: func() error {
			_, err := w.QuizCommand(ctx, "operator", control.QuizCommand{}, allow)
			return err
		}},
		{name: "Bosses", call: func() error {
			_, err := w.BossesCommand(ctx, "operator", control.BossesCommand{}, allow)
			return err
		}},
	}
	for _, check := range checks {
		t.Run(check.name, func(t *testing.T) {
			if err := check.call(); err == nil {
				t.Fatal("invalid command was accepted")
			}
			if len(w.commands) != 0 {
				t.Fatal("invalid command reached the world queue")
			}
		})
	}
}

func TestMessageCatalogUsesOverrideAndDefaultFallback(t *testing.T) {
	configured := lang.Default()
	if got := (&World{messages: configured}).messageCatalog(); got != configured {
		t.Fatal("world did not retain its configured message catalog")
	}
	if got := (&World{}).messageCatalog(); got == nil {
		t.Fatal("world without an override has no default message catalog")
	}
}

func TestControlCommandEndpointsPassValidCommandsToEnqueue(t *testing.T) {
	w := &World{commands: make(chan command, 1)}
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	epoch := "00000000000000000000000000000000"
	checks := []struct {
		name string
		call func() error
	}{
		{name: "Colosseum", call: func() error {
			_, err := w.ColosseumCommand(ctx, "operator", control.ColosseumCommand{
				Version: 1, OperationID: "colosseum-valid-0001", Epoch: epoch, ExpectedRevision: 1, Tier: "N", Reason: "start event early",
			}, func() bool { return true })
			return err
		}},
		{name: "GlobalDrop", call: func() error {
			_, err := w.GlobalDropCommand(ctx, "operator", control.GlobalDropCommand{
				Version: 1, OperationID: "globaldrop-valid-001", Epoch: epoch, ExpectedEventID: "event-1",
				Action: "start", Reason: "start fixture event", ItemID: 500, RatePercent: 100, MaxDrops: 2,
			}, func() bool { return true })
			return err
		}},
		{name: "Quiz", call: func() error {
			_, err := w.QuizCommand(ctx, "operator", control.QuizCommand{
				Version: 1, OperationID: "quiz-valid-command-01", Epoch: epoch, Action: "start", Reason: "start fixture quiz",
				ItemID: 500, RewardQuantity: 2, DurationSeconds: 120,
			}, func() bool { return true })
			return err
		}},
		{name: "Bosses", call: func() error {
			_, err := w.BossesCommand(ctx, "operator", control.BossesCommand{
				Version: 1, OperationID: "boss-valid-command-01", Epoch: epoch, BossID: "boss-test", ExpectedRevision: 1,
				Action: "summon", Reason: "community encounter",
			}, func() bool { return true })
			return err
		}},
	}
	for _, check := range checks {
		t.Run(check.name, func(t *testing.T) {
			if err := check.call(); !errors.Is(err, context.Canceled) {
				t.Fatalf("valid command enqueue error = %v, want canceled context", err)
			}
			if len(w.commands) != 0 {
				t.Fatal("canceled command reached the world queue")
			}
		})
	}
}
