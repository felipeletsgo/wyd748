package networkpolicy

import (
	"net/netip"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestReadPrefixesCoversMissingEmptyMalformedAndRecognizedTypeErrors(t *testing.T) {
	if _, err := ReadPrefixes(filepath.Join(t.TempDir(), "missing")); err == nil {
		t.Fatal("arquivo ausente foi aceito")
	}
	for name, contents := range map[string]string{
		"empty": "  \n\t",
		"malformed-json": `{"prefixes":[`,
		"recognized-non-string": `{"cidr":123}`,
		"no-prefix": `{"syncToken":"203.0.113.0/24"}`,
		"mapped": `{"cidr":"::ffff:192.0.2.1/128"}`,
	} {
		t.Run(name, func(t *testing.T) {
			if _, err := ReadPrefixes(writeInput(t, name, contents)); err == nil {
				t.Fatalf("entrada invalida aceita: %q", contents)
			}
		})
	}
}

func TestCollectJSONPrefixesIgnoresUnrecognizedStringsAndParsesPluralContext(t *testing.T) {
	var out []netip.Prefix
	value := map[string]any{
		"description": "203.0.113.0/24",
		"prefixes": []any{"198.51.100.7/24", map[string]any{"cidr": "2001:db8::/32"}},
	}
	if err := collectJSONPrefixes(value, false, &out); err != nil {
		t.Fatal(err)
	}
	out = uniqueSortedPrefixes(out)
	if len(out) != 2 || out[0].String() != "198.51.100.0/24" || out[1].String() != "2001:db8::/32" {
		t.Fatalf("prefixos coletados=%v", out)
	}
	if err := collectJSONPrefixes([]any{"invalid"}, true, &out); err == nil {
		t.Fatal("string invalida em contexto plural foi aceita")
	}
}

func TestParseAndUniqueSortCoverIPv4IPv6MaskingDuplicatesAndOrder(t *testing.T) {
	if _, err := parsePrefix("bad"); err == nil {
		t.Fatal("CIDR invalido aceito")
	}
	if _, err := parsePrefix("::ffff:192.0.2.1/128"); err == nil {
		t.Fatal("IPv4-mapped IPv6 aceito")
	}
	masked, err := parsePrefix("203.0.113.7/24")
	if err != nil || masked.String() != "203.0.113.0/24" {
		t.Fatalf("mask=%v err=%v", masked, err)
	}
	got := uniqueSortedPrefixes([]netip.Prefix{
		netip.MustParsePrefix("2001:db8:2::/48"),
		netip.MustParsePrefix("203.0.113.7/24"),
		netip.MustParsePrefix("203.0.113.0/24"),
		netip.MustParsePrefix("198.51.100.0/25"),
		netip.MustParsePrefix("198.51.100.0/24"),
	})
	want := []string{"198.51.100.0/24", "198.51.100.0/25", "203.0.113.0/24", "2001:db8:2::/48"}
	if len(got) != len(want) {
		t.Fatalf("unique len=%d want=%d: %v", len(got), len(want), got)
	}
	for i := range want {
		if got[i].String() != want[i] {
			t.Fatalf("sort[%d]=%s want=%s", i, got[i], want[i])
		}
	}
}

func TestMergeCoversInvalidBaseImportSourcePathsReadFailureAndCompatibleDuplicate(t *testing.T) {
	invalidBase := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion + 1}
	if _, err := Merge(invalidBase, Import{}); err == nil {
		t.Fatal("base invalida foi aceita")
	}
	base := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion}
	path := writeInput(t, "one.txt", "203.0.113.0/24\n")
	if _, err := Merge(base, Import{Source: "", Action: model.NetworkAdmissionDeny, Paths: []string{path}}); err == nil {
		t.Fatal("source vazio foi aceito")
	}
	if _, err := Merge(base, Import{Source: "provider", Action: model.NetworkAdmissionDeny}); err == nil {
		t.Fatal("import sem input foi aceito")
	}
	if _, err := Merge(base, Import{Source: "provider", Action: model.NetworkAdmissionLimit, MaxClients: 0, Paths: []string{path}}); err == nil {
		t.Fatal("parametros de importacao invalidos foram aceitos")
	}
	if _, err := Merge(base, Import{Source: "provider", Action: model.NetworkAdmissionDeny, Paths: []string{filepath.Join(t.TempDir(), "missing")}}); err == nil {
		t.Fatal("erro de leitura do input foi engolido")
	}

	base = model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion, Rules: []model.NetworkAdmissionRule{{
		CIDR: netip.MustParsePrefix("203.0.113.0/24"), Action: model.NetworkAdmissionDeny, Source: "manual",
	}}}
	result, err := Merge(base, Import{Source: "provider", Action: model.NetworkAdmissionDeny, Paths: []string{path}})
	if err != nil || len(result.Rules) != 1 || result.Rules[0].Source != "manual" {
		t.Fatalf("duplicata compativel deveria preservar regra original: %+v err=%v", result, err)
	}
}

func TestWriteAtomicRejectsInvalidPolicyAndMissingParent(t *testing.T) {
	invalid := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion + 1}
	if err := WriteAtomic(filepath.Join(t.TempDir(), "policy.json"), invalid); err == nil {
		t.Fatal("politica invalida foi escrita")
	}
	valid := model.NetworkAdmissionFile{Version: model.NetworkAdmissionVersion}
	if err := WriteAtomic(filepath.Join(t.TempDir(), "missing", "policy.json"), valid); err == nil {
		t.Fatal("parent ausente foi criado implicitamente")
	}

	dir := t.TempDir()
	path := filepath.Join(dir, "policy.json")
	if err := os.WriteFile(path, []byte("old"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := WriteAtomic(path, valid); err != nil {
		t.Fatal(err)
	}
	if body, err := os.ReadFile(path); err != nil || len(body) == 0 || body[len(body)-1] != '\n' {
		t.Fatalf("saida atomica invalida: %q err=%v", body, err)
	}
}
