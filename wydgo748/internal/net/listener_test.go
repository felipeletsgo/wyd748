package net

import (
	"io"
	stdnet "net"
	"sync"
	"sync/atomic"
	"testing"
	"time"
)

func TestConnectionLimiterEnforcesGlobalAndPerIPLimits(t *testing.T) {
	limiter := newConnectionLimiter(3, 2)
	if !limiter.acquire("10.0.0.1") || !limiter.acquire("10.0.0.1") {
		t.Fatal("conexoes dentro do limite por IP foram recusadas")
	}
	if limiter.acquire("10.0.0.1") {
		t.Fatal("terceira conexao do mesmo IP deveria ser recusada")
	}
	if !limiter.acquire("10.0.0.2") {
		t.Fatal("terceira conexao global deveria ser aceita")
	}
	if limiter.acquire("10.0.0.3") {
		t.Fatal("limite global nao foi aplicado")
	}
	limiter.release("10.0.0.1")
	if !limiter.acquire("10.0.0.3") {
		t.Fatal("slot liberado nao voltou a ficar disponivel")
	}
}

func TestRemoteIPStripsPort(t *testing.T) {
	if got := remoteIP(testAddr("[2001:db8::1]:8281")); got != "2001:db8::1" {
		t.Fatalf("IPv6 remoto=%q", got)
	}
}

func TestSessionRemoteIPHandlesNilAndFallbackAddresses(t *testing.T) {
	if got := (*Session)(nil).RemoteIP(); got != "" {
		t.Fatalf("nil session remote IP=%q", got)
	}
	if got := (&Session{}).RemoteIP(); got != "" {
		t.Fatalf("session without a connection remote IP=%q", got)
	}
	valid := &Session{conn: &scriptedConn{remote: testAddr("[2001:db8::1]:8281")}}
	if got := valid.RemoteIP(); got != "2001:db8::1" {
		t.Fatalf("session IPv6 remote IP=%q", got)
	}
	invalid := &Session{conn: &scriptedConn{remote: testAddr("unparsed-origin")}}
	if got := invalid.RemoteIP(); got != "unparsed-origin" {
		t.Fatalf("invalid remote address fallback=%q", got)
	}
}

func TestOriginLimitKeyCanonicalizesIPv4AndGroupsIPv6By64(t *testing.T) {
	tests := []struct {
		ip   string
		want string
		ok   bool
	}{
		{ip: "192.0.2.44", want: "192.0.2.44", ok: true},
		{ip: "::ffff:192.0.2.44", want: "192.0.2.44", ok: true},
		{ip: "2001:db8:1:2::1", want: "2001:db8:1:2::/64", ok: true},
		{ip: "2001:db8:1:2::abcd", want: "2001:db8:1:2::/64", ok: true},
		{ip: "not-an-ip", ok: false},
	}
	for _, test := range tests {
		got, ok := OriginLimitKey(test.ip)
		if ok != test.ok || got != test.want {
			t.Errorf("OriginLimitKey(%q)=(%q,%t), quer (%q,%t)",
				test.ip, got, ok, test.want, test.ok)
		}
	}
}

func TestConnectionLimiterCannotBeBypassedWithIPv6Aliases(t *testing.T) {
	limiter := newConnectionLimiter(100, 1)
	first, ok := OriginLimitKey("2001:db8:1:2::1")
	if !ok {
		t.Fatal("fixture IPv6 invalida")
	}
	second, ok := OriginLimitKey("2001:db8:1:2::2")
	if !ok || second != first {
		t.Fatal("enderecos do mesmo /64 nao compartilharam a chave")
	}
	if !limiter.acquire(first) || limiter.acquire(second) {
		t.Fatal("alias IPv6 contornou max_connections_per_ip")
	}
	other, ok := OriginLimitKey("2001:db8:1:3::1")
	if !ok || !limiter.acquire(other) {
		t.Fatal("outro /64 foi bloqueado indevidamente")
	}
}

