package webadmin

import (
	"net/http"
	"net/http/httptest"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func newTestSite(t *testing.T) http.Handler {
	t.Helper()
	h, _, _, _ := newTestHandler(t)
	dir := t.TempDir()
	for name, body := range map[string]string{
		"admin/index.html": "<!doctype html><script src=\"/_astro/admin.js\"></script>",
		"_astro/admin.js":  "console.log('admin')",
		".secret":          "hidden",
	} {
		path := filepath.Join(dir, filepath.FromSlash(name))
		if err := os.MkdirAll(filepath.Dir(path), 0755); err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(path, []byte(body), 0600); err != nil {
			t.Fatal(err)
		}
	}
	site, err := h.Site(dir)
	if err != nil {
		t.Fatal(err)
	}
	return site
}

func siteRequest(site http.Handler, method, target, host string) *httptest.ResponseRecorder {
	r := httptest.NewRequest(method, "https://"+host+target, nil)
	r.Host = host
	w := httptest.NewRecorder()
	site.ServeHTTP(w, r)
	return w
}

func TestSiteStaticRoutingAndHeaders(t *testing.T) {
	site := newTestSite(t)

	w := siteRequest(site, http.MethodGet, "/", "admin.test")
	if w.Code != http.StatusTemporaryRedirect || w.Header().Get("Location") != "/admin/" {
		t.Fatalf("root redirect: %d %q", w.Code, w.Header().Get("Location"))
	}

	w = siteRequest(site, http.MethodGet, "/admin/", "admin.test")
	if w.Code != http.StatusOK || !strings.Contains(w.Body.String(), "/_astro/admin.js") {
		t.Fatalf("admin page: %d %s", w.Code, w.Body.String())
	}
	if w.Header().Get("Cache-Control") != "no-store" || w.Header().Get("Content-Security-Policy") == "" || w.Header().Get("X-Content-Type-Options") != "nosniff" {
		t.Fatal("static security headers missing")
	}

	w = siteRequest(site, http.MethodGet, "/_astro/admin.js", "admin.test")
	if w.Code != http.StatusOK || !strings.Contains(w.Body.String(), "console.log") {
		t.Fatalf("asset: %d %s", w.Code, w.Body.String())
	}

	w = siteRequest(site, http.MethodGet, "/healthz", "admin.test")
	if w.Code != http.StatusOK || !strings.Contains(w.Body.String(), `"status":"ok"`) {
		t.Fatalf("api dispatch: %d %s", w.Code, w.Body.String())
	}
}

func TestSiteRejectsUnexpectedRequests(t *testing.T) {
	site := newTestSite(t)

	for _, target := range []string{"/.secret", "/admin/.secret", "/../.secret", "/%2e%2e/.secret"} {
		w := siteRequest(site, http.MethodGet, target, "admin.test")
		if w.Code != http.StatusNotFound {
			t.Fatalf("hidden path %q returned %d", target, w.Code)
		}
	}

	w := siteRequest(site, http.MethodGet, "/admin/", "evil.test")
	if w.Code != http.StatusForbidden {
		t.Fatalf("invalid host returned %d", w.Code)
	}

	w = siteRequest(site, http.MethodPost, "/admin/", "admin.test")
	if w.Code != http.StatusMethodNotAllowed || w.Header().Get("Allow") != "GET, HEAD" {
		t.Fatalf("invalid method returned %d allow=%q", w.Code, w.Header().Get("Allow"))
	}
}

func TestSiteRequiresCompiledAdminEntry(t *testing.T) {
	h, _, _, _ := newTestHandler(t)
	if _, err := h.Site(t.TempDir()); err == nil {
		t.Fatal("missing compiled admin entry accepted")
	}
}
