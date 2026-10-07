package store

import (
	"context"
	"os"
	"strings"
	"sync"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestPostgresCriticalAccountsRejectsInvalidAdmission(t *testing.T) {
	for _, accounts := range [][]*model.Account{nil, {nil}, {{Name: "owner"}, nil}} {
		st := &PostgresStore{}
		if err := st.SaveCriticalAccounts(accounts...); !IsWriteRejected(err) {
			t.Fatalf("invalid participants did not return a certified rejection: %v", err)
		}
		if st.writeSeq.Load() != 0 {
			t.Fatal("invalid participants reached write admission")
		}
	}
	if err := (&PostgresStore{readOnly: true}).SaveCriticalAccounts(&model.Account{Name: "owner"}); !IsWriteRejected(err) {
		t.Fatalf("read-only adapter did not return a certified rejection: %v", err)
	}
}

func criticalAccountsPostgresFixture(t *testing.T) (*PostgresStore, []*model.Account) {
	t.Helper()
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 3})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	suffix := time.Now().UTC().Format("150405.000000000")
	accounts := []*model.Account{
		postgresTestAccount("criticalowner"+suffix, "CriticalOwner", model.Item{Index: 4011}),
		postgresTestAccount("criticalpeer"+suffix, "CriticalPeer", model.Item{Index: 4011}),
		postgresTestAccount("criticalforeign"+suffix, "CriticalForeign", model.Item{Index: 4011}),
	}
	keys := make([]string, len(accounts))
	for i, account := range accounts {
		keys[i] = strings.ToLower(account.Name)
	}
	t.Cleanup(func() {
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=ANY($1)`, keys)
	})
	for _, account := range accounts {
		if err := st.CreateAccount(account); err != nil {
			t.Fatal(err)
		}
	}
	return st, accounts
}

func TestPostgresCriticalAccountsTransferRepetitionAndRollback(t *testing.T) {
	st, accounts := criticalAccountsPostgresFixture(t)
	owner, peer, foreign := accounts[0], accounts[1], accounts[2]
	transferred := owner.Chars[0].Inv[0]
	owner.Chars[0].Inv[0], peer.Chars[0].Inv[1] = model.Item{}, transferred
	owner.Chars[0].Gold, peer.CargoGold = 81, 32
	for range 2 {
		if err := st.SaveCriticalAccounts(owner, peer); err != nil {
			t.Fatal(err)
		}
	}
	assertCommitted := func() {
		t.Helper()
		loadedOwner, err := st.LoadAccount(owner.Name)
		if err != nil || loadedOwner == nil || loadedOwner.Chars[0].Gold != 81 || loadedOwner.Chars[0].Inv[0].Index != 0 {
			t.Fatalf("owner transfer was lost or partially overwritten: account=%+v error=%v", loadedOwner, err)
		}
		loadedPeer, err := st.LoadAccount(peer.Name)
		if err != nil || loadedPeer == nil || loadedPeer.CargoGold != 32 || loadedPeer.Chars[0].Inv[1] != transferred || loadedPeer.Chars[0].Inv[2].Index != 0 {
			t.Fatalf("peer transfer was lost or partially overwritten: account=%+v error=%v", loadedPeer, err)
		}
		var holders int
		if err := st.pool.QueryRow(context.Background(), `SELECT count(*) FROM item_instances WHERE uid=$1`, transferred.UID).Scan(&holders); err != nil || holders != 1 {
			t.Fatalf("repeated transfer duplicated or lost the ledger item: holders=%d error=%v", holders, err)
		}
	}
	assertCommitted()
	owner.Chars[0].Gold, peer.CargoGold = 99, 55
	peer.Chars[0].Inv[2] = foreign.Chars[0].Inv[0]
	if err := st.SaveCriticalAccounts(owner, peer); !IsWriteRejected(err) {
		t.Fatalf("foreign-ledger conflict did not certify rollback: %v", err)
	}
	assertCommitted()
	loadedForeign, err := st.LoadAccount(foreign.Name)
	if err != nil || loadedForeign == nil || loadedForeign.Chars[0].Inv[0] != foreign.Chars[0].Inv[0] {
		t.Fatalf("rejected transfer changed the foreign owner: error=%v", err)
	}
}

func TestPostgresCriticalAccountsFencesQueuedAutosaves(t *testing.T) {
	st, accounts := criticalAccountsPostgresFixture(t)
	owner, peer := accounts[0], accounts[1]
	started, release := make(chan struct{}), make(chan struct{})
	var once sync.Once
	doRelease := func() { once.Do(func() { close(release) }) }
	t.Cleanup(doRelease)
	if err := st.enqueue(postgresWriteJob{label: "test: hold account autosave worker", run: func() error {
		close(started)
		<-release
		return nil
	}}); err != nil {
		t.Fatal(err)
	}
	select {
	case <-started:
	case <-time.After(2 * time.Second):
		t.Fatal("autosave worker did not reach the held job")
	}
	for _, account := range []*model.Account{owner, peer} {
		account.CargoGold = 111
		if err := st.SaveAccountAsync(account); err != nil {
			t.Fatal(err)
		}
		account.CargoGold = 222
	}
	done := make(chan error, 1)
	go func() { done <- st.SaveCriticalAccounts(owner, peer) }()
	select {
	case err := <-done:
		if err != nil {
			t.Fatal(err)
		}
	case <-time.After(2 * time.Second):
		doRelease()
		t.Fatal("critical account write waited for an unrelated async-worker barrier")
	}
	doRelease()
	st.Flush()
	for _, account := range []*model.Account{owner, peer} {
		loaded, err := st.LoadAccount(account.Name)
		if err != nil || loaded == nil || loaded.CargoGold != 222 {
			t.Fatalf("old autosave overwrote a critical account: account=%s loaded=%+v error=%v", account.Name, loaded, err)
		}
	}
}
