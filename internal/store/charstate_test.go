package store

import (
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestCharStateRoundTripAndEmptyRemoval(t *testing.T) {
	dir := t.TempDir()
	s := NewJSONStore(filepath.Join(dir, "accounts"))

	st := &model.CharState{
		Version:      model.CharStateVersion,
		Affects:      []model.PersistedAffect{{Type: 4, Value: 25, ExpiresUnix: 1234567890}},
		SpecialCoins: map[string]uint32{"wyden": 3},
	}
	if err := s.SaveCharState("Felipe", st); err != nil {
		t.Fatal(err)
	}
	got, err := s.LoadCharState("Felipe")
	if err != nil || got == nil {
		t.Fatalf("load falhou: got=%v err=%v", got, err)
	}
	if len(got.Affects) != 1 || got.Affects[0].Value != 25 || got.SpecialCoins["wyden"] != 3 {
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
