package game

import (
	"net/netip"
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestAuthenticatedClientLimitAllowsFourPerIP(t *testing.T) {
	w := &World{operational: OperationalConfig{MaxAuthenticatedClientsPerIP: 4}}
	var sessions []*gameNet.Session
	for id := int64(1); id <= 4; id++ {
		s := gameNet.NewTestSessionWithRemoteIP(id, 1, "198.51.100.20")
		sessions = append(sessions, s)
		if !w.claimAuthenticatedClientSlot(s) {
			t.Fatalf("janela %d dentro do limite foi recusada", id)
		}
	}
	fifth := gameNet.NewTestSessionWithRemoteIP(5, 1, "198.51.100.20")
	if w.claimAuthenticatedClientSlot(fifth) {
		t.Fatal("quinta janela do mesmo IP foi aceita")
	}
	otherIP := gameNet.NewTestSessionWithRemoteIP(6, 1, "198.51.100.21")
	if !w.claimAuthenticatedClientSlot(otherIP) {
		t.Fatal("limite de um IP bloqueou outro IP")
	}

	w.releaseAuthenticatedClientSlot(sessions[0])
	if !w.claimAuthenticatedClientSlot(fifth) {
		t.Fatal("vaga liberada no logout nao voltou a ficar disponivel")
	}
}

func TestAuthenticatedClientLimitReclaimsClosedSession(t *testing.T) {
	w := &World{operational: OperationalConfig{MaxAuthenticatedClientsPerIP: 1}}
	closed := gameNet.NewTestSessionWithRemoteIP(1, 1, "203.0.113.9")
	if !w.claimAuthenticatedClientSlot(closed) {
		t.Fatal("primeira janela foi recusada")
	}
	closed.Close() // socket caiu antes de o comando de disconnect chegar ao World
	replacement := gameNet.NewTestSessionWithRemoteIP(2, 1, "203.0.113.9")
	if !w.claimAuthenticatedClientSlot(replacement) {
		t.Fatal("socket encerrado manteve a vaga de janela presa")
	}
}

func TestDisconnectReleasesAuthenticatedClientSlot(t *testing.T) {
	w := &World{
		operational: OperationalConfig{MaxAuthenticatedClientsPerIP: 1},
		players:     make(map[*gameNet.Session]*Player),
		authPending: make(map[*gameNet.Session]bool),
		security:    make(map[*gameNet.Session]*securityState),
	}
	first := gameNet.NewTestSessionWithRemoteIP(1, 1, "203.0.113.20")
	if !w.claimAuthenticatedClientSlot(first) {
		t.Fatal("primeira janela foi recusada")
	}
	w.onDisconnect(first)
	second := gameNet.NewTestSessionWithRemoteIP(2, 1, "203.0.113.20")
	if !w.claimAuthenticatedClientSlot(second) {
		t.Fatal("disconnect nao liberou a vaga autenticada")
	}
}

func TestAuthenticatedClientIPCanonicalizesIPv4MappedIPv6(t *testing.T) {
	w := &World{operational: OperationalConfig{MaxAuthenticatedClientsPerIP: 1}}
	v4 := gameNet.NewTestSessionWithRemoteIP(1, 1, "192.0.2.80")
	mapped := gameNet.NewTestSessionWithRemoteIP(2, 1, "::ffff:192.0.2.80")
	if !w.claimAuthenticatedClientSlot(v4) {
		t.Fatal("primeira origem foi recusada")
	}
	if w.claimAuthenticatedClientSlot(mapped) {
		t.Fatal("IPv4-mapped IPv6 burlou o limite do mesmo endereco")
	}
}

func TestAuthenticatedClientLimitGroupsIPv6Prefix(t *testing.T) {
	w := &World{operational: OperationalConfig{MaxAuthenticatedClientsPerIP: 1}}
	first := gameNet.NewTestSessionWithRemoteIP(1, 1, "2001:db8:10:20::1")
	sameNetwork := gameNet.NewTestSessionWithRemoteIP(2, 1, "2001:db8:10:20::abcd")
	otherNetwork := gameNet.NewTestSessionWithRemoteIP(3, 1, "2001:db8:10:21::1")
	if !w.claimAuthenticatedClientSlot(first) {
		t.Fatal("primeiro IPv6 foi recusado")
	}
	if w.claimAuthenticatedClientSlot(sameNetwork) {
		t.Fatal("outro endereco do mesmo /64 IPv6 burlou o limite")
	}
	if !w.claimAuthenticatedClientSlot(otherNetwork) {
		t.Fatal("outro prefixo IPv6 foi bloqueado")
	}
}

func TestNetworkAdmissionUsesMostSpecificRule(t *testing.T) {
	file := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion, Rules: []model.NetworkAdmissionRule{
		{CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: model.NetworkAdmissionDeny, Reason: "hosting"},
		{CIDR: netip.MustParsePrefix("203.0.113.8/32"), Action: model.NetworkAdmissionAllow, Reason: "whitelist"},
	}}
	w := &World{operational: OperationalConfig{MaxAuthenticatedClientsPerIP: 4}}
	WithNetworkAdmission(file)(w)
	blocked := gameNet.NewTestSessionWithRemoteIP(1, 1, "203.0.113.7")
	allowed := gameNet.NewTestSessionWithRemoteIP(2, 1, "203.0.113.8")
	if w.claimAuthenticatedClientSlot(blocked) {
		t.Fatal("faixa deny admitiu client")
	}
	if !w.claimAuthenticatedClientSlot(allowed) {
		t.Fatal("allow especifico nao sobrescreveu deny amplo")
	}
}

