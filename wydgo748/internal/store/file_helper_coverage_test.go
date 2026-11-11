package store

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestDecodeAccountJSONRejectsMalformedAndTrailingContent(t *testing.T) {
	valid, err := json.Marshal(model.Account{Name: "Player", PasswordHash: "hash"})
	if err != nil {
		t.Fatal(err)
	}
	var account model.Account
	if err := decodeAccountJSON(valid, &account); err != nil || account.Name != "Player" {
		t.Fatalf("valid account JSON: account=%+v err=%v", account, err)
	}
	for name, input := range map[string][]byte{
		"malformed":        []byte("{"),
		"unknown field":    []byte(`{"name":"Player","unknown":true}`),
		"second JSON row":  append(append([]byte(nil), valid...), []byte(` {"name":"Other"}`)...),
		"trailing garbage": append(append([]byte(nil), valid...), []byte(" trailing")...),
	} {
		t.Run(name, func(t *testing.T) {
			var got model.Account
			if err := decodeAccountJSON(input, &got); err == nil {
				t.Fatal("invalid or trailing JSON was accepted")
			}
		})
	}
}

func TestJSONStoreAccountNameExistsAndDurableFileFailures(t *testing.T) {
	missing := &JSONStore{dir: filepath.Join(t.TempDir(), "missing")}
	if exists, err := missing.AccountNameExists("player"); err != nil || exists {
		t.Fatalf("missing account directory exists=%t err=%v", exists, err)
	}
	dir := t.TempDir()
	if err := os.WriteFile(filepath.Join(dir, "Player.JSON"), []byte("unused"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dir, "notes.txt"), []byte("unused"), 0o600); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(filepath.Join(dir, "Directory.json"), 0o700); err != nil {
		t.Fatal(err)
	}
	store := &JSONStore{dir: dir}
	for name, want := range map[string]bool{"player": true, "missing": false, "directory": false} {
		exists, err := store.AccountNameExists(name)
		if err != nil || exists != want {
			t.Errorf("AccountNameExists(%q)=%t err=%v, want %t", name, exists, err, want)
		}
	}

	root := t.TempDir()
	blocker := filepath.Join(root, "file-not-directory")
	if err := os.WriteFile(blocker, []byte("block"), 0o600); err != nil {
		t.Fatal(err)
	}
	target := filepath.Join(blocker, "state.json")
	if err := writeFileAtomic(target, []byte("{}")); err == nil {
		t.Fatal("atomic write through a file parent succeeded")
	}
	if err := (&JSONStore{dir: blocker}).writeAccountFile(target, []byte("{}")); err == nil {
		t.Fatal("account write through a file directory succeeded")
	}
	if err := writeSyncedFile(target, []byte("{}"), 0o600); err == nil {
		t.Fatal("synced write through a file parent succeeded")
	}
	if err := syncDirectory(target); err == nil {
		t.Fatal("directory sync accepted a nonexistent path")
	}
}
