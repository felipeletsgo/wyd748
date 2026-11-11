package game

import (
	"context"
	"errors"
	"testing"
	"time"

	"wydgo/internal/control"
)

func TestPvPDoorPublicQueueStatusAndCommand(t *testing.T) {
	w, _ := fixtureWorld(t)
	w.commands = make(chan command, 1)

	canceled, cancel := context.WithCancel(context.Background())
	cancel()
	if _, err := w.PvPDoorStatus(canceled); !errors.Is(err, context.Canceled) {
		t.Fatalf("status with canceled context = %v", err)
	}
	if w.pvpDoorPending.Load() {
		t.Fatal("canceled status left the door request marked pending")
	}

	w.commands <- command{}
	if _, err := w.PvPDoorStatus(context.Background()); !errors.Is(err, control.ErrBusy) {
		t.Fatalf("status with a full command queue = %v", err)
	}
	if w.pvpDoorPending.Load() {
		t.Fatal("full queue left the door request marked pending")
	}
	<-w.commands

	type statusResult struct {
		status control.PvPDoorStatus
		err    error
	}
	statusDone := make(chan statusResult, 1)
	go func() {
		status, err := w.PvPDoorStatus(context.Background())
		statusDone <- statusResult{status, err}
	}()
	queued := <-w.commands
	if queued.pvpDoor == nil {
		t.Fatal("status request was not queued")
	}
	if _, err := w.PvPDoorStatus(context.Background()); !errors.Is(err, control.ErrBusy) {
		t.Fatalf("overlapping status request = %v", err)
	}
	w.handlePvPDoor(queued.pvpDoor)
	select {
	case result := <-statusDone:
		if result.err != nil || !result.status.Available || result.status.Locked {
			t.Fatalf("initial public status=%+v err=%v", result.status, result.err)
		}
	case <-time.After(time.Second):
		t.Fatal("status caller did not receive its world-loop result")
	}

	ctx, stop := context.WithCancel(context.Background())
	canceledDone := make(chan error, 1)
	go func() {
		_, err := w.PvPDoorStatus(ctx)
		canceledDone <- err
	}()
	queued = <-w.commands
	stop()
	select {
	case err := <-canceledDone:
		if !errors.Is(err, context.Canceled) {
			t.Fatalf("queued status cancellation = %v", err)
		}
	case <-time.After(time.Second):
		t.Fatal("canceled status caller did not return")
	}
	w.handlePvPDoor(queued.pvpDoor)
	if w.pvpDoorPending.Load() {
		t.Fatal("canceled world-loop request left the door marked pending")
	}

	invalid := control.PvPDoorCommand{Version: 1, OperationID: "operation-lock-0001", Action: "destroy", Reason: "event control"}
	if _, err := w.PvPDoorCommand(context.Background(), "operator", invalid, func() bool { return true }); err == nil {
		t.Fatal("invalid public PvP door command was queued")
	}
	if len(w.commands) != 0 {
		t.Fatal("invalid PvP door command reached the world queue")
	}

	cmd := control.PvPDoorCommand{Version: 1, OperationID: "operation-lock-0002", Action: "lock", Reason: "event control"}
	commandDone := make(chan struct {
		result control.PvPDoorResult
		err    error
	}, 1)
	go func() {
		result, err := w.PvPDoorCommand(context.Background(), "operator", cmd, func() bool { return true })
		commandDone <- struct {
			result control.PvPDoorResult
			err    error
		}{result, err}
	}()
	queued = <-w.commands
	if queued.pvpDoor == nil || queued.pvpDoor.command == nil {
		t.Fatal("valid door command was not queued")
	}
	w.handlePvPDoor(queued.pvpDoor)
	select {
	case result := <-commandDone:
		if result.err != nil || result.result.Code != "ok" || !result.result.Status.Locked ||
			result.result.OperationID != cmd.OperationID {
			t.Fatalf("public PvP door command=%+v err=%v", result.result, result.err)
		}
	case <-time.After(time.Second):
		t.Fatal("command caller did not receive its world-loop result")
	}
}

func TestPvPDoorHandlerRejectsInvalidCapabilityAndMaintenance(t *testing.T) {
	w, _ := fixtureWorld(t)
	valid := control.PvPDoorCommand{Version: 1, OperationID: "operation-open-000001", Action: "open", Reason: "event control"}
	run := func(actor string, cmd *control.PvPDoorCommand, authorize func() bool) control.PvPDoorResult {
		req := &pvpDoorRequest{ctx: context.Background(), actor: actor, command: cmd, authorize: authorize,
			reply: make(chan control.PvPDoorResult, 1)}
		w.handlePvPDoor(req)
		return <-req.reply
	}
	invalid := valid
	invalid.Action = "remove"
	if result := run("operator", &invalid, func() bool { return true }); result.Code != "invalid_command" {
		t.Fatalf("handler accepted invalid command: %+v", result)
	}
	if result := run("", &valid, func() bool { return true }); result.Code != "capability_required" {
		t.Fatalf("handler accepted empty actor: %+v", result)
	}
	if result := run("operator", &valid, nil); result.Code != "capability_required" {
		t.Fatalf("handler accepted missing authorization callback: %+v", result)
	}
	w.shuttingDown = true
	if result := run("operator", &valid, func() bool { return true }); result.Code != "server_maintenance" {
		t.Fatalf("handler accepted a maintenance-time command: %+v", result)
	}

	missingDoor := &World{groundItems: map[uint16]*GroundItem{}}
	missingRequest := &pvpDoorRequest{ctx: context.Background(), actor: "operator", command: &valid,
		authorize: func() bool { return true }, reply: make(chan control.PvPDoorResult, 1)}
	missingDoor.handlePvPDoor(missingRequest)
	if result := <-missingRequest.reply; result.Code != "door_unavailable" || result.Status.Available {
		t.Fatalf("missing door result=%+v", result)
	}
}
