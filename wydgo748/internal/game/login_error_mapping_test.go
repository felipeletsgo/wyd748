package game

import (
	"errors"
	"fmt"
	"os"
	"strings"
	"testing"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/lang"
	"wydgo/internal/wire"
)

// W2/D8: only invalid credentials or a missing account report a password
// error. A storage failure is temporary, in the recipient channel's language.
func TestLoginResultMapsStorageFailureToTemporaryUnavailable(t *testing.T) {
	for _, language := range []lang.Language{lang.Portuguese, lang.English} {
		for _, tc := range []struct {
			name string
			err  error
			key  string
		}{
			{"invalid-credentials", account.ErrInvalidCredentials, "_WG_Wrong_Account_Or_Password"},
			{"missing-transfer-account", fmt.Errorf("%w: account %q", os.ErrNotExist, "Felipe"), "_WG_Wrong_Account_Or_Password"},
			{"read-timeout", errors.New("timeout: context deadline exceeded"), "_WG_Login_Temporarily_Unavailable_Try_Again_Shortly"},
			{"corrupted-account", errors.New("store: corrupted account"), "_WG_Login_Temporarily_Unavailable_Try_Again_Shortly"},
		} {
			t.Run(fmt.Sprintf("%d/%s", language, tc.name), func(t *testing.T) {
				s := newLoginTestSession(1, 8)
				defer s.Close()
				w := loginResultWorld(&loginReloadStore{account: loginTestAccount()}, s)
				w.language = language
				w.onLoginResult(s, &loginResult{accountName: "Felipe", err: tc.err})
				pkt, ok := s.DequeuePacketForTest()
				if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpMessagePanel {
					t.Fatal("login failure notice missing")
				}
				want := w.msg(tc.key)
				if got := strings.TrimRight(string(pkt[12:12+len(want)+1]), "\x00"); got != want {
					t.Fatalf("notice = %q, want %q", got, want)
				}
				if w.players[s] != nil || len(w.accountSessions) != 0 || w.authPending[s] {
					t.Fatal("failed login admitted the account or stayed pending")
				}
			})
		}
	}
}

func TestLateLoginResultAfterDisconnectSendsNothing(t *testing.T) {
	s := newLoginTestSession(1, 8)
	w := loginResultWorld(&loginReloadStore{account: loginTestAccount()}, s)
	s.Close()
	w.onLoginResult(s, &loginResult{accountName: "Felipe", err: errors.New("late storage error")})
	if s.QueuedPacketsForTest() != 0 || w.authPending[s] || len(w.accountSessions) != 0 {
		t.Fatal("a late authentication result reached a closed session")
	}
}

// W4/D4: a closed previous transport that is still settling is not another
// player. The new login waits for it, bounded, and then claims the account;
// an open previous session still reports a duplicate login at once.
func TestLoginWaitsForSettlingPreviousSession(t *testing.T) {
	for _, tc := range []string{"open", "released", "timeout"} {
		t.Run(tc, func(t *testing.T) {
			s := newLoginTestSession(2, 8)
			defer s.Close()
			w := loginResultWorld(&loginReloadStore{account: loginTestAccount()}, s)
			previous := newLoginTestSession(1, 8)
			w.accountSessions["felipe"] = previous
			if tc != "open" {
				previous.Close()
			}
			w.onLoginResult(s, &loginResult{accountName: "Felipe", account: loginTestAccount()})
			if tc == "open" {
				pkt, ok := s.DequeuePacketForTest()
				if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type == wire.OpMessagePanel ||
					w.accountSessions["felipe"] != previous || len(w.waitingLogins) != 0 {
					t.Fatal("an open previous session did not keep the duplicate-login reply")
				}
				return
			}
			if s.QueuedPacketsForTest() != 0 || w.waitingLogins["felipe"] == nil || w.accountSessions["felipe"] != previous {
				t.Fatal("login did not wait behind the settling previous session")
			}
			w.resumeWaitingLogins(w.now())
			if w.waitingLogins["felipe"] == nil || s.QueuedPacketsForTest() != 0 {
				t.Fatal("waiting login gave up while the previous owner was settling")
			}
			if tc == "released" {
				delete(w.accountSessions, "felipe")
				w.resumeWaitingLogins(w.now())
				if len(w.waitingLogins) != 0 || w.accountSessions["felipe"] != s {
					t.Fatal("released account was not claimed by the waiting login")
				}
				return
			}
			w.resumeWaitingLogins(w.now().Add(loginReplacementWait + time.Second))
			pkt, ok := s.DequeuePacketForTest()
			if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpMessagePanel ||
				!strings.Contains(string(pkt[12:]), w.msg("_WG_Login_Temporarily_Unavailable_Try_Again_Shortly")) ||
				len(w.waitingLogins) != 0 || w.accountSessions["felipe"] != previous {
				t.Fatal("a login that waited past its deadline did not ask to retry")
			}
		})
	}
}
