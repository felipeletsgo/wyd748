package store

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestCharStateRejectsMissingDirectoryAndUnsafeUIDs(t *testing.T) {
	s := &JSONStore{}
	if _, err := s.charStatePath("uid"); err != nil {
		t.Fatalf("path helper nao depende do diretorio: %v", err)
	}
	if _, err := s.charStatePath("../escape"); err == nil {
		t.Fatal("UID inseguro foi aceito")
	}
	if _, err := s.charStateWrite("uid", &model.CharState{}); err == nil {
		t.Fatal("write sem charStateDir foi aceito")
	}
	if err := s.SaveCharState("uid", nil); err == nil {
		t.Fatal("SaveCharState sem diretorio foi aceito")
	}
	if err := s.SaveCharStateAsync("uid", nil); err == nil {
		t.Fatal("SaveCharStateAsync sem diretorio foi aceito")
	}
	if _, err := s.LoadCharState("uid"); err == nil {
		t.Fatal("LoadCharState sem diretorio foi aceito")
	}
}

func TestCharStateWriteRemovalIsIdempotentAndSetsVersion(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(filepath.Join(dir, "accounts"), WithCharStatePath(filepath.Join(dir, "state")))
	run, err := s.charStateWrite("safeuid", nil)
	if err != nil || run == nil {
		t.Fatalf("remove closure invalido: run=%v err=%v", run, err)
	}
	if err := run(); err != nil {
		t.Fatalf("remocao de arquivo ausente nao foi idempotente: %v", err)
	}

	state := &model.CharState{SpecialCoins: map[string]uint32{"fame": 1}}
	run, err = s.charStateWrite("safeuid", state)
	if err != nil {
		t.Fatal(err)
	}
	if state.Version != model.CharStateVersion {
		t.Fatalf("versao nao materializada: %d", state.Version)
	}
	if err := run(); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(filepath.Join(dir, "state", "safeuid.json")); err != nil {
		t.Fatal(err)
	}
}

func TestLoadCharStateRejectsMalformedExtraAndWrongVersion(t *testing.T) {
	dir := t.TempDir()
	stateDir := filepath.Join(dir, "state")
	if err := os.MkdirAll(stateDir, 0o755); err != nil {
		t.Fatal(err)
	}
	s := NewJSONStore(filepath.Join(dir, "accounts"), WithCharStatePath(stateDir))
	uid := "safeuid"
	path := filepath.Join(stateDir, uid+".json")
	for _, tc := range []struct{name, body string}{
		{"malformed", `{"version":`},
		{"unknown field", `{"version":1,"unknown":1}`},
		{"extra json", `{"version":1} {}`},
		{"wrong version", `{"version":999}`},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if err := os.WriteFile(path, []byte(tc.body), 0o600); err != nil {
				t.Fatal(err)
			}
			if _, err := s.LoadCharState(uid); err == nil {
				t.Fatalf("charstate invalido foi aceito: %s", tc.body)
			}
		})
	}
}

func TestMigrateNamedCharStatesCoversSkipMissingAndExistingCurrent(t *testing.T) {
	root := t.TempDir()
	stateDir := filepath.Join(root, "state")
	if err := os.MkdirAll(stateDir, 0o755); err != nil {
		t.Fatal(err)
	}
	s := NewJSONStore(filepath.Join(root, "accounts"), WithCharStatePath(stateDir))
	if err := s.migrateNamedCharStates(nil); err != nil {
		t.Fatal(err)
	}

	acc := uidTestAccount("acc")
	acc.Chars[0] = model.Char{Name: "", UID: "11111111111141118111111111111111"}
	acc.Chars[1] = model.Char{Name: "Unsafe/Name", UID: "22222222222242228222222222222222"}
	acc.Chars[2] = model.Char{Name: "Missing", UID: "33333333333343338333333333333333"}
	acc.Chars[3] = model.Char{Name: "Current", UID: "44444444444444448444444444444444"}
	current := filepath.Join(stateDir, acc.Chars[3].UID+".json")
	if err := os.WriteFile(current, []byte(`{"version":1}`), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := s.migrateNamedCharStates(acc); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(current); err != nil {
		t.Fatal("arquivo atual existente foi alterado/removido")
	}
	if _, err := os.Stat(filepath.Join(stateDir, "Missing.json")); !os.IsNotExist(err) {
		t.Fatalf("legacy ausente apareceu: %v", err)
	}
}

func TestCharStatePathUsesConfiguredDirectory(t *testing.T) {
	dir := t.TempDir()
	s := &JSONStore{charStateDir: dir}
	path, err := s.charStatePath("abc")
	if err != nil || !strings.HasSuffix(path, filepath.Join(dir, "abc.json")) {
		t.Fatalf("path=%q err=%v", path, err)
	}
}
