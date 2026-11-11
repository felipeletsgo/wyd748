package store

import (
	"context"
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestJSONStoreCharacterNamesIndexesOnlyNamedCharacters(t *testing.T) {
	missing := &JSONStore{dir: filepath.Join(t.TempDir(), "missing")}
	if names, err := missing.CharacterNames(); err != nil || len(names) != 0 {
		t.Fatalf("missing directory names=%v err=%v", names, err)
	}

	dir := t.TempDir()
	account := model.Account{Name: "account", PasswordHash: "hash", Chars: []model.Char{
		validStoredChar("Alice", 0), {}, validStoredChar("Bob", 0),
	}}
	body, err := json.Marshal(account)
	if err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dir, "account.JSON"), body, 0o644); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dir, "notes.txt"), []byte("ignored"), 0o644); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(filepath.Join(dir, "nested.json"), 0o755); err != nil {
		t.Fatal(err)
	}
	names, err := (&JSONStore{dir: dir}).CharacterNames()
	if err != nil {
		t.Fatal(err)
	}
	if len(names) != 2 {
		t.Fatalf("indexed names=%v", names)
	}
	for _, name := range []string{"alice", "bob"} {
		if _, ok := names[name]; !ok {
			t.Errorf("name %q absent from %v", name, names)
		}
	}

	badDir := t.TempDir()
	if err := os.WriteFile(filepath.Join(badDir, "bad.json"), []byte("{"), 0o644); err != nil {
		t.Fatal(err)
	}
	if _, err := (&JSONStore{dir: badDir}).CharacterNames(); err == nil || !strings.Contains(err.Error(), "indexar nomes") {
		t.Fatalf("corrupt account error=%v", err)
	}
}

func TestJSONStoreSaveAccountAsyncAndInstanceStatePaths(t *testing.T) {
	root := t.TempDir()
	st := NewJSONStore(filepath.Join(root, "accounts"))
	account := postgresTestAccount("async", "Async", model.Item{Index: 4011})
	if err := st.SaveAccountAsync(account); err != nil {
		t.Fatalf("async save: %v", err)
	}
	st.Flush()
	loaded, err := st.LoadAccount("async")
	if err != nil || loaded.Chars[0].Name != "Async" {
		t.Fatalf("async account=%+v err=%v", loaded, err)
	}
	if err := st.SaveAccountAsync(nil); err == nil {
		t.Fatal("nil async account was accepted")
	}

	if err := st.SaveInstanceState(nil); err != nil {
		t.Fatalf("default instance state save: %v", err)
	}
	state, err := st.LoadInstanceState()
	if err != nil || state.Version != model.InstanceStateVersion {
		t.Fatalf("instance state=%+v err=%v", state, err)
	}
	if err := st.SaveInstanceState(&model.InstanceStateSnapshot{Version: -1}); err == nil {
		t.Fatal("invalid instance-state version was accepted")
	}
	emptyPath := &JSONStore{instanceStatePath: ""}
	if got, err := emptyPath.LoadInstanceState(); err != nil || got.Version != model.InstanceStateVersion {
		t.Fatalf("empty instance-state path=%+v err=%v", got, err)
	}

	badRoot := filepath.Join(root, "not-a-directory")
	if err := os.WriteFile(badRoot, []byte("file"), 0o644); err != nil {
		t.Fatal(err)
	}
	badStore := &JSONStore{dir: badRoot, instanceStatePath: filepath.Join(badRoot, "state.json")}
	if err := badStore.SaveInstanceState(&model.InstanceStateSnapshot{Version: model.InstanceStateVersion}); err == nil {
		t.Fatal("instance-state write through a file path succeeded")
	}
}

