package store

import (
	"encoding/json"
	"fmt"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestCharStateRoundTripAndEmptyRemoval(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(filepath.Join(dir, "accounts"))

	st := &model.CharState{
		Version: model.CharStateVersion,
		// PostgreSQL usa o mesmo payload JSON deste teste; manter a origem aqui
		// prova que UID, indice e expiracao atravessam a serializacao do sidecar.
		Affects: []model.PersistedAffect{{
			Type: 4, Value: 25, SourceItemUID: "11111111111141118111111111110414",
			SourceItemIndex: 4140, ExpiresUnix: 1234567890,
		}},
		SpecialCoins: map[string]uint32{"wyden": 3},
	}
	if err := s.SaveCharState("Felipe", st); err != nil {
		t.Fatal(err)
	}
	got, err := s.LoadCharState("Felipe")
	if err != nil || got == nil {
		t.Fatalf("load falhou: got=%v err=%v", got, err)
	}
	if len(got.Affects) != 1 || got.Affects[0].Value != 25 ||
		got.Affects[0].SourceItemUID != "11111111111141118111111111110414" ||
		got.Affects[0].SourceItemIndex != 4140 || got.SpecialCoins["wyden"] != 3 {
		t.Fatalf("round-trip incorreto: %+v", got)
	}

	// Estado vazio remove o arquivo e o load volta nil (nao erro).
	if err := s.SaveCharState("Felipe", &model.CharState{Version: model.CharStateVersion}); err != nil {
		t.Fatal(err)
	}
	if got, err := s.LoadCharState("Felipe"); err != nil || got != nil {
		t.Fatalf("estado vazio deveria remover o arquivo: got=%v err=%v", got, err)
	}
}

// TestCharStateAsyncThenSyncOrdering garante a invariante do I/O async: um save
// SINCRONO nunca e sobrescrito por um autosave ASYNC pendente com estado mais
// velho (o sync da flush na fila antes de gravar).
func TestCharStateAsyncThenSyncOrdering(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(filepath.Join(dir, "accounts"))
	stateOld := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"c": 1}}
	stateNew := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"c": 2}}
	// Enfileira o async (velho) e logo em seguida o sincrono (novo).
	if err := s.SaveCharStateAsync("Felipe", stateOld); err != nil {
		t.Fatal(err)
	}
	if err := s.SaveCharState("Felipe", stateNew); err != nil {
		t.Fatal(err)
	}
	got, err := s.LoadCharState("Felipe")
	if err != nil || got == nil || got.SpecialCoins["c"] != 2 {
		t.Fatalf("sync deveria vencer o async pendente: got=%v err=%v", got, err)
	}
}

func TestLoadCharStateMissingIsNilNotError(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(filepath.Join(dir, "accounts"))
	got, err := s.LoadCharState("NaoExiste")
	if err != nil || got != nil {
		t.Fatalf("personagem sem sidecar deveria dar nil/nil: got=%v err=%v", got, err)
	}
}

func TestJSONStoreMigratesCharacterAndNamedCharStateToUID(t *testing.T) {
	root := t.TempDir()
	accountsDir := filepath.Join(root, "accounts")
	stateDir := filepath.Join(root, "charstate")
	if err := os.MkdirAll(accountsDir, 0o755); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(stateDir, 0o755); err != nil {
		t.Fatal(err)
	}
	account := &model.Account{
		Name: "felipe", PasswordHash: "hash",
		Chars: []model.Char{validStoredChar("Felipe", 0)},
	}
	account.Chars[0].UID = ""
	rawAccount, err := json.Marshal(account)
	if err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(accountsDir, "felipe.json"), rawAccount, 0o600); err != nil {
		t.Fatal(err)
	}
	rawState, err := json.Marshal(&model.CharState{
		Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 10},
	})
	if err != nil {
		t.Fatal(err)
	}
	legacyPath := filepath.Join(stateDir, "Felipe.json")
	if err := os.WriteFile(legacyPath, rawState, 0o600); err != nil {
		t.Fatal(err)
	}

	st := NewJSONStore(accountsDir, WithCharStatePath(stateDir))
	loaded, err := st.LoadAccount("felipe")
	if err != nil {
		t.Fatal(err)
	}
	uid := loaded.Chars[0].UID
	if uid == "" {
		t.Fatal("personagem legado permaneceu sem UID")
	}
	if _, err := os.Stat(legacyPath); !os.IsNotExist(err) {
		t.Fatalf("sidecar por nickname nao foi removido: %v", err)
	}
	state, err := st.LoadCharState(uid)
	if err != nil || state == nil || state.SpecialCoins["fame"] != 10 {
		t.Fatalf("sidecar nao migrou para UID: state=%v err=%v", state, err)
	}
}

