package game

import (
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type criticalCertifiedAccountTestStore struct{ *criticalTestStore }

func (s *criticalCertifiedAccountTestStore) SaveCriticalAccounts(accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts})
}

func (*criticalCertifiedAccountTestStore) SaveAccount(*model.Account) error {
	panic("certified account write reached the legacy adapter")
}

type criticalBatchOnlyStateTestStore struct{ *criticalTestStore }

type criticalCertifiedInstanceTestStore struct{ *criticalTestStore }

func (s *criticalCertifiedInstanceTestStore) SaveCriticalInstanceState(state *model.InstanceStateSnapshot, accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts, instances: state})
}

type criticalPreferredInstanceTestStore struct {
	*criticalCertifiedInstanceTestStore
}

func (*criticalPreferredInstanceTestStore) LoadInstanceState() (*model.InstanceStateSnapshot, error) {
	return nil, nil
}

func (*criticalPreferredInstanceTestStore) SaveInstanceState(*model.InstanceStateSnapshot) error {
	panic("certified instance write reached the legacy adapter")
}

func (*criticalPreferredInstanceTestStore) SaveGameStateWithInstanceState(*model.GuildRegistry, *model.InstanceStateSnapshot, ...*model.Account) error {
	panic("certified atomic instance write reached the legacy adapter")
}

func TestCriticalPersistencePrefersCertifiedInstanceAdapter(t *testing.T) {
	for _, legacy := range []bool{false, true} {
		for count := range 3 {
			t.Run(string(rune('0'+count))+map[bool]string{false: "/only", true: "/preferred"}[legacy], func(t *testing.T) {
				base := &criticalTestStore{entered: make(chan criticalWrite, 1), failure: criticalCertifiedRejection(t)}
				certified := &criticalCertifiedInstanceTestStore{base}
				var backend store.Store = certified
				if legacy {
					backend = &criticalPreferredInstanceTestStore{certified}
				}
				q := newCriticalTestQueue(t, backend, 1, 1)
				write := criticalWrite{instances: &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, GamblePool: 123}}
				for i := range count {
					write.accounts = append(write.accounts, &model.Account{Name: string(rune('a' + i))})
				}
				id, err := q.submit(write)
				if err != nil {
					t.Fatal(err)
				}
				attempt := awaitCriticalWrite(t, base.entered)
				result := awaitCriticalResult(t, q)
				if len(attempt.accounts) != count || attempt.instances.GamblePool != 123 || !store.IsWriteRejected(result.err) || q.instances != id {
					t.Fatal("certified instance routing lost state, certification or reservation")
				}
				if err := q.acknowledge(id); err != nil {
					t.Fatal(err)
				}
			})
		}
	}
}

func (s *criticalBatchOnlyStateTestStore) SavePlayerStateBatch(_ *model.GuildRegistry,
	uid string, state *model.CharState, accounts ...*model.Account) error {
	return s.record(criticalWrite{accounts: accounts,
		character: &criticalCharacterState{uid: uid, state: state}})
}

type criticalPreferredStateTestStore struct {
	*criticalBatchOnlyStateTestStore
}

func (*criticalPreferredStateTestStore) SavePlayerState(*model.GuildRegistry,
	*model.Account, string, *model.CharState) error {
	panic("certified sidecar write reached the legacy adapter")
}

func TestCriticalPersistencePrefersCertifiedAdapters(t *testing.T) {
	for _, mode := range []string{"accounts", "sidecar-batch-only", "sidecar-preferred"} {
		for _, participants := range []int{1, 2} {
			t.Run(mode+string(rune('0'+participants)), func(t *testing.T) {
				base := &criticalTestStore{entered: make(chan criticalWrite, 1), failure: criticalCertifiedRejection(t)}
				var backend store.Store
				switch mode {
				case "accounts":
					backend = &criticalCertifiedAccountTestStore{base}
				case "sidecar-batch-only":
					backend = &criticalBatchOnlyStateTestStore{base}
				default:
					backend = &criticalPreferredStateTestStore{&criticalBatchOnlyStateTestStore{base}}
				}
				q := newCriticalTestQueue(t, backend, 1, 1)
				write := criticalWrite{accounts: []*model.Account{{Name: "owner", Chars: []model.Char{{UID: "owner-uid"}}}}}
				if participants == 2 {
					write.accounts = append(write.accounts, &model.Account{Name: "peer"})
				}
				if mode != "accounts" {
					write.character = &criticalCharacterState{uid: "owner-uid", state: &model.CharState{}}
				}
				id, err := q.submit(write)
				if err != nil {
					t.Fatal(err)
				}
				attempt := awaitCriticalWrite(t, base.entered)
				if len(attempt.accounts) != participants || (attempt.character != nil) != (mode != "accounts") {
					t.Fatalf("adapter lost participants or the sidecar: %+v", attempt)
				}
				result := awaitCriticalResult(t, q)
				if result.id != id || !store.IsWriteRejected(result.err) {
					t.Fatalf("certified adapter was bypassed or lost its rejection: %+v", result)
				}
				if len(q.accounts) != participants || len(q.pending) != 1 {
					t.Fatal("worker completion released actor-owned reservations")
				}
				if err := q.acknowledge(id); err != nil {
					t.Fatal(err)
				}
			})
		}
	}
}
