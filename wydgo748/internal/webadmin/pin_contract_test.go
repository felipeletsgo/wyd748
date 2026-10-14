package webadmin

import (
	"encoding/json"
	"net/http"
	"strings"
	"testing"
)

func TestSixDigitPINLoginAndRegistrationRejections(t *testing.T) {
	for _, pin := range []string{"12345", "0012345", "00123456", "１２３４５６", "12a456", "999999"} {
		t.Run(pin, func(t *testing.T) {
			h, _, _, logs := newTestHandler(t)
			cookie, session := anonymousSession(t, h)
			login, err := json.Marshal(map[string]string{"username": "operator", "password": "TestPass10", "adminPin": pin, "turnstileToken": "test-token"})
			if err != nil {
				t.Fatal(err)
			}
			w := request(h, "POST", "/api/v1/auth/login", string(login), cookie, session.CSRF)
			if w.Code != http.StatusUnauthorized {
				t.Fatalf("invalid PIN login: %d", w.Code)
			}
			cookie, session, _ = loginTest(t, h)
			create, err := json.Marshal(map[string]string{"username": "newplayer", "password": "SafePass9!", "passwordConfirmation": "SafePass9!", "adminPin": pin})
			if err != nil {
				t.Fatal(err)
			}
			w = request(h, "POST", "/api/v1/accounts", string(create), cookie, session.CSRF)
			// PIN rejection precedes registration validation and never discloses
			// whether an account or a correctly shaped PIN exists.
			want := http.StatusUnauthorized
			if w.Code != want {
				t.Fatalf("invalid PIN registration: got %d want %d", w.Code, want)
			}
			if len(h.cfg.Registration.(*testRegistration).accounts) != 0 {
				t.Fatal("rejected request created an account")
			}
			if strings.Contains(logs.String(), pin) || strings.Contains(logs.String(), "SafePass9!") || strings.Contains(logs.String(), "TestPass10") {
				t.Fatal("audit leaked a secret")
			}
		})
	}
	h, _, _, _ := newTestHandler(t)
	_, session, _ := loginTest(t, h)
	if !session.Authenticated {
		t.Fatal("six-digit PIN with leading zeros did not authenticate")
	}
}
