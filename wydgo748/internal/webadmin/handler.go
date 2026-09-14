// Package webadmin implements the authenticated local staff console.
package webadmin

import (
	"context"
	"crypto/rand"
	"crypto/sha256"
	"crypto/subtle"
	"encoding/base64"
	"encoding/json"
	"errors"
	"io"
	"log/slog"
	"mime"
	"net/http"
	"net/netip"
	"net/url"
	"strings"
	"sync"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/control"
)

type Control interface {
	Overview(context.Context, string, control.Query) (control.Overview, error)
}

type Config struct {
	Origin         string
	DevHTTP        bool
	AdminAccessPIN string
	Staff          func() (map[string]Staff, error)
	Accounts       account.AuthStore
	Control        Control
	GlobalDrop     control.GlobalDropSource
	Quiz           control.QuizSource
	Bosses         control.BossesSource
	Persistent     PersistentReader
	Ready          func(context.Context) error
	Audit          *slog.Logger
	TrustedProxies []netip.Prefix
}

type session struct {
	Actor             string
	CSRF              string
	Expires, LastSeen time.Time
	StaffFingerprint  [32]byte
	Peer              string
}
type rate struct {
	until time.Time
	count int
}
type rateLimit struct {
	key   string
	limit int
}

const (
	maxSessions          = 256
	maxAnonymousSessions = 64
	maxAnonymousPerPeer  = 8
)

type requestIDContextKey struct{}

type Handler struct {
	cfg            Config
	host, cookie   string
	mu             sync.Mutex
	sessions       map[[32]byte]session
	rates          map[string]rate
	hashSlots      chan struct{}
	trustedProxies []netip.Prefix
	now            func() time.Time
	mux            *http.ServeMux
}

func New(cfg Config) (*Handler, error) {
	u, err := url.Parse(cfg.Origin)
	if err != nil || u.Host == "" || u.User != nil || u.Path != "" || u.RawQuery != "" || u.Fragment != "" {
		return nil, errors.New("expected an exact public origin without path")
	}
	if cfg.DevHTTP {
		if u.Scheme != "http" || control.LoopbackAddress(u.Host) != nil {
			return nil, errors.New("dev HTTP requires a literal loopback origin")
		}
	} else if u.Scheme != "https" {
		return nil, errors.New("HTTPS origin required")
	}
	if cfg.Accounts == nil || cfg.Control == nil || cfg.Persistent == nil || cfg.Staff == nil || cfg.Audit == nil {
		return nil, errors.New("accounts, control, persistent reader, staff and audit required")
	}
	if !validAdminPIN(cfg.AdminAccessPIN) {
		return nil, errors.New("admin access PIN must contain 6..12 digits")
	}
	if _, err := cfg.Staff(); err != nil {
		return nil, err
	}
	h := &Handler{cfg: cfg, host: u.Host, cookie: "__Host-wyd_session", sessions: make(map[[32]byte]session), rates: make(map[string]rate), hashSlots: make(chan struct{}, 2), trustedProxies: append([]netip.Prefix(nil), cfg.TrustedProxies...), now: time.Now, mux: http.NewServeMux()}
	if cfg.DevHTTP {
		h.cookie = "wyd_local_session"
	}
	h.mux.HandleFunc("GET /api/v1/auth/session", h.sessionInfo)
	h.mux.HandleFunc("POST /api/v1/auth/login", h.login)
	h.mux.HandleFunc("POST /api/v1/auth/logout", h.logout)
	h.mux.HandleFunc("GET /api/v1/staff/overview", h.overview)
	h.mux.HandleFunc("GET /api/v1/staff/events/global-drop", h.globalDrop)
	h.mux.HandleFunc("POST /api/v1/staff/events/global-drop", h.globalDrop)
	h.mux.HandleFunc("GET /api/v1/staff/events/quiz", h.quiz)
	h.mux.HandleFunc("POST /api/v1/staff/events/quiz", h.quiz)
	h.mux.HandleFunc("GET /api/v1/staff/bosses", h.bosses)
	h.mux.HandleFunc("POST /api/v1/staff/bosses", h.bosses)
	h.mux.HandleFunc("GET /api/v1/staff/players/{uid}/persistent", h.persistentPlayer)
	h.mux.HandleFunc("GET /healthz", func(w http.ResponseWriter, r *http.Request) { writeJSON(w, 200, map[string]string{"status": "ok"}) })
	h.mux.HandleFunc("GET /readyz", func(w http.ResponseWriter, r *http.Request) {
		ctx, cancel := context.WithTimeout(r.Context(), 2*time.Second)
		defer cancel()
		if h.cfg.Ready == nil || h.cfg.Ready(ctx) != nil {
			writeError(w, 503, "not_ready")
			return
		}
		writeJSON(w, 200, map[string]string{"status": "ready"})
	})
	return h, nil
}

