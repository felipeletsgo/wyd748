package net

import "testing"

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

type testAddr string

func (a testAddr) Network() string { return "tcp" }
func (a testAddr) String() string  { return string(a) }
