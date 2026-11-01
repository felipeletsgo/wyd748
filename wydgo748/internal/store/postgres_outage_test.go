package store

import (
	"context"
	"io"
	stdnet "net"
	"net/url"
	"os"
	"sync"
	"testing"
	"time"

	"wydgo/internal/model"
)

// faultProxy forwards TCP to PostgreSQL and can drop every connection, which
// models a database restart without touching the real service.
type faultProxy struct {
	listener stdnet.Listener
	target   string
	mu       sync.Mutex
	conns    []stdnet.Conn
	refuse   bool
}

func newFaultProxy(t *testing.T, target string) *faultProxy {
	t.Helper()
	ln, err := stdnet.Listen("tcp4", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	p := &faultProxy{listener: ln, target: target}
	t.Cleanup(func() { _ = ln.Close(); p.cut() })
	go p.serve()
	return p
}

func (p *faultProxy) serve() {
	for {
		client, err := p.listener.Accept()
		if err != nil {
			return
		}
		p.mu.Lock()
		refuse := p.refuse
		p.mu.Unlock()
		if refuse {
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
		go func() { _, _ = io.Copy(server, client); _ = server.Close() }()
		go func() { _, _ = io.Copy(client, server); _ = client.Close() }()
	}
}

// cut drops every proxied connection, like a server restart.
func (p *faultProxy) cut() {
	p.mu.Lock()
	defer p.mu.Unlock()
	for _, c := range p.conns {
		_ = c.Close()
	}
	p.conns = nil
}

func (p *faultProxy) setRefuse(refuse bool) {
	p.mu.Lock()
	p.refuse = refuse
	p.mu.Unlock()
}

// W5/D10: the game store survives a database restart. During the outage
// writes are certified rejections and reads fail fast; afterwards the pool
// reconnects without a process restart.
func TestPostgresStoreRecoversAfterDatabaseRestart(t *testing.T) {
	raw := os.Getenv("WYD_TEST_POSTGRES_URL")
	if raw == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	parsed, err := url.Parse(raw)
	if err != nil {
		t.Fatal(err)
	}
	proxy := newFaultProxy(t, parsed.Host)
	parsed.Host = proxy.listener.Addr().String()
	st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: parsed.String(), MaxConns: 2,
		OperationTimeout: 500 * time.Millisecond, ReadTimeout: 500 * time.Millisecond})
	if err != nil {
		t.Fatal(err)
	}
	defer st.Close()
	if _, err := st.AccountNameExists("outage-probe"); err != nil {
		t.Fatalf("baseline read: %v", err)
	}

	proxy.setRefuse(true)
	proxy.cut()
	if _, err := st.AccountNameExists("outage-probe"); err == nil {
		t.Fatal("a read succeeded while the database was down")
	}
	if err := st.SaveCriticalAccounts(postgresTestAccount("outageacct", "OutageChar", model.Item{Index: 4011})); err == nil || !IsWriteRejected(err) {
		t.Fatalf("a write during the outage was not a certified rejection: %v", err)
	}

	proxy.setRefuse(false)
	deadline := time.Now().Add(5 * time.Second)
	for {
		_, err := st.AccountNameExists("outage-probe")
		if err == nil {
			break
		}
		if time.Now().After(deadline) {
			t.Fatalf("pool did not recover after the database returned: %v", err)
		}
		time.Sleep(100 * time.Millisecond)
	}
}
