// Package playerportal exposes the versioned public website API, not staff powers.
package playerportal

import (
	"context"
	"crypto/rand"
	"crypto/sha256"
	"crypto/subtle"
	"encoding/hex"
	"encoding/json"
	"errors"
	"io"
	"mime"
	"net/http"
	"net/netip"
	"net/url"
	"strings"
	"sync"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/authlimit"
)

type Character struct {
	Slot      int    `json:"slot"`
	Name      string `json:"name"`
	Class     byte   `json:"class"`
	Level     uint32 `json:"level"`
	Evolution string `json:"evolution"`
}
type Profile struct {
	Username   string      `json:"username"`
	Donate     uint32      `json:"donate"`
	Characters []Character `json:"characters"`
	CreatedAt  time.Time   `json:"createdAt"`
	AsOf       time.Time   `json:"asOf"`
}
type Backend interface {
	account.AuthStore
	account.RegistrationStore
	Profile(context.Context, string) (Profile, error)
	CharacterDetail(context.Context, string, int) (CharacterDetail, error)
	Inventory(context.Context, string, int) (StorageSnapshot, error)
	Cargo(context.Context, string) (StorageSnapshot, error)
	Ranking(context.Context) ([]Character, error)
	PublicGuilds(context.Context) (PublicGuildSnapshot, error)
	Ping(context.Context) error
}
type Config struct {
	Origin                string
	DevHTTP               bool
	TrustLoopbackProxy    bool
	Backend               Backend
	RegistrationVerifier  RegistrationVerifier
	LoginVerifier         RegistrationVerifier
	AuthenticationLimiter authlimit.Limiter
	ItemNames             map[uint16]string
}
type session struct {
	Account, CSRF, Peer string
	Expires, Idle       time.Time
	ID                  string
	Created             time.Time
}
type rate struct {
	Start time.Time
	Count int
}
type Handler struct {
	cfg               Config
	host, cookie      string
	mux               *http.ServeMux
	mu                sync.Mutex
	sessions          map[[32]byte]session
	rates             map[string]rate
	nextRateSweep     time.Time
	slots             chan struct{}
	registrationSlots chan struct{}
	registrationLimit registrationLimiter
	now               func() time.Time
	rankMu            sync.Mutex
	rank              []Character
	rankAt            time.Time
	guildMu           sync.Mutex
	guildSnapshot     PublicGuildSnapshot
	guildAt           time.Time
	guildError        error
	guildFlight       chan struct{}
}

