package game

import (
	"errors"
	"testing"
	"time"

	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
)

// W9 phase 2: a durable ticket that will not be delivered is cancelled.

type fakeTicketStore struct {
	issueErr  error
	issued    chan []byte
	cancelled chan []byte
}

func newFakeTicketStore() *fakeTicketStore {
	return &fakeTicketStore{issued: make(chan []byte, 4), cancelled: make(chan []byte, 4)}
}

func (f *fakeTicketStore) IssueChannelTicket(hash []byte, _ string, _ byte, _ int, _ time.Duration) error {
	f.issued <- hash
	return f.issueErr
}
func (f *fakeTicketStore) ChannelTicketValid([]byte, string, byte) (bool, error) { return true, nil }
func (f *fakeTicketStore) ClaimAccountLeaseWithTicket(string, string, byte, int64, []byte) (int, error) {
	return 0, nil
}
func (f *fakeTicketStore) CancelChannelTicket(hash []byte) error {
	f.cancelled <- hash
	return nil
}

func remoteTransferWorld(t *testing.T, failure error) (*World, *Player, *gameNet.Session, *fakeTicketStore, *criticalTestStore) {
	t.Helper()
	w, p, s := newEnterWorldPlayer(t, 742, 1000)
	p.Account.Chars[0].UID = "remote-transfer"
	w.onEnterWorld(s, enterWorldPacket(0))
	if !p.InWorld {
		t.Fatal("player did not enter")
	}
	for s.QueuedPacketsForTest() > 0 {
		s.DequeuePacketForTest()
	}
	base := &criticalTestStore{entered: make(chan criticalWrite, 4), failure: failure}
	w.store = &channelTransferStore{criticalStateTestStore: &criticalStateTestStore{criticalBatchTestStore: &criticalBatchTestStore{base}}}
	w.channel, w.commands = 1, make(chan command, 4)
	tickets := newFakeTicketStore()
	w.leases = &AccountLeases{store: newFakeLeaseStore(), tickets: tickets, process: "p1"}
	WithRemoteChannels(2)(w)
	return w, p, s, tickets, base
}

func awaitTicket(t *testing.T, ch chan []byte, what string) []byte {
	t.Helper()
	select {
	case hash := <-ch:
		return hash
	case <-time.After(2 * time.Second):
		t.Fatalf("ticket was not %s", what)
		return nil
	}
}

func TestRemoteTransferCancelsTicketWhenPlayerLeavesDuringIssue(t *testing.T) {
	w, p, s, tickets, _ := remoteTransferWorld(t, nil)
	w.requestChannelTransfer(p, "2")
	issued := awaitTicket(t, tickets.issued, "issued")
	cmd := <-w.commands
	delete(w.players, s) // the player left while the ticket was stored
	w.onChannelTicketIssued(cmd.s, cmd.channelTicket)
	if cancelled := awaitTicket(t, tickets.cancelled, "cancelled"); string(cancelled) != string(issued) {
		t.Fatal("a different ticket was cancelled")
	}
	if len(w.remoteTransfers) != 0 {
		t.Fatal("remote transfer stayed pending")
	}
}

func TestRemoteTransferCancelsTicketWhenFinalWriteFails(t *testing.T) {
	w, p, s, tickets, base := remoteTransferWorld(t, errors.New("injected final write failure"))
	w.requestChannelTransfer(p, "2")
	issued := awaitTicket(t, tickets.issued, "issued")
	cmd := <-w.commands
	w.onChannelTicketIssued(cmd.s, cmd.channelTicket)
	awaitCriticalWrite(t, base.entered)
	if cancelled := awaitTicket(t, tickets.cancelled, "cancelled"); string(cancelled) != string(issued) {
		t.Fatal("a different ticket was cancelled")
	}
	if w.players[s] != p || !p.InWorld {
		t.Fatal("a failed transfer detached the player")
	}
}

func TestRemoteTransferReportsPendingTicketAsBusy(t *testing.T) {
	w, p, s, tickets, base := remoteTransferWorld(t, nil)
	tickets.issueErr = store.ErrChannelTransferPending
	w.requestChannelTransfer(p, "2")
	awaitTicket(t, tickets.issued, "issued")
	if w.channelTransferBlocked(p) != true {
		t.Fatal("a second transfer is allowed while the first ticket is stored")
	}
	cmd := <-w.commands
	w.onChannelTicketIssued(cmd.s, cmd.channelTicket)
	if len(base.entered) != 0 || w.players[s] != p || len(w.remoteTransfers) != 0 {
		t.Fatal("a refused ticket started the final write or stayed pending")
	}
	select {
	case <-tickets.cancelled:
		t.Fatal("a refused ticket was cancelled")
	case <-time.After(50 * time.Millisecond):
	}
}
