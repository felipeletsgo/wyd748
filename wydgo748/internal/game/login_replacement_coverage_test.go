package game

import (
	"strings"
	"testing"
	"time"

	"wydgo/internal/wire"
)

func TestResumeWaitingLoginsDropsClosedAndRejectedSessions(t *testing.T) {
	for _, test := range []string{"closed", "admission rejected"} {
		t.Run(test, func(t *testing.T) {
			session := newLoginTestSession(2, 8)
			t.Cleanup(session.Close)
			w := loginResultWorld(&loginReloadStore{account: loginTestAccount()}, session)
			if test == "closed" {
				session.Close()
			} else {
				session.RejectAdmission(wire.MessagePanel("test rejection"))
			}
			before := session.QueuedPacketsForTest()
			w.waitingLogins = map[string]*waitingLogin{
				"felipe": {session: session, result: &loginResult{accountName: "Felipe"}, deadline: w.now().Add(time.Minute)},
			}

			w.resumeWaitingLogins(w.now())

			if len(w.waitingLogins) != 0 || session.QueuedPacketsForTest() != before || w.accountSessions["felipe"] != nil {
				t.Fatal("closed or rejected login remained queued, received another reply, or claimed the account")
			}
		})
	}
}

func TestResumeWaitingLoginsRejectsDuringShutdown(t *testing.T) {
	session := newLoginTestSession(2, 8)
	t.Cleanup(session.Close)
	w := loginResultWorld(&loginReloadStore{account: loginTestAccount()}, session)
	w.shuttingDown = true
	w.waitingLogins = map[string]*waitingLogin{
		"felipe": {session: session, result: &loginResult{accountName: "Felipe"}, deadline: w.now().Add(time.Minute)},
	}
	before := session.QueuedPacketsForTest()

	w.resumeWaitingLogins(w.now())

	pkt, ok := session.DequeuePacketForTest()
	if len(w.waitingLogins) != 0 || session.QueuedPacketsForTest() != before || !ok || !wire.Decrypt(pkt) ||
		wire.ParseHeader(pkt).Type != wire.OpMessagePanel ||
		strings.TrimRight(string(pkt[12:]), "\x00") != w.msg("_NN_ServerReboot_Cant_Connect") {
		if !ok || len(pkt) < 12 {
			t.Fatalf("shutdown reply missing: waiting=%d queued=%d want queued=%d packet=%v", len(w.waitingLogins),
				session.QueuedPacketsForTest(), before, ok)
		}
		t.Fatalf("shutdown reply mismatch: waiting=%d queued=%d type=%#x text=%q want=%q", len(w.waitingLogins),
			session.QueuedPacketsForTest(), wire.ParseHeader(pkt).Type, string(pkt[12:]), w.msg("_NN_ServerReboot_Cant_Connect"))
	}
}
