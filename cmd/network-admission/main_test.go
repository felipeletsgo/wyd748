package main

import (
	"bytes"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/data"
)

func TestRunCompilesAndDryRunDoesNotWrite(t *testing.T) {
	dir := t.TempDir()
	base := filepath.Join(dir, "base.json")
	input := filepath.Join(dir, "ranges.txt")
	out := filepath.Join(dir, "compiled.json")
	if err := os.WriteFile(base, []byte("{\"version\":1,\"rules\":[]}\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(input, []byte("203.0.113.0/24\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	args := []string{"-base", base, "-out", out, "-source", "provider",
		"-action", "deny", "-reason", "hosting", "-input", input}
	var stdout bytes.Buffer
	if err := run(append(args, "-dry-run"), &stdout); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(out); !os.IsNotExist(err) {
		t.Fatal("dry-run gravou o arquivo de saida")
	}
	if err := run(args, &stdout); err != nil {
		t.Fatal(err)
	}
	policy, err := data.LoadNetworkAdmission(out)
	if err != nil {
		t.Fatal(err)
	}
	if len(policy.Rules) != 1 || policy.Rules[0].Source != "provider" ||
		policy.Rules[0].CIDR.String() != "203.0.113.0/24" {
		t.Fatalf("saida incorreta: %+v", policy)
	}
}

func TestRunRejectsInvalidSourceAndMissingInput(t *testing.T) {
	dir := t.TempDir()
	base := filepath.Join(dir, "base.json")
	if err := os.WriteFile(base, []byte("{\"version\":1,\"rules\":[]}\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	for _, args := range [][]string{
		{"-base", base, "-source", "Invalid Source"},
		{"-base", base, "-source", "valid"},
	} {
		if err := run(args, &bytes.Buffer{}); err == nil {
			t.Fatalf("argumentos invalidos aceitos: %v", args)
		}
	}
}
