package game

import (
	"errors"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

// Commit through the held worker, then expose only the committed copy to the
// destination's normal login reload. No live account pointer crosses worlds.
type channelTransferStore struct {
	*criticalStateTestStore
	account *model.Account
	state   *model.CharState
}

func (s *channelTransferStore) LoadAccount(string) (*model.Account, error) {
	return accountStateSnapshot(s.account), nil
}

func (s *channelTransferStore) LoadCharState(string) (*model.CharState, error) { return s.state, nil }
func (s *channelTransferStore) SaveCharState(_ string, state *model.CharState) error {
	s.state = state
	return nil
}
func (s *channelTransferStore) SavePlayerState(guilds *model.GuildRegistry, account *model.Account, uid string, state *model.CharState) error {
	if err := s.criticalStateTestStore.SavePlayerState(guilds, account, uid, state); err != nil {
		return err
	}
	s.account, s.state = accountStateSnapshot(account), state
	return nil
}

func TestChannelTransferWaitsForCommitAndReloadsDestination(t *testing.T) {
	for _, fail := range []bool{false, true} {
		name := "committed"
		if fail {
			name = "rejected"
		}
		t.Run(name, func(t *testing.T) {
			one, p, source := newEnterWorldPlayer(t, 742, 1000)
			p.Account.Name, p.Account.Chars[0].Name, p.Account.Chars[0].UID = "TransferAcc", "TransferHero", "transfer-hero"
			p.Account.Chars[0].Citizenship, p.Account.Chars[0].Gold = 2, 12345
			one.onEnterWorld(source, enterWorldPacket(0))
			if !p.InWorld {
				t.Fatal("source did not enter")
			}
			for source.QueuedPacketsForTest() > 0 {
				source.DequeuePacketForTest()
			}
			gate, release := criticalGate(t)
			t.Cleanup(release)
			base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if fail {
				base.failure = errors.New("injected transfer write rejection")
			}
			backend := &channelTransferStore{criticalStateTestStore: &criticalStateTestStore{criticalBatchTestStore: &criticalBatchTestStore{base}}}
			one.store = backend
			one.accountSessions = make(map[string]*gameNet.Session)
			two := newZoneTestWorld()
			two.authPending = make(map[*gameNet.Session]bool)
			two.store, one.channel, two.channel = backend, 1, 2
			c, err := NewChannelCluster(backend)
			if err != nil {
				t.Fatal(err)
			}
			q := newCriticalTestQueue(t, backend, 1, 4)
			c.owner = newCriticalPersistenceOwner(q)
			one.criticalWrites, two.criticalWrites = c.owner, c.owner
			for _, w := range []*World{one, two} {
				if err := c.attach(w); err != nil {
					t.Fatal(err)
				}
			}
			c.enter(one)
			if !one.claimAccountSession(source, p.Account.Name) {
				t.Fatal("source account claim failed")
			}
			p.SpecialCoins = map[string]uint32{"fame": 27}
			one.requestChannelTransfer(p, "2")
			proposal := awaitCriticalWrite(t, base.entered)
			if proposal.character == nil || proposal.accounts[0] == p.Account || proposal.character.state.SpecialCoins["fame"] != 27 {
				t.Fatal("transfer did not save detached account and character state atomically")
			}
			for _, phase := range []string{"held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, q, 1)
				}
				if one.players[source] != p || len(c.tickets) != 0 || source.QueuedPacketsForTest() != 0 ||
					!one.criticalPlayerReserved(p) || two.claimAccountSession(gameNet.NewTestSession(8, 8), p.Account.Name) {
					t.Fatalf("%s transfer detached, published or released ownership early", phase)
				}
			}
			// Settle while the destination is active, as the shared actor may do.
			c.enter(two)
			two.processCriticalCompletions(1)
			if fail {
				if len(c.tickets) != 0 || backend.account != nil || one.players[source] != p {
					t.Fatal("rejected transfer detached or issued a ticket")
				}
				return
			}
			if one.players[source] != nil || one.playersByID[p.ID] != nil || len(c.tickets) != 1 || backend.account == nil {
				t.Fatal("acknowledged transfer did not detach the source")
			}
			packet, ok := source.DequeuePacketForTest()
			if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != 0x52A {
				t.Fatal("missing migration response")
			}
			token := cstr(packet[28:80])
			destination := gameNet.NewTestSessionWithRemoteIP(9, 256, "192.0.2.9")
			ticket, ok := two.consumeTicket(token, "TransferAcc", destination)
			if !ok {
				t.Fatal("destination rejected committed ticket")
			}
			two.authPending[destination] = true
			two.onLoginResult(destination, &loginResult{account: accountStateSnapshot(p.Account), transfer: ticket})
			arrived := two.players[destination]
			if arrived == nil || !arrived.InWorld || arrived.Char.UID != p.Char.UID || arrived.Char.Gold != 12345 ||
				arrived.Char.Citizenship != 2 || arrived.SpecialCoins["fame"] != 27 || arrived.Account == p.Account ||
				one.accountSessions[accountSessionKey(p.Account.Name)] != destination || len(c.transfers) != 0 {
				t.Fatalf("destination did not reload committed state: player=%+v state=%+v owner=%p destination=%p transfers=%d", arrived, backend.state,
					one.accountSessions[accountSessionKey(p.Account.Name)], destination, len(c.transfers))
			}
		})
	}
}

