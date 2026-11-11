package game

import (
	"context"
	"errors"
	"testing"
	"time"

	"wydgo/internal/control"
)

func TestControlEnqueueRejectsCanceledBusyAndFullQueues(t *testing.T) {
	tests := []struct {
		name       string
		setPending func(*World, bool)
		isPending  func(*World) bool
		enqueue    func(*World, context.Context) error
	}{
		{
			name:       "colosseum",
			setPending: func(w *World, pending bool) { w.colosseumPending.Store(pending) },
			isPending:  func(w *World) bool { return w.colosseumPending.Load() },
			enqueue: func(w *World, ctx context.Context) error {
				_, err := w.enqueueColosseum(&colosseumRequest{ctx: ctx})
				return err
			},
		},
		{
			name:       "global drop",
			setPending: func(w *World, pending bool) { w.globalDropPending.Store(pending) },
			isPending:  func(w *World) bool { return w.globalDropPending.Load() },
			enqueue: func(w *World, ctx context.Context) error {
				_, err := w.enqueueGlobalDrop(&globalDropRequest{ctx: ctx})
				return err
			},
		},
		{
			name:       "quiz",
			setPending: func(w *World, pending bool) { w.quizPending.Store(pending) },
			isPending:  func(w *World) bool { return w.quizPending.Load() },
			enqueue: func(w *World, ctx context.Context) error {
				_, err := w.enqueueQuiz(&quizRequest{ctx: ctx})
				return err
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			t.Run("pre-canceled context", func(t *testing.T) {
				w := &World{commands: make(chan command, 1)}
				ctx, cancel := context.WithCancel(context.Background())
				cancel()
				err := tc.enqueue(w, ctx)
				if !errors.Is(err, context.Canceled) || tc.isPending(w) || len(w.commands) != 0 {
					t.Fatalf("canceled request returned err=%v pending=%v queued=%d", err, tc.isPending(w), len(w.commands))
				}
			})

			t.Run("already pending", func(t *testing.T) {
				w := &World{commands: make(chan command, 1)}
				tc.setPending(w, true)
				err := tc.enqueue(w, context.Background())
				if !errors.Is(err, control.ErrBusy) || !tc.isPending(w) || len(w.commands) != 0 {
					t.Fatalf("busy request returned err=%v pending=%v queued=%d", err, tc.isPending(w), len(w.commands))
				}
			})

			t.Run("full command queue", func(t *testing.T) {
				w := &World{commands: make(chan command, 1)}
				w.commands <- command{}
				err := tc.enqueue(w, context.Background())
				if !errors.Is(err, control.ErrBusy) || tc.isPending(w) || len(w.commands) != 1 {
					t.Fatalf("full-queue request returned err=%v pending=%v queued=%d", err, tc.isPending(w), len(w.commands))
				}
			})
		})
	}
}

func TestControlEnqueueReturnsActorResults(t *testing.T) {
	tests := []struct {
		name    string
		enqueue func(*World, context.Context) (string, error)
		reply   func(command) bool
	}{
		{
			name: "colosseum",
			enqueue: func(w *World, ctx context.Context) (string, error) {
				result, err := w.enqueueColosseum(&colosseumRequest{ctx: ctx})
				return result.Code, err
			},
			reply: func(cmd command) bool {
				if cmd.colosseum == nil {
					return false
				}
				cmd.colosseum.reply <- control.ColosseumResult{Code: "accepted"}
				return true
			},
		},
		{
			name: "global drop",
			enqueue: func(w *World, ctx context.Context) (string, error) {
				result, err := w.enqueueGlobalDrop(&globalDropRequest{ctx: ctx})
				return result.Code, err
			},
			reply: func(cmd command) bool {
				if cmd.globalDrop == nil {
					return false
				}
				cmd.globalDrop.reply <- control.GlobalDropResult{Code: "accepted"}
				return true
			},
		},
		{
			name: "quiz",
			enqueue: func(w *World, ctx context.Context) (string, error) {
				result, err := w.enqueueQuiz(&quizRequest{ctx: ctx})
				return result.Code, err
			},
			reply: func(cmd command) bool {
				if cmd.quiz == nil {
					return false
				}
				cmd.quiz.reply <- control.QuizResult{Code: "accepted"}
				return true
			},
		},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w := &World{commands: make(chan command, 1)}
			ctx, cancel := context.WithCancel(context.Background())
			defer cancel()
			type response struct {
				code string
				err  error
			}
			done := make(chan response, 1)
			go func() {
				code, err := tc.enqueue(w, ctx)
				done <- response{code: code, err: err}
			}()

			var queued command
			select {
			case queued = <-w.commands:
			case early := <-done:
				t.Fatalf("enqueue returned before queuing a request: code=%q err=%v", early.code, early.err)
			case <-time.After(time.Second):
				cancel()
				<-done
				t.Fatal("enqueue did not queue a request")
			}
			if !tc.reply(queued) {
				cancel()
				<-done
				t.Fatal("wrong control request was queued")
			}
			got := <-done
			if got.err != nil || got.code != "accepted" {
				t.Fatalf("enqueue returned code=%q err=%v", got.code, got.err)
			}
		})
	}
}
