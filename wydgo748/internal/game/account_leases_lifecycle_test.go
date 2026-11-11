package game

import (
	"context"
	"errors"
	"testing"
	"time"

	"wydgo/internal/store"
)

type accountLeaseLifecycleStore struct {
	store.Store
	process          string
	registerErr      error
	claimOwner       string
	claimErr         error
	ticketSlot       int
	ticketErr        error
	heartbeatResults chan error
	heartbeatCalls   chan error
	processReleases  chan string
	accountReleases  chan leaseCall
	cancelledTickets chan []byte
}

func newAccountLeaseLifecycleStore() *accountLeaseLifecycleStore {
	return &accountLeaseLifecycleStore{
		process:          "process-1",
		processReleases:  make(chan string, 1),
		accountReleases:  make(chan leaseCall, 1),
		cancelledTickets: make(chan []byte, 1),
	}
}

func (s *accountLeaseLifecycleStore) RegisterGameProcess() (string, error) {
	return s.process, s.registerErr
}

func (s *accountLeaseLifecycleStore) HeartbeatGameProcess(string) error {
	if s.heartbeatResults == nil {
		return nil
	}
	err := <-s.heartbeatResults
	s.heartbeatCalls <- err
	return err
}

func (s *accountLeaseLifecycleStore) ClaimAccountLease(string, string, byte, int64) (string, error) {
	return s.claimOwner, s.claimErr
}

func (s *accountLeaseLifecycleStore) ReleaseAccountLease(account, _ string, session int64) error {
	s.accountReleases <- leaseCall{account: account, session: session}
	return nil
}

func (s *accountLeaseLifecycleStore) ReconcileAccountLeases(string, []string) error { return nil }

func (s *accountLeaseLifecycleStore) ReleaseProcessLeases(process string) error {
	s.processReleases <- process
	return nil
}

func (s *accountLeaseLifecycleStore) IssueChannelTicket([]byte, string, byte, int, time.Duration) error {
	return nil
}

func (s *accountLeaseLifecycleStore) ChannelTicketValid([]byte, string, byte) (bool, error) {
	return true, nil
}

func (s *accountLeaseLifecycleStore) ClaimAccountLeaseWithTicket(string, string, byte, int64, []byte) (int, error) {
	return s.ticketSlot, s.ticketErr
}

func (s *accountLeaseLifecycleStore) CancelChannelTicket(ticket []byte) error {
	s.cancelledTickets <- append([]byte(nil), ticket...)
	return nil
}

func TestNewAccountLeasesHandlesOptionalAndFailedBackends(t *testing.T) {
	if got, err := NewAccountLeases(nil); got != nil || err != nil {
		t.Fatalf("nil backend: leases=%v err=%v", got, err)
	}
	if got, err := NewAccountLeases(&accountLeaseUnsupportedStore{}); got != nil || err != nil {
		t.Fatalf("backend without leases: leases=%v err=%v", got, err)
	}

	registerErr := errors.New("registration unavailable")
	backend := newAccountLeaseLifecycleStore()
	backend.registerErr = registerErr
	if got, err := NewAccountLeases(backend); got != nil || !errors.Is(err, registerErr) {
		t.Fatalf("registration failure: leases=%v err=%v", got, err)
	}

	backend.registerErr = nil
	leases, err := NewAccountLeases(backend)
	if err != nil || leases == nil || leases.Process() != backend.process || leases.tickets != backend {
		t.Fatalf("registered leases=%v err=%v", leases, err)
	}
	if (*AccountLeases)(nil).Process() != "" {
		t.Fatal("nil lease client returned a process name")
	}
}

type accountLeaseUnsupportedStore struct{ store.Store }

func TestAccountLeasesLifecycleOptionsAndShutdown(t *testing.T) {
	if WithAccountLeases(nil) == nil {
		t.Fatal("lease option is nil")
	}
	leases := &AccountLeases{process: "process-1", store: newFakeLeaseStore()}
	w := &World{}
	WithAccountLeases(leases)(w)
	if w.leases != leases {
		t.Fatal("world option did not install its lease client")
	}

	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	leases.Run(ctx)
	(*AccountLeases)(nil).Run(ctx)
	leases.Close()
	(*AccountLeases)(nil).Close()
}

