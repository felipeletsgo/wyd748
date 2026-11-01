package game

import (
	"errors"
	"strings"
	"sync"
	"testing"
	"time"

	"wydgo/internal/wire"
)

// W9 phase 1: login claims a cross-process account lease before the reload.

type leaseCall struct {
	account string
	session int64
}

type fakeLeaseStore struct {
	mu       sync.Mutex
	owner    string // another process that holds every account
	claimErr error
	claims   []leaseCall
	released chan leaseCall
	held     [][]string
}

func newFakeLeaseStore() *fakeLeaseStore {
	return &fakeLeaseStore{released: make(chan leaseCall, 8)}
}

func (f *fakeLeaseStore) RegisterGameProcess() (string, error) { return "p1", nil }
func (f *fakeLeaseStore) HeartbeatGameProcess(string) error    { return nil }
func (f *fakeLeaseStore) ReleaseProcessLeases(string) error    { return nil }

func (f *fakeLeaseStore) ClaimAccountLease(account, _ string, _ byte, session int64) (string, error) {
	f.mu.Lock()
	defer f.mu.Unlock()
	f.claims = append(f.claims, leaseCall{account, session})
	return f.owner, f.claimErr
}

func (f *fakeLeaseStore) ReleaseAccountLease(account, _ string, session int64) error {
	f.released <- leaseCall{account, session}
	return nil
}

func (f *fakeLeaseStore) ReconcileAccountLeases(_ string, held []string) error {
	f.mu.Lock()
	defer f.mu.Unlock()
	f.held = append(f.held, held)
	return nil
}

func (f *fakeLeaseStore) waitRelease(t *testing.T) leaseCall {
	t.Helper()
	select {
	case call := <-f.released:
		return call
	case <-time.After(2 * time.Second):
		t.Fatal("lease was not released")
		return leaseCall{}
	}
}

func (f *fakeLeaseStore) noRelease(t *testing.T) {
	t.Helper()
	select {
	case call := <-f.released:
		t.Fatalf("unexpected lease release %+v", call)
	case <-time.After(50 * time.Millisecond):
	}
}

func TestLoginRefusedWhenAnotherProcessHoldsTheAccount(t *testing.T) {
	s := newLoginTestSession(4, 8)
	defer s.Close()
	st := &loginReloadStore{account: loginTestAccount()}
	w := loginResultWorld(st, s)
	leases := newFakeLeaseStore()
	leases.owner = "p2"
	w.leases = &AccountLeases{store: leases, process: "p1"}
	w.onLoginResult(s, &loginResult{accountName: "Felipe", account: loginTestAccount()})
	pkt, ok := s.DequeuePacketForTest()
	if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpAlreadyPlaying {
		t.Fatal("login held by another process did not report the account as playing")
	}
	if st.loads != 0 || w.players[s] != nil || w.accountSessions["felipe"] != nil || len(w.loginReloads) != 0 {
		t.Fatal("refused login read the account or kept a claim")
	}
	leases.noRelease(t) // nothing was leased
}

func TestLoginLeaseFailureAsksToRetry(t *testing.T) {
	s := newLoginTestSession(5, 8)
	defer s.Close()
	st := &loginReloadStore{account: loginTestAccount()}
	w := loginResultWorld(st, s)
	leases := newFakeLeaseStore()
	leases.claimErr = errors.New("database unavailable")
	w.leases = &AccountLeases{store: leases, process: "p1"}
	w.onLoginResult(s, &loginResult{accountName: "Felipe", account: loginTestAccount()})
	pkt, ok := s.DequeuePacketForTest()
	if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpMessagePanel ||
		!strings.Contains(string(pkt[12:]), w.msg("_WG_Login_Temporarily_Unavailable_Try_Again_Shortly")) {
		t.Fatal("failed lease claim did not ask to retry")
	}
	if st.loads != 0 || w.accountSessions["felipe"] != nil {
		t.Fatal("failed lease claim read the account or kept the claim")
	}
}

// The lease ends with the account claim, but only after the account's
// critical writes settle; a quarantined account keeps it through reconcile.
func TestAccountLeaseReleaseWaitsForSettledWrites(t *testing.T) {
	s := newLoginTestSession(6, 64)
	defer s.Close()
	st := &loginReloadStore{account: loginTestAccount()}
	w := loginResultWorld(st, s)
	leases := newFakeLeaseStore()
	w.leases = &AccountLeases{store: leases, process: "p1"}
	w.onLoginResult(s, &loginResult{accountName: "Felipe", account: loginTestAccount()})
	p := w.players[s]
	if p == nil || len(leases.claims) != 1 || leases.claims[0] != (leaseCall{"Felipe", 6}) {
		t.Fatalf("admitted login: player=%v claims=%+v", p != nil, leases.claims)
	}
	queue := newCriticalTestQueue(t, &criticalTestStore{entered: make(chan criticalWrite, 1)}, 1, 1)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	queue.failedAccounts = map[string]uint64{"felipe": 3}
	w.releaseAccountSession(s, p.Account)
	leases.noRelease(t)
	if w.leaseReleases["felipe"] != 6 {
		t.Fatalf("quarantined account lease is not waiting: %+v", w.leaseReleases)
	}
	w.reconcileAccountLeases(time.Now())
	leases.noRelease(t)
	waitFor(t, func() bool {
		leases.mu.Lock()
		defer leases.mu.Unlock()
		return len(leases.held) == 1
	})
	if held := leases.held[0]; len(held) == 0 || held[0] != "felipe" {
		t.Fatalf("reconcile would drop the quarantined lease: held=%v", held)
	}
	delete(queue.failedAccounts, "felipe")
	w.reconcileAccountLeases(time.Now())
	if call := leases.waitRelease(t); call != (leaseCall{"felipe", 6}) {
		t.Fatalf("release %+v", call)
	}
	if len(w.leaseReleases) != 0 {
		t.Fatal("settled lease release stayed pending")
	}
}

// A reload result that arrives after its admission was cancelled ends the
// lease it claimed.
func TestCancelledLoginReloadReleasesItsLease(t *testing.T) {
	s := newLoginTestSession(7, 8)
	defer s.Close()
	w := loginResultWorld(&loginReloadStore{account: loginTestAccount()}, s)
	leases := newFakeLeaseStore()
	w.leases = &AccountLeases{store: leases, process: "p1"}
	w.onLoginReload(s, &loginReloadResult{reload: &loginReload{name: "Felipe"}, leased: true})
	if call := leases.waitRelease(t); call != (leaseCall{"felipe", 7}) {
		t.Fatalf("release %+v", call)
	}
}

func waitFor(t *testing.T, done func() bool) {
	t.Helper()
	deadline := time.Now().Add(2 * time.Second)
	for !done() {
		if time.Now().After(deadline) {
			t.Fatal("condition was not reached")
		}
		time.Sleep(5 * time.Millisecond)
	}
}
