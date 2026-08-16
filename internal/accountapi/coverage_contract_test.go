package accountapi

import (
	"encoding/json"
	"net/http"
	"net/http/httptest"
	"testing"
)

func TestHealthEndpointsReturnJSONAndSecurityHeaders(t *testing.T) {
	h := New(nil)
	for _, path := range []string{"/health", "/healthz"} {
		t.Run(path, func(t *testing.T) {
			req := httptest.NewRequest(http.MethodGet, path, nil)
			w := httptest.NewRecorder()
			h.ServeHTTP(w, req)
			if w.Code != http.StatusOK {
				t.Fatalf("status=%d", w.Code)
			}
			var body map[string]string
			if err := json.Unmarshal(w.Body.Bytes(), &body); err != nil {
				t.Fatal(err)
			}
			if body["status"] != "ok" {
				t.Fatalf("body=%v", body)
			}
			if w.Header().Get("X-Content-Type-Options") == "" {
				t.Fatal("security headers ausentes no health endpoint")
			}
		})
	}
}
