package game

import (
	"bytes"
	"context"
	"encoding/binary"
	"expvar"
	"io"
	stdnet "net"
	"net/url"
	"os"
	"strings"
	"sync"
	"sync/atomic"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// W4 acceptance with real TCP loss: a proxy between the game store and
// PostgreSQL recognizes the frontend COMMIT message and drops the connection
// before or after PostgreSQL receives it. The World, the coordinator, the
// store reconciliation and the quarantine release all run unmodified.

type commitCutMode int

const (
	cutAfterCommitSent   commitCutMode = iota // PostgreSQL commits; the client never hears it
	cutBeforeCommitSent                       // the COMMIT never reaches PostgreSQL
	cutAfterAndIsolateDB                      // as after, then every connection drops and new ones are refused
)

type commitCutProxy struct {
	listener stdnet.Listener
	target   string
	armed    atomic.Int32
	mode     atomic.Int32
	refuse   atomic.Bool
	cuts     atomic.Int32
	mu       sync.Mutex
	conns    []stdnet.Conn
	// marker selects the connection that wrote the target account; only its
	// COMMIT is cut, never a concurrent store transaction.
	marker atomic.Pointer[[]byte]
}

func newCommitCutProxy(t *testing.T, target string) *commitCutProxy {
	t.Helper()
	ln, err := stdnet.Listen("tcp4", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	p := &commitCutProxy{listener: ln, target: target}
	t.Cleanup(func() { _ = ln.Close(); p.dropAll() })
	go p.serve()
	return p
}

func (p *commitCutProxy) arm(mode commitCutMode, marker string) {
	lowered := bytes.ToLower([]byte(marker))
	p.marker.Store(&lowered)
	p.mode.Store(int32(mode))
	p.armed.Store(1)
}

func (p *commitCutProxy) dropAll() {
	p.mu.Lock()
	defer p.mu.Unlock()
	for _, c := range p.conns {
		_ = c.Close()
	}
	p.conns = nil
}

func (p *commitCutProxy) serve() {
	for {
		client, err := p.listener.Accept()
		if err != nil {
			return
		}
		if p.refuse.Load() {
			_ = client.Close()
			continue
		}
		server, err := stdnet.DialTimeout("tcp", p.target, time.Second)
		if err != nil {
			_ = client.Close()
			continue
		}
		p.mu.Lock()
		p.conns = append(p.conns, client, server)
		p.mu.Unlock()
		var blackhole atomic.Bool
		go func() { // server to client; silenced once the COMMIT is cut
			buf := make([]byte, 32*1024)
			for {
				n, err := server.Read(buf)
				if n > 0 && !blackhole.Load() {
					if _, werr := client.Write(buf[:n]); werr != nil {
						break
					}
				}
				if err != nil {
					break
				}
			}
			_ = client.Close()
		}()
		go p.forwardFrontend(client, server, &blackhole)
	}
}

// forwardFrontend copies PostgreSQL frontend messages one at a time.
func (p *commitCutProxy) forwardFrontend(client, server stdnet.Conn, blackhole *atomic.Bool) {
	defer server.Close()
	readUntyped := func() ([]byte, error) {
		var size [4]byte
		if _, err := io.ReadFull(client, size[:]); err != nil {
			return nil, err
		}
		body := make([]byte, binary.BigEndian.Uint32(size[:])-4)
		if _, err := io.ReadFull(client, body); err != nil {
			return nil, err
		}
		return append(size[:], body...), nil
	}
	first, err := readUntyped()
	if err != nil {
		return
	}
	if _, err := server.Write(first); err != nil {
		return
	}
	if len(first) == 8 && binary.BigEndian.Uint32(first[4:]) == 80877103 { // SSLRequest
		startup, err := readUntyped()
		if err != nil {
			return
		}
		if _, err := server.Write(startup); err != nil {
			return
		}
	}
	marked := false
	for {
		var header [5]byte
		if _, err := io.ReadFull(client, header[:]); err != nil {
			return
		}
		body := make([]byte, binary.BigEndian.Uint32(header[1:])-4)
		if _, err := io.ReadFull(client, body); err != nil {
			return
		}
		if marker := p.marker.Load(); marker != nil && len(*marker) > 0 && bytes.Contains(bytes.ToLower(body), *marker) {
			marked = true
		}
		commit := header[0] == 'Q' && bytes.HasPrefix(bytes.ToLower(body), []byte("commit"))
		if !commit && header[0] == 'Q' && bytes.HasPrefix(bytes.ToLower(body), []byte("rollback")) {
			marked = false
		}
		if commit && marked && p.armed.CompareAndSwap(1, 0) {
			p.cuts.Add(1)
			mode := commitCutMode(p.mode.Load())
			blackhole.Store(true)
			if mode != cutBeforeCommitSent {
				_, _ = server.Write(append(header[:], body...))
				time.Sleep(200 * time.Millisecond) // PostgreSQL finishes the COMMIT
			}
			if mode == cutAfterAndIsolateDB {
				// Refuse new connections first, so the store cannot reach
				// PostgreSQL to resolve the outcome during its budget.
				p.refuse.Store(true)
			}
			_ = client.Close()
			if mode == cutAfterAndIsolateDB {
				time.Sleep(50 * time.Millisecond)
				p.dropAll()
			}
			return
		}
		if _, err := server.Write(append(header[:], body...)); err != nil {
			return
		}
	}
}

func commitLossWorld(t *testing.T) (*World, *Player, *store.PostgresStore, *commitCutProxy, *pgx.Conn) {
	t.Helper()
	raw := os.Getenv("WYD_TEST_POSTGRES_URL")
	if raw == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	parsed, err := url.Parse(raw)
	if err != nil {
		t.Fatal(err)
	}
	proxy := newCommitCutProxy(t, parsed.Host)
	parsed.Host = proxy.listener.Addr().String()
	ctx := context.Background()
	st, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: parsed.String(), MaxConns: 4})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	direct, err := pgx.Connect(ctx, raw)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = direct.Close(ctx) })
	p, _ := networkedTestPlayer(1, "CutPG", 2100, 2100)
	suffix := time.Now().UTC().Format("150405.000")
	suffix = strings.ReplaceAll(suffix, ".", "")
	p.Account.Name, p.Char.Name, p.Char.UID = "cutpg"+suffix, "CutPG"+suffix[len(suffix)-5:], ""
	t.Cleanup(func() {
		_, _ = direct.Exec(ctx, `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(p.Account.Name))
	})
	if err := st.CreateAccount(p.Account); err != nil {
		t.Fatal(err)
	}
	w, err := NewWorld(st, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
		model.CharacterTemplateFile{}, model.TerrainMap{}, WithCriticalPersistence(2, 16), WithCriticalAccountContainment())
	if err != nil {
		t.Fatal(err)
	}
	w.commands = make(chan command, 8)
	w.players[p.Session], w.playersByID[p.ID] = p, p
	w.updatePlayerSpatial(p)
	claimCriticalTestPlayers(t, w, p)
	return w, p, st, proxy, direct
}

func settleCritical(t *testing.T, w *World) {
	t.Helper()
	deadline := time.Now().Add(15 * time.Second)
	for len(w.criticalWrites.queue.pending)-w.criticalWrites.queue.quarantined > 0 && time.Now().Before(deadline) {
		w.processCriticalCompletions(criticalCompletionBatchLimit)
		time.Sleep(5 * time.Millisecond)
	}
	if len(w.criticalWrites.queue.pending)-w.criticalWrites.queue.quarantined > 0 {
		t.Fatal("critical writes did not settle")
	}
}

func durableGold(t *testing.T, st *store.PostgresStore, name string) uint32 {
	t.Helper()
	loaded, err := st.LoadAccount(name)
	if err != nil || loaded == nil {
		t.Fatalf("durable load: %v", err)
	}
	return loaded.Chars[0].Gold
}

// ACK lost after PostgreSQL committed: the worker resolves the exact
// transaction at once; the player is never isolated.
func TestCommitLossAcknowledgementReconciledEndToEnd(t *testing.T) {
	w, p, st, proxy, _ := commitLossWorld(t)
	p.Char.Gold = 4321
	proxy.arm(cutAfterCommitSent, p.Account.Name)
	if err := w.queueAccountAutoSave(p, w.now()); err != nil {
		t.Fatal(err)
	}
	settleCritical(t, w)
	if proxy.cuts.Load() != 1 || p.PersistencePoisoned || p.Session.IsClosed() || len(w.criticalQuarantine) != 0 ||
		p.autoSaveRejections != 0 {
		t.Fatalf("cuts=%d poisoned=%t closed=%t quarantined=%d rejections=%d", proxy.cuts.Load(), p.PersistencePoisoned,
			p.Session.IsClosed(), len(w.criticalQuarantine), p.autoSaveRejections)
	}
	if got := durableGold(t, st, p.Account.Name); got != 4321 {
		t.Fatalf("durable gold %d after a reconciled commit", got)
	}
}

// The COMMIT never reached PostgreSQL: the transaction aborted with its
// connection, the autosave is a certified rejection and the player stays.
func TestCommitLossBeforeCommitIsRejectionEndToEnd(t *testing.T) {
	w, p, st, proxy, _ := commitLossWorld(t)
	before := durableGold(t, st, p.Account.Name)
	p.Char.Gold = before + 77
	proxy.arm(cutBeforeCommitSent, p.Account.Name)
	if err := w.queueAccountAutoSave(p, w.now()); err != nil {
		t.Fatal(err)
	}
	settleCritical(t, w)
	if proxy.cuts.Load() != 1 || p.PersistencePoisoned || p.Session.IsClosed() || p.autoSaveRejections != 1 ||
		len(w.criticalQuarantine) != 0 {
		t.Fatalf("cuts=%d poisoned=%t closed=%t rejections=%d", proxy.cuts.Load(), p.PersistencePoisoned,
			p.Session.IsClosed(), p.autoSaveRejections)
	}
	if got := durableGold(t, st, p.Account.Name); got != before {
		t.Fatalf("durable gold %d changed after an unsent COMMIT", got)
	}
	// The next autosave writes the retained live state.
	if err := w.queueAccountAutoSave(p, w.now()); err != nil {
		t.Fatal(err)
	}
	settleCritical(t, w)
	if got := durableGold(t, st, p.Account.Name); got != before+77 || p.autoSaveRejections != 0 {
		t.Fatalf("retry wrote %d (rejections %d)", got, p.autoSaveRejections)
	}
}

// ACK lost and PostgreSQL unreachable for the whole reconciliation budget:
// the account is quarantined with its transaction, released once the exact
// status is final, and signs in again to the committed state.
func TestCommitLossUnresolvedQuarantineReleasedEndToEnd(t *testing.T) {
	w, p, st, proxy, _ := commitLossWorld(t)
	p.Char.Gold = 9876
	proxy.arm(cutAfterAndIsolateDB, p.Account.Name)
	if err := w.queueAccountAutoSave(p, w.now()); err != nil {
		t.Fatal(err)
	}
	settleCritical(t, w)
	if !p.PersistencePoisoned || !p.Session.IsClosed() || len(w.criticalQuarantineXID) != 1 ||
		!w.criticalAccountReserved(p.Account.Name) {
		t.Fatalf("unresolved COMMIT was not quarantined: poisoned=%t xid=%v rejections=%d cuts=%d reconciled=%s",
			p.PersistencePoisoned, w.criticalQuarantineXID, p.autoSaveRejections,
			proxy.cuts.Load(), expvarString("postgres_commit_reconciled_total"))
	}
	proxy.refuse.Store(false)
	w.onDisconnect(p.Session)
	w.quarantineCheckAt = time.Time{}
	w.checkQuarantinedCommits(w.now())
	select {
	case cmd := <-w.commands:
		w.handle(cmd)
	case <-time.After(10 * time.Second):
		t.Fatal("quarantine status was not delivered")
	}
	if len(w.criticalQuarantine) != 0 || w.criticalAccountReserved(p.Account.Name) {
		t.Fatalf("final status did not release the account: %v", w.quarantineLastStatus)
	}
	if got := durableGold(t, st, p.Account.Name); got != 9876 {
		t.Fatalf("durable gold %d after a committed-but-unacknowledged autosave", got)
	}
}

func expvarString(name string) string {
	if v := expvar.Get(name); v != nil {
		return v.String()
	}
	return ""
}