func New(cfg Config) (*Handler, error) {
	u, err := url.Parse(cfg.Origin)
	if err != nil || u == nil || u.Host == "" || u.User != nil || u.Path != "" || u.RawQuery != "" || u.ForceQuery || u.Fragment != "" || u.Opaque != "" || cfg.Backend == nil {
		return nil, errors.New("portal: backend and exact HTTPS origin are required")
	}
	if cfg.DevHTTP {
		ip, err := netip.ParseAddr(u.Hostname())
		if err != nil || !ip.IsLoopback() || u.Scheme != "http" {
			return nil, errors.New("portal: development HTTP requires literal loopback")
		}
	} else if u.Scheme != "https" {
		return nil, errors.New("portal: HTTPS origin is required")
	}
	h := &Handler{cfg: cfg, host: u.Host, cookie: "__Host-wyd_player", mux: http.NewServeMux(), sessions: make(map[[32]byte]session), rates: make(map[string]rate), slots: make(chan struct{}, 2), registrationSlots: make(chan struct{}, 1), now: time.Now}
	if cfg.DevHTTP {
		h.cookie = "wyd_player_local"
	}
	h.mux.HandleFunc("GET /api/player/v1/session", h.getSession)
	h.mux.HandleFunc("POST /api/player/v1/login", h.login)
	h.mux.HandleFunc("POST /api/player/v1/logout", h.logout)
	h.mux.HandleFunc("GET /api/player/v1/sessions", h.listSessions)
	h.mux.HandleFunc("POST /api/player/v1/sessions/revoke", h.revokeSession)
	h.mux.HandleFunc("POST /api/player/v1/sessions/revoke-all", h.revokeAllSessions)
	h.mux.HandleFunc("POST /api/player/v1/accounts", h.register)
	h.mux.HandleFunc("GET /api/player/v1/profile", h.profile)
	h.mux.HandleFunc("GET /api/player/v1/characters/{slot}", h.characterDetail)
	h.mux.HandleFunc("GET /api/player/v1/characters/{slot}/inventory", h.inventory)
	h.mux.HandleFunc("GET /api/player/v1/cargo", h.cargo)
	h.mux.HandleFunc("GET /api/player/v1/ranking", h.ranking)
	h.mux.HandleFunc("GET /api/player/v1/guilds/{name}", h.guildProfile)
	h.mux.HandleFunc("GET /healthz", func(w http.ResponseWriter, r *http.Request) {
		reply(w, 200, map[string]any{"version": 1, "status": "ok"})
	})
	h.mux.HandleFunc("GET /readyz", func(w http.ResponseWriter, r *http.Request) {
		ctx, cancel := context.WithTimeout(r.Context(), time.Second)
		defer cancel()
		if h.cfg.Backend.Ping(ctx) != nil {
			failure(w, 503, "Database unavailable")
			return
		}
		reply(w, 200, map[string]any{"version": 1, "status": "ready"})
	})
	return h, nil
}
func (h *Handler) ServeHTTP(w http.ResponseWriter, r *http.Request) {
	w.Header().Set("Cache-Control", "no-store")
	w.Header().Set("X-Content-Type-Options", "nosniff")
	w.Header().Set("Referrer-Policy", "no-referrer")
	w.Header().Set("Content-Security-Policy", "default-src 'none'; frame-ancestors 'none'")
	if r.Host != h.host {
		failure(w, 403, "Invalid host")
		return
	}
	// Local readiness must remain available when public request quotas are full.
	// Proxied requests carry X-Forwarded-For and never receive this exemption.
	if localHealthProbe(r) {
		h.mux.ServeHTTP(w, r)
		return
	}
	if !h.allow("read:"+limitNetwork(h.peer(r)), 120) || !h.allow("read:global", 1200) {
		failure(w, 429, "Please wait before trying again")
		return
	}
	if r.Method == http.MethodPost {
		if r.Header.Get("Origin") != h.cfg.Origin {
			failure(w, 403, "Invalid origin")
			return
		}
		typeName, _, err := mime.ParseMediaType(r.Header.Get("Content-Type"))
		if err != nil || typeName != "application/json" {
			failure(w, 415, "Use application/json")
			return
		}
		s, _, ok := h.current(r)
		if !ok || subtle.ConstantTimeCompare([]byte(s.CSRF), []byte(r.Header.Get("X-CSRF-Token"))) != 1 {
			failure(w, 403, "Session expired; reload this page")
			return
		}
	}
	h.mux.ServeHTTP(w, r)
}
func (h *Handler) peer(r *http.Request) string {
	addr, err := netip.ParseAddrPort(r.RemoteAddr)
	if err != nil {
		return "unknown"
	}
	ip := addr.Addr().Unmap()
	if h.cfg.TrustLoopbackProxy && ip.IsLoopback() {
		if forwarded, err := netip.ParseAddr(r.Header.Get("X-Forwarded-For")); err == nil {
			ip = forwarded.Unmap()
		}
	}
	return ip.String()
}
func (h *Handler) allow(key string, limit int) bool {
	h.mu.Lock()
	defer h.mu.Unlock()
	now := h.now()
	if !now.Before(h.nextRateSweep) {
		for k, v := range h.rates {
			if now.Sub(v.Start) >= time.Minute {
				delete(h.rates, k)
			}
		}
		h.nextRateSweep = now.Add(time.Minute)
	}
	v, exists := h.rates[key]
	if exists && now.Sub(v.Start) >= time.Minute {
		v, exists = rate{}, false
	}
	if !exists {
		if _, present := h.rates[key]; !present && len(h.rates) >= 4096 {
			return false
		}
		v.Start = now
	}
	if v.Count >= limit {
		return false
	}
	v.Count++
	h.rates[key] = v
	return true
}
func token() string {
	var b [32]byte
	if _, err := rand.Read(b[:]); err != nil {
		panic("portal: cryptographic entropy unavailable")
	}
	return hex.EncodeToString(b[:])
}
func (h *Handler) current(r *http.Request) (session, [32]byte, bool) {
	c, err := r.Cookie(h.cookie)
	if err != nil || len(c.Value) != 64 {
		return session{}, [32]byte{}, false
	}
	key := sha256.Sum256([]byte(c.Value))
	h.mu.Lock()
	defer h.mu.Unlock()
	s, ok := h.sessions[key]
	if !ok || !h.now().Before(s.Expires) || h.now().Sub(s.Idle) > 30*time.Minute || (s.Account == "" && s.Peer != h.peer(r)) {
		delete(h.sessions, key)
		return session{}, key, false
	}
	s.Idle = h.now()
	h.sessions[key] = s
	return s, key, true
}
func (h *Handler) issue(w http.ResponseWriter, r *http.Request, name string) (session, bool) {
	value, csrf := token(), token()
	now, peer := h.now(), h.peer(r)
	h.mu.Lock()
	defer h.mu.Unlock()
	anonymous, peerAnonymous := 0, 0
	for key, s := range h.sessions {
		if !now.Before(s.Expires) || now.Sub(s.Idle) > 30*time.Minute {
			delete(h.sessions, key)
			continue
		}
		if s.Account == "" {
			anonymous++
			if limitNetwork(s.Peer) == limitNetwork(peer) {
				peerAnonymous++
			}
		}
	}
	if len(h.sessions) >= 512 || (name == "" && (anonymous >= 128 || peerAnonymous >= 4)) {
		return session{}, false
	}
	if c, err := r.Cookie(h.cookie); err == nil {
		delete(h.sessions, sha256.Sum256([]byte(c.Value)))
	}
	lifetime := 8 * time.Hour
	if name == "" {
		lifetime = 20 * time.Minute
	}
	s := session{Account: name, CSRF: csrf, Peer: peer, Expires: now.Add(lifetime), Idle: now, Created: now, ID: token()}
	h.sessions[sha256.Sum256([]byte(value))] = s
	http.SetCookie(w, &http.Cookie{Name: h.cookie, Value: value, Path: "/", Secure: !h.cfg.DevHTTP, HttpOnly: true, SameSite: http.SameSiteStrictMode, MaxAge: int(lifetime.Seconds())})
	return s, true
}
func sessionReply(w http.ResponseWriter, s session) {
	reply(w, 200, map[string]any{"version": 1, "authenticated": s.Account != "", "username": s.Account, "csrf": s.CSRF})
}
func (h *Handler) getSession(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.current(r)
	if !ok {
		if !h.allow("session:"+limitNetwork(h.peer(r)), 10) {
			failure(w, 429, "Please wait before trying again")
			return
		}
		s, ok = h.issue(w, r, "")
	}
	if !ok {
		failure(w, 503, "Session capacity reached")
		return
	}
	sessionReply(w, s)
}

