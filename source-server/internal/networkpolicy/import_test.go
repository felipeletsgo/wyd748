package networkpolicy

import (
	"net/netip"
	"os"
	"path/filepath"
	"reflect"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func writeInput(t *testing.T, name, contents string) string {
	t.Helper()
	path := filepath.Join(t.TempDir(), name)
	if err := os.WriteFile(path, []byte(contents), 0o600); err != nil {
		t.Fatal(err)
	}
	return path
}

func TestReadPrefixesTextNormalizesAndDeduplicates(t *testing.T) {
	path := writeInput(t, "ranges.txt", "# provider\n203.0.113.7/24\n203.0.113.0/24 # duplicate\n2001:db8::/32\n")
	prefixes, err := ReadPrefixes(path)
	if err != nil {
		t.Fatal(err)
	}
	got := make([]string, len(prefixes))
	for i, prefix := range prefixes {
		got[i] = prefix.String()
	}
	want := []string{"203.0.113.0/24", "2001:db8::/32"}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("prefixos=%v, quer %v", got, want)
	}
}

func TestReadPrefixesKnownJSONFeedFields(t *testing.T) {
	path := writeInput(t, "ranges.json", `{
        "syncToken":"not a prefix",
        "prefixes":[
            {"ip_prefix":"192.0.2.0/24","service":"TEST"},
            {"ipv6Prefix":"2001:db8:1::/48"}
        ],
        "properties":{"addressPrefixes":["198.51.100.0/24"]}
    }`)
	prefixes, err := ReadPrefixes(path)
	if err != nil {
		t.Fatal(err)
	}
	got := make([]string, len(prefixes))
	for i, prefix := range prefixes {
		got[i] = prefix.String()
	}
	want := []string{"192.0.2.0/24", "198.51.100.0/24", "2001:db8:1::/48"}
	if !reflect.DeepEqual(got, want) {
		t.Fatalf("prefixos=%v, quer %v", got, want)
	}
}

func TestReadPrefixesRejectsInvalidRecognizedFieldAndTrailingJSON(t *testing.T) {
	for _, contents := range []string{
		`{"prefixes":[{"ip_prefix":"invalid"}]}`,
		`{"prefixes":[]} {}`,
		"not-a-prefix\n",
	} {
		if _, err := ReadPrefixes(writeInput(t, "invalid", contents)); err == nil {
			t.Fatalf("entrada invalida aceita: %s", contents)
		}
	}
}

func TestMergeReplacesOnlyItsSourceAndRejectsConflict(t *testing.T) {
	basePath := writeInput(t, "base.txt", "203.0.113.0/24\n")
	newPath := writeInput(t, "new.txt", "198.51.100.0/24\n")
	base := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion, Rules: []model.NetworkAdmissionRule{
		{CIDR: mustPrefix("203.0.113.0/24"), Action: model.NetworkAdmissionDeny, Source: "provider"},
		{CIDR: mustPrefix("192.0.2.8/32"), Action: model.NetworkAdmissionAllow, Source: "manual"},
	}}
	result, err := Merge(base, Import{Source: "provider", Action: model.NetworkAdmissionDeny, Paths: []string{newPath}})
	if err != nil {
		t.Fatal(err)
	}
	if len(result.Rules) != 2 || result.Rules[0].CIDR.String() != "192.0.2.8/32" ||
		result.Rules[1].CIDR.String() != "198.51.100.0/24" {
		t.Fatalf("substituicao incorreta: %+v", result.Rules)
	}
	_, err = Merge(base, Import{Source: "other", Action: model.NetworkAdmissionAllow, Paths: []string{basePath}})
	if err == nil {
		t.Fatal("conflito exato de decisoes foi aceito")
	}
}

func TestWriteAtomicProducesLoadableDeterministicPolicy(t *testing.T) {
	path := filepath.Join(t.TempDir(), "policy.json")
	policy := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion, Rules: []model.NetworkAdmissionRule{{
		CIDR: mustPrefix("203.0.113.0/24"), Action: model.NetworkAdmissionLimit,
		MaxClients: 1, Source: "test", Reason: "test range",
	}}}
	if err := WriteAtomic(path, policy); err != nil {
		t.Fatal(err)
	}
	first, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	if err := WriteAtomic(path, policy); err != nil {
		t.Fatal(err)
	}
	second, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	if !reflect.DeepEqual(first, second) {
		t.Fatal("mesma politica gerou bytes diferentes")
	}
	loaded, err := data.LoadNetworkAdmission(path)
	if err != nil || len(loaded.Rules) != 1 || loaded.Rules[0].Source != "test" {
		t.Fatalf("saida nao carregavel: policy=%+v err=%v", loaded, err)
	}
}

func mustPrefix(value string) netip.Prefix {
	return netip.MustParsePrefix(value)
}
