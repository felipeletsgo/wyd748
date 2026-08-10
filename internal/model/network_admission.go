package model

import (
	"fmt"
	"net/netip"
	"strings"
)

const (
	NetworkAdmissionVersion  = 1
	MaxNetworkAdmissionRules = 100_000
	MaxNetworkRuleReasonSize = 120
	MaxNetworkRuleSourceSize = 64
)

type NetworkAdmissionAction string

const (
	NetworkAdmissionAllow NetworkAdmissionAction = "allow"
	NetworkAdmissionDeny  NetworkAdmissionAction = "deny"
	NetworkAdmissionLimit NetworkAdmissionAction = "limit"
)

// NetworkAdmissionRule classifica uma faixa observada pelo socket. Regras mais
// especificas vencem regras amplas, permitindo allowlists dentro de um ASN ou
// datacenter bloqueado.
type NetworkAdmissionRule struct {
	CIDR       netip.Prefix           `json:"cidr"`
	Action     NetworkAdmissionAction `json:"action"`
	MaxClients int                    `json:"maxClients,omitempty"`
	Source     string                 `json:"source,omitempty"`
	Reason     string                 `json:"reason,omitempty"`
}

type NetworkAdmissionFile struct {
	Version int                    `json:"version"`
	Rules   []NetworkAdmissionRule `json:"rules"`
}

func (f NetworkAdmissionFile) Validate() error {
	if f.Version != NetworkAdmissionVersion {
		return fmt.Errorf("version=%d, esperada %d", f.Version, NetworkAdmissionVersion)
	}
	if len(f.Rules) > MaxNetworkAdmissionRules {
		return fmt.Errorf("rules possui %d entradas, maximo %d", len(f.Rules), MaxNetworkAdmissionRules)
	}
	seen := make(map[netip.Prefix]struct{}, len(f.Rules))
	for i, rule := range f.Rules {
		if !rule.CIDR.IsValid() || rule.CIDR != rule.CIDR.Masked() {
			return fmt.Errorf("rules[%d].cidr deve ser prefixo canonico", i)
		}
		if rule.CIDR.Addr().Is4In6() {
			return fmt.Errorf("rules[%d].cidr deve usar notacao IPv4, nao IPv4-mapped IPv6", i)
		}
		if _, duplicate := seen[rule.CIDR]; duplicate {
			return fmt.Errorf("rules[%d].cidr %s duplicado", i, rule.CIDR)
		}
		seen[rule.CIDR] = struct{}{}
		switch rule.Action {
		case NetworkAdmissionAllow, NetworkAdmissionDeny:
			if rule.MaxClients != 0 {
				return fmt.Errorf("rules[%d].maxClients somente e valido para action=limit", i)
			}
		case NetworkAdmissionLimit:
			if rule.MaxClients < 1 || rule.MaxClients > 10_000 {
				return fmt.Errorf("rules[%d].maxClients deve ficar entre 1 e 10000", i)
			}
		default:
			return fmt.Errorf("rules[%d].action %q invalida", i, rule.Action)
		}
		if len(rule.Reason) > MaxNetworkRuleReasonSize || strings.ContainsAny(rule.Reason, "\r\n") {
			return fmt.Errorf("rules[%d].reason invalido", i)
		}
		if len(rule.Source) > MaxNetworkRuleSourceSize || !validNetworkRuleSource(rule.Source) {
			return fmt.Errorf("rules[%d].source invalido", i)
		}
	}
	return nil
}

func validNetworkRuleSource(source string) bool {
	if source != strings.ToLower(source) {
		return false
	}
	for _, char := range source {
		if (char >= 'a' && char <= 'z') || (char >= '0' && char <= '9') ||
			char == '-' || char == '_' || char == '.' {
			continue
		}
		return false
	}
	return true
}