type credentials struct {
	Username             string `json:"username"`
	Password             string `json:"password"`
	PasswordConfirmation string `json:"passwordConfirmation,omitempty"`
}

func decode(w http.ResponseWriter, r *http.Request, value any) bool {
	return decodeLimited(w, r, value, 1024)
}
func decodeLimited(w http.ResponseWriter, r *http.Request, value any, limit int64) bool {
	r.Body = http.MaxBytesReader(w, r.Body, limit)
	d := json.NewDecoder(r.Body)
	d.DisallowUnknownFields()
	if d.Decode(value) != nil || d.Decode(&struct{}{}) != io.EOF {
		failure(w, 400, "Invalid JSON request")
		return false
	}
	return true
}
func (h *Handler) work(w http.ResponseWriter, r *http.Request, username, action string) bool {
	if len(username) > 12 {
		failure(w, 422, "Username is too long")
		return false
	}
	if !h.allow(action+":"+limitNetwork(h.peer(r)), 8) || !h.allow(action+":account:"+strings.ToLower(username), 8) || !h.allow("hash:global", 60) {
		failure(w, 429, "Please wait before trying again")
		return false
	}
	select {
	case h.slots <- struct{}{}:
		return true
	default:
		failure(w, 503, "Service busy; please try again")
		return false
	}
}
func (h *Handler) login(w http.ResponseWriter, r *http.Request) {
	var c struct {
		Username       string `json:"username"`
		Password       string `json:"password"`
		TurnstileToken string `json:"turnstileToken"`
	}
	if !decodeLimited(w, r, &c, 4096) || !h.work(w, r, c.Username, "login") {
		return
	}
	defer func() { <-h.slots }()
	if !h.durableLimit(w, r, "player_login", c.Username) || !h.verifyHuman(w, r, h.cfg.LoginVerifier, c.TurnstileToken) {
		return
	}
	a, err := account.Authenticate(h.cfg.Backend, c.Username, c.Password)
	if errors.Is(err, account.ErrInvalidCredentials) {
		failure(w, 401, "Invalid username or password")
		return
	}
	if err != nil {
		failure(w, 503, "Sign-in unavailable")
		return
	}
	s, ok := h.issue(w, r, a.Name)
	if !ok {
		failure(w, 503, "Session capacity reached")
		return
	}
	sessionReply(w, s)
}
func (h *Handler) logout(w http.ResponseWriter, r *http.Request) {
	_, key, _ := h.current(r)
	h.mu.Lock()
	delete(h.sessions, key)
	h.mu.Unlock()
	http.SetCookie(w, &http.Cookie{Name: h.cookie, Value: "", Path: "/", MaxAge: -1, Secure: !h.cfg.DevHTTP, HttpOnly: true, SameSite: http.SameSiteStrictMode})
	reply(w, 200, map[string]any{"version": 1, "status": "signed-out"})
}
func (h *Handler) register(w http.ResponseWriter, r *http.Request) {
	// Charge attempts before parsing, hashing or accessing the database. Rotating
	// usernames, sessions or IPv6 addresses within a /64 cannot reset the quota.
	if wait := h.registrationLimit.admit(limitNetwork(h.peer(r)), h.now()); wait > 0 {
		w.Header().Set("Retry-After", retrySeconds(wait))
		failure(w, 429, "Too many registration attempts. Please wait before trying again.")
		return
	}
	var c struct {
		credentials
		TurnstileToken string `json:"turnstileToken"`
	}
	if !decodeLimited(w, r, &c, 4096) {
		return
	}
	if !h.durableLimit(w, r, "registration", c.Username) {
		return
	}
	// At most one registration may occupy the two shared password-work slots;
	// registration never consumes the login endpoint's global hash quota.
	select {
	case h.registrationSlots <- struct{}{}:
		defer func() { <-h.registrationSlots }()
	default:
		failure(w, 503, "Registration busy; please try again later")
		return
	}
	if h.cfg.RegistrationVerifier == nil {
		failure(w, 503, "Human verification unavailable; please try again later")
		return
	}
	if !validTurnstileToken(c.TurnstileToken) {
		failure(w, 403, "Complete human verification before creating an account")
		return
	}
	if err := h.cfg.RegistrationVerifier.Verify(r.Context(), c.TurnstileToken, h.peer(r)); err != nil {
		if errors.Is(err, errVerificationRejected) {
			failure(w, 403, "Human verification expired or failed; please try again")
		} else {
			failure(w, 503, "Human verification unavailable; please try again later")
		}
		return
	}
	select {
	case h.slots <- struct{}{}:
		defer func() { <-h.slots }()
	default:
		failure(w, 503, "Service busy; please try again")
		return
	}
	if len(c.Password) < 8 {
		failure(w, 422, "New passwords must contain 8 to 10 characters")
		return
	}
	a, err := account.Create(h.cfg.Backend, c.Username, c.Password, c.PasswordConfirmation)
	var validation *account.ValidationError
	switch {
	case errors.As(err, &validation):
		failure(w, 422, validation.Message)
	case errors.Is(err, account.ErrUsernameUnavailable):
		failure(w, 409, "Username unavailable")
	case err != nil:
		failure(w, 503, "Registration unavailable")
	default:
		reply(w, 201, map[string]any{"version": 1, "status": "created", "username": a.Name})
	}
}

