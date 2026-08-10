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

type testAddr string

func (a testAddr) Network() string { return "tcp" }
func (a testAddr) String() string  { return string(a) }
