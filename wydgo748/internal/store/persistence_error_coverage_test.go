package store

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func fileStoreForCoverage(t *testing.T, options ...Option) (*JSONStore, string) {
	t.Helper()
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0700); err != nil {
		t.Fatal(err)
	}
	return NewJSONStore(accounts, options...), root
}

func TestLoadGuildsRejectsInvalidFilesAndMissingPath(t *testing.T) {
	for _, test := range []struct{ name, contents string }{
		{name: "malformed JSON", contents: "{"},
		{name: "unknown field", contents: `{"version":1,"unknown":true}`},
		{name: "trailing JSON", contents: `{"version":1,"guilds":[]} {}`},
		{name: "invalid registry", contents: `{"version":1,"guilds":[{"id":0}]}`},
	} {
		t.Run(test.name, func(t *testing.T) {
			st, root := fileStoreForCoverage(t)
			if err := os.WriteFile(filepath.Join(root, "guilds.json"), []byte(test.contents), 0600); err != nil {
				t.Fatal(err)
			}
			if _, err := st.LoadGuilds(); err == nil {
				t.Fatal("accepted invalid guild registry")
			}
		})
	}
	st, _ := fileStoreForCoverage(t, WithGuildsPath(""))
	if _, err := st.LoadGuilds(); err == nil {
		t.Fatal("loaded guilds without a configured path")
	}
}

func TestLoadInstanceStateRejectsMalformedUnknownTrailingAndWrongVersion(t *testing.T) {
	for _, test := range []struct{ name, contents string }{
		{name: "malformed JSON", contents: "{"},
		{name: "unknown field", contents: `{"version":1,"unknown":true}`},
		{name: "trailing JSON", contents: `{"version":1} {}`},
		{name: "wrong version", contents: `{"version":-1}`},
	} {
		t.Run(test.name, func(t *testing.T) {
			st, root := fileStoreForCoverage(t)
			if err := os.WriteFile(filepath.Join(root, "instance_state.json"), []byte(test.contents), 0600); err != nil {
				t.Fatal(err)
			}
			if _, err := st.LoadInstanceState(); err == nil {
				t.Fatal("accepted invalid instance state")
			}
		})
	}
	st, root := fileStoreForCoverage(t)
	if err := os.Mkdir(filepath.Join(root, "instance_state.json"), 0700); err != nil {
		t.Fatal(err)
	}
	if _, err := st.LoadInstanceState(); err == nil {
		t.Fatal("accepted an unreadable instance state path")
	}
}

func TestSaveGameStateRejectsInvalidTransactionalInputs(t *testing.T) {
	st, _ := fileStoreForCoverage(t)
	if err := st.SaveGameState(nil); err != nil {
		t.Fatalf("empty transaction should be a no-op: %v", err)
	}
	if err := st.SaveGameStateWithInstanceState(nil, &model.InstanceStateSnapshot{Version: -1}); err == nil {
		t.Fatal("accepted invalid instance state version")
	}
	if err := st.SaveGameState(&model.GuildRegistry{Version: -1}); err == nil {
		t.Fatal("accepted invalid guild registry")
	}
	withoutGuildPath, _ := fileStoreForCoverage(t, WithGuildsPath(""))
	if err := withoutGuildPath.SaveGameState(sampleGuild()); err == nil {
		t.Fatal("saved guild registry without a configured path")
	}
	for _, test := range []struct {
		name     string
		accounts []*model.Account
	}{
		{name: "nil account", accounts: []*model.Account{nil}},
		{name: "unsafe account name", accounts: []*model.Account{{Name: "../outside", PasswordHash: "hash"}}},
		{name: "duplicate case-insensitive account", accounts: []*model.Account{
			{Name: "same", PasswordHash: "hash"}, {Name: "SAME", PasswordHash: "hash"},
		}},
	} {
		t.Run(test.name, func(t *testing.T) {
			if err := st.SaveAccounts(test.accounts...); err == nil {
				t.Fatal("accepted invalid transaction accounts")
			}
		})
	}
}

func TestLoadGuildsValidRegistryAndDirectoryError(t *testing.T) {
	registry := sampleGuild()
	contents, err := json.Marshal(registry)
	if err != nil {
		t.Fatal(err)
	}
	st, root := fileStoreForCoverage(t)
	path := filepath.Join(root, "guilds.json")
	if err := os.WriteFile(path, contents, 0600); err != nil {
		t.Fatal(err)
	}
	loaded, err := st.LoadGuilds()
	if err != nil || loaded.Guilds[0].ID != registry.Guilds[0].ID {
		t.Fatalf("valid guild registry=%+v err=%v", loaded, err)
	}
	if err := os.Remove(path); err != nil {
		t.Fatal(err)
	}
	if err := os.Mkdir(path, 0700); err != nil {
		t.Fatal(err)
	}
	if _, err := st.LoadGuilds(); err == nil {
		t.Fatal("accepted a directory as guild registry")
	}
}
