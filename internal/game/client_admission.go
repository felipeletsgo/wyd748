package game

import (
	"fmt"
	"net/netip"
	"strconv"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

type compiledNetworkAdmission struct {
	v4 map[netip.Prefix]model.NetworkAdmissionRule
	v6 map[netip.Prefix]model.NetworkAdmissionRule
}

func compileNetworkAdmission(file model.NetworkAdmissionFile) compiledNetworkAdmission {
	policy := compiledNetworkAdmission{
		v4: make(map[netip.Prefix]model.NetworkAdmissionRule),
		v6: make(map[netip.Prefix]model.NetworkAdmissionRule),
	}
	for _, rule := range file.Rules {
		if rule.CIDR.Addr().Is4() {
			policy.v4[rule.CIDR] = rule
		} else {
			policy.v6[rule.CIDR] = rule
		}
	}
	return policy
}

func (p compiledNetworkAdmission) match(addr netip.Addr) (model.NetworkAdmissionRule, bool) {
	addr = addr.Unmap()
	rules, bits := p.v6, 128
	if addr.Is4() {
		rules, bits = p.v4, 32
	}
	for prefixBits := bits; prefixBits >= 0; prefixBits-- {
		prefix := netip.PrefixFrom(addr, prefixBits).Masked()
		if rule, ok := rules[prefix]; ok {
			return rule, true
		}
	}
	return model.NetworkAdmissionRule{}, false
}

func WithNetworkAdmission(file model.NetworkAdmissionFile) WorldOption {
	return func(w *World) {
		if err := file.Validate(); err != nil {
			w.networkAdmissionErr = fmt.Errorf("politica de admissao de rede: %w", err)
			return
		}
		w.networkAdmission = compileNetworkAdmission(file)
	}
}

func sessionRemoteIP(s *net.Session) (netip.Addr, bool) {
	if s == nil {
		return netip.Addr{}, false
	}
	addr, err := netip.ParseAddr(strings.TrimSpace(s.RemoteIP()))
	if err != nil {
		return netip.Addr{}, false
	}
	return addr.Unmap(), true
}

func (w *World) networkAdmissionRule(s *net.Session) (model.NetworkAdmissionRule, bool) {
	addr, ok := sessionRemoteIP(s)
	if !ok {
		return model.NetworkAdmissionRule{}, false
	}
	return w.networkAdmission.match(addr)
}

// authenticatedClientIPKey canonicaliza a origem observada pelo socket. O
// servidor nao usa IP/MAC enviados no pacote: ambos seriam triviais de forjar.
// Sessao sem socket existe apenas em testes e recebe uma chave isolada.
func authenticatedClientIPKey(s *net.Session) string {
	if s == nil {
		return ""
	}
	ip := strings.TrimSpace(s.RemoteIP())
	if parsed, err := netip.ParseAddr(ip); err == nil {
		parsed = parsed.Unmap()
		// Uma unica maquina/rede IPv6 pode criar muitos enderecos temporarios
		// dentro do mesmo /64 e burlar um limite por endereco literal. Agrupar o
		// prefixo residencial preserva a mesma semantica pratica do IP publico
		// IPv4 compartilhado.
		if parsed.Is6() {
			return netip.PrefixFrom(parsed, 64).Masked().String()
		}
		return parsed.String()
	}
	if ip == "" {
		return "test-session:" + strconv.FormatInt(s.ID, 10)
	}
	return strings.ToLower(ip)
}

func (w *World) authenticatedClientLimit() int {
	limit := w.operational.MaxAuthenticatedClientsPerIP
	if limit <= 0 {
		limit = DefaultOperationalConfig().MaxAuthenticatedClientsPerIP
	}
	return limit
}

func (w *World) authenticatedClientLimitFor(s *net.Session) int {
	limit := w.authenticatedClientLimit()
	if rule, ok := w.networkAdmissionRule(s); ok &&
		rule.Action == model.NetworkAdmissionLimit && rule.MaxClients < limit {
		limit = rule.MaxClients
	}
	return limit
}

func (w *World) networkDenied(s *net.Session) (model.NetworkAdmissionRule, bool) {
	rule, ok := w.networkAdmissionRule(s)
	return rule, ok && rule.Action == model.NetworkAdmissionDeny
}

// claimAuthenticatedClientSlot roda somente no ator World, depois da senha
// correta. Diferentemente do limite de sockets do listener, conta somente
// janelas autenticadas e nao permite que conexoes pre-auth ocupem as quatro
// vagas de uma residencia.
func (w *World) claimAuthenticatedClientSlot(s *net.Session) bool {
	if s == nil || s.IsClosed() {
		return false
	}
	if _, denied := w.networkDenied(s); denied {
		return false
	}
	if w.authClientsByIP == nil {
		w.authClientsByIP = make(map[string]map[*net.Session]struct{})
	}
	key := authenticatedClientIPKey(s)
	bucket := w.authClientsByIP[key]
	if bucket == nil {
		bucket = make(map[*net.Session]struct{})
		w.authClientsByIP[key] = bucket
	}
	for existing := range bucket {
		if existing == nil || existing.IsClosed() {
			delete(bucket, existing)
		}
	}
	if _, exists := bucket[s]; exists {
		return true
	}
	if len(bucket) >= w.authenticatedClientLimitFor(s) {
		return false
	}
	bucket[s] = struct{}{}
	return true
}

func (w *World) releaseAuthenticatedClientSlot(s *net.Session) {
	if s == nil || w.authClientsByIP == nil {
		return
	}
	key := authenticatedClientIPKey(s)
	bucket := w.authClientsByIP[key]
	delete(bucket, s)
	if len(bucket) == 0 {
		delete(w.authClientsByIP, key)
	}
}