func TestTestSessionExposesConfiguredRemoteIP(t *testing.T) {
	s := NewTestSessionWithRemoteIP(1, 1, "192.0.2.44")
	if got := s.RemoteIP(); got != "192.0.2.44" {
		t.Fatalf("RemoteIP=%q", got)
	}
}

type scriptedListener struct {
	mu    sync.Mutex
	conns []stdnet.Conn
}

func (l *scriptedListener) Accept() (stdnet.Conn, error) {
	l.mu.Lock()
	defer l.mu.Unlock()
	if len(l.conns) == 0 {
		return nil, stdnet.ErrClosed
	}
	conn := l.conns[0]
	l.conns = l.conns[1:]
	return conn, nil
}
func (*scriptedListener) Close() error      { return nil }
func (*scriptedListener) Addr() stdnet.Addr { return testAddr("127.0.0.1:8281") }

type scriptedConn struct {
	remote stdnet.Addr
	closed atomic.Bool
}

func (c *scriptedConn) Read([]byte) (int, error)       { return 0, io.EOF }
func (c *scriptedConn) Write(data []byte) (int, error) { return len(data), nil }
func (c *scriptedConn) Close() error                   { c.closed.Store(true); return nil }
func (*scriptedConn) LocalAddr() stdnet.Addr           { return testAddr("127.0.0.1:8281") }
func (c *scriptedConn) RemoteAddr() stdnet.Addr        { return c.remote }
func (*scriptedConn) SetDeadline(time.Time) error      { return nil }
func (*scriptedConn) SetReadDeadline(time.Time) error  { return nil }
func (*scriptedConn) SetWriteDeadline(time.Time) error { return nil }

func TestServeListenerWithConfigLimitsOriginsAndClosesRejectedSockets(t *testing.T) {
	invalid := &scriptedConn{remote: testAddr("not-a-socket")}
	if err := ServeListenerWithConfig(&scriptedListener{conns: []stdnet.Conn{invalid}}, ListenerConfig{}, func(*Session) {
		t.Fatal("invalid origin reached the session callback")
	}); err != nil || !invalid.closed.Load() {
		t.Fatalf("invalid origin close=%v serve error=%v", invalid.closed.Load(), err)
	}

	first := &scriptedConn{remote: testAddr("192.0.2.44:8281")}
	second := &scriptedConn{remote: testAddr("192.0.2.44:8282")}
	listener := &scriptedListener{conns: []stdnet.Conn{first, second}}
	sessions := make(chan *Session, 2)
	release := make(chan struct{})
	callbackDone := make(chan struct{})
	serveDone := make(chan error, 1)
	go func() {
		serveDone <- ServeListenerWithConfig(listener, ListenerConfig{MaxConnectionsPerIP: 1}, func(s *Session) {
			sessions <- s
			<-release
			s.Close()
			close(callbackDone)
		})
	}()
	var session *Session
	select {
	case session = <-sessions:
	case <-time.After(time.Second):
		t.Fatal("accepted connection did not reach its session callback")
	}
	if session.RemoteIP() != "192.0.2.44" || cap(session.out) != defaultOutputQueueSize {
		t.Fatalf("session remote IP=%q output queue=%d", session.RemoteIP(), cap(session.out))
	}
	select {
	case err := <-serveDone:
		if err != nil {
			t.Fatalf("ServeListenerWithConfig() error = %v", err)
		}
	case <-time.After(time.Second):
		t.Fatal("listener did not stop on closed accept source")
	}
	if !second.closed.Load() || len(sessions) != 0 {
		t.Fatal("per-origin limit did not close the second socket before creating a session")
	}
	close(release)
	select {
	case <-callbackDone:
	case <-time.After(time.Second):
		t.Fatal("session callback did not release its connection")
	}
}

func TestListenWithConfigReturnsBindFailures(t *testing.T) {
	if err := ListenWithConfig("not-a-valid-listener-address", ListenerConfig{}, nil); err == nil {
		t.Fatal("ListenWithConfig() error = nil, want address bind error")
	}
}

type testAddr string

func (a testAddr) Network() string { return "tcp" }
func (a testAddr) String() string  { return string(a) }
