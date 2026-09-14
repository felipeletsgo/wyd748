package game

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"log"
	"strings"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/net"
)

// EXTENSAO_COORDENADA: versioned quiz with server-owned rounds and rewards.
// Event lifetime is explicitly process-local; a restart never resumes issuance.
type quizState struct {
	status       control.QuizStatus
	participants map[*net.Session]*quizParticipant
	claimed      map[string]bool
}
type quizReceipt struct {
	command control.QuizCommand
	result  control.QuizResult
}
type quizRequest struct {
	ctx       context.Context
	actor     string
	command   *control.QuizCommand
	authorize func() bool
	reply     chan control.QuizResult
}

func (w *World) QuizStatus(ctx context.Context) (control.QuizStatus, error) {
	r, err := w.enqueueQuiz(&quizRequest{ctx: ctx})
	return r.Status, err
}
func (w *World) QuizCommand(ctx context.Context, actor string, cmd control.QuizCommand, authorize func() bool) (control.QuizResult, error) {
	if err := cmd.Validate(); err != nil {
		return control.QuizResult{}, err
	}
	return w.enqueueQuiz(&quizRequest{ctx: ctx, actor: actor, command: &cmd, authorize: authorize})
}
func (w *World) enqueueQuiz(req *quizRequest) (control.QuizResult, error) {
	if err := req.ctx.Err(); err != nil {
		return control.QuizResult{}, err
	}
	if !w.quizPending.CompareAndSwap(false, true) {
		return control.QuizResult{}, control.ErrBusy
	}
	req.reply = make(chan control.QuizResult, 1)
	select {
	case w.commands <- command{quiz: req, queuedAt: time.Now()}:
	default:
		w.quizPending.Store(false)
		return control.QuizResult{}, control.ErrBusy
	}
	select {
	case result := <-req.reply:
		return result, nil
	case <-req.ctx.Done():
		return control.QuizResult{}, req.ctx.Err()
	}
}
func (w *World) quizSnapshot() control.QuizStatus {
	if w.quizEpoch == "" {
		var b [16]byte
		if _, err := rand.Read(b[:]); err != nil {
			panic(err)
		}
		w.quizEpoch = hex.EncodeToString(b[:])
	}
	if w.quiz != nil && w.quiz.status.EndsAt != nil && !w.now().Before(*w.quiz.status.EndsAt) {
		w.endQuiz("time")
	}
	s := control.QuizStatus{}
	if w.quiz != nil {
		s = w.quiz.status
	}
	s.Version, s.Epoch, s.AsOf = 1, w.quizEpoch, w.now().UTC()
	return s
}
func (w *World) handleQuiz(req *quizRequest) {
	defer w.quizPending.Store(false)
	if req.ctx.Err() != nil {
		return
	}
	result := control.QuizResult{Code: "ok", Status: w.quizSnapshot()}
	if req.command == nil {
		req.reply <- result
		return
	}
	c := *req.command
	result.OperationID = c.OperationID
	finish := func(code string) { result.Code = code; req.reply <- result }
	if c.Validate() != nil {
		finish("invalid_command")
		return
	}
	if req.actor == "" || req.authorize == nil || !req.authorize() {
		finish("capability_required")
		return
	}
	if req.ctx.Err() != nil {
		return
	}
	if w.shuttingDown {
		finish("server_maintenance")
		return
	}
	if c.Epoch != w.quizEpoch {
		finish("stale_event")
		return
	}
	key := req.actor + ":" + c.OperationID
	if receipt, exists := w.quizReceipts[key]; exists {
		if receipt.command != c {
			finish("operation_conflict")
			return
		}
		result = receipt.result
		result.Replayed = true
		// Return current state, not a stale copy from before later kills/stops.
		result.Status = w.quizSnapshot()
		req.reply <- result
		return
	}
	// Exhausting the bounded receipt cache must never prevent stopping the
	// current event. This permits at most one extra receipt until restart.
	canStop := c.Action == "stop" && result.Status.Active && c.ExpectedEventID == result.Status.EventID
	if len(w.quizReceipts) >= 1024 && !canStop {
		finish("operation_capacity")
		return
	}
	code := "ok"
	switch {
	case c.ExpectedEventID != result.Status.EventID:
		code = "stale_event"
	case c.Action == "start" && result.Status.Active:
		code = "event_active"
	case c.Action == "stop" && !result.Status.Active:
		code = "event_inactive"
	case c.Action == "start":
		item, exists := w.items[c.ItemID]
		if !exists || strings.TrimSpace(item.Name) == "" {
			code = "unknown_item"
			break
		}
		now := w.now().UTC()
		var eventID [16]byte
		if _, err := rand.Read(eventID[:]); err != nil {
			code = "event_id_unavailable"
			break
		}
		s := control.QuizStatus{EventID: hex.EncodeToString(eventID[:]), Active: true, ItemID: c.ItemID, ItemName: item.Name,
			RewardQuantity: c.RewardQuantity, StartedAt: &now, Actor: req.actor, NextRoundAt: &now}
		if c.DurationSeconds > 0 {
			end := now.Add(time.Duration(c.DurationSeconds) * time.Second)
			s.EndsAt = &end
		}
		w.quiz = &quizState{status: s}
		w.tickQuiz(now)
	case c.Action == "stop":
		w.endQuiz("manual")
	}
	result.Code, result.Status = code, w.quizSnapshot()
	if w.quizReceipts == nil {
		w.quizReceipts = make(map[string]quizReceipt)
	}
	w.quizReceipts[key] = quizReceipt{command: c, result: result}
	log.Printf("[web-admin] quiz actor=%q operation=%q action=%q reason=%q result=%q item=%d quantity=%d seconds=%d", req.actor, c.OperationID, c.Action, c.Reason, code, c.ItemID, c.RewardQuantity, c.DurationSeconds)
	req.reply <- result
}