func (h *Handler) durableLimit(w http.ResponseWriter, r *http.Request, scope, username string) bool {
	if h.cfg.AuthenticationLimiter == nil {
		failure(w, 503, "Authentication protection unavailable")
		return false
	}
	wait, err := h.cfg.AuthenticationLimiter.Admit(r.Context(), authlimit.Policy(scope, h.peer(r), username))
	if err != nil {
		failure(w, 503, "Authentication protection unavailable")
		return false
	}
	if wait > 0 {
		w.Header().Set("Retry-After", retrySeconds(wait))
		failure(w, 429, "Please wait before trying again")
		return false
	}
	return true
}

func (h *Handler) verifyHuman(w http.ResponseWriter, r *http.Request, verifier RegistrationVerifier, token string) bool {
	if verifier == nil {
		failure(w, 503, "Human verification unavailable; please try again later")
		return false
	}
	if !validTurnstileToken(token) {
		failure(w, 403, "Complete human verification before signing in")
		return false
	}
	if err := verifier.Verify(r.Context(), token, h.peer(r)); err != nil {
		if errors.Is(err, errVerificationRejected) {
			failure(w, 403, "Human verification expired or failed; please try again")
		} else {
			failure(w, 503, "Human verification unavailable; please try again later")
		}
		return false
	}
	return true
}
func (h *Handler) profile(w http.ResponseWriter, r *http.Request) {
	s, _, ok := h.current(r)
	if !ok || s.Account == "" {
		failure(w, 401, "Sign in to view your characters")
		return
	}
	p, err := h.cfg.Backend.Profile(r.Context(), s.Account)
	if err != nil {
		failure(w, 503, "Profile unavailable")
		return
	}
	reply(w, 200, map[string]any{"version": 1, "profile": p, "source": "persisted snapshot"})
}
func (h *Handler) ranking(w http.ResponseWriter, r *http.Request) {
	h.rankMu.Lock()
	defer h.rankMu.Unlock()
	if h.rankAt.IsZero() || h.now().Sub(h.rankAt) > 30*time.Second {
		rows, err := h.cfg.Backend.Ranking(r.Context())
		if err != nil {
			failure(w, 503, "Ranking unavailable")
			return
		}
		h.rank = rows
		h.rankAt = h.now()
	}
	reply(w, 200, map[string]any{"version": 1, "characters": h.rank, "asOf": h.rankAt.UTC(), "source": "persisted snapshot"})
}
func reply(w http.ResponseWriter, status int, value any) {
	w.Header().Set("Content-Type", "application/json; charset=utf-8")
	w.WriteHeader(status)
	_ = json.NewEncoder(w).Encode(value)
}
func failure(w http.ResponseWriter, status int, message string) {
	if status == http.StatusTooManyRequests && w.Header().Get("Retry-After") == "" {
		w.Header().Set("Retry-After", "60")
	}
	reply(w, status, map[string]any{"version": 1, "error": message})
}