func TestChannelTicketIsBoundSingleUseAndExpiring(t *testing.T) {
	now := time.Date(2026, 10, 25, 12, 0, 0, 0, time.UTC)
	c := &ChannelCluster{}
	one, two := &World{channel: 1, cluster: c}, &World{channel: 2, cluster: c}
	c.worlds = []*World{one, two}
	clock := newFakeClock(now)
	one.clock, two.clock = clock, clock
	token, err := c.issueTicket("Shared", 2, 3, now)
	if err != nil {
		t.Fatal(err)
	}
	if !strings.HasPrefix(token, "*2.") || len(token) >= 52 {
		t.Fatalf("invalid ticket field: %q", token)
	}
	a, b := gameNet.NewTestSession(1, 8), gameNet.NewTestSession(2, 8)
	if _, ok := one.consumeTicket(token, "shared", a); ok {
		t.Fatal("wrong channel accepted ticket")
	}
	if _, ok := two.consumeTicket(token, "other", a); ok {
		t.Fatal("wrong account accepted ticket")
	}
	if two.transferAdmissionAllowed("shared", a) {
		t.Fatal("credential login bypassed transfer reservation")
	}
	ticket, ok := two.consumeTicket(token, "SHARED", a)
	if !ok || ticket.slot != 3 {
		t.Fatal("destination rejected valid ticket")
	}
	if !two.transferAdmissionAllowed("Shared", a) || two.transferAdmissionAllowed("Shared", b) || one.transferAdmissionAllowed("Shared", a) {
		t.Fatal("transfer admission is not bound to the destination session")
	}
	if _, ok := two.consumeTicket(token, "Shared", a); ok {
		t.Fatal("same socket replay accepted")
	}
	if _, ok := two.consumeTicket(token, "Shared", b); ok {
		t.Fatal("different socket replay accepted")
	}
	if _, err := c.issueTicket("Shared", 1, 0, now); err == nil {
		t.Fatal("overlapping transfer accepted")
	}
	now = now.Add(channelTicketTTL)
	clock.Advance(channelTicketTTL)
	if _, ok := two.consumeTicket(token, "Shared", b); ok {
		t.Fatal("expired ticket accepted")
	}
	if !one.transferAdmissionAllowed("Shared", b) {
		t.Fatal("expired reservation was not released")
	}
}

func TestChannelMigrationWireMatchesNativeTicketFields(t *testing.T) {
	packet := wire.ChannelMigration(123, "Shared", "*2.ticket")
	if len(packet) != 80 || string(packet[12:18]) != "Shared" || packet[27] != 0 || string(packet[28:37]) != "*2.ticket" || packet[79] != 0 {
		t.Fatalf("migration packet does not match the native 0x52A layout: %v", packet)
	}
}