func TestNetworkAdmissionCanReduceClientLimit(t *testing.T) {
	file := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion, Rules: []model.NetworkAdmissionRule{
		{CIDR: netip.MustParsePrefix("198.51.100.0/24"), Action: model.NetworkAdmissionLimit, MaxClients: 1},
	}}
	w := &World{operational: OperationalConfig{MaxAuthenticatedClientsPerIP: 4}}
	WithNetworkAdmission(file)(w)
	first := gameNet.NewTestSessionWithRemoteIP(1, 1, "198.51.100.10")
	second := gameNet.NewTestSessionWithRemoteIP(2, 1, "198.51.100.10")
	if !w.claimAuthenticatedClientSlot(first) || w.claimAuthenticatedClientSlot(second) {
		t.Fatal("limite reduzido da faixa nao foi aplicado")
	}
}

func TestNetworkAdmissionCannotRaiseGlobalClientLimit(t *testing.T) {
	file := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion, Rules: []model.NetworkAdmissionRule{
		{CIDR: netip.MustParsePrefix("198.51.100.0/24"), Action: model.NetworkAdmissionLimit, MaxClients: 10},
	}}
	w := &World{operational: OperationalConfig{MaxAuthenticatedClientsPerIP: 4}}
	WithNetworkAdmission(file)(w)
	for id := int64(1); id <= 4; id++ {
		if !w.claimAuthenticatedClientSlot(gameNet.NewTestSessionWithRemoteIP(id, 1, "198.51.100.10")) {
			t.Fatalf("janela %d dentro do teto global foi recusada", id)
		}
	}
	if w.claimAuthenticatedClientSlot(gameNet.NewTestSessionWithRemoteIP(5, 1, "198.51.100.10")) {
		t.Fatal("regra de rede elevou o teto global de quatro clients")
	}
}

func TestDeniedNetworkDoesNotStartAuthentication(t *testing.T) {
	session := gameNet.NewTestSessionWithRemoteIP(1, 4, "203.0.113.10")
	w := &World{
		players:     make(map[*gameNet.Session]*Player),
		authPending: make(map[*gameNet.Session]bool),
		operational: DefaultOperationalConfig(),
	}
	WithNetworkAdmission(model.NetworkAdmissionFile{
		Version: model.NetworkAdmissionVersion,
		Rules: []model.NetworkAdmissionRule{{
			CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: model.NetworkAdmissionDeny,
			Reason: "hosting",
		}},
	})(w)
	pkt := make([]byte, 116)
	copy(pkt[12:28], "account")
	w.onLogin(session, pkt)
	if w.authPending[session] || session.QueuedPacketsForTest() == 0 {
		t.Fatal("rede negada iniciou PBKDF2 ou nao recebeu aviso")
	}
}