func TestPostgresStoreReadAndWriteAdmissionValidation(t *testing.T) {
	closed := &PostgresStore{closed: true}
	if err := closed.Ping(context.Background()); err == nil {
		t.Fatal("closed store ping succeeded")
	}
	var nilStore *PostgresStore
	if err := nilStore.Ping(context.Background()); err == nil {
		t.Fatal("nil store ping succeeded")
	}

	if _, err := closed.ReadPersistentPlayer(nil, "user", "uid"); err == nil || !strings.Contains(err.Error(), "missing context") {
		t.Fatalf("nil persistent-player context error=%v", err)
	}
	if _, err := closed.ReadPersistentPlayer(context.Background(), " ", "uid"); err == nil {
		t.Fatal("blank account was accepted")
	}
	if _, err := closed.ReadPersistentPlayer(context.Background(), strings.Repeat("a", 13), "uid"); err == nil {
		t.Fatal("long account was accepted")
	}
	if _, err := closed.ReadPersistentPlayer(context.Background(), "user", "bad-uid"); err == nil {
		t.Fatal("invalid character UID was accepted")
	}
	if _, err := closed.ReadPersistentPlayer(context.Background(), "user", "aaaaaaaaaaaa4aaa8aaaaaaaaaaaaaaa"); err == nil || !strings.Contains(err.Error(), "closed") {
		t.Fatalf("closed persistent-player store error=%v", err)
	}

	if _, err := closed.ReadAccountDirectory(nil, "", "", 20); err == nil {
		t.Fatal("nil account-directory context was accepted")
	}
	for _, query := range []struct {
		search, cursor string
		limit          int
	}{{"bad-name", "", 20}, {"", "bad cursor", 20}, {"", "", 0}, {"", "", 51}} {
		if _, err := closed.ReadAccountDirectory(context.Background(), query.search, query.cursor, query.limit); err == nil {
			t.Errorf("invalid query was accepted: %+v", query)
		}
	}
	if _, err := closed.ReadAccountDirectory(context.Background(), "valid", "", 20); err == nil || !strings.Contains(err.Error(), "closed") {
		t.Fatalf("closed account-directory store error=%v", err)
	}
	if validDirectoryKey("", false) || !validDirectoryKey("", true) || validDirectoryKey("a_b", true) {
		t.Fatal("directory key validation accepted an invalid value")
	}

	readOnly := &PostgresStore{readOnly: true}
	for name, err := range map[string]error{
		"save account":          readOnly.SaveAccount(postgresTestAccount("user", "User", model.Item{})),
		"save account async":    readOnly.SaveAccountAsync(postgresTestAccount("user", "User", model.Item{})),
		"create account":        readOnly.CreateAccount(postgresTestAccount("user", "User", model.Item{})),
		"save accounts":         readOnly.SaveAccounts(),
		"save instance state":   readOnly.SaveInstanceState(nil),
		"save game state":       readOnly.SaveGameState(nil),
		"save char state":       readOnly.SaveCharState("bad", nil),
		"save char state async": readOnly.SaveCharStateAsync("bad", nil),
	} {
		if err == nil || !strings.Contains(err.Error(), "read-only") {
			t.Errorf("%s error=%v", name, err)
		}
	}
	if err := (&PostgresStore{}).SaveInstanceState(&model.InstanceStateSnapshot{Version: -1}); err == nil {
		t.Fatal("invalid PostgreSQL instance-state version was accepted")
	}
	if err := (&PostgresStore{closed: true}).SaveInstanceState(nil); err == nil || !strings.Contains(err.Error(), "closed") {
		t.Fatalf("closed instance-state writer error=%v", err)
	}
	if err := (&PostgresStore{closed: true}).SaveGameStateWithInstanceState(nil,
		&model.InstanceStateSnapshot{Version: -1}); err == nil || !strings.Contains(err.Error(), "closed") {
		t.Fatalf("closed game-state writer error=%v", err)
	}
	if err := (&PostgresStore{}).SaveAccounts(); err != nil {
		t.Fatalf("empty account batch should be a no-op: %v", err)
	}
	if err := (&PostgresStore{}).SaveGameStateWithInstanceState(nil, nil); err != nil {
		t.Fatalf("empty game-state batch should be a no-op: %v", err)
	}
	if err := (&PostgresStore{}).SaveGameStateWithInstanceState(&model.GuildRegistry{Version: -1}, nil); err == nil || !strings.Contains(err.Error(), "invalid guilds") {
		t.Fatalf("invalid guild state error=%v", err)
	}
	if err := (&PostgresStore{}).SaveGameStateWithInstanceState(nil, &model.InstanceStateSnapshot{Version: -1}); err == nil {
		t.Fatal("invalid instance state was accepted")
	}
	if err := (&PostgresStore{}).SaveCharState("bad", nil); err == nil {
		t.Fatal("invalid character UID was accepted")
	}
}
