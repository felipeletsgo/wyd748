package game

import (
	"context"
	"errors"
	"testing"
	"time"

	"wydgo/internal/control"
)

func TestEnqueuePersistenceRecoveryHandlesBusyCanceledAndFullQueues(t *testing.T) {
	t.Run("already canceled", func(t *testing.T) {
		ctx, cancel := context.WithCancel(context.Background())
		cancel()
		w := &World{commands: make(chan command, 1)}
		if _, err := w.enqueuePersistenceRecovery(&persistenceRecoveryRequest{ctx: ctx}); !errors.Is(err, context.Canceled) || w.recoveryPending.Load() {
			t.Fatalf("canceled request error=%v pending=%v", err, w.recoveryPending.Load())
		}
	})

	t.Run("request already pending", func(t *testing.T) {
		w := &World{commands: make(chan command, 1)}
		w.recoveryPending.Store(true)
		if _, err := w.enqueuePersistenceRecovery(&persistenceRecoveryRequest{ctx: context.Background()}); !errors.Is(err, control.ErrBusy) {
			t.Fatalf("pending request error=%v, want busy", err)
		}
	})

	t.Run("command queue full", func(t *testing.T) {
		w := &World{commands: make(chan command)}
		if _, err := w.enqueuePersistenceRecovery(&persistenceRecoveryRequest{ctx: context.Background()}); !errors.Is(err, control.ErrBusy) || w.recoveryPending.Load() {
			t.Fatalf("full queue error=%v pending=%v", err, w.recoveryPending.Load())
		}
	})

	t.Run("caller cancels after actor accepts request", func(t *testing.T) {
		ctx, cancel := context.WithCancel(context.Background())
		defer cancel()
		w := &World{commands: make(chan command, 1)}
		done := make(chan error, 1)
		go func() {
			_, err := w.enqueuePersistenceRecovery(&persistenceRecoveryRequest{ctx: ctx})
			done <- err
		}()
		var queued command
		select {
		case queued = <-w.commands:
		case <-time.After(time.Second):
			t.Fatal("recovery request was not queued")
		}
		cancel()
		select {
		case err := <-done:
			if !errors.Is(err, context.Canceled) {
				t.Fatalf("canceled caller error=%v", err)
			}
		case <-time.After(time.Second):
			t.Fatal("canceled recovery caller did not return")
		}
		if !w.recoveryPending.Load() {
			t.Fatal("caller cancellation released the actor's pending request")
		}
		w.handle(queued)
		if w.recoveryPending.Load() {
			t.Fatal("actor did not release the canceled request")
		}
	})
}
