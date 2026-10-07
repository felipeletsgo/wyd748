package game

import (
	"errors"
	"runtime"
	"sync"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type criticalTestStore struct {
	entered         chan criticalWrite
	release         <-chan struct{}
	failure         error
	panicAfterWrite bool
}

func (s *criticalTestStore) LoadAccount(string) (*model.Account, error) { return nil, nil }
func (s *criticalTestStore) CharacterNameExists(string) (bool, error)   { return false, nil }
func (s *criticalTestStore) SaveAccount(account *model.Account) error {
	return s.record(criticalWrite{accounts: []*model.Account{account}})
}
func (s *criticalTestStore) record(write criticalWrite) error {
	s.entered <- write
	if s.release != nil {
		<-s.release
	}
	if s.panicAfterWrite {
		panic("injected after potential commit")
	}
	return s.failure
}

type criticalBatchTestStore struct{ *criticalTestStore }

type criticalSelectiveTestStore struct {
	*criticalTestStore
	blocked <-chan struct{}
}

func (s *criticalSelectiveTestStore) SaveAccount(account *model.Account) error {
	s.entered <- criticalWrite{accounts: []*model.Account{account}}
	if account.Name == "slow" {
		<-s.blocked
	}
	return nil
}

func (s *criticalBatchTestStore) SaveAccounts(accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts})
}

type criticalStateTestStore struct{ *criticalBatchTestStore }

func (s *criticalStateTestStore) SavePlayerState(_ *model.GuildRegistry, account *model.Account,
	uid string, state *model.CharState) error {
	return s.record(criticalWrite{accounts: []*model.Account{account},
		character: &criticalCharacterState{uid: uid, state: state}})
}

func newCriticalTestQueue(t *testing.T, backend store.Store, workers, capacity int) *criticalPersistence {
	t.Helper()
	q, err := newCriticalPersistence(backend, workers, capacity)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		q.closeAdmission()
		select {
		case <-q.stopped:
		case <-time.After(2 * time.Second):
			t.Error("critical workers did not stop")
		}
	})
	return q
}

func criticalGate(t *testing.T) (chan struct{}, func()) {
	t.Helper()
	gate := make(chan struct{})
	var once sync.Once
	release := func() { once.Do(func() { close(gate) }) }
	return gate, release
}

func awaitCriticalResult(t *testing.T, q *criticalPersistence) criticalPersistenceResult {
	t.Helper()
	deadline := time.Now().Add(2 * time.Second)
	for time.Now().Before(deadline) {
		if result, ok := q.poll(); ok {
			return result
		}
		runtime.Gosched()
	}
	t.Fatal("critical completion not delivered")
	return criticalPersistenceResult{}
}

func awaitCriticalWrite(t *testing.T, entered <-chan criticalWrite) criticalWrite {
	t.Helper()
	select {
	case write := <-entered:
		return write
	case <-time.After(2 * time.Second):
		t.Fatal("critical store call did not start")
	}
	return criticalWrite{}
}

func TestCriticalPersistenceRetainsFenceUntilAcknowledgement(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	q := newCriticalTestQueue(t, backend, 1, 3)
	t.Cleanup(release)
	id, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "Account"}}})
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	if err := q.acknowledge(id); err == nil {
		t.Fatal("acknowledged before completion")
	}
	if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: " ACCOUNT "}}}); !errors.Is(err, errCriticalPersistenceBusy) {
		t.Fatalf("canonical fence: %v", err)
	}
	release()
	result := awaitCriticalResult(t, q)
	if result.id != id || result.err != nil {
		t.Fatalf("completion: %+v", result)
	}
	if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "account"}}}); !errors.Is(err, errCriticalPersistenceBusy) {
		t.Fatalf("released before World settled state: %v", err)
	}
	if err := q.acknowledge(id); err != nil {
		t.Fatal(err)
	}
	if err := q.acknowledge(id); err == nil {
		t.Fatal("duplicate acknowledgement accepted")
	}
	next, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "account"}}})
	if err != nil || next <= id {
		t.Fatalf("next submission: %d %v", next, err)
	}
}

