package game

import (
	"context"
	"encoding/binary"
	"errors"
	"fmt"
	"math/rand"
	"strings"
	"testing"
	"time"
	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func executeQuiz(w *World, c control.QuizCommand, allowed bool) control.QuizResult {
	r := &quizRequest{ctx: context.Background(), actor: "operator", command: &c, authorize: func() bool { return allowed }, reply: make(chan control.QuizResult, 1)}
	w.handleQuiz(r)
	return <-r.reply
}
func startTestQuiz(t *testing.T, w *World) control.QuizCommand {
	t.Helper()
	c := control.QuizCommand{Version: 1, OperationID: "quiz-operation-0001", Epoch: w.quizSnapshot().Epoch, Action: "start", Reason: "test quiz", ItemID: 500, RewardQuantity: 2, DurationSeconds: 120}
	if r := executeQuiz(w, c, true); r.Code != "ok" {
		t.Fatalf("start: %+v", r)
	}
	return c
}
func quizResponse(p *Player, q *quizParticipant) []byte {
	b := wire.Build(wire.OpQuizAnswer, p.ID, wire.QuizAnswerSize)
	binary.LittleEndian.PutUint16(b[0:2], wire.QuizAnswerSize)
	binary.LittleEndian.PutUint16(b[12:14], 1)
	binary.LittleEndian.PutUint16(b[14:16], q.correct)
	copy(b[16:32], q.token[:])
	return b
}
func TestQuizRewardExactlyOnceAndBeforePublish(t *testing.T) {
	w, p, _, st := killRewardTestWorld(t, nil)
	c := startTestQuiz(t, w)
	q := w.quiz.participants[p.Session]
	if q == nil {
		t.Fatal("not issued")
	}
	before := p.Session.QueuedPacketsForTest()
	b := quizResponse(p, q)
	w.handle(command{s: p.Session, pkt: b})
	w.handle(command{s: p.Session, pkt: b})
	if st.saves != 1 || st.queuedAtSave != before+1 || w.quiz.status.Rewarded != 1 || w.quiz.status.Correct != 1 {
		t.Fatal("duplicate or publication before commit", st.saves, st.queuedAtSave, w.quiz.status)
	}
	if p.Char.Inv[0].Index != 500 || p.Char.Inv[1].Index != 500 || p.Char.Inv[2].Index != 0 || p.Char.Inv[0].UID == "" || p.Char.Inv[0].UID == p.Char.Inv[1].UID {
		t.Fatal("reward materialization")
	}
	if r := executeQuiz(w, c, true); !r.Replayed || r.Status.Rounds != 1 {
		t.Fatal("start replay created round", r)
	}
	c.RewardQuantity++
	if r := executeQuiz(w, c, true); r.Code != "operation_conflict" {
		t.Fatal(r)
	}
}
func TestQuizRejectsForgedLateAndRepeatedAnswers(t *testing.T) {
	for _, mode := range []string{"no-event", "token", "choice", "id", "version", "reserved", "truncated", "tail", "wrong-first", "deadline", "old-round", "stopped", "logout", "account", "not-in-world"} {
		t.Run(mode, func(t *testing.T) {
			w, p, _, st := killRewardTestWorld(t, nil)
			clock := newFakeClock(time.Now())
			w.clock = clock
			startTestQuiz(t, w)
			q := w.quiz.participants[p.Session]
			b := quizResponse(p, q)
			switch mode {
			case "no-event":
				w.quiz = nil
			case "token":
				b[16] ^= 1
			case "choice":
				binary.LittleEndian.PutUint16(b[14:16], 4)
			case "id":
				binary.LittleEndian.PutUint16(b[6:8], p.ID+1)
			case "version":
				b[12] = 2
			case "reserved":
				b[32] = 1
			case "truncated":
				b = b[:35]
			case "tail":
				b = append(b, 0)
			case "wrong-first":
				wrong := append([]byte(nil), b...)
				binary.LittleEndian.PutUint16(wrong[14:16], (q.correct+1)%4)
				w.onQuizAnswer(p.Session, wrong)
			case "deadline":
				clock.Advance(10 * time.Second)
			case "old-round":
				clock.Advance(time.Minute)
				w.tickQuiz(clock.Now())
			case "stopped":
				w.endQuiz("manual")
			case "logout":
				w.removePlayerFromWorld(p, "quiz test")
				p.Char = q.character
				p.ID = binary.LittleEndian.Uint16(b[6:8])
				p.InWorld = true
			case "account":
				p.Account.Name = "other"
			case "not-in-world":
				p.InWorld = false
			}
			w.onQuizAnswer(p.Session, b)
			if st.saves != 0 || p.Char.Inv[0].Index != 0 {
				t.Fatal("forged answer rewarded")
			}
		})
	}
}
func TestQuizRoundScheduleAndTokenIsolation(t *testing.T) {
	w, p, _, _ := killRewardTestWorld(t, nil)
	clock := newFakeClock(time.Now())
	w.clock = clock
	startTestQuiz(t, w)
	old := w.quiz.participants[p.Session].token
	if w.quiz.status.Rounds != 1 {
		t.Fatal("first round not immediate")
	}
	late, _ := networkedTestPlayer(2, "Late", 2100, 2100)
	w.players[late.Session] = late
	clock.Advance(10 * time.Second)
	w.tickQuiz(clock.Now())
	if len(w.quiz.participants) != 0 || w.quiz.status.RoundClosesAt != nil {
		t.Fatal("round did not close")
	}
	clock.Advance(49 * time.Second)
	w.tickQuiz(clock.Now())
	if w.quiz.status.Rounds != 1 {
		t.Fatal("early question")
	}
	clock.Advance(time.Second)
	w.tickQuiz(clock.Now())
	if w.quiz.status.Rounds != 2 || w.quiz.participants[p.Session].token == old || w.quiz.participants[late.Session].token == w.quiz.participants[p.Session].token {
		t.Fatal("round/token isolation")
	}
	// A valid token is not transferable to another session, even with its ID.
	b := quizResponse(late, w.quiz.participants[p.Session])
	w.onQuizAnswer(late.Session, b)
	if w.quiz.participants[late.Session].answered {
		t.Fatal("cross-session token accepted")
	}
	clock.Advance(time.Minute)
	w.tickQuiz(clock.Now())
	if w.quiz.status.Active || w.quiz.status.EndedBy != "time" {
		t.Fatal("duration")
	}
}
func TestQuizRewardRollback(t *testing.T) {
	for _, failDB := range []bool{false, true} {
		t.Run(fmt.Sprint(failDB), func(t *testing.T) {
			var err error
			if failDB {
				err = errors.New("ambiguous commit")
			}
			w, p, _, st := killRewardTestWorld(t, err)
			startTestQuiz(t, w)
			if !failDB {
				for i := 1; i < len(p.Char.Inv); i++ {
					p.Char.Inv[i] = model.Item{Index: 1}
				}
			}
			before := cloneCharacterState(p.Char)
			b := quizResponse(p, w.quiz.participants[p.Session])
			w.onQuizAnswer(p.Session, b)
			w.onQuizAnswer(p.Session, b)
			if p.Char.Inv != before.Inv || w.quiz.status.Rewarded != 0 || w.quiz.status.DeliveryFailures != 1 {
				t.Fatal("partial/duplicate reward")
			}
			if failDB && (st.saves != 1 || !p.PersistencePoisoned || !p.Session.IsClosed()) {
				t.Fatal("ambiguous commit not isolated")
			}
			if !failDB && st.saves != 0 {
				t.Fatal("full inventory saved")
			}
		})
	}
}
func TestQuizMathAndChoices(t *testing.T) {
	w := &World{rng: rand.New(rand.NewSource(7))}
	both := map[string]bool{}
	for i := 0; i < 10000; i++ {
		q, a := w.quizQuestion()
		var left, right int
		var op string
		if n, _ := fmt.Sscanf(q, "%d %s %d = ?", &left, &op, &right); n != 3 || left < 10 || left > 99 || right < 2 || right > 9 {
			t.Fatal(q)
		}
		both[op] = true
		if op == "x" && int(a) != left*right || op == "/" && (left%right != 0 || int(a) != left/right) {
			t.Fatal(q, a)
		}
	}
	if len(both) != 2 {
		t.Fatal("missing operation")
	}
}
func TestQuizStopAtReceiptCapacityAndRevocation(t *testing.T) {
	w, _, _, _ := killRewardTestWorld(t, nil)
	startTestQuiz(t, w)
	for i := len(w.quizReceipts); i < 1024; i++ {
		w.quizReceipts[fmt.Sprint(i)] = quizReceipt{}
	}
	s := w.quizSnapshot()
	c := control.QuizCommand{Version: 1, Epoch: s.Epoch, ExpectedEventID: s.EventID, OperationID: "stop-operation-0001", Action: "stop", Reason: "test stop"}
	if r := executeQuiz(w, c, false); r.Code != "capability_required" || !r.Status.Active {
		t.Fatal(r)
	}
	if r := executeQuiz(w, c, true); r.Code != "ok" || r.Status.Active {
		t.Fatal("cannot stop at capacity", r)
	}
	if r := executeQuiz(w, c, true); !r.Replayed {
		t.Fatal("stop retry", r)
	}
}

func TestQuizQueueCancellationAndMaintenance(t *testing.T) {
	w, _, _, _ := killRewardTestWorld(t, nil)
	w.commands = make(chan command, 1)
	c := control.QuizCommand{Version: 1, OperationID: "quiz-queued-00001", Epoch: w.quizSnapshot().Epoch, Action: "start", Reason: "test queue", ItemID: 500, RewardQuantity: 1, DurationSeconds: 60}
	ctx, cancel := context.WithCancel(context.Background())
	done := make(chan error, 1)
	go func() { _, err := w.QuizCommand(ctx, "operator", c, func() bool { return true }); done <- err }()
	queued := <-w.commands
	if _, err := w.QuizStatus(context.Background()); !errors.Is(err, control.ErrBusy) {
		t.Fatal("unbounded admission", err)
	}
	cancel()
	if err := <-done; !errors.Is(err, context.Canceled) {
		t.Fatal(err)
	}
	w.handle(queued)
	if w.quiz != nil || w.quizPending.Load() {
		t.Fatal("canceled command applied or admission stuck")
	}
	w.shuttingDown = true
	if r := executeQuiz(w, c, true); r.Code != "server_maintenance" || r.Status.Active {
		t.Fatal(r)
	}
}

func TestQuizOneAttemptPerAccountAcrossSessions(t *testing.T) {
	w, p, _, st := killRewardTestWorld(t, nil)
	other, _ := networkedTestPlayer(2, "Other", 2100, 2100)
	other.Account.Name = strings.ToUpper(p.Account.Name)
	w.players[other.Session] = other
	startTestQuiz(t, w)
	q := w.quiz.participants[p.Session]
	wrong := quizResponse(p, q)
	binary.LittleEndian.PutUint16(wrong[14:16], (q.correct+1)%4)
	w.handle(command{s: p.Session, pkt: wrong})
	w.handle(command{s: other.Session, pkt: quizResponse(other, w.quiz.participants[other.Session])})
	if st.saves != 0 || w.quiz.status.Correct != 0 || w.quiz.status.Rewarded != 0 {
		t.Fatal("second session bypassed consumed account attempt")
	}
}
