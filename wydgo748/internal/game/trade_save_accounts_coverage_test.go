package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
)

type tradeAccountsTestStore struct {
	saves    int
	accounts []*model.Account
	err      error
}

func (s *tradeAccountsTestStore) LoadAccount(string) (*model.Account, error) {
	return nil, errors.New("unused")
}
func (s *tradeAccountsTestStore) SaveAccount(*model.Account) error {
	s.saves++
	return s.err
}
func (s *tradeAccountsTestStore) CharacterNameExists(string) (bool, error) { return false, nil }
func (s *tradeAccountsTestStore) SaveAccounts(accounts ...*model.Account) error {
	s.saves++
	s.accounts = accounts
	return s.err
}

func TestSaveTradeAccountsSelectsSafePersistenceCapability(t *testing.T) {
	first := &model.Account{Name: "first"}
	second := &model.Account{Name: "second"}

	t.Run("batch transaction receives detached snapshots", func(t *testing.T) {
		store := &tradeAccountsTestStore{}
		world := &World{store: store}
		if err := world.saveTradeAccounts(first, second); err != nil {
			t.Fatal(err)
		}
		if store.saves != 1 || len(store.accounts) != 2 || store.accounts[0] == first || store.accounts[1] == second ||
			store.accounts[0].Name != first.Name || store.accounts[1].Name != second.Name {
			t.Fatalf("batch persistence did not receive detached account snapshots: %+v", store.accounts)
		}
	})

	t.Run("batch transaction errors propagate", func(t *testing.T) {
		want := errors.New("batch unavailable")
		store := &tradeAccountsTestStore{err: want}
		if err := (&World{store: store}).saveTradeAccounts(first, second); !errors.Is(err, want) || store.saves != 1 {
			t.Fatalf("batch save error=%v saves=%d", err, store.saves)
		}
	})

	t.Run("legacy store rejects multiple accounts", func(t *testing.T) {
		store := &craftStore{}
		err := (&World{store: store}).saveTradeAccounts(first, second)
		if err == nil || err.Error() != "store lacks multi-account transactions; trade rejected" || store.saves != 0 {
			t.Fatalf("unsafe multi-account fallback error=%v saves=%d", err, store.saves)
		}
	})

	t.Run("legacy single-account save propagates errors", func(t *testing.T) {
		want := errors.New("single account unavailable")
		store := &craftStore{err: want}
		if err := (&World{store: store}).saveTradeAccounts(first); !errors.Is(err, want) || store.saves != 1 {
			t.Fatalf("legacy save error=%v saves=%d", err, store.saves)
		}
	})

	t.Run("critical coordinator blocks synchronous fallback", func(t *testing.T) {
		store := &tradeAccountsTestStore{}
		world := &World{store: store, criticalWrites: &criticalPersistenceOwner{}}
		if err := world.saveTradeAccounts(first); !errors.Is(err, errSynchronousPersistenceDisabled) || store.saves != 0 {
			t.Fatalf("coordinator guard error=%v saves=%d", err, store.saves)
		}
	})
}
