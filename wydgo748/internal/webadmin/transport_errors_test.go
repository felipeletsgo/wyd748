package webadmin

import (
	"errors"
	"net/http/httptest"
	"strings"
	"testing"

	"wydgo/internal/control"
)

func TestTransportErrorResponses(t *testing.T) {
	assertResponse := func(name string, write func(*httptest.ResponseRecorder), status int, code string) {
		t.Helper()
		t.Run(name, func(t *testing.T) {
			response := httptest.NewRecorder()
			write(response)
			if response.Code != status || !strings.Contains(response.Body.String(), code) {
				t.Fatalf("response=%d body=%q want status=%d code=%q", response.Code, response.Body.String(), status, code)
			}
		})
	}
	assertResponse("global drop busy", func(w *httptest.ResponseRecorder) { globalDropTransportError(w, control.ErrBusy) }, 429, "control_busy")
	assertResponse("global drop unavailable", func(w *httptest.ResponseRecorder) { globalDropTransportError(w, errors.New("closed")) }, 503, "server_unavailable")
	assertResponse("premium unknown account", func(w *httptest.ResponseRecorder) { premiumTransportError(w, control.ErrUnknownAccount) }, 404, "unknown_account")
	assertResponse("premium unavailable", func(w *httptest.ResponseRecorder) { premiumTransportError(w, control.ErrPremiumUnavailable) }, 503, "premium_unavailable")
	assertResponse("premium busy", func(w *httptest.ResponseRecorder) { premiumTransportError(w, control.ErrBusy) }, 429, "control_busy")
	assertResponse("premium failed", func(w *httptest.ResponseRecorder) { premiumTransportError(w, errors.New("closed")) }, 503, "server_unavailable")
	assertResponse("staff busy", func(w *httptest.ResponseRecorder) { staffOpsTransportError(w, control.ErrBusy) }, 429, "control_busy")
	assertResponse("staff unknown account", func(w *httptest.ResponseRecorder) { staffOpsTransportError(w, control.ErrUnknownAccount) }, 404, "unknown_account")
	assertResponse("staff sanctions unavailable", func(w *httptest.ResponseRecorder) { staffOpsTransportError(w, control.ErrSanctionsUnavailable) }, 503, "sanctions_unavailable")
	assertResponse("staff audit unavailable", func(w *httptest.ResponseRecorder) { staffOpsTransportError(w, control.ErrAuditUnavailable) }, 503, "audit_unavailable")
	assertResponse("staff failed", func(w *httptest.ResponseRecorder) { staffOpsTransportError(w, errors.New("closed")) }, 503, "server_unavailable")
	for _, tc := range []struct {
		code   string
		status int
		want   string
	}{
		{code: "cursor_expired", status: 410, want: "cursor_expired"},
		{code: "invalid_request", status: 400, want: "invalid_query"},
		{code: "invalid_source", status: 400, want: "invalid_query"},
		{code: "invalid_window", status: 400, want: "invalid_query"},
		{code: "busy", status: 429, want: "control_busy"},
		{code: "internal", status: 503, want: "logs_unavailable"},
	} {
		assertResponse("logs "+tc.code, func(w *httptest.ResponseRecorder) { logReaderError(w, tc.code) }, tc.status, tc.want)
	}
}
