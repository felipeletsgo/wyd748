package game

import (
	"context"
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

type durableLoginTestStore struct {
	*loginReloadStore

	valid          bool
	ticketErr      error
	checkedHash    []byte
	checkedName    string
	checkedChannel byte
	checkCalls     int
	sanctionCalls  int
	sanctionErr    error
}

func (s *durableLoginTestStore) IssueChannelTicket([]byte, string, byte, int, time.Duration) error {
	return nil
}

func (s *durableLoginTestStore) ChannelTicketValid(hash []byte, account string, channel byte) (bool, error) {
	s.checkedHash = append([]byte(nil), hash...)
	s.checkedName = account
	s.checkedChannel = channel
	s.checkCalls++
	return s.valid, s.ticketErr
}

func (*durableLoginTestStore) ClaimAccountLeaseWithTicket(string, string, byte, int64, []byte) (int, error) {
	return 0, nil
}

func (*durableLoginTestStore) CancelChannelTicket([]byte) error { return nil }

func (*durableLoginTestStore) AddSanction(context.Context, string, control.SanctionCommand) (control.Sanction, bool, error) {
	return control.Sanction{}, false, nil
}

func (*durableLoginTestStore) RevokeSanction(context.Context, int64, string, string, string, string) (control.Sanction, bool, error) {
	return control.Sanction{}, false, nil
}

func (*durableLoginTestStore) ListSanctions(context.Context, string, int) ([]control.Sanction, time.Time, error) {
	return nil, time.Time{}, nil
}

func (s *durableLoginTestStore) ActiveSanctionState(context.Context, string) (store.SanctionState, error) {
	s.sanctionCalls++
	return store.SanctionState{}, s.sanctionErr
}

func TestOnLoginValidatesDurableTicketBeforeAccountRead(t *testing.T) {
	for _, tc := range []struct {
		name        string
		valid       bool
		ticketErr   error
		sanctionErr error
		wantLoad    bool
	}{
		{name: "valid ticket", valid: true, wantLoad: true},
		{name: "sanction store unavailable", valid: true, sanctionErr: errors.New("sanction database unavailable"), wantLoad: true},
		{name: "certified invalid ticket"},
		{name: "ticket store unavailable", ticketErr: errors.New("ticket database unavailable")},
	} {
		t.Run(tc.name, func(t *testing.T) {
			session := newLoginTestSession(20, 16)
			defer session.Close()
			base := &loginReloadStore{account: loginTestAccount()}
			backend := &durableLoginTestStore{
				loginReloadStore: base, valid: tc.valid, ticketErr: tc.ticketErr, sanctionErr: tc.sanctionErr,
			}
			w := loginResultWorld(base, session)
			w.store = backend
			w.leases = &AccountLeases{tickets: backend, process: "destination"}
			w.channel = 2
			w.operational = DefaultOperationalConfig()
			w.authSlots = make(chan struct{}, 1)
			w.commands = make(chan command, 1)
			delete(w.authPending, session)

			packet := clientBuildLoginPacket(wire.ClientProtocolVersion, wire.CurrentClientBuild)
			copy(packet[48:100], "single-use-transfer-token")
			if !wire.CompatibleClientLogin(packet) {
				t.Fatal("durable ticket fixture is not a valid 7.48 account packet")
			}
			tokenHash := channelTicketHash("single-use-transfer-token")
			w.onLogin(session, packet)

			var result command
			select {
			case result = <-w.commands:
			case <-time.After(2 * time.Second):
				t.Fatal("durable ticket authentication did not return to the World queue")
			}
			if result.login == nil || result.s != session || backend.checkCalls != 1 ||
				!reflect.DeepEqual(backend.checkedHash, tokenHash) || backend.checkedName != "Felipe" ||
				backend.checkedChannel != 2 {
				t.Fatalf("ticket validation used the wrong identity: command=%+v store=%+v", result, backend)
			}
			if tc.wantLoad {
				if base.loads != 1 || result.login.account != base.account ||
					!reflect.DeepEqual(result.login.durableTicket, tokenHash) || backend.sanctionCalls != 1 ||
					!errors.Is(result.login.sanctionErr, tc.sanctionErr) {
					t.Fatalf("valid ticket did not reload account and sanctions: loads=%d result=%+v sanctions=%d",
						base.loads, result.login, backend.sanctionCalls)
				}
				if tc.sanctionErr != nil {
					w.onLoginResult(session, result.login)
					if w.players[session] != nil || session.QueuedPacketsForTest() == 0 {
						t.Fatal("sanction lookup failure admitted the account or failed to notify the client")
					}
				}
				return
			}
			if base.loads != 0 || backend.sanctionCalls != 0 {
				t.Fatalf("rejected ticket reached account or sanction reads: loads=%d sanctions=%d",
					base.loads, backend.sanctionCalls)
			}
			if tc.ticketErr == nil {
				if !errors.Is(result.login.err, errChannelTicketInvalid) {
					t.Fatalf("invalid ticket error=%v, want %v", result.login.err, errChannelTicketInvalid)
				}
				w.onLoginResult(session, result.login)
				if !session.AdmissionRejected() {
					t.Fatal("invalid durable ticket did not reject the session")
				}
			} else if !errors.Is(result.login.err, tc.ticketErr) {
				t.Fatalf("ticket store error=%v, want %v", result.login.err, tc.ticketErr)
			}
		})
	}
}
