package protocol

import (
	"bytes"
	"os"
	"path/filepath"
	"testing"

	"wydclient748/internal/diagnostics"
)

// O diagnóstico deve preservar o wire e excluir todo corpo, inclusive códigos
// de autenticação que poderiam aparecer em packets de RX.
func TestDiagnosticHeaderDumpPreservesWireAndOmitsBody(t *testing.T) {
	root := t.TempDir()
	recorder := diagnostics.Open(root)
	defer recorder.Close()
	session := NewSession("", SessionOptions{Diagnostics: recorder})
	plain := NewPacket(0x10A, 7, 123, []byte("private-auth-body"))
	frame, err := Encode(plain, 42)
	if err != nil {
		t.Fatal(err)
	}
	original := append([]byte(nil), frame...)
	session.recordFrame("rx", frame)
	if !bytes.Equal(frame, original) {
		t.Fatal("diagnostic changed wire")
	}
	files, err := filepath.Glob(filepath.Join(root, "dumps", "*.bin"))
	if err != nil || len(files) != 1 {
		t.Fatalf("dumps=%v err=%v", files, err)
	}
	dump, err := os.ReadFile(files[0])
	if err != nil {
		t.Fatal(err)
	}
	decoded, err := Decode(frame)
	if err != nil {
		t.Fatal(err)
	}
	if !bytes.Equal(dump, decoded.Raw[:HeaderSize]) {
		t.Fatal("dump is not the exact logical header")
	}
	log, err := os.ReadFile(filepath.Join(root, "client.log"))
	if err != nil {
		t.Fatal(err)
	}
	if bytes.Contains(log, []byte("private-auth-body")) {
		t.Fatal("body leaked into log")
	}
}
