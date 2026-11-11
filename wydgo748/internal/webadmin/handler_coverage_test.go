package webadmin

import (
	"strings"
	"testing"

	"wydgo/internal/control"
)

func TestOverviewRejectsBadQueriesAndReportsBusyControl(t *testing.T) {
	t.Run("invalid query", func(t *testing.T) {
		h, _, _, _ := newTestHandler(t)
		cookie, _, _ := loginTest(t, h)
		response := request(h, "GET", "/api/v1/staff/overview?offset=invalid", "", cookie, "")
		if response.Code != 400 || !strings.Contains(response.Body.String(), "invalid_query") {
			t.Fatalf("invalid overview query: %d %s", response.Code, response.Body.String())
		}
	})

	t.Run("control busy", func(t *testing.T) {
		h, _, source, _ := newTestHandler(t)
		cookie, _, _ := loginTest(t, h)
		source.err = control.ErrBusy
		response := request(h, "GET", "/api/v1/staff/overview?offset=1", "", cookie, "")
		if response.Code != 429 || response.Header().Get("Retry-After") != "1" ||
			!strings.Contains(response.Body.String(), "control_busy") {
			t.Fatalf("busy control response: %d %s", response.Code, response.Body.String())
		}
	})
}

func TestOverviewEnforcesActorRateLimit(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	cookie, _, _ := loginTest(t, h)
	for attempt := range 31 {
		response := request(h, "GET", "/api/v1/staff/overview", "", cookie, "")
		if attempt < 30 && response.Code != 200 {
			t.Fatalf("overview request %d: %d %s", attempt+1, response.Code, response.Body.String())
		}
		if attempt == 30 && (response.Code != 429 || response.Header().Get("Retry-After") != "60" ||
			!strings.Contains(response.Body.String(), "rate_limited")) {
			t.Fatalf("overview rate limit: %d %s", response.Code, response.Body.String())
		}
	}
}
