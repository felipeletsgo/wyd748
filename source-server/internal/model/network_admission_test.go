package model

import (
	"net/netip"
	"testing"
)

func TestNetworkAdmissionValidate(t *testing.T) {
	valid := NetworkAdmissionFile{Version: NetworkAdmissionVersion, Rules: []NetworkAdmissionRule{
		{CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: NetworkAdmissionDeny, Reason: "hosting"},
		{CIDR: netip.MustParsePrefix("2001:db8::/32"), Action: NetworkAdmissionLimit, MaxClients: 1},
	}}
	if err := valid.Validate(); err != nil {
		t.Fatal(err)
	}
	invalid := []NetworkAdmissionFile{
		{Version: 2},
		{Version: 1, Rules: []NetworkAdmissionRule{{CIDR: netip.MustParsePrefix("203.0.113.1/24"), Action: NetworkAdmissionDeny}}},
		{Version: 1, Rules: []NetworkAdmissionRule{{CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: "block"}}},
		{Version: 1, Rules: []NetworkAdmissionRule{{CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: NetworkAdmissionLimit}}},
		{Version: 1, Rules: []NetworkAdmissionRule{
			{CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: NetworkAdmissionDeny},
			{CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: NetworkAdmissionAllow},
		}},
		{Version: 1, Rules: []NetworkAdmissionRule{{
			CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: NetworkAdmissionDeny,
			Source: "invalid source",
		}}},
	}
	for i, file := range invalid {
		if err := file.Validate(); err == nil {
			t.Errorf("caso invalido %d foi aceito", i)
		}
	}
}
