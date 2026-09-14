package webadmin

import (
	"errors"
	"io/fs"
	"net/http"
	"os"
	"strings"
)

// Site serves only compiled public assets, never source or configuration files.
// The directory must be the Astro dist, not the repository or its web parent.
func (h *Handler) Site(directory string) (http.Handler, error) {
	root := os.DirFS(directory)
	if _, err := fs.Stat(root, "admin/index.html"); err != nil {
		return nil, errors.New("admin build missing; run npm run build in web/portal")
	}
	files := http.FileServer(http.FS(root))
	return http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) {
		if strings.HasPrefix(r.URL.Path, "/api/") || r.URL.Path == "/healthz" || r.URL.Path == "/readyz" {
			h.ServeHTTP(w, r)
			return
		}
		w.Header().Set("Content-Security-Policy", "default-src 'self'; script-src 'self'; style-src 'self'; img-src 'self' data:; connect-src 'self'; object-src 'none'; base-uri 'none'; frame-ancestors 'none'; form-action 'self'")
		w.Header().Set("X-Content-Type-Options", "nosniff")
		w.Header().Set("Referrer-Policy", "no-referrer")
		w.Header().Set("Cache-Control", "no-store")
		if r.Host != h.host {
			http.Error(w, "invalid host", 403)
			return
		}
		if r.Method != "GET" && r.Method != "HEAD" {
			w.Header().Set("Allow", "GET, HEAD")
			http.Error(w, "method not allowed", 405)
			return
		}
		if r.URL.Path == "/" {
			http.Redirect(w, r, "/admin/", http.StatusTemporaryRedirect)
			return
		}
		path := strings.TrimPrefix(r.URL.Path, "/")
		if strings.HasPrefix(path, ".") || strings.Contains(path, "/.") {
			http.NotFound(w, r)
			return
		}
		if strings.HasSuffix(path, "/") {
			path += "index.html"
		}
		info, err := fs.Stat(root, path)
		if err != nil {
			http.NotFound(w, r)
			return
		}
		if info.IsDir() {
			if _, err := fs.Stat(root, path+"/index.html"); err != nil {
				http.NotFound(w, r)
				return
			}
			http.Redirect(w, r, r.URL.Path+"/", http.StatusTemporaryRedirect)
			return
		}
		files.ServeHTTP(w, r)
	}), nil
}