func validAdminPIN(pin string) bool {
	if len(pin) < 6 || len(pin) > 12 {
		return false
	}
	for i := 0; i < len(pin); i++ {
		if pin[i] < '0' || pin[i] > '9' {
			return false
		}
	}
	return true
}

func (h *Handler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	requestID := token()
	w.Header().Set("X-Request-ID", requestID)
	r = r.WithContext(context.WithValue(r.Context(), requestIDContextKey{}, requestID))
	w.Header().Set("Cache-Control", "no-store")
	w.Header().Set("X-Content-Type-Options", "nosniff")
	w.Header().Set("Referrer-Policy", "no-referrer")
	w.Header().Set("X-Frame-Options", "DENY")
	w.Header().Set("X-Robots-Tag", "noindex, nofollow")
	if r.Host != h.host {
		writeError(w, 403, "invalid_host")
		return
	}
	if r.Method != "GET" && r.Method != "HEAD" {
		if r.Header.Get("Origin") != h.cfg.Origin {
			writeError(w, 403, "invalid_origin")
			return
		}
	}
	h.mux.ServeHTTP(w, r)
}

func writeJSON(w http.ResponseWriter, status int, value any) {
	w.Header().Set("Content-Type", "application/json")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(value)
}
func writeError(w http.ResponseWriter, status int, code string) {
	writeJSON(w, status, map[string]string{"error": code})
}
func token() string {
	b := make([]byte, 32)
	_, _ = rand.Read(b)
	return base64.RawURLEncoding.EncodeToString(b)
}

func (h *Handler) allow(key string, limit int) bool {
	return h.allowAll(rateLimit{key: key, limit: limit})
}

func (h *Handler) allowAll(limits ...rateLimit) bool {
	h.mu.Lock()
	defer h.mu.Unlock()
	now := h.now()
	for k, v := range h.rates {
		if !now.Before(v.until) {
			delete(h.rates, k)
		}
	}
	newKeys := 0
	for _, item := range limits {
		v, exists := h.rates[item.key]
		if !exists {
			newKeys++
			v.until = now.Add(time.Minute)
		}
		if v.count >= item.limit {
			return false
		}
	}
	if len(h.rates)+newKeys > 2048 {
		return false
	}
	for _, item := range limits {
		v, exists := h.rates[item.key]
		if !exists {
			v.until = now.Add(time.Minute)
		}
		v.count++
		h.rates[item.key] = v
	}
	return true
}

func remoteIP(r *http.Request) (netip.Addr, bool) {
	addrPort, err := netip.ParseAddrPort(r.RemoteAddr)
	if err == nil {
		return addrPort.Addr().Unmap(), true
	}
	addr, err := netip.ParseAddr(r.RemoteAddr)
	if err != nil {
		return netip.Addr{}, false
	}
	return addr.Unmap(), true
}

func (h *Handler) trustedProxy(addr netip.Addr) bool {
	for _, prefix := range h.trustedProxies {
		if prefix.Contains(addr) {
			return true
		}
	}
	return false
}

func (h *Handler) peer(r *http.Request) string {
	remote, ok := remoteIP(r)
	if !ok {
		return "unknown"
	}
	if !h.trustedProxy(remote) {
		return remote.String()
	}
	forwarded := strings.Join(r.Header.Values("X-Forwarded-For"), ",")
	if forwarded == "" {
		return remote.String()
	}
	parts := strings.Split(forwarded, ",")
	current := remote
	for i := len(parts) - 1; i >= 0; i-- {
		candidate, err := netip.ParseAddr(strings.TrimSpace(parts[i]))
		if err != nil {
			return remote.String()
		}
		candidate = candidate.Unmap()
		current = candidate
		if !h.trustedProxy(candidate) {
			return candidate.String()
		}
	}
	return current.String()
}

