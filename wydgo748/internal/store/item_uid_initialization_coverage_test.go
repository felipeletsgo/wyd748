package store

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestInitializeItemUIDsMigratesAccountsAndNamedCharState(t *testing.T) {
	dir := t.TempDir()
	charStateDir := filepath.Join(dir, "charstate")
	if err := os.Mkdir(charStateDir, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(filepath.Join(dir, "nested"), 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dir, "notes.txt"), []byte("ignored"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(charStateDir, "Player.json"), []byte(`{"legacy":true}`), 0o600); err != nil {
		t.Fatal(err)
	}
	account := postgresTestAccount("player", "Player", model.Item{Index: 4011})
	raw, err := json.MarshalIndent(account, "", "  ")
	if err != nil {
		t.Fatal(err)
	}
	accountPath := filepath.Join(dir, "player.json")
	if err := os.WriteFile(accountPath, raw, 0o600); err != nil {
		t.Fatal(err)
	}
	store := &JSONStore{dir: dir, charStateDir: charStateDir}
	if err := store.initializeItemUIDs(); err != nil {
		t.Fatal(err)
	}
	data, err := os.ReadFile(accountPath)
	if err != nil {
		t.Fatal(err)
	}
	var migrated model.Account
	if err := json.Unmarshal(data, &migrated); err != nil {
		t.Fatal(err)
	}
	character := migrated.Chars[0]
	itemUID := character.Inv[0].UID
	if character.UID == "" || itemUID == "" || len(store.itemOwners) != 1 {
		t.Fatalf("legacy account identities not migrated: char=%q item=%q owners=%v", character.UID, itemUID, store.itemOwners)
	}
	if owner := store.itemOwners[itemUID]; owner.Account != "player" || owner.Location != "char["+character.UID+"]/inv[0]" {
		t.Fatalf("migrated item owner=%+v", owner)
	}
	if _, err := os.Stat(filepath.Join(charStateDir, character.UID+".json")); err != nil {
		t.Fatalf("legacy character state was not moved to its stable UID: %v", err)
	}
	if _, err := os.Stat(filepath.Join(charStateDir, "Player.json")); !os.IsNotExist(err) {
		t.Fatalf("legacy character-state name remains: %v", err)
	}
}

func TestInitializeItemUIDsRejectsMalformedAndDuplicateAccounts(t *testing.T) {
	t.Run("malformed account", func(t *testing.T) {
		dir := t.TempDir()
		if err := os.WriteFile(filepath.Join(dir, "broken.json"), []byte(`{"name":`), 0o600); err != nil {
			t.Fatal(err)
		}
		err := (&JSONStore{dir: dir}).initializeItemUIDs()
		if err == nil || !strings.Contains(err.Error(), "indexar UIDs") {
			t.Fatalf("malformed account error=%v", err)
		}
	})

	t.Run("duplicate item identity", func(t *testing.T) {
		dir := t.TempDir()
		const uid = "00112233445566778899aabbccddeeff"
		for _, name := range []string{"first", "second"} {
			account := postgresTestAccount(name, "Player", model.Item{Index: 4011, UID: uid})
			raw, err := json.Marshal(account)
			if err != nil {
				t.Fatal(err)
			}
			if err := os.WriteFile(filepath.Join(dir, name+".json"), raw, 0o600); err != nil {
				t.Fatal(err)
			}
		}
		err := (&JSONStore{dir: dir}).initializeItemUIDs()
		if err == nil || !strings.Contains(err.Error(), "duplicado") {
			t.Fatalf("duplicate item identity accepted: %v", err)
		}
	})
}

func TestInitializeItemUIDsTreatsMissingDirectoryAsEmpty(t *testing.T) {
	store := &JSONStore{dir: filepath.Join(t.TempDir(), "accounts")}
	if err := store.initializeItemUIDs(); err != nil || len(store.itemOwners) != 0 {
		t.Fatalf("missing account directory: owners=%v err=%v", store.itemOwners, err)
	}
}
