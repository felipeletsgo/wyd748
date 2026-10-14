package playerportal

import (
	"encoding/hex"
	"net/http"
	"sort"
	"time"
)

// Display IDs are independent random values, never bearer cookies or CSRF tokens.
// Sessions belong to this API process; restart revokes every web session.
type sessionView struct {
	ID         string    `json:"id"`
	Current    bool      `json:"current"`
	CreatedAt  time.Time `json:"createdAt"`
	LastSeenAt time.Time `json:"lastSeenAt"`
	ExpiresAt  time.Time `json:"expiresAt"`
}

func (h *Handler) activeSessionLocked(key [32]byte, now time.Time) (session, bool) {
	s, ok := h.sessions[key]
	return s, ok && s.Account != "" && now.Before(s.Expires) && now.Sub(s.Idle) <= 30*time.Minute
}

func (h *Handler) listSessions(w http.ResponseWriter, r *http.Request) {
	_, key, _ := h.current(r)
	now := h.now()
	h.mu.Lock()
	s, ok := h.activeSessionLocked(key, now)
	if !ok {
		h.mu.Unlock()
		failure(w, 401, "Sign in to manage web sessions")
		return
	}
	views := make([]sessionView, 0)
	for otherKey, other := range h.sessions {
		if other.Account != s.Account || !now.Before(other.Expires) || now.Sub(other.Idle) > 30*time.Minute {
			continue
		}
		expires := other.Expires
		if idleDeadline := other.Idle.Add(30 * time.Minute); idleDeadline.Before(expires) {
			expires = idleDeadline
		}
		views = append(views, sessionView{other.ID, otherKey == key, other.Created.UTC(), other.Idle.UTC(), expires.UTC()})
	}
	h.mu.Unlock()
	sort.Slice(views, func(i, j int) bool {
		if views[i].Current != views[j].Current {
			return views[i].Current
		}
		if !views[i].CreatedAt.Equal(views[j].CreatedAt) {
			return views[i].CreatedAt.After(views[j].CreatedAt)
		}
		return views[i].ID < views[j].ID
	})
	reply(w, 200, map[string]any{"version": 1, "sessions": views, "asOf": now.UTC()})
}

func (h *Handler) revokeSession(w http.ResponseWriter, r *http.Request) {
	var body struct {
		ID string `json:"id"`
	}
	if !decode(w, r, &body) {
		return
	}
	if decoded, err := hex.DecodeString(body.ID); err != nil || len(decoded) != 32 {
		failure(w, 400, "Invalid session ID")
		return
	}
	h.revokeSessions(w, r, body.ID, false)
}

func (h *Handler) revokeAllSessions(w http.ResponseWriter, r *http.Request) {
	if !decode(w, r, &struct{}{}) {
		return
	}
	h.revokeSessions(w, r, "", true)
}

func (h *Handler) revokeSessions(w http.ResponseWriter, r *http.Request, id string, all bool) {
	_, key, _ := h.current(r)
	h.mu.Lock()
	s, ok := h.activeSessionLocked(key, h.now())
	if !ok {
		h.mu.Unlock()
		failure(w, 401, "Sign in to manage web sessions")
		return
	}
	found, signedOut := false, false
	for targetKey, target := range h.sessions {
		if target.Account == s.Account && (all || target.ID == id) {
			delete(h.sessions, targetKey)
			found = true
			signedOut = signedOut || targetKey == key
		}
	}
	h.mu.Unlock()
	if !found {
		failure(w, 404, "Session not found")
		return
	}
	if signedOut {
		http.SetCookie(w, &http.Cookie{Name: h.cookie, Path: "/", MaxAge: -1, Secure: !h.cfg.DevHTTP, HttpOnly: true, SameSite: http.SameSiteStrictMode})
	}
	reply(w, 200, map[string]any{"version": 1, "status": "revoked", "signedOut": signedOut})
}