func (h *Handler) readSession(r *http.Request) ([32]byte, session, bool) {
	c, err := r.Cookie(h.cookie)
	if err != nil || len(c.Value) != 43 {
		return [32]byte{}, session{}, false
	}
	key := sha256.Sum256([]byte(c.Value))
	h.mu.Lock()
	defer h.mu.Unlock()
	s, ok := h.sessions[key]
	now := h.now()
	if !ok || !now.Before(s.Expires) || now.Sub(s.LastSeen) >= 15*time.Minute {
		delete(h.sessions, key)
		return key, session{}, false
	}
	s.LastSeen = now
	h.sessions[key] = s
	return key, s, true
}
func (h *Handler) issue(w http.ResponseWriter, r *http.Request, old [32]byte, s session) bool {
	h.mu.Lock()
	defer h.mu.Unlock()
	now := h.now()
	for k, v := range h.sessions {
		if !now.Before(v.Expires) || now.Sub(v.LastSeen) >= 15*time.Minute {
			delete(h.sessions, k)
		}
	}
	delete(h.sessions, old)
	if s.Peer == "" {
		s.Peer = h.peer(r)
	}
	if s.Actor != "" {
		for k, v := range h.sessions {
			if v.Actor == s.Actor {
				delete(h.sessions, k)
			}
		}
	}
	oldestAnonymous := func(peer string) ([32]byte, bool) {
		var selected [32]byte
		var selectedAt time.Time
		found := false
		for k, v := range h.sessions {
			if v.Actor != "" || (peer != "" && v.Peer != peer) {
				continue
			}
			if !found || v.LastSeen.Before(selectedAt) {
				selected, selectedAt, found = k, v.LastSeen, true
			}
		}
		return selected, found
	}
	if s.Actor == "" {
		anonymous, samePeer := 0, 0
		for _, v := range h.sessions {
			if v.Actor == "" {
				anonymous++
				if v.Peer == s.Peer {
					samePeer++
				}
			}
		}
		for samePeer >= maxAnonymousPerPeer {
			k, ok := oldestAnonymous(s.Peer)
			if !ok {
				break
			}
			delete(h.sessions, k)
			samePeer--
			anonymous--
		}
		for anonymous >= maxAnonymousSessions {
			k, ok := oldestAnonymous("")
			if !ok {
				break
			}
			delete(h.sessions, k)
			anonymous--
		}
	}
	for len(h.sessions) >= maxSessions {
		k, ok := oldestAnonymous("")
		if !ok {
			return false
		}
		delete(h.sessions, k)
	}
	if len(h.sessions) >= maxSessions {
		return false
	}
	raw := token()
	s.CSRF = token()
	s.LastSeen = now
	if s.Actor == "" {
		s.Expires = now.Add(5 * time.Minute)
	} else {
		s.Expires = now.Add(time.Hour)
	}
	h.sessions[sha256.Sum256([]byte(raw))] = s
	h.setCookie(w, raw, int(s.Expires.Sub(now).Seconds()))
	writeJSON(w, 200, sessionDTO(s))
	return true
}
func (h *Handler) setCookie(w http.ResponseWriter, value string, age int) {
	http.SetCookie(w, &http.Cookie{Name: h.cookie, Value: value, Path: "/", HttpOnly: true, Secure: !h.cfg.DevHTTP, SameSite: http.SameSiteLaxMode, MaxAge: age})
}
func sessionDTO(s session) any {
	return struct {
		Authenticated bool      `json:"authenticated"`
		Account       string    `json:"account"`
		CSRF          string    `json:"csrf"`
		Expires       time.Time `json:"expiresAt"`
	}{s.Actor != "", s.Actor, s.CSRF, s.Expires}
}

