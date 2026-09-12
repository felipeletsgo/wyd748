package game

import (
	"fmt"
	"net/netip"

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
	return net.ParseOriginIP(s.RemoteIP())
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
func authenticatedClientIPKey(s *net.Session) (string, bool) {
	if s == nil {
		return "", false
	}
	return net.OriginLimitKey(s.RemoteIP())
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

func (w *World) networkRejected(s *net.Session) (string, bool) {
	if _, ok := sessionRemoteIP(s); !ok {
		return "invalid remote IP", true
	}
	rule, ok := w.networkAdmissionRule(s)
	return rule.Reason, ok && rule.Action == model.NetworkAdmissionDeny
}

func (w *World) authenticatedNetworkLimit(s *net.Session) (string, int, bool) {
	rule, ok := w.networkAdmissionRule(s)
	if !ok || rule.Action != model.NetworkAdmissionLimit {
		return "", 0, false
	}
	return rule.CIDR.String(), w.authenticatedClientLimitFor(s), true
}

func pruneClosedSessions(bucket map[*net.Session]struct{}) {
	for existing := range bucket {
		if existing == nil || existing.IsClosed() {
			delete(bucket, existing)
		}
	}
}

// claimAuthenticatedClientSlot roda somente no ator World, depois da senha
// correta. Diferentemente do limite de sockets do listener, conta somente
// janelas autenticadas e nao permite que conexoes pre-auth ocupem as quatro
// vagas de uma residencia.
func (w *World) claimAuthenticatedClientSlot(s *net.Session) bool {
	if s == nil || s.IsClosed() {
		return false
	}
	if _, rejected := w.networkRejected(s); rejected {
		return false
	}
	if w.authClientsByIP == nil {
		w.authClientsByIP = make(map[string]map[*net.Session]struct{})
	}
	if w.authClientsByNetwork == nil {
		w.authClientsByNetwork = make(map[string]map[*net.Session]struct{})
	}
	ipKey, validOrigin := authenticatedClientIPKey(s)
	if !validOrigin {
		return false
	}
	ipBucket := w.authClientsByIP[ipKey]
	pruneClosedSessions(ipBucket)
	_, alreadyInIPBucket := ipBucket[s]
	if !alreadyInIPBucket && len(ipBucket) >= w.authenticatedClientLimit() {
		return false
	}

	networkKey, networkLimit, hasNetworkLimit := w.authenticatedNetworkLimit(s)
	var networkBucket map[*net.Session]struct{}
	if hasNetworkLimit {
		networkBucket = w.authClientsByNetwork[networkKey]
		pruneClosedSessions(networkBucket)
		_, alreadyInNetworkBucket := networkBucket[s]
		if !alreadyInNetworkBucket && len(networkBucket) >= networkLimit {
			return false
		}
	}

	if ipBucket == nil {
		ipBucket = make(map[*net.Session]struct{})
		w.authClientsByIP[ipKey] = ipBucket
	}
	ipBucket[s] = struct{}{}
	if hasNetworkLimit {
		if networkBucket == nil {
			networkBucket = make(map[*net.Session]struct{})
			w.authClientsByNetwork[networkKey] = networkBucket
		}
		networkBucket[s] = struct{}{}
	}
	return true
}

func (w *World) releaseAuthenticatedClientSlot(s *net.Session) {
	if s == nil {
		return
	}
	if ipKey, ok := authenticatedClientIPKey(s); ok {
		bucket := w.authClientsByIP[ipKey]
		delete(bucket, s)
		if len(bucket) == 0 {
			delete(w.authClientsByIP, ipKey)
		}
	}
	if networkKey, _, ok := w.authenticatedNetworkLimit(s); ok {
		bucket := w.authClientsByNetwork[networkKey]
		delete(bucket, s)
		if len(bucket) == 0 {
			delete(w.authClientsByNetwork, networkKey)
		}
	}
}