func TestCriticalPersistenceCapacityIncludesUnappliedCompletions(t *testing.T) {
	backend := &criticalTestStore{entered: make(chan criticalWrite, 3)}
	q := newCriticalTestQueue(t, backend, 1, 1)
	id, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "one"}}})
	if err != nil {
		t.Fatal(err)
	}
	result := awaitCriticalResult(t, q)
	if result.id != id {
		t.Fatal("wrong operation")
	}
	if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "two"}}}); !errors.Is(err, errCriticalPersistenceFull) {
		t.Fatalf("unapplied capacity: %v", err)
	}
	if err := q.acknowledge(id); err != nil {
		t.Fatal(err)
	}
	if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "two"}}}); err != nil {
		t.Fatal(err)
	}
}

func TestCriticalPersistenceSnapshotsNestedState(t *testing.T) {
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 2), release: gate}
	backend := &criticalStateTestStore{&criticalBatchTestStore{base}}
	q := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(release)
	account := &model.Account{Name: "snapshot", Chars: []model.Char{{
		Name: "character", X: 123, Y: 456, QuestsDone: []int32{7}, Score: &model.Score{Level: 8},
		RuntimeScore:       &model.Score{Level: 9},
		AlternateCelestial: &model.CelestialForm{Score: &model.Score{Level: 10}},
	}}}
	state := &model.CharState{Version: 1, Affects: []model.PersistedAffect{{Value: 11}},
		SpecialCoins: map[string]uint32{"fame": 12}}
	account.CelestialCapsules = []model.CelestialCapsule{{
		Character: model.Char{Score: &model.Score{Level: 13}, QuestsDone: []int32{14}},
	}}
	_, err := q.submit(criticalWrite{accounts: []*model.Account{account},
		character: &criticalCharacterState{uid: "character", state: state}})
	if err != nil {
		t.Fatal(err)
	}
	written := awaitCriticalWrite(t, base.entered)
	account.Chars[0].QuestsDone[0] = 70
	account.Chars[0].Score.Level = 80
	account.Chars[0].RuntimeScore.Level = 90
	account.Chars[0].AlternateCelestial.Score.Level = 100
	account.CelestialCapsules[0].Character.Score.Level = 130
	account.CelestialCapsules[0].Character.QuestsDone[0] = 140
	state.Affects[0].Value = 110
	state.SpecialCoins["fame"] = 120
	ch := written.accounts[0].Chars[0]
	if ch.QuestsDone[0] != 7 || ch.Score.Level != 8 || ch.RuntimeScore.Level != 9 ||
		ch.AlternateCelestial.Score.Level != 10 || written.character.state.Affects[0].Value != 11 ||
		written.character.state.SpecialCoins["fame"] != 12 ||
		written.accounts[0].CelestialCapsules[0].Character.Score.Level != 13 ||
		written.accounts[0].CelestialCapsules[0].Character.QuestsDone[0] != 14 {
		t.Fatal("worker snapshot aliases live state")
	}
	if ch.X != playerEntryX || ch.Y != playerEntryY || account.Chars[0].X != 123 ||
		account.Chars[0].Y != 456 {
		t.Fatal("persistence projection changed live coordinates")
	}
	release()
	if result := awaitCriticalResult(t, q); result.err != nil {
		t.Fatal(result.err)
	}
}

