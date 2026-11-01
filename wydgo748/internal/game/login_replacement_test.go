package game

import (
	"strings"
	"testing"

	"wydgo/internal/wire"
)

// A quarantined account is not settling: the login does not wait and does
// not claim another player is connected. It asks to retry later.
func TestLoginForQuarantinedAccountAsksToRetryWithoutWaiting(t *testing.T) {
	s := newLoginTestSession(2, 8)
	defer s.Close()
	w := loginResultWorld(&loginReloadStore{account: loginTestAccount()}, s)
	queue := newCriticalTestQueue(t, &criticalTestStore{entered: make(chan criticalWrite, 1)}, 1, 1)
	w.criticalWrites = newCriticalPersistenceOwner(queue)
	queue.failedAccounts = map[string]uint64{"felipe": 9}
	w.onLoginResult(s, &loginResult{accountName: "Felipe", account: loginTestAccount()})
	pkt, ok := s.DequeuePacketForTest()
	if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpMessagePanel ||
		!strings.Contains(string(pkt[12:]), w.msg("_WG_Login_Temporarily_Unavailable_Try_Again_Shortly")) {
		t.Fatal("quarantined account login did not ask to retry")
	}
	if len(w.waitingLogins) != 0 || w.accountSessions["felipe"] != nil {
		t.Fatal("quarantined account login waited or claimed the account")
	}
}
