package diagnostics

import (
	"bytes"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRecorderWritesEventsRedactsSecretsAndDumpsExactBytes(t *testing.T) {
	root := t.TempDir()
	r := Open(root)
	if err := r.InitError(); err != nil {
		t.Fatalf("Open: %v", err)
	}
	r.Event("auth_begin",
		F("account_present", true),
		F("account_len", 6),
		F("password", "hunter2"),
		F("SecretCode", "1234"),
	)
	want := []byte{0x10, 0x20, 0x30, 0x40}
	name := r.DumpFrame("tx", 0x20D, want)
	if name != "tx-000001-op020D-size4.bin" {
		t.Fatalf("dump name=%q", name)
	}
	if err := r.Close(); err != nil {
		t.Fatalf("Close: %v", err)
	}
	if err := r.Close(); err != nil {
		t.Fatalf("second Close: %v", err)
	}

	got, err := os.ReadFile(filepath.Join(root, "dumps", name))
	if err != nil {
		t.Fatalf("ReadFile dump: %v", err)
	}
	if !bytes.Equal(got, want) {
		t.Fatalf("dump=% X want=% X", got, want)
	}
	logData, err := os.ReadFile(filepath.Join(root, "client.log"))
	if err != nil {
		t.Fatalf("ReadFile log: %v", err)
	}
	logText := string(logData)
	if strings.Contains(logText, "hunter2") || strings.Contains(logText, "1234") {
		t.Fatalf("secret leaked in log: %s", logText)
	}
	if !strings.Contains(logText, `password="<redacted>"`) || !strings.Contains(logText, `SecretCode="<redacted>"`) {
		t.Fatalf("redaction missing: %s", logText)
	}
}

func TestRecorderCreationFailureDoesNotPanicOrWrite(t *testing.T) {
	root := t.TempDir()
	blockingFile := filepath.Join(root, "not-a-directory")
	if err := os.WriteFile(blockingFile, []byte("x"), 0o644); err != nil {
		t.Fatalf("WriteFile: %v", err)
	}
	r := Open(blockingFile)
	if r.InitError() == nil {
		t.Fatal("InitError=nil, want creation failure")
	}
	r.Event("client_start", F("password", "must-not-appear"))
	if name := r.DumpFrame("rx", 0x101, []byte{1, 2, 3}); name != "" {
		t.Fatalf("dump name=%q, want disabled recorder", name)
	}
	if err := r.Close(); err != nil {
		t.Fatalf("Close: %v", err)
	}
}
