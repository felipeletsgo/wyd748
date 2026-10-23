package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/wire"
)

func clientBuildLoginPacket(protocol, build uint32) []byte {
	packet := wire.Build(wire.OpConnectAccount, 0, wire.AccountLoginSize)
	copy(packet[12:28], "Felipe")
	copy(packet[28:40], "123456")
	binary.LittleEndian.PutUint32(packet[40:44], protocol)
	binary.LittleEndian.PutUint32(packet[44:48], build)
	return packet
}

func TestClientBuildRejectsBeforeAuthentication(t *testing.T) {
	for _, tc := range []struct {
		name     string
		protocol uint32
		build    uint32
	}{
		{"legacy zero", 748, 0},
		{"legacy selector", 748, 1},
		{"old revision", 748, wire.ClientBuildPrefix | (wire.MinimumClientBuildRevision - 1)},
		{"wrong prefix", 748, 0x12340001},
		{"wrong protocol", 769, wire.CurrentClientBuild},
	} {
		t.Run(tc.name, func(t *testing.T) {
			session := newLoginTestSession(1, 16)
			defer session.Close()
			st := &loginReloadStore{account: loginTestAccount()}
			w := loginResultWorld(st, session)
			delete(w.authPending, session)
			w.authSlots = make(chan struct{}, 1)
			w.commands = make(chan command, 1)
			w.onLogin(session, clientBuildLoginPacket(tc.protocol, tc.build))
			if !session.AdmissionRejected() || session.IsAuthenticated() ||
				w.authPending[session] || len(w.authSlots) != 0 || st.loads != 0 ||
				len(w.authRateByIP) != 0 || len(w.authRateByAccount) != 0 ||
				len(w.accountSessions) != 0 || len(w.authClientsByIP) != 0 ||
				len(w.players) != 0 {
				t.Fatal("incompatible build reached authentication or acquired admission state")
			}
			packet, ok := session.DequeuePacketForTest()
			if !ok || !wire.Decrypt(packet) {
				t.Fatal("missing encrypted update-required message")
			}
			header := wire.ParseHeader(packet)
			if header.Type != wire.OpMessagePanel || header.ID != 0 || len(packet) != 108 ||
				cstr(packet[12:]) != "Client update required. Close the game and run the launcher to update." {
				t.Fatal("update-required message does not match the existing client panel contract")
			}
			// A pipelined corrected packet cannot undo rejection on this socket.
			w.onLogin(session, clientBuildLoginPacket(wire.ClientProtocolVersion, wire.CurrentClientBuild))
			if w.authPending[session] || session.QueuedPacketsForTest() != 0 || st.loads != 0 {
				t.Fatal("same-socket retry bypassed the terminal build rejection")
			}
			// An already queued asynchronous result must not restore admission.
			w.authPending[session] = true
			w.onLoginResult(session, &loginResult{accountName: "Felipe", account: st.account})
			if w.authPending[session] || len(w.players) != 0 || st.loads != 0 ||
				session.IsAuthenticated() || len(w.accountSessions) != 0 {
				t.Fatal("late authentication result admitted a rejected socket")
			}
		})
	}
}

func TestClientBuildCurrentAndFutureRevisionPreserveAuthentication(t *testing.T) {
	hash, err := account.HashPassword("123456")
	if err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct {
		name     string
		build    uint32
		password string
		accept   bool
	}{
		{"current", wire.CurrentClientBuild, "123456", true},
		{"newer", wire.CurrentClientBuild + 1, "123456", true},
		{"wrong password", wire.CurrentClientBuild, "abcdef", false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			session := newLoginTestSession(2, 64)
			defer session.Close()
			acc := loginTestAccount()
			acc.PasswordHash = hash
			st := &loginReloadStore{account: acc}
			w := loginResultWorld(st, session)
			delete(w.authPending, session)
			w.authSlots = make(chan struct{}, 1)
			w.commands = make(chan command, 1)
			packet := clientBuildLoginPacket(wire.ClientProtocolVersion, tc.build)
			clear(packet[28:40])
			copy(packet[28:40], tc.password)
			w.onLogin(session, packet)
			if !w.authPending[session] || session.AdmissionRejected() {
				t.Fatal("compatible build did not enter normal authentication")
			}
			select {
			case result := <-w.commands:
				if result.s != session || result.login == nil {
					t.Fatal("missing asynchronous authentication result")
				}
				w.onLoginResult(session, result.login)
			case <-time.After(5 * time.Second):
				t.Fatal("authentication result timed out")
			}
			if (w.players[session] != nil) != tc.accept || session.IsAuthenticated() != tc.accept ||
				w.authPending[session] || session.AdmissionRejected() {
				t.Fatal("compiled-build check changed credential admission behavior")
			}
			if !tc.accept && (len(w.accountSessions) != 0 || len(w.authClientsByIP) != 0) {
				t.Fatal("invalid credentials acquired account or IP reservations")
			}
		})
	}
}
