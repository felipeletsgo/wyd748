package logreader

import (
	"context"
	"errors"
	"fmt"
	"net"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"
)

type fakeJournal struct {
	entries []Entry
	unit    string
	request Request
	err     error
}

func (f *fakeJournal) Read(_ context.Context, unit string, r Request, scanLimit int) ([]Entry, bool, error) {
	f.unit, f.request = unit, r
	if f.err != nil {
		return nil, false, f.err
	}
	start := 0
	if r.Cursor != "" {
		for i, e := range f.entries {
			if e.Cursor == r.Cursor {
				start = i + 1
			}
		}
	}
	rest := f.entries[start:]
	if len(rest) > scanLimit {
		return rest[:scanLimit], true, nil
	}
	return rest, false, nil
}

func validRequest() Request {
	now := time.Now().UTC()
	return Request{Version: 1, Kind: KindLogs, Source: "game", Since: now.Add(-time.Hour), Until: now, Priority: 6, Limit: 2}
}

func TestRequestValidation(t *testing.T) {
	if err := validRequest().Validate(); err != nil {
		t.Fatal(err)
	}
	if err := (Request{Version: 1, Kind: KindBackup}).Validate(); err != nil {
		t.Fatal(err)
	}
	for name, mutate := range map[string]func(*Request){
		"unknown source":   func(r *Request) { r.Source = "sshd" },
		"unit name":        func(r *Request) { r.Source = "wyd-game.service" },
		"limit":            func(r *Request) { r.Limit = MaxLimit + 1 },
		"priority":         func(r *Request) { r.Priority = 8 },
		"control in query": func(r *Request) { r.Query = "a\nb" },
		"cursor spaces":    func(r *Request) { r.Cursor = "s=1 --unit=ssh" },
		"window":           func(r *Request) { r.Until = r.Since.Add(MaxWindow + time.Second) },
		"inverted window":  func(r *Request) { r.Until = r.Since.Add(-time.Second) },
		"backup filters":   func(r *Request) { r.Kind = KindBackup },
		"version":          func(r *Request) { r.Version = 2 },
	} {
		r := validRequest()
		mutate(&r)
		if r.Validate() == nil {
			t.Fatal("accepted", name)
		}
	}
}

func TestSanitizeStripsEscapesAndBoundsLength(t *testing.T) {
	got := Sanitize("\x1b[31mred\x1b[0m <b>x</b>\ttab\x00\xff")
	if got != "red <b>x</b>\ttab �" {
		t.Fatalf("%q", got)
	}
	if long := Sanitize(strings.Repeat("é", MaxMessage)); len(long) > MaxMessage || !strings.HasPrefix(long, "é") {
		t.Fatal(len(long))
	}
}

func TestJournalArgsAreFixed(t *testing.T) {
	r := validRequest()
	r.Cursor = "s=abc;i=1"
	args := strings.Join(journalArgs("wyd-game.service", r), " ")
	for _, want := range []string{"--unit=wyd-game.service", "--output=json", "--reverse", "--priority=0..6", "--cursor=s=abc;i=1", "--since=@"} {
		if !strings.Contains(args, want) {
			t.Fatal(want, args)
		}
	}
	if journalMessage([]byte(`[104,105]`)) != "hi" || journalMessage([]byte(`"ok"`)) != "ok" {
		t.Fatal("message decoding")
	}
}

func serveTest(t *testing.T, server *Server) Client {
	t.Helper()
	path := filepath.Join(t.TempDir(), "socket")
	listener, err := net.Listen("unix", path)
	if err != nil {
		t.Skip("unix sockets unavailable:", err)
	}
	t.Cleanup(func() { listener.Close() })
	go server.Serve(listener)
	return Client{Path: path}
}

