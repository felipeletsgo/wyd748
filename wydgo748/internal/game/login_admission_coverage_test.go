package game

import (
	"net/netip"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

func TestOnLoginIgnoresMalformedOrAlreadyHandledSessions(t *testing.T) {
	tests := []struct {
		name    string
		prepare func(*World, *net.Session)
		packet  []byte
	}{
		{name: "short packet", packet: make([]byte, 43)},
		{name: "closed session", prepare: func(_ *World, s *net.Session) { s.Close() }},
		{name: "admission already rejected", prepare: func(_ *World, s *net.Session) {
			s.RejectAdmission(wire.MessagePanel("rejected"))
		}},
		{name: "authentication already pending", prepare: func(w *World, s *net.Session) { w.authPending[s] = true }},
		{name: "session already has player", prepare: func(w *World, s *net.Session) { w.players[s] = &Player{Session: s} }},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			s := newLoginTestSession(1, 16)
			w := &World{authPending: make(map[*net.Session]bool), players: make(map[*net.Session]*Player)}
			if tc.prepare != nil {
				tc.prepare(w, s)
			}
			pkt := tc.packet
			if pkt == nil {
				pkt = clientBuildLoginPacket(wire.ClientProtocolVersion, wire.CurrentClientBuild)
			}
			beforePending := len(w.authPending)
			beforePlayers := len(w.players)
			beforePackets := s.QueuedPacketsForTest()

			w.onLogin(s, pkt)

			if len(w.authPending) != beforePending || len(w.players) != beforePlayers ||
				s.QueuedPacketsForTest() != beforePackets {
				t.Fatalf("ignored login changed pending state or sent a reply: pending=%v queued=%d",
					w.authPending, s.QueuedPacketsForTest())
			}
		})
	}
}

func TestOnLoginRejectsAfterShutdownBegins(t *testing.T) {
	s := newLoginTestSession(2, 16)
	w := &World{shuttingDown: true, authPending: make(map[*net.Session]bool), players: make(map[*net.Session]*Player)}
	w.onLogin(s, clientBuildLoginPacket(wire.ClientProtocolVersion, wire.CurrentClientBuild))
	if s.QueuedPacketsForTest() != 1 {
		t.Fatalf("shutdown login queued %d packets, want one rejection", s.QueuedPacketsForTest())
	}
}

func TestOnLoginRejectsNetworkRateCapacityAndInvalidTicketBeforeLoadingAccount(t *testing.T) {
	tests := []struct {
		name  string
		setup func(*testing.T, *World, *net.Session)
	}{
		{name: "network denied", setup: func(_ *testing.T, w *World, _ *net.Session) {
			WithNetworkAdmission(model.NetworkAdmissionFile{
				Version: model.NetworkAdmissionVersion,
				Rules: []model.NetworkAdmissionRule{{
					CIDR: netip.MustParsePrefix("192.0.2.0/24"), Action: model.NetworkAdmissionDeny,
					Reason: "test network",
				}},
			})(w)
		}},
		{name: "rate limited", setup: func(t *testing.T, w *World, s *net.Session) {
			w.operational = DefaultOperationalConfig()
			w.operational.AuthAttemptsPerMinuteIP = 1
			w.operational.AuthAttemptsPerMinuteAccount = 1
			if !w.allowLoginAttempt(s.RemoteIP(), "Felipe", w.now()) {
				t.Fatal("fixture failed to consume first login attempt")
			}
		}},
		{name: "authentication capacity full", setup: func(_ *testing.T, w *World, _ *net.Session) {
			w.authSlots <- struct{}{}
		}},
		{name: "invalid channel ticket", setup: func(_ *testing.T, _ *World, _ *net.Session) {}},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			session := newLoginTestSession(3, 16)
			defer session.Close()
			st := &loginReloadStore{account: loginTestAccount()}
			w := loginResultWorld(st, session)
			delete(w.authPending, session)
			w.authSlots = make(chan struct{}, 1)
			w.commands = make(chan command, 1)
			tc.setup(t, w, session)
			packet := clientBuildLoginPacket(wire.ClientProtocolVersion, wire.CurrentClientBuild)
			if tc.name == "invalid channel ticket" {
				packet[48] = 'x'
			}
			if !wire.CompatibleClientLogin(packet) {
				t.Fatal("login fixture is not a valid 7.48 account packet")
			}
			wantSlots := len(w.authSlots)

			w.onLogin(session, packet)

			if w.authPending[session] || st.loads != 0 || len(w.authSlots) != wantSlots || len(w.commands) != 0 {
				t.Fatalf("rejected login reached authentication: pending=%t loads=%d slots=%d/%d commands=%d",
					w.authPending[session], st.loads, len(w.authSlots), wantSlots, len(w.commands))
			}
			if session.QueuedPacketsForTest() == 0 {
				t.Fatal("login rejection did not explain the failed admission")
			}
			if tc.name == "invalid channel ticket" && !session.AdmissionRejected() {
				t.Fatal("invalid channel ticket did not reject the session")
			}
		})
	}
}
