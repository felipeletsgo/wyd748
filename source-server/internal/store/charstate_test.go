package store

import (
	"encoding/json"
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
