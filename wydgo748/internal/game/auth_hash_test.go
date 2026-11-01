package game

import (
	"errors"
	"strings"
	"testing"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/wire"
)

func TestAuthInFlightCapacityStaysBounded(t *testing.T) {
	for hash, want := range map[int]int{1: 16, 4: 16, 8: 32, 64: 256} {
		if got := authInFlightCapacity(hash); got != want {
			t.Fatalf("capacity(%d) = %d, want %d", hash, got, want)
		}
	}
}

// W2/D9: a waiting attempt stops when its session closes and returns the
// slot count unchanged; a free slot is taken at once.
func TestAuthHashGateCancelsOnSessionCloseAndReleases(t *testing.T) {
	w := &World{hashSlots: make(chan struct{}, 1)}
	s := newLoginTestSession(1, 8)
	gate := w.hashGate(s)
	if !gate.Acquire() || len(w.hashSlots) != 1 {
		t.Fatal("a free hash slot was not taken")
	}
	waiting := w.hashGate(newLoginTestSession(2, 8))
	other := newLoginTestSession(3, 8)
	blocked := w.hashGate(other)
	done := make(chan bool, 1)
	go func() { done <- blocked.Acquire() }()
	busy := metricAuthHashBusy.Value()
	other.Close()
	select {
	case ok := <-done:
		if ok || metricAuthHashBusy.Value() != busy+1 {
			t.Fatal("a closed session kept waiting or took the slot")
		}
	case <-time.After(time.Second):
		t.Fatal("a closed session did not stop waiting for a hash slot")
	}
	gate.Release()
	if !waiting.Acquire() {
		t.Fatal("a released slot was not reusable")
	}
	waiting.Release()
	if len(w.hashSlots) != 0 {
		t.Fatal("hash slots leaked")
	}
	if (&World{}).hashGate(s) != nil {
		t.Fatal("a World without hash slots must hash without a gate")
	}
}

func TestLoginResultReportsBusyHashCapacity(t *testing.T) {
	s := newLoginTestSession(1, 8)
	defer s.Close()
	w := loginResultWorld(&loginReloadStore{account: loginTestAccount()}, s)
	w.onLoginResult(s, &loginResult{accountName: "Felipe", err: errors.Join(account.ErrHashBusy)})
	pkt, ok := s.DequeuePacketForTest()
	want := w.msg("_WG_Login_Server_Busy_Try_Again")
	if !ok || !wire.Decrypt(pkt) || !strings.Contains(string(pkt[12:]), want) {
		t.Fatal("busy hash capacity was not reported as a busy server")
	}
}
