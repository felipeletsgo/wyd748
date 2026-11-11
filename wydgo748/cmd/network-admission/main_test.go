package main

import (
	"bytes"
	"os"
	"os/exec"
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
	previousArgs := os.Args
	os.Args = []string{"network-admission", "-base", base, "-out", out,
		"-source", "provider", "-input", input, "-dry-run"}
	defer func() { os.Args = previousArgs }()
	main()
}

func TestMainRunsSafeDryRun(t *testing.T) {
	if os.Getenv("WYD_NETWORK_ADMISSION_MAIN_HELPER") == "1" {
		os.Args = []string{"network-admission", "-base", os.Getenv("WYD_NETWORK_ADMISSION_BASE"),
			"-out", os.Getenv("WYD_NETWORK_ADMISSION_OUT"), "-source", "provider", "-reason", "review",
			"-input", os.Getenv("WYD_NETWORK_ADMISSION_INPUT"), "-dry-run"}
		main()
		return
	}
	dir := t.TempDir()
	base := filepath.Join(dir, "base.json")
	input := filepath.Join(dir, "ranges.txt")
	output := filepath.Join(dir, "compiled.json")
	if err := os.WriteFile(base, []byte("{\"version\":1,\"rules\":[]}\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(input, []byte("203.0.113.0/24\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	cmd := exec.Command(os.Args[0], "-test.run=^TestMainRunsSafeDryRun$")
	cmd.Env = append(os.Environ(), "WYD_NETWORK_ADMISSION_MAIN_HELPER=1",
		"WYD_NETWORK_ADMISSION_BASE="+base, "WYD_NETWORK_ADMISSION_OUT="+output,
		"WYD_NETWORK_ADMISSION_INPUT="+input)
	if combined, err := cmd.CombinedOutput(); err != nil {
		t.Fatalf("main dry-run failed: %v: %s", err, combined)
	}
	if _, err := os.Stat(output); !os.IsNotExist(err) {
		t.Fatal("main dry-run wrote the output file")
	}
}

func TestMainExitsWhenPolicyInputIsInvalid(t *testing.T) {
	if os.Getenv("WYD_NETWORK_ADMISSION_ERROR_HELPER") == "1" {
		os.Args = []string{"network-admission", "-base", os.Getenv("WYD_NETWORK_ADMISSION_BAD_BASE")}
		main()
		return
	}
	base := filepath.Join(t.TempDir(), "invalid.json")
	if err := os.WriteFile(base, []byte("{"), 0o600); err != nil {
		t.Fatal(err)
	}
	cmd := exec.Command(os.Args[0], "-test.run=^TestMainExitsWhenPolicyInputIsInvalid$")
	cmd.Env = append(os.Environ(), "WYD_NETWORK_ADMISSION_ERROR_HELPER=1", "WYD_NETWORK_ADMISSION_BAD_BASE="+base)
	output, err := cmd.CombinedOutput()
	if err == nil || !bytes.Contains(output, []byte("carregar base")) {
		t.Fatalf("main did not report invalid policy input: err=%v output=%s", err, output)
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