func TestCriticalPersistenceRejectsUnsupportedAtomicWrites(t *testing.T) {
	backend := &criticalTestStore{entered: make(chan criticalWrite, 2)}
	q := newCriticalTestQueue(t, backend, 1, 4)
	for _, write := range []criticalWrite{
		{accounts: []*model.Account{{Name: "one"}, {Name: "two"}}},
		{accounts: []*model.Account{{Name: "one"}}, character: &criticalCharacterState{uid: "uid"}},
	} {
		if _, err := q.submit(write); !errors.Is(err, errCriticalPersistenceUnsupported) {
			t.Fatalf("unsupported operation: %v", err)
		}
	}
	if len(q.accounts) != 0 || len(q.pending) != 0 || len(backend.entered) != 0 {
		t.Fatal("rejected operation reserved or wrote state")
	}
}

func TestCriticalPersistenceAtomicBatchReservesAllParticipants(t *testing.T) {
	base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
	q := newCriticalTestQueue(t, &criticalBatchTestStore{base}, 1, 1)
	id, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "two"}, {Name: "one"}}})
	if err != nil {
		t.Fatal(err)
	}
	written := awaitCriticalWrite(t, base.entered)
	if len(written.accounts) != 2 {
		t.Fatal("batch degraded into separate saves")
	}
	for _, name := range []string{"ONE", "two"} {
		if q.accounts[accountSessionKey(name)] != id {
			t.Fatalf("participant %q not reserved", name)
		}
	}
	result := awaitCriticalResult(t, q)
	if result.id != id || result.err != nil {
		t.Fatalf("batch completion: %+v", result)
	}
	if err := q.acknowledge(id); err != nil {
		t.Fatal(err)
	}
	if len(q.accounts) != 0 {
		t.Fatal("batch retained a participant after acknowledgement")
	}
}

func TestCriticalPersistenceUnrelatedAccountCompletesDuringSlowSave(t *testing.T) {
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
	q := newCriticalTestQueue(t, &criticalSelectiveTestStore{criticalTestStore: base, blocked: gate}, 2, 3)
	t.Cleanup(release)
	slowID, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "slow"}}})
	if err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, base.entered)
	fastID, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "fast"}}})
	if err != nil {
		t.Fatal(err)
	}
	result := awaitCriticalResult(t, q)
	if result.id != fastID || result.err != nil {
		t.Fatalf("unrelated account stalled: %+v", result)
	}
	if q.pending[slowID].completed {
		t.Fatal("held transaction reported completed")
	}
	if err := q.acknowledge(fastID); err != nil {
		t.Fatal(err)
	}
	release()
	result = awaitCriticalResult(t, q)
	if result.id != slowID || result.err != nil {
		t.Fatalf("slow transaction completion: %+v", result)
	}
}

func TestCriticalPersistencePreservesSidecarDeletion(t *testing.T) {
	base := &criticalTestStore{entered: make(chan criticalWrite, 2)}
	q := newCriticalTestQueue(t, &criticalStateTestStore{&criticalBatchTestStore{base}}, 1, 1)
	_, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "sealed"}},
		character: &criticalCharacterState{uid: "removed-character", state: nil}})
	if err != nil {
		t.Fatal(err)
	}
	written := awaitCriticalWrite(t, base.entered)
	if written.character == nil || written.character.uid != "removed-character" || written.character.state != nil {
		t.Fatal("sidecar deletion lost explicit identity or changed nil payload")
	}
	if result := awaitCriticalResult(t, q); result.err != nil {
		t.Fatal(result.err)
	}
}

func TestCriticalPersistenceRejectsInvalidConfigurationAndIDExhaustion(t *testing.T) {
	base := &criticalTestStore{entered: make(chan criticalWrite, 2)}
	for _, config := range [][2]int{{0, 1}, {1, 0}, {2, 1}, {-1, 2}} {
		if q, err := newCriticalPersistence(base, config[0], config[1]); err == nil || q != nil {
			t.Fatalf("invalid configuration accepted: %v", config)
		}
	}
	if q, err := newCriticalPersistence(nil, 1, 1); err == nil || q != nil {
		t.Fatal("nil store accepted")
	}
	q := newCriticalTestQueue(t, base, 1, 1)
	q.lastID = ^uint64(0)
	if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "one"}}}); err == nil {
		t.Fatal("operation ID wrapped")
	}
	if len(q.accounts) != 0 || len(q.pending) != 0 || len(base.entered) != 0 {
		t.Fatal("exhausted ID caused side effects")
	}
}

