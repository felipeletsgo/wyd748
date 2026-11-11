package store

import (
	"errors"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestJSONStoreAccountOperationsRejectFileAsDirectory(t *testing.T) {
	root := t.TempDir()
	blocker := filepath.Join(root, "accounts")
	if err := os.WriteFile(blocker, []byte("file"), 0o600); err != nil {
		t.Fatal(err)
	}
	s := &JSONStore{dir: blocker}

	if exists, err := s.CharacterNameExists("Player"); exists {
		t.Fatalf("character-name lookup reported a character in a file path: exists=%v err=%v", exists, err)
	}
	if names, err := s.CharacterNames(); len(names) != 0 {
		t.Fatalf("character-name index returned entries through a file path: names=%v err=%v", names, err)
	}
	if exists, err := s.AccountNameExists("Player"); exists {
		t.Fatalf("account-name lookup reported an account in a file path: exists=%v err=%v", exists, err)
	}
	if _, err := s.LoadAccount("Player"); err == nil {
		t.Fatal("account load accepted a file as its directory")
	}
	account := postgresTestAccount("player", "Player", model.Item{})
	if err := s.CreateAccount(account); err == nil {
		t.Fatal("account creation accepted a file as its directory")
	}
	if err := s.SaveAccount(account); err == nil {
		t.Fatal("synchronous account save accepted a file as its directory")
	}
	if err := s.SaveAccountAsync(account); err == nil {
		t.Fatal("asynchronous account save accepted a file as its directory")
	}
}

func TestJSONStoreCharacterIndexesRejectMalformedAccountFiles(t *testing.T) {
	dir := t.TempDir()
	if err := os.WriteFile(filepath.Join(dir, "player.json"), []byte(`{"account":`), 0o600); err != nil {
		t.Fatal(err)
	}
	s := &JSONStore{dir: dir}
	if exists, err := s.CharacterNameExists("Player"); err == nil || exists {
		t.Fatalf("character-name scan accepted malformed account JSON: exists=%v err=%v", exists, err)
	}
	if names, err := s.CharacterNames(); err == nil || names != nil {
		t.Fatalf("character index accepted malformed account JSON: names=%v err=%v", names, err)
	}
}

func TestJSONStoreInitializationErrorsAndInlineQueueFallback(t *testing.T) {
	failed := &JSONStore{initErr: errors.New("store initialization failed")}
	account := postgresTestAccount("player", "Player", model.Item{})
	if _, err := failed.LoadAccount("player"); err == nil {
		t.Fatal("load ignored the store initialization failure")
	}
	if err := failed.CreateAccount(account); err == nil {
		t.Fatal("create ignored the store initialization failure")
	}
	if err := failed.SaveAccount(account); err == nil {
		t.Fatal("save ignored the store initialization failure")
	}
	if err := failed.SaveAccountAsync(account); err == nil {
		t.Fatal("async save ignored the store initialization failure")
	}
	if _, err := failed.LoadGuilds(); err == nil {
		t.Fatal("guild load ignored the store initialization failure")
	}
	if _, err := failed.LoadInstanceState(); err == nil {
		t.Fatal("instance load ignored the store initialization failure")
	}
	if err := failed.SaveInstanceState(nil); err == nil {
		t.Fatal("instance save ignored the store initialization failure")
	}

	inline := &JSONStore{}
	inline.Flush()
	ran := false
	inline.enqueueWrite(writeJob{run: func() { ran = true }})
	if !ran {
		t.Fatal("queue-less store did not run the autosave inline")
	}
	done := make(chan struct{})
	inline.enqueueWrite(writeJob{done: done})
	select {
	case <-done:
	default:
		t.Fatal("queue-less store did not complete the flush barrier")
	}
	inline.enqueueWrite(writeJob{})
}