func (h *Handler) authorized(w http.ResponseWriter, r *http.Request) (session, bool) {
	key, s, ok := h.readSession(r)
	if !ok || s.Actor == "" {
		writeError(w, 401, "authentication_required")
		return session{}, false
	}
	entries, err := h.cfg.Staff()
	staff, exists := entries[s.Actor]
	if err != nil {
		writeError(w, 503, "staff_unavailable")
		return session{}, false
	}
	if !exists || staff.fingerprint() != s.StaffFingerprint {
		h.mu.Lock()
		delete(h.sessions, key)
		h.mu.Unlock()
		writeError(w, 403, "staff_revoked")
		return session{}, false
	}
	return s, true
}
func (h *Handler) csrf(w http.ResponseWriter, r *http.Request) ([32]byte, session, bool) {
	key, s, ok := h.readSession(r)
	if !ok || subtle.ConstantTimeCompare([]byte(s.CSRF), []byte(r.Header.Get("X-CSRF-Token"))) != 1 {
		writeError(w, 403, "invalid_csrf")
		return key, s, false
	}
	return key, s, true
}

func (h *Handler) sessionInfo(w http.ResponseWriter, r *http.Request) {
	if !h.allow("session:"+h.peer(r), 60) {
		w.Header().Set("Retry-After", "60")
		writeError(w, 429, "rate_limited")
		return
	}
	key, s, ok := h.readSession(r)
	if ok {
		if s.Actor != "" {
			if _, ok = h.authorized(w, r); !ok {
				return
			}
		}
		writeJSON(w, 200, sessionDTO(s))
		return
	}
	if !h.issue(w, r, key, session{}) {
		writeError(w, 503, "session_capacity")
	}
}

func (h *Handler) login(w http.ResponseWriter, r *http.Request) {
	old, _, ok := h.csrf(w, r)
	if !ok {
		return
	}
	media, _, _ := mime.ParseMediaType(r.Header.Get("Content-Type"))
	if media != "application/json" {
		writeError(w, 415, "json_required")
		return
	}
	var body struct {
		Username string `json:"username"`
		Password string `json:"password"`
		AdminPIN string `json:"adminPin"`
	}
	r.Body = http.MaxBytesReader(w, r.Body, 1024)
	d := json.NewDecoder(r.Body)
	d.DisallowUnknownFields()
	var extra any
	if d.Decode(&body) != nil || d.Decode(&extra) != io.EOF {
		writeError(w, 400, "invalid_request")
		return
	}
	name := strings.ToLower(body.Username)
	if len(name) > 12 || len(body.Password) > 12 || !validAdminPIN(body.AdminPIN) {
		writeError(w, 401, "invalid_credentials")
		return
	}
	if !h.allowAll(
		rateLimit{key: "login:ip:" + h.peer(r), limit: 10},
		rateLimit{key: "login:account:" + name, limit: 5},
		rateLimit{key: "login:global", limit: 60},
	) {
		w.Header().Set("Retry-After", "60")
		writeError(w, 429, "rate_limited")
		return
	}
	select {
	case h.hashSlots <- struct{}{}:
		defer func() { <-h.hashSlots }()
	default:
		writeError(w, 503, "auth_busy")
		return
	}
	// Even a non-staff account runs the same password verification path.
	acc, err := account.Authenticate(h.cfg.Accounts, body.Username, body.Password)
	entries, staffErr := h.cfg.Staff()
	staff, exists := entries[name]
	if staffErr != nil {
		writeError(w, 503, "staff_unavailable")
		return
	}
	if err != nil && !errors.Is(err, account.ErrInvalidCredentials) {
		h.audit(r, "login", "", "unavailable")
		writeError(w, 503, "authentication_unavailable")
		return
	}
	validPIN := subtle.ConstantTimeCompare([]byte(h.cfg.AdminAccessPIN), []byte(body.AdminPIN)) == 1
	valid := err == nil && acc != nil && exists && validPIN
	if !valid {
		h.audit(r, "login", "", "rejected")
		writeError(w, 401, "invalid_credentials")
		return
	}
	if !h.issue(w, r, old, session{Actor: name, StaffFingerprint: staff.fingerprint()}) {
		writeError(w, 503, "session_capacity")
		return
	}
	h.audit(r, "login", name, "succeeded")
}