func TestCriticalPersistenceDrainDoesNotDiscardAcceptedWork(t *testing.T) {
	gate, release := criticalGate(t)
	backend := &criticalTestStore{entered: make(chan criticalWrite, 5), release: gate}
	q := newCriticalTestQueue(t, backend, 2, 3)
	t.Cleanup(release)
	for _, name := range []string{"one", "two", "three"} {
		if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
			t.Fatal(err)
		}
	}
	q.closeAdmission()
	q.closeAdmission()
	if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: "four"}}}); !errors.Is(err, errCriticalPersistenceStopping) {
		t.Fatalf("admission after close: %v", err)
	}
	select {
	case <-q.stopped:
		t.Fatal("drain falsely completed while store held")
	default:
	}
	release()
	select {
	case <-q.stopped:
	case <-time.After(2 * time.Second):
		t.Fatal("drain depends on World polling results")
	}
	if len(backend.entered) != 3 || len(q.pending) != 3 {
		t.Fatal("accepted work lost or auto-acknowledged")
	}
	seen := make(map[uint64]bool)
	for range 3 {
		result := awaitCriticalResult(t, q)
		if result.err != nil || seen[result.id] {
			t.Fatalf("duplicate/failed completion: %+v", result)
		}
		seen[result.id] = true
		if err := q.acknowledge(result.id); err != nil {
			t.Fatal(err)
		}
	}
	if _, ok := q.poll(); ok || len(q.pending) != 0 {
		t.Fatal("drain left extra work")
	}
}

func TestCriticalPersistenceReportsErrorsAndIndeterminatePanics(t *testing.T) {
	failure := errors.New("injected store error without commit certificate")
	for _, panics := range []bool{false, true} {
		t.Run(map[bool]string{false: "error", true: "panic"}[panics], func(t *testing.T) {
			backend := &criticalTestStore{entered: make(chan criticalWrite, 4), failure: failure, panicAfterWrite: panics}
			q := newCriticalTestQueue(t, backend, 1, 4)
			for _, name := range []string{"one", "two"} {
				if _, err := q.submit(criticalWrite{accounts: []*model.Account{{Name: name}}}); err != nil {
					t.Fatal(err)
				}
				result := awaitCriticalResult(t, q)
				want := failure
				if panics {
					want = errCriticalPersistenceIndeterminate
				}
				if !errors.Is(result.err, want) {
					t.Fatalf("outcome: %v", result.err)
				}
				if !errors.Is(result.err, errCriticalPersistenceIndeterminate) {
					t.Fatal("uncertified store failure was treated as definite rollback")
				}
				if len(q.accounts) == 0 {
					t.Fatal("error prematurely released fence")
				}
				if err := q.acknowledge(result.id); err != nil {
					t.Fatal(err)
				}
			}
		})
	}
}

func TestCriticalPersistenceRejectsInvalidIdentity(t *testing.T) {
	backend := &criticalTestStore{entered: make(chan criticalWrite, 3)}
	q := newCriticalTestQueue(t, backend, 1, 4)
	for _, write := range []criticalWrite{
		{}, {accounts: []*model.Account{nil}}, {accounts: []*model.Account{{Name: " "}}},
		{accounts: []*model.Account{{Name: "one"}, {Name: "ONE"}}},
		{accounts: []*model.Account{{Name: "one"}}, character: &criticalCharacterState{}},
	} {
		if _, err := q.submit(write); err == nil {
			t.Fatal("invalid write admitted")
		}
	}
	if len(q.pending) != 0 || len(q.accounts) != 0 || len(backend.entered) != 0 {
		t.Fatal("invalid request caused side effects")
	}
}