func TestCharStateRejectsUnconfiguredPathsInvalidUIDsAndCorruptFiles(t *testing.T) {
	unconfigured := &JSONStore{}
	if err := unconfigured.SaveCharState("uid", nil); err == nil {
		t.Fatal("synchronous save accepted an unset charstate directory")
	}
	if err := unconfigured.SaveCharStateAsync("uid", nil); err == nil {
		t.Fatal("asynchronous save accepted an unset charstate directory")
	}
	if _, err := unconfigured.LoadCharState("uid"); err == nil {
		t.Fatal("load accepted an unset charstate directory")
	}
	if err := unconfigured.migrateNamedCharStates(&model.Account{}); err != nil {
		t.Fatalf("migration without a directory: %v", err)
	}

	stateDir := t.TempDir()
	store := NewJSONStore(filepath.Join(t.TempDir(), "accounts"), WithCharStatePath(stateDir))
	for _, uid := range []string{"", "../escape", `bad/name`} {
		if err := store.SaveCharState(uid, nil); err == nil {
			t.Errorf("synchronous save accepted invalid UID %q", uid)
		}
		if err := store.SaveCharStateAsync(uid, nil); err == nil {
			t.Errorf("asynchronous save accepted invalid UID %q", uid)
		}
		if _, err := store.LoadCharState(uid); err == nil {
			t.Errorf("load accepted invalid UID %q", uid)
		}
	}

	cases := []struct {
		name string
		body string
	}{
		{name: "malformed JSON", body: `{"version":`},
		{name: "unknown field", body: `{"version":1,"secret":true}`},
		{name: "trailing JSON", body: `{"version":1}{"version":1}`},
		{name: "wrong version", body: `{"version":999}`},
	}
	for i, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			uid := fmt.Sprintf("bad-state-%d", i)
			if err := os.WriteFile(filepath.Join(stateDir, uid+".json"), []byte(tc.body), 0o600); err != nil {
				t.Fatal(err)
			}
			if _, err := store.LoadCharState(uid); err == nil {
				t.Fatal("corrupt sidecar was accepted")
			}
		})
	}
}

func TestCharStateMigrationKeepsExistingUIDAndSkipsUnsafeNames(t *testing.T) {
	stateDir := t.TempDir()
	store := &JSONStore{charStateDir: stateDir}
	current := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 20}}
	legacy := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 10}}
	currentRaw, _ := json.Marshal(current)
	legacyRaw, _ := json.Marshal(legacy)
	if err := os.WriteFile(filepath.Join(stateDir, "uid-current.json"), currentRaw, 0o600); err != nil {
		t.Fatal(err)
	}
	legacyPath := filepath.Join(stateDir, "Knight.json")
	if err := os.WriteFile(legacyPath, legacyRaw, 0o600); err != nil {
		t.Fatal(err)
	}
	account := &model.Account{Chars: []model.Char{
		{Name: "Knight", UID: "uid-current"},
		{Name: "../unsafe", UID: "uid-unsafe"},
		{Name: "", UID: "uid-empty-name"},
		{Name: "Knight", UID: "../unsafe-uid"},
	}}
	if err := store.migrateNamedCharStates(account); err != nil {
		t.Fatal(err)
	}
	if _, err := os.Stat(legacyPath); err != nil {
		t.Fatalf("existing UID sidecar replaced or removed the legacy file: %v", err)
	}
	got, err := store.LoadCharState("uid-current")
	if err != nil || got == nil || got.SpecialCoins["fame"] != 20 {
		t.Fatalf("existing UID sidecar=%+v err=%v", got, err)
	}
}