func (h *Handler) logout(w http.ResponseWriter, r *http.Request) {
	key, s, ok := h.csrf(w, r)
	if !ok {
		return
	}
	h.mu.Lock()
	delete(h.sessions, key)
	h.mu.Unlock()
	h.setCookie(w, "", -1)
	h.audit(r, "logout", s.Actor, "succeeded")
	writeJSON(w, 200, map[string]string{"status": "signed_out"})
}

func (h *Handler) overview(w http.ResponseWriter, r *http.Request) {
	s, ok := h.authorized(w, r)
	if !ok {
		return
	}
	entries, err := h.cfg.Staff()
	staff := entries[s.Actor]
	if err != nil {
		writeError(w, 503, "staff_unavailable")
		return
	}
	if staff.fingerprint() != s.StaffFingerprint || !staff.permits(StatusCapability) || !staff.permits(PlayersCapability) {
		h.audit(r, "overview", s.Actor, "forbidden")
		writeError(w, 403, "capability_required")
		return
	}
	q, err := control.ParseQuery(r.URL.Query())
	if err != nil {
		writeError(w, 400, "invalid_query")
		return
	}
	if !h.allow("overview:"+s.Actor, 30) {
		w.Header().Set("Retry-After", "60")
		writeError(w, 429, "rate_limited")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 3*time.Second)
	defer cancel()
	result, err := h.cfg.Control.Overview(ctx, s.Actor, q)
	if errors.Is(err, control.ErrBusy) {
		w.Header().Set("Retry-After", "1")
		writeError(w, 429, "control_busy")
		return
	}
	if err != nil {
		h.audit(r, "overview", s.Actor, "unavailable")
		writeError(w, 503, "server_unavailable")
		return
	}
	h.audit(r, "overview", s.Actor, "succeeded")
	writeJSON(w, 200, result)
}

func (h *Handler) persistentPlayer(w http.ResponseWriter, r *http.Request) {
	s, ok := h.authorized(w, r)
	if !ok {
		return
	}
	entries, err := h.cfg.Staff()
	staff := entries[s.Actor]
	if err != nil {
		writeError(w, 503, "staff_unavailable")
		return
	}
	if staff.fingerprint() != s.StaffFingerprint || !staff.permits(PlayersCapability) {
		h.audit(r, "persistent_player", s.Actor, "forbidden", "target_uid", r.PathValue("uid"))
		writeError(w, 403, "capability_required")
		return
	}
	accounts := r.URL.Query()["account"]
	if len(accounts) != 1 || accounts[0] == "" || len(accounts[0]) > 12 || len(r.PathValue("uid")) > 64 {
		writeError(w, 400, "invalid_target")
		return
	}
	if !h.allow("persistent_player:"+s.Actor, 60) {
		w.Header().Set("Retry-After", "60")
		writeError(w, 429, "rate_limited")
		return
	}
	ctx, cancel := context.WithTimeout(r.Context(), 4*time.Second)
	defer cancel()
	result, err := h.cfg.Persistent.Player(ctx, accounts[0], r.PathValue("uid"))
	if errors.Is(err, ErrInvalidPersistentTarget) {
		writeError(w, 400, "invalid_target")
		return
	}
	if errors.Is(err, ErrPersistentPlayerNotFound) {
		h.audit(r, "persistent_player", s.Actor, "not_found", "target_account", accounts[0], "target_uid", r.PathValue("uid"))
		writeError(w, 404, "persistent_not_found")
		return
	}
	if err != nil {
		h.audit(r, "persistent_player", s.Actor, "unavailable", "target_account", accounts[0], "target_uid", r.PathValue("uid"))
		writeError(w, 503, "persistent_unavailable")
		return
	}
	h.audit(r, "persistent_player", s.Actor, "succeeded", "target_account", accounts[0], "target_uid", r.PathValue("uid"))
	writeJSON(w, 200, result)
}
func (h *Handler) audit(r *http.Request, action, actor, result string, attrs ...any) {
	base := []any{
		"action", action,
		"actor", actor,
		"result", result,
		"request_id", r.Context().Value(requestIDContextKey{}),
		"client_ip", h.peer(r),
		"method", r.Method,
		"path", r.URL.Path,
	}
	h.cfg.Audit.Info("web_admin", append(base, attrs...)...)
}
