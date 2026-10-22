// Package logreader is the private protocol between the staff panel and the
// wyd-log-reader helper (DOCS/server/staff-operations.md). The helper owns
// journal access; the game process only sends one bounded JSON request per
// Unix socket connection and never receives a path, unit name or command.
package logreader

import (
	"errors"
	"strings"
	"time"
	"unicode"
	"unicode/utf8"
)

const (
	KindLogs   = "logs"
	KindBackup = "backup"

	DefaultLimit   = 200
	MaxLimit       = 500
	MaxQueryRunes  = 80
	MaxCursorBytes = 512
	MaxMessage     = 2048
	MaxWindow      = 7 * 24 * time.Hour
	// MaxResponseBytes bounds one encoded reply on both sides of the socket.
	MaxResponseBytes = 512 * 1024
	responseEnvelope = 4096
	MaxRequestBytes  = 4096
)

// Sources maps the public source names to their fixed systemd units. Nothing
// outside this map can be read.
var Sources = map[string]string{
	"game":   "wyd-game.service",
	"player": "wyd-player.service",
	"backup": "wyd-backup.service",
}

type Request struct {
	Version  int       `json:"version"`
	Kind     string    `json:"kind"`
	Source   string    `json:"source,omitempty"`
	Since    time.Time `json:"since,omitempty"`
	Until    time.Time `json:"until,omitempty"`
	Priority int       `json:"priority"`
	Query    string    `json:"query,omitempty"`
	Limit    int       `json:"limit,omitempty"`
	Cursor   string    `json:"cursor,omitempty"`
}

// Validate accepts only allowlisted sources and bounded filters. Priority is
// the journal maximum level (0 emerg … 7 debug).
func (r Request) Validate() error {
	if r.Version != 1 {
		return errors.New("invalid_request")
	}
	switch r.Kind {
	case KindBackup:
		if r.Source != "" || r.Query != "" || r.Cursor != "" || r.Limit != 0 {
			return errors.New("invalid_request")
		}
		return nil
	case KindLogs:
	default:
		return errors.New("invalid_request")
	}
	if _, ok := Sources[r.Source]; !ok {
		return errors.New("invalid_source")
	}
	if r.Priority < 0 || r.Priority > 7 || r.Limit < 1 || r.Limit > MaxLimit ||
		!utf8.ValidString(r.Query) || utf8.RuneCountInString(r.Query) > MaxQueryRunes ||
		strings.IndexFunc(r.Query, unicode.IsControl) >= 0 || len(r.Cursor) > MaxCursorBytes ||
		strings.IndexFunc(r.Cursor, func(c rune) bool { return c <= ' ' || c > '~' }) >= 0 {
		return errors.New("invalid_request")
	}
	if r.Since.IsZero() || (!r.Until.IsZero() && (!r.Until.After(r.Since) || r.Until.Sub(r.Since) > MaxWindow)) {
		return errors.New("invalid_window")
	}
	return nil
}

type Entry struct {
	Cursor   string    `json:"cursor"`
	Time     time.Time `json:"time"`
	Priority int       `json:"priority"`
	Message  string    `json:"message"`
}

type Receipt struct {
	At      time.Time `json:"at"`
	Archive string    `json:"archive"`
	Bytes   int64     `json:"bytes,omitempty"`
	SHA256  string    `json:"sha256,omitempty"`
	Records int64     `json:"restoredAccounts,omitempty"`
}

type Response struct {
	Version     int      `json:"version"`
	Error       string   `json:"error,omitempty"`
	Entries     []Entry  `json:"entries,omitempty"`
	NextCursor  string   `json:"nextCursor,omitempty"`
	Truncated   bool     `json:"truncated,omitempty"`
	Backup      *Receipt `json:"backup,omitempty"`
	RestoreTest *Receipt `json:"restoreTest,omitempty"`
}

// Sanitize keeps log text inert for the browser: no ANSI escapes or control
// characters except tab, valid UTF-8 only, bounded length.
func Sanitize(message string) string {
	message = strings.ToValidUTF8(message, "�")
	var b strings.Builder
	b.Grow(min(len(message), MaxMessage))
	escape := false
	for _, r := range message {
		switch {
		case escape:
			// CSI sequences end with a byte in @..~; other escapes end at once.
			if r >= '@' && r <= '~' && r != '[' {
				escape = false
			}
			continue
		case r == 0x1b:
			escape = true
			continue
		case r == '\t':
		case unicode.IsControl(r):
			r = ' '
		}
		if b.Len()+utf8.RuneLen(r) > MaxMessage {
			break
		}
		b.WriteRune(r)
	}
	return b.String()
}
