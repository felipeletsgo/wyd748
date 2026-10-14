package game

import (
	"errors"
	"testing"

	"wydgo/internal/account"
)

func TestLoginOnlyAdmittedAccountsDisableTransportIdleTimeout(t *testing.T) {
	for _, failure := range []string{"none", "credentials", "reload", "duplicate", "closed", "canceled"} {
		t.Run(failure, func(t *testing.T) {
			session := newLoginTestSession(2, 64)
			st := &loginReloadStore{account: loginTestAccount()}
			world := loginResultWorld(st, session)
			result := &loginResult{accountName: "Felipe", account: loginTestAccount()}
			switch failure {
			case "credentials":
				result.err = account.ErrInvalidCredentials
			case "reload":
				st.loadErr = errors.New("unavailable")
			case "duplicate":
				world.claimAccountSession(newLoginTestSession(1, 64), "Felipe")
			case "closed":
				session.Close()
			case "canceled":
				delete(world.authPending, session)
			}
			world.onLoginResult(session, result)
			if got, want := session.IsAuthenticated(), failure == "none"; got != want {
				t.Fatalf("transport authentication=%t, want %t", got, want)
			}
		})
	}
}