func TestAccountLeasesRunCountsFailuresAndResetsAfterHeartbeat(t *testing.T) {
	backend := newAccountLeaseLifecycleStore()
	backend.heartbeatResults = make(chan error, 2)
	heartbeatFailure := errors.New("heartbeat unavailable")
	backend.heartbeatResults <- heartbeatFailure
	backend.heartbeatResults <- nil
	backend.heartbeatCalls = make(chan error, 2)
	leases := &AccountLeases{store: backend, process: backend.process}
	ctx, cancel := context.WithCancel(context.Background())
	defer cancel()
	done := make(chan struct{})
	go func() {
		leases.Run(ctx)
		close(done)
	}()

	awaitHeartbeat := func(wantErr error) {
		t.Helper()
		select {
		case got := <-backend.heartbeatCalls:
			if !errors.Is(got, wantErr) && !(got == nil && wantErr == nil) {
				t.Fatalf("heartbeat error=%v want %v", got, wantErr)
			}
		case <-time.After(accountLeaseHeartbeat + 2*time.Second):
			t.Fatal("account lease heartbeat was not sent on schedule")
		}
	}
	awaitHeartbeat(heartbeatFailure)
	if got := leases.heartbeatFailures.Load(); got != 1 {
		t.Fatalf("consecutive heartbeat failures=%d want 1", got)
	}
	awaitHeartbeat(nil)
	if got := leases.heartbeatFailures.Load(); got != 0 {
		t.Fatalf("successful heartbeat did not reset failure count: %d", got)
	}

	cancel()
	select {
	case <-done:
	case <-time.After(time.Second):
		t.Fatal("lease heartbeat loop did not stop after cancellation")
	}
}

func TestAccountLeaseLifecycleTranslatesClaimAndTicketOutcomes(t *testing.T) {
	backend := newAccountLeaseLifecycleStore()
	leases, err := NewAccountLeases(backend)
	if err != nil {
		t.Fatal(err)
	}

	if err := leases.claim("account", 1, 10); err != nil {
		t.Fatalf("successful claim: %v", err)
	}
	backend.claimOwner = "other-process"
	if err := leases.claim("account", 1, 10); !errors.Is(err, errAccountLeaseHeld) {
		t.Fatalf("held claim: %v", err)
	}
	backend.claimOwner = ""
	backend.claimErr = store.ErrChannelTransferPending
	if err := leases.claim("account", 2, 11); !errors.Is(err, errAccountTransferPending) {
		t.Fatalf("pending transfer claim: %v", err)
	}
	backend.claimErr = errors.New("database unavailable")
	if err := leases.claim("account", 1, 12); err == nil || err.Error() != "database unavailable" {
		t.Fatalf("backend claim failure: %v", err)
	}

	backend.ticketSlot = 3
	if slot, err := leases.claimWithTicket("account", 2, 13, []byte("valid")); err != nil || slot != 3 {
		t.Fatalf("ticket claim slot=%d err=%v", slot, err)
	}
	backend.ticketErr = store.ErrChannelTicketInvalid
	if slot, err := leases.claimWithTicket("account", 2, 14, []byte("invalid")); slot != 0 || !errors.Is(err, errChannelTicketInvalid) {
		t.Fatalf("invalid ticket slot=%d err=%v", slot, err)
	}
	backend.ticketErr = errors.New("database unavailable")
	if _, err := leases.claimWithTicket("account", 2, 15, []byte("retry")); err == nil || err.Error() != "database unavailable" {
		t.Fatalf("ticket backend failure: %v", err)
	}
}

func TestAccountLeaseLifecycleReleasesAndCancelsAsynchronously(t *testing.T) {
	backend := newAccountLeaseLifecycleStore()
	leases, err := NewAccountLeases(backend)
	if err != nil {
		t.Fatal(err)
	}

	leases.release("account", 21)
	select {
	case got := <-backend.accountReleases:
		if got != (leaseCall{account: "account", session: 21}) {
			t.Fatalf("account release=%+v", got)
		}
	case <-time.After(time.Second):
		t.Fatal("account lease release did not reach the store")
	}

	ticket := []byte("undelivered")
	leases.cancelTicket(ticket)
	select {
	case got := <-backend.cancelledTickets:
		if string(got) != string(ticket) {
			t.Fatalf("cancelled ticket=%q", got)
		}
	case <-time.After(time.Second):
		t.Fatal("ticket cancellation did not reach the store")
	}
	(*AccountLeases)(nil).release("account", 22)
	(*AccountLeases)(nil).cancelTicket(ticket)

	leases.Close()
	select {
	case got := <-backend.processReleases:
		if got != backend.process {
			t.Fatalf("released process=%q", got)
		}
	case <-time.After(time.Second):
		t.Fatal("process leases were not released on shutdown")
	}
}
