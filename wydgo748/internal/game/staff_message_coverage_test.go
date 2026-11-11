package game

import (
	"context"
	"testing"

	"wydgo/internal/control"
)

func TestMessagePlayerCoversAuthorizationEpochCapacityReplayAndConflict(t *testing.T) {
	w, player, _ := handlerTestWorld(t)
	player.Account.Name = "player"
	w.commands = make(chan command, 1)
	stop, done := make(chan struct{}), make(chan struct{})
	go func() {
		defer close(done)
		runStaffQueue(w, stop)
	}()
	t.Cleanup(func() {
		close(stop)
		<-done
	})

	newCommand := func(operation, message string) control.PlayerMessageCommand {
		return control.PlayerMessageCommand{Version: 1, OperationID: operation, Epoch: w.moderationEpoch(),
			UID: player.Char.UID, Account: "player", ExpectedSessionID: player.Session.ID, Message: message}
	}

	unauthorized := newCommand("operation-0000001", "Contact staff for assistance.")
	if result, err := w.MessagePlayer(context.Background(), "operator", unauthorized, func() bool { return false }); err != nil || result.Code != "capability_required" {
		t.Fatalf("unauthorized message returned result=%+v err=%v", result, err)
	}

	staleEpoch := newCommand("operation-0000002", "Contact staff for assistance.")
	if staleEpoch.Epoch[0] == '0' {
		staleEpoch.Epoch = "1" + staleEpoch.Epoch[1:]
	} else {
		staleEpoch.Epoch = "0" + staleEpoch.Epoch[1:]
	}
	if result, err := w.MessagePlayer(context.Background(), "operator", staleEpoch, func() bool { return true }); err != nil || result.Code != "stale_target" {
		t.Fatalf("stale moderation epoch returned result=%+v err=%v", result, err)
	}

	w.staffOps.receiptCount = map[string]int{"message": maxStaffReceiptsPerKind}
	if result, err := w.MessagePlayer(context.Background(), "operator",
		newCommand("operation-0000003", "Contact staff for assistance."), func() bool { return true }); err != nil || result.Code != "operation_capacity" {
		t.Fatalf("full message receipt set returned result=%+v err=%v", result, err)
	}
	w.staffOps.receiptCount["message"] = 0

	cmd := newCommand("operation-0000004", "Contact staff for assistance.")
	before := player.Session.QueuedPacketsForTest()
	result, err := w.MessagePlayer(context.Background(), "operator", cmd, func() bool { return true })
	if err != nil || result.Code != "ok" || player.Session.QueuedPacketsForTest() != before+2 {
		t.Fatalf("valid message returned result=%+v err=%v packets=%d", result, err, player.Session.QueuedPacketsForTest()-before)
	}

	result, err = w.MessagePlayer(context.Background(), "operator", cmd, func() bool { return true })
	if err != nil || result.Code != "ok" || !result.Replayed || player.Session.QueuedPacketsForTest() != before+2 {
		t.Fatalf("message replay returned result=%+v err=%v or emitted new packets", result, err)
	}

	conflict := cmd
	conflict.Message = "This operation ID now has another message."
	result, err = w.MessagePlayer(context.Background(), "operator", conflict, func() bool { return true })
	if err != nil || result.Code != "operation_conflict" || player.Session.QueuedPacketsForTest() != before+2 {
		t.Fatalf("conflicting message returned result=%+v err=%v or emitted packets", result, err)
	}
}
