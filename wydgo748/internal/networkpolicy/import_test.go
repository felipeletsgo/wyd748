package networkpolicy

import (
	"net/netip"
	"os"
	"path/filepath"
	"reflect"
	"strings"
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
	path := writeInput(t, "ranges.txt", "# provider\n203.0.113.7/24\n203.0.113.0/24 # duplicate\n2001:db8::/32\n192.0.2.0/25\n192.0.2.0/24\n")
	prefixes, err := ReadPrefixes(path)
	if err != nil {
		t.Fatal(err)
	}
	got := make([]string, len(prefixes))
	for i, prefix := range prefixes {
		got[i] = prefix.String()
	}
	want := []string{"192.0.2.0/24", "192.0.2.0/25", "203.0.113.0/24", "2001:db8::/32"}
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

func TestReadPrefixesCoversEmptyUnknownMappedAndOversizedInputs(t *testing.T) {
	for _, test := range []struct{ name, input string }{
		{name: "empty", input: " \n\t"},
		{name: "JSON without recognized prefixes", input: `{"addresses":["203.0.113.0/24"]}`},
		{name: "top-level strings are not inferred", input: `["203.0.113.0/24"]`},
		{name: "singular prefix must be a string", input: `{"cidr":12}`},
		{name: "invalid plural prefix", input: `{"prefixes":["not-a-cidr"]}`},
		{name: "mapped IPv4 prefix", input: `{"cidr":"::ffff:192.0.2.0/120"}`},
		{name: "scanner token limit", input: strings.Repeat("x", 70*1024)},
	} {
		t.Run(test.name, func(t *testing.T) {
			if _, err := ReadPrefixes(writeInput(t, "input", test.input)); err == nil {
				t.Fatal("accepted input without a safe CIDR set")
			}
		})
	}
	if _, err := ReadPrefixes(filepath.Join(t.TempDir(), "missing.txt")); err == nil {
		t.Fatal("accepted missing feed")
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

func TestMergeRejectsInvalidParametersAndSkipsIdenticalSharedDecisions(t *testing.T) {
	path := writeInput(t, "ranges.txt", "203.0.113.0/24\n")
	base := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion, Rules: []model.NetworkAdmissionRule{{
		CIDR: mustPrefix("203.0.113.0/24"), Action: model.NetworkAdmissionDeny, Source: "manual",
	}}}
	for _, test := range []struct {
		name  string
		base  model.NetworkAdmissionFile
		input Import
	}{
		{name: "invalid base", base: model.NetworkAdmissionFile{}, input: Import{Source: "provider", Action: model.NetworkAdmissionDeny, Paths: []string{path}}},
		{name: "invalid action", base: model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion}, input: Import{Source: "provider", Action: "block", Paths: []string{path}}},
		{name: "missing source", base: model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion}, input: Import{Action: model.NetworkAdmissionDeny, Paths: []string{path}}},
		{name: "missing input", base: model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion}, input: Import{Source: "provider", Action: model.NetworkAdmissionDeny}},
		{name: "unreadable input", base: model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion}, input: Import{Source: "provider", Action: model.NetworkAdmissionDeny, Paths: []string{filepath.Join(t.TempDir(), "missing.txt")}}},
	} {
		t.Run(test.name, func(t *testing.T) {
			if _, err := Merge(test.base, test.input); err == nil {
				t.Fatal("accepted invalid merge input")
			}
		})
	}
	result, err := Merge(base, Import{Source: "provider", Action: model.NetworkAdmissionDeny, Paths: []string{path}})
	if err != nil || len(result.Rules) != 1 || result.Rules[0].Source != "manual" {
		t.Fatalf("identical shared decision was duplicated: rules=%+v err=%v", result.Rules, err)
	}
	base.Rules[0].Action = model.NetworkAdmissionLimit
	base.Rules[0].MaxClients = 2
	if _, err := Merge(base, Import{Source: "provider", Action: model.NetworkAdmissionLimit, MaxClients: 3, Paths: []string{path}}); err == nil {
		t.Fatal("same action with a different client cap was accepted")
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

func TestWriteAtomicRejectsInvalidPolicyAndUnwritableDirectory(t *testing.T) {
	if err := WriteAtomic(filepath.Join(t.TempDir(), "policy.json"), model.NetworkAdmissionFile{}); err == nil {
		t.Fatal("wrote a policy with an invalid version")
	}
	path := filepath.Join(t.TempDir(), "missing", "policy.json")
	policy := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion, Rules: []model.NetworkAdmissionRule{}}
	if err := WriteAtomic(path, policy); err == nil {
		t.Fatal("wrote into a missing parent directory")
	}
	directory := filepath.Join(t.TempDir(), "policy.json")
	if err := os.Mkdir(directory, 0700); err != nil {
		t.Fatal(err)
	}
	if err := WriteAtomic(directory, policy); err == nil {
		t.Fatal("replaced a directory with a policy file")
	}
}

func mustPrefix(value string) netip.Prefix {
	return netip.MustParsePrefix(value)
}
