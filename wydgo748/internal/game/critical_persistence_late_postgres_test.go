package game

import (
	"context"
	"errors"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
)

// Model the uncertain interval separately from settlement: the worker reports
// an unknown outcome before a real PostgreSQL write finishes. This controlled
// adapter is not a TCP fault injector and does not certify restart recovery.
type criticalLatePostgresStore struct {
	*store.PostgresStore
	entered  chan criticalWrite
	release  <-chan struct{}
	finished chan struct{}
	err      error // Read only after finished closes.
}

func (s *criticalLatePostgresStore) SaveCriticalAccounts(accounts ...*model.Account) error {
	proposal := accountStateSnapshotsForTest(accounts)
	s.entered <- criticalWrite{accounts: proposal}
	go func() {
		<-s.release
		s.err = s.PostgresStore.SaveCriticalAccounts(proposal...)
		close(s.finished)
	}()
	return errors.New("transaction outcome unavailable before delayed settlement")
}

func TestCriticalAccountPostgresLateCommitRetainsContainment(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	st, err := store.NewPostgresStore(context.Background(), store.PostgresConfig{URL: url, MaxConns: 2})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	conn, err := pgx.Connect(context.Background(), url)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = conn.Close(context.Background()) })
	w, p, _ := handlerTestWorld(t)
	name := "latepg" + time.Now().UTC().Format("150405.000000000")
	p.Account.Name, p.Char.UID = name, ""
	t.Cleanup(func() {
		if _, err := conn.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(name)); err != nil {
			t.Errorf("fixture cleanup: %v", err)
		}
	})
	if err := st.CreateAccount(p.Account); err != nil {
		t.Fatal(err)
	}
	w.indexPlayerCharacter(p)
	claimCriticalTestPlayers(t, w, p)
	w.store = st
	clock := newFakeClock(time.Date(2026, 10, 7, 12, 0, 0, 0, time.UTC))
	w.clock = clock
	gate, release := criticalGate(t)
	backend := &criticalLatePostgresStore{PostgresStore: st, entered: make(chan criticalWrite, 1),
		release: gate, finished: make(chan struct{})}
	q := newCriticalTestQueue(t, backend, 1, 2)
	t.Cleanup(func() {
		release()
		select {
		case <-backend.finished:
		case <-time.After(5 * time.Second):
			t.Error("delayed PostgreSQL writer did not settle before fixture cleanup")
		}
	})
	w.criticalWrites = newCriticalPersistenceOwner(q)
	w.criticalAccountContainment = true
	gold := p.Char.Gold
	proposal := accountStateSnapshot(p.Account)
	proposal.Chars[p.CharSlot].Gold += 123
	if _, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposal}}, []*Player{p},
		func(criticalPersistenceResult) error { t.Fatal("unknown result applied"); return nil }); err != nil {
		t.Fatal(err)
	}
	awaitCriticalWrite(t, backend.entered)
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	assertContained := func() {
		t.Helper()
		if w.shuttingDown || !p.PersistencePoisoned || !p.Session.IsClosed() || p.Char.Gold != gold ||
			!w.criticalAccountReserved(name) || len(q.pending) != 1 || len(w.criticalQuarantine) != 1 ||
			w.claimAccountSession(gameNet.NewTestSession(99, 8), name) {
			t.Fatal("uncertain account was installed, unlocked or readmitted")
		}
	}
	assertContained()
	clock.Advance(24 * time.Hour)
	w.tick()
	assertContained()
	loaded, err := st.LoadAccount(name)
	if err != nil || loaded == nil || loaded.Chars[p.CharSlot].Gold != gold {
		t.Fatalf("held database state changed before delayed settlement: %v", err)
	}
	release()
	select {
	case <-backend.finished:
	case <-time.After(5 * time.Second):
		t.Fatal("delayed PostgreSQL write timed out")
	}
	if backend.err != nil {
		t.Fatal(backend.err)
	}
	loaded, err = st.LoadAccount(name)
	if err != nil || loaded == nil || loaded.Chars[p.CharSlot].Gold != gold+123 {
		t.Fatalf("real delayed commit did not become durable: %v", err)
	}
	clock.Advance(30 * 24 * time.Hour)
	w.tick()
	assertContained()
}
