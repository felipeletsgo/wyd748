package game

import (
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type criticalQuizStore struct{ *criticalTestStore }

func (s *criticalQuizStore) SaveCriticalAccounts(accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts})
}

func criticalQuizWorld(t *testing.T, failure error, release <-chan struct{}, capacity int) (*World, *Player, *criticalTestStore, *criticalPersistence) {
	t.Helper()
	w, p, _, _ := killRewardTestWorld(t, nil)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: release, failure: failure}
	backend := &criticalQuizStore{criticalTestStore: base}
	queue := newCriticalTestQueue(t, backend, 1, capacity)
	owner := newCriticalPersistenceOwner(queue)
	owner.activeWorld = w
	w.store, w.criticalWrites = backend, owner
	claimCriticalTestPlayers(t, w, p)
	return w, p, base, queue
}

func TestCriticalQuizRewardPublishesOnlyAfterAcknowledgement(t *testing.T) {
	gate, release := criticalGate(t)
	w, p, backend, queue := criticalQuizWorld(t, nil, gate, 2)
	t.Cleanup(release)
	startTestQuiz(t, w)
	participant := w.quiz.participants[p.Session]
	answer := quizResponse(p, participant)
	beforePackets := p.Session.QueuedPacketsForTest()
	w.onQuizAnswer(p.Session, answer)
	write := awaitCriticalWrite(t, backend.entered)
	if len(write.accounts) != 1 || write.accounts[0].Chars[p.CharSlot].Inv[0].Index != 500 ||
		!participant.answered || !w.quiz.claimed[participant.account] || w.quiz.criticalRewards != 1 ||
		w.quiz.status.Rewarded != 0 || w.quiz.status.Correct != 0 || p.Char.Inv[0].Index != 0 ||
		p.Session.QueuedPacketsForTest() != beforePackets {
		t.Fatal("quiz reward escaped before its durable acknowledgement")
	}
	release()
	result := awaitCriticalResult(t, queue)
	if result.err != nil {
		t.Fatalf("save critical quiz reward: %v", result.err)
	}
	w.applyCriticalCompletion(result)
	if p.Char.Inv[0].Index != 500 || p.Char.Inv[1].Index != 500 || w.quiz.criticalRewards != 0 ||
		w.quiz.status.Rewarded != 1 || w.quiz.status.Correct != 1 || len(queue.pending) != 0 || p.PersistencePoisoned {
		t.Fatalf("acknowledged quiz reward did not settle exactly once: slots=%d,%d critical=%d status=%+v pending=%d poisoned=%t",
			p.Char.Inv[0].Index, p.Char.Inv[1].Index, w.quiz.criticalRewards, w.quiz.status, len(queue.pending), p.PersistencePoisoned)
	}
	w.onQuizAnswer(p.Session, answer)
	if len(backend.entered) != 0 || w.quiz.status.Rewarded != 1 {
		t.Fatal("replayed quiz answer submitted another reward")
	}
}

func TestCriticalQuizRewardCertifiedRejectionKeepsSessionAndConsumesAttempt(t *testing.T) {
	failure := store.RejectUnsentWrite("quiz rejection fixture")
	w, p, backend, queue := criticalQuizWorld(t, failure, nil, 2)
	startTestQuiz(t, w)
	participant := w.quiz.participants[p.Session]
	answer := quizResponse(p, participant)
	w.onQuizAnswer(p.Session, answer)
	awaitCriticalWrite(t, backend.entered)
	result := awaitCriticalResult(t, queue)
	if result.err == nil {
		t.Fatal("critical quiz save rejection was lost")
	}
	w.applyCriticalCompletion(result)
	if p.Char.Inv[0].Index != 0 || w.quiz.status.Rewarded != 0 || w.quiz.status.Correct != 1 ||
		w.quiz.status.DeliveryFailures != 1 || w.quiz.criticalRewards != 0 ||
		!participant.answered || !w.quiz.claimed[participant.account] || p.PersistencePoisoned || p.Session.IsClosed() ||
		len(queue.pending) != 0 {
		t.Fatalf("certified quiz rejection did not settle safely: slot=%d critical=%d status=%+v answered=%t claimed=%t poisoned=%t closed=%t pending=%d",
			p.Char.Inv[0].Index, w.quiz.criticalRewards, w.quiz.status, participant.answered, w.quiz.claimed[participant.account],
			p.PersistencePoisoned, p.Session.IsClosed(), len(queue.pending))
	}
	w.onQuizAnswer(p.Session, answer)
	if len(backend.entered) != 0 {
		t.Fatal("rejected quiz answer was retried for a second durable reward")
	}
}

func TestCriticalQuizRewardPreparationFailureDoesNotSubmit(t *testing.T) {
	for _, scenario := range []string{"full inventory"} {
		t.Run(scenario, func(t *testing.T) {
			w, p, backend, queue := criticalQuizWorld(t, nil, nil, 2)
			startTestQuiz(t, w)
			for i := range p.Char.Inv {
				p.Char.Inv[i] = model.Item{Index: 1}
			}
			participant := w.quiz.participants[p.Session]
			w.onQuizAnswer(p.Session, quizResponse(p, participant))
			if len(backend.entered) != 0 || !participant.answered || !w.quiz.claimed[participant.account] ||
				w.quiz.status.Correct != 1 || w.quiz.status.DeliveryFailures != 1 || w.quiz.criticalRewards != 0 ||
				len(queue.pending) != 0 {
				t.Fatalf("full inventory reward reached persistence or remained retryable: answered=%t claimed=%t status=%+v pending=%d",
					participant.answered, w.quiz.claimed[participant.account], w.quiz.status, len(queue.pending))
			}
		})
	}
}

func TestCriticalQuizBusyAdmissionCanRetryAfterEarlierWriteSettles(t *testing.T) {
	gate, release := criticalGate(t)
	w, p, backend, queue := criticalQuizWorld(t, nil, gate, 1)
	t.Cleanup(release)
	id, err := queue.submit(criticalWrite{accounts: []*model.Account{{Name: "unrelated"}}})
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	startTestQuiz(t, w)
	participant := w.quiz.participants[p.Session]
	answer := quizResponse(p, participant)
	w.onQuizAnswer(p.Session, answer)
	if participant.answered || w.quiz.claimed[participant.account] || w.quiz.criticalRewards != 0 ||
		w.quiz.status.Rewarded != 0 || w.quiz.status.Correct != 0 || p.Char.Inv[0].Index != 0 {
		t.Fatal("busy critical queue consumed or published the quiz attempt")
	}
	release()
	result := awaitCriticalResult(t, queue)
	if result.id != id || result.err != nil {
		t.Fatalf("settle earlier write: %+v", result)
	}
	if err := queue.acknowledge(id); err != nil {
		t.Fatal(err)
	}
	w.onQuizAnswer(p.Session, answer)
	awaitCriticalWrite(t, backend.entered)
	result = awaitCriticalResult(t, queue)
	if result.err != nil {
		t.Fatalf("save retried quiz reward: %v", result.err)
	}
	w.applyCriticalCompletion(result)
	if p.Char.Inv[0].Index != 500 || w.quiz.status.Rewarded != 1 || len(queue.pending) != 0 {
		t.Fatalf("quiz attempt did not succeed after queue capacity became available: slot=%d status=%+v critical=%d pending=%d",
			p.Char.Inv[0].Index, w.quiz.status, w.quiz.criticalRewards, len(queue.pending))
	}
}