func TestServerFiltersPagesAndAuthorizes(t *testing.T) {
	journal := &fakeJournal{}
	for i := range 10 {
		message := fmt.Sprintf("tick %d", i)
		if i%3 == 0 {
			message = fmt.Sprintf("LOGIN account=%d", i)
		}
		journal.entries = append(journal.entries, Entry{Cursor: fmt.Sprintf("c%d", i), Time: time.Now(), Priority: 6, Message: message})
	}
	allowed := true
	client := serveTest(t, &Server{Journal: journal, MaxConcurrent: 1, Authorize: func(net.Conn) error {
		if !allowed {
			return errors.New("no")
		}
		return nil
	}})
	r := validRequest()
	r.Query = "login"
	page, err := client.Do(context.Background(), r)
	if err != nil || page.Error != "" || len(page.Entries) != 2 || page.Entries[1].Cursor != "c3" || page.NextCursor != "c3" || journal.unit != "wyd-game.service" {
		t.Fatal(page, err)
	}
	r.Cursor = page.NextCursor
	page, _ = client.Do(context.Background(), r)
	// c9 is the last entry: the page is full and nothing older remains.
	if len(page.Entries) != 2 || page.Entries[0].Cursor != "c6" || page.NextCursor != "" {
		t.Fatal(page)
	}
	r.Cursor, r.Query = "c9", ""
	if page, _ = client.Do(context.Background(), r); len(page.Entries) != 0 || page.NextCursor != "" {
		t.Fatal("end of journal", page)
	}
	allowed = false
	if page, _ = client.Do(context.Background(), validRequest()); page.Error != "forbidden" {
		t.Fatal("unauthorized peer served", page)
	}
	allowed = true
	journal.err = ErrCursorExpired
	if page, _ = client.Do(context.Background(), validRequest()); page.Error != "cursor_expired" {
		t.Fatal(page)
	}
}

func TestBackupReceipts(t *testing.T) {
	dir := t.TempDir()
	status := filepath.Join(dir, "status.json")
	if err := os.WriteFile(status, []byte(`{"at":"2026-10-22T04:15:00Z","archive":"wydgo-x.dump","bytes":42,"sha256":"ab"}`), 0o644); err != nil {
		t.Fatal(err)
	}
	client := serveTest(t, &Server{Journal: &fakeJournal{}, Authorize: func(net.Conn) error { return nil },
		BackupStatus: status, RestoreStatus: filepath.Join(dir, "missing.json")})
	page, err := client.Do(context.Background(), Request{Version: 1, Kind: KindBackup})
	if err != nil || page.Backup == nil || page.Backup.Bytes != 42 || page.RestoreTest != nil {
		t.Fatal(page, err)
	}
	if _, err := (Client{Path: filepath.Join(dir, "absent")}).Do(context.Background(), validRequest()); !errors.Is(err, ErrUnavailable) {
		t.Fatal(err)
	}
}

func TestLargeEscapedPagesStayWithinTheReplyLimit(t *testing.T) {
	journal := &fakeJournal{}
	// Quotes and HTML characters double or sextuple in JSON: raw counting
	// would let this page exceed the client's limit.
	message := strings.Repeat(`<a href="x">&`, MaxMessage/14)
	for i := range 600 {
		journal.entries = append(journal.entries, Entry{Cursor: fmt.Sprintf("cursor-%04d", i), Time: time.Now(), Priority: 6, Message: message})
	}
	client := serveTest(t, &Server{Journal: journal, MaxConcurrent: 1, Authorize: func(net.Conn) error { return nil }})
	r := validRequest()
	r.Limit = MaxLimit
	page, err := client.Do(context.Background(), r)
	if err != nil || page.Error != "" || !page.Truncated || len(page.Entries) == 0 || len(page.Entries) >= MaxLimit {
		t.Fatal(len(page.Entries), page.Truncated, err)
	}
	if want := page.Entries[len(page.Entries)-1].Cursor; page.NextCursor != want {
		t.Fatal("next page must start after the last sent entry", page.NextCursor, want)
	}
	r.Cursor = page.NextCursor
	next, err := client.Do(context.Background(), r)
	if err != nil || len(next.Entries) == 0 || next.Entries[0].Cursor != fmt.Sprintf("cursor-%04d", len(page.Entries)) {
		t.Fatal("continuation skipped or repeated entries", err)
	}
	// A filter counts only matching lines toward the budget.
	journal.entries = append(journal.entries[:0:0], Entry{Cursor: "match", Time: time.Now(), Message: "LOGIN ok"})
	for i := range 400 {
		journal.entries = append(journal.entries, Entry{Cursor: fmt.Sprintf("noise-%04d", i), Time: time.Now(), Message: message})
	}
	journal.entries = append(journal.entries, Entry{Cursor: "match-2", Time: time.Now(), Message: "LOGIN two"})
	r.Cursor, r.Query = "", "login"
	if page, err := client.Do(context.Background(), r); err != nil || len(page.Entries) != 2 || page.Truncated {
		t.Fatal("filtered-out lines consumed the budget", len(page.Entries), page.Truncated, err)
	}
}
