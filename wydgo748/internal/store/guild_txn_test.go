package store

import (
	"encoding/json"
	"os"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestGuildWarFameAndCompletionPersistTogether(t *testing.T) {
	s, _, _ := newTestStore(t)
	r := sampleGuild()
	r.Guilds[0].Fame = 100
	r.Wars.Tower = model.TowerWarState{Day: "2026-09-14", EndsAt: time.Date(2026, 9, 15, 0, 35, 0, 0, time.UTC), Owner: 1, Started: true, Finished: true, Notices: 15}
	if err := s.SaveGameState(r); err != nil {
		t.Fatal(err)
	}
	got, err := s.LoadGuilds()
	if err != nil {
		t.Fatal(err)
	}
	if got.Guilds[0].Fame != 100 || got.Wars.Tower != r.Wars.Tower {
		t.Fatal("fame/completion split on reload")
	}
	got.Wars.Cities.Territories[0].Owner = 1
	got.Wars.Cities.Territories[0].Treasury = 5_500_000_000
	if err := s.SaveGameState(got); err != nil {
		t.Fatal(err)
	}
	again, err := s.LoadGuilds()
	if err != nil {
		t.Fatal(err)
	}
	if again.Guilds[0].Fame != 100 || !again.Wars.Tower.Finished || again.Wars.Cities.Territories[0] != got.Wars.Cities.Territories[0] {
		t.Fatal("state lost")
	}
}

func TestKefraDefeatPersistsWithGuildFame(t *testing.T) {
	s, _, _ := newTestStore(t)
	r := sampleGuild()
	r.Guilds[0].Fame = 100
	r.Wars.Kefra = model.KefraState{Defeated: true, Guild: r.Guilds[0].ID,
		DefeatedAt: time.Date(2026, 9, 14, 21, 0, 0, 0, time.UTC)}
	if err := s.SaveGameState(r); err != nil {
		t.Fatal(err)
	}
	got, err := s.LoadGuilds()
	if err != nil {
		t.Fatal(err)
	}
	if got.Guilds[0].Fame != 100 || got.Wars.Kefra.Guild != r.Wars.Kefra.Guild ||
		!got.Wars.Kefra.Defeated || !got.Wars.Kefra.DefeatedAt.Equal(r.Wars.Kefra.DefeatedAt) {
		t.Fatalf("Kefra defeat lost on reload: %+v", got.Wars.Kefra)
	}
}

// newTestStore builds a store with an accounts dir and a sibling guilds.json,
// the real layout (guilds.json must NOT live inside the accounts dir, or
// AccountNameExists/CharacterNameExists try to parse it as an account).
func newTestStore(t *testing.T) (*JSONStore, string, string) {
	t.Helper()
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	guilds := filepath.Join(root, "guilds.json")
	return NewJSONStore(accounts, WithGuildsPath(guilds)), accounts, guilds
}

func sampleGuild() *model.GuildRegistry {
	return &model.GuildRegistry{
		Version: model.GuildRegistryVersion,
		Guilds: []model.Guild{{
			ID:   1,
			Name: "Alfa",
			Members: []model.GuildMember{
				{Character: "Felipe", Account: "account1", Rank: model.GuildRankLeader},
			},
		}},
	}
}

func TestDefaultGuildsPathStaysOutsideAccountsDirectory(t *testing.T) {
	got := defaultGuildsPath(filepath.Join("data", "accounts"))
	want := filepath.Join("data", "guilds.json")
	if got != want {
		t.Fatalf("guilds.json should sit next to the accounts dir: got=%q want=%q", got, want)
	}
}

func TestSaveGameStatePersistsGuildAndAccountTogether(t *testing.T) {
	s, accounts, guildsPath := newTestStore(t)
	acc := &model.Account{Name: "account1", PasswordHash: "hash"}

	if err := s.SaveGameState(sampleGuild(), acc); err != nil {
		t.Fatalf("SaveGameState: %v", err)
	}
	if _, err := os.Stat(filepath.Join(accounts, "account1.json")); err != nil {
		t.Fatalf("account not persisted: %v", err)
	}
	if _, err := os.Stat(guildsPath); err != nil {
		t.Fatalf("guilds.json not persisted: %v", err)
	}
	// guilds.json must not have landed inside the accounts dir.
	if _, err := os.Stat(filepath.Join(accounts, "guilds.json")); !os.IsNotExist(err) {
		t.Fatal("guilds.json leaked into the accounts directory")
	}
	registry, err := s.LoadGuilds()
	if err != nil {
		t.Fatalf("LoadGuilds: %v", err)
	}
	if len(registry.Guilds) != 1 || registry.Guilds[0].Name != "Alfa" {
		t.Fatalf("wrong reloaded registry: %+v", registry)
	}
}

func TestSaveGameStateGuildExportFailureDoesNotInvalidateCommit(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	guildsPath := filepath.Join(root, "guilds.json")
	blocker := filepath.Join(root, "not-a-directory")
	if err := os.WriteFile(blocker, []byte("blocker"), 0o644); err != nil {
		t.Fatal(err)
	}
	s := NewJSONStore(
		accounts,
		WithGuildsPath(guildsPath),
		WithGuildsTxtPath(filepath.Join(blocker, "Guilds.txt")),
	)
	acc := &model.Account{Name: "account1", PasswordHash: "hash"}

	if err := s.SaveGameState(sampleGuild(), acc); err != nil {
		t.Fatalf("a derived-artifact failure must not deny an authoritative commit: %v", err)
	}
	if _, err := s.LoadAccount("account1"); err != nil {
		t.Fatalf("the committed account was not persisted: %v", err)
	}
	registry, err := s.LoadGuilds()
	if err != nil {
		t.Fatalf("the committed guild was not persisted: %v", err)
	}
	if len(registry.Guilds) != 1 || registry.Guilds[0].Name != "Alfa" {
		t.Fatalf("wrong authoritative registry after the Guilds.txt failure: %+v", registry)
	}
}

func TestLoadGuildsWithoutFileReturnsEmptyRegistry(t *testing.T) {
	s, _, _ := newTestStore(t)
	registry, err := s.LoadGuilds()
	if err != nil {
		t.Fatalf("a new server must not fail for lack of guilds.json: %v", err)
	}
	if registry.Version != model.GuildRegistryVersion || len(registry.Guilds) != 0 {
		t.Fatalf("expected an empty registry at the current version: %+v", registry)
	}
}

// writeJournal builds a journal by hand to simulate a process crash.
func writeJournal(t *testing.T, accounts string, manifest *txnManifest, files map[string][]byte, commit bool) string {
	t.Helper()
	txnDir, err := os.MkdirTemp(accounts, ".account-txn-")
	if err != nil {
		t.Fatal(err)
	}
	for name, data := range files {
		if err := os.WriteFile(filepath.Join(txnDir, name), data, 0o644); err != nil {
			t.Fatal(err)
		}
	}
	if manifest != nil {
		b, err := json.Marshal(manifest)
		if err != nil {
			t.Fatal(err)
		}
		if err := os.WriteFile(filepath.Join(txnDir, txnManifestName), b, 0o644); err != nil {
			t.Fatal(err)
		}
	}
	if commit {
		if err := os.WriteFile(filepath.Join(txnDir, txnCommitName), []byte("commit\n"), 0o644); err != nil {
			t.Fatal(err)
		}
	}
	return txnDir
}

func TestRecoveryAppliesCommittedJournal(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	guildsPath := filepath.Join(root, "guilds.json")

	accBytes, _ := json.Marshal(&model.Account{Name: "account1", PasswordHash: "hash"})
	guildBytes, _ := json.Marshal(sampleGuild())
	writeJournal(t, accounts, &txnManifest{
		Version: txnManifestVer,
		Entries: []txnEntry{
			{File: "account1.json", Kind: txnKindAccount, Name: "account1"},
			{File: "guilds.registry.json", Kind: txnKindGuilds},
		},
	}, map[string][]byte{
		"account1.json":        accBytes,
		"guilds.registry.json": guildBytes,
	}, true)

	s := NewJSONStore(accounts, WithGuildsPath(guildsPath))
	if s.initErr != nil {
		t.Fatalf("recovery failed: %v", s.initErr)
	}
	if _, err := os.Stat(filepath.Join(accounts, "account1.json")); err != nil {
		t.Fatalf("account not recovered: %v", err)
	}
	if _, err := os.Stat(guildsPath); err != nil {
		t.Fatalf("guilds.json not recovered: %v", err)
	}
}

func TestRecoveryDiscardsUncommittedJournal(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	guildsPath := filepath.Join(root, "guilds.json")

	accBytes, _ := json.Marshal(&model.Account{Name: "account1", PasswordHash: "hash"})
	txnDir := writeJournal(t, accounts, &txnManifest{
		Version: txnManifestVer,
		Entries: []txnEntry{{File: "account1.json", Kind: txnKindAccount, Name: "account1"}},
	}, map[string][]byte{"account1.json": accBytes}, false)

	s := NewJSONStore(accounts, WithGuildsPath(guildsPath))
	if s.initErr != nil {
		t.Fatalf("a journal without COMMIT must be discarded without an error: %v", s.initErr)
	}
	if _, err := os.Stat(filepath.Join(accounts, "account1.json")); !os.IsNotExist(err) {
		t.Fatal("a journal without COMMIT must not be applied")
	}
	if _, err := os.Stat(txnDir); !os.IsNotExist(err) {
		t.Fatal("a journal without COMMIT should have been removed")
	}
}

// A journal written before the manifest existed must stay recoverable.
func TestRecoveryHandlesLegacyJournalWithoutManifest(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	accBytes, _ := json.Marshal(&model.Account{Name: "account1", PasswordHash: "hash"})
	writeJournal(t, accounts, nil, map[string][]byte{"account1.json": accBytes}, true)

	s := NewJSONStore(accounts, WithGuildsPath(filepath.Join(root, "guilds.json")))
	if s.initErr != nil {
		t.Fatalf("a legacy journal must be recovered: %v", s.initErr)
	}
	if _, err := os.Stat(filepath.Join(accounts, "account1.json")); err != nil {
		t.Fatalf("account of the legacy journal not recovered: %v", err)
	}
}

func TestRecoveryRejectsUnknownDestinationType(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	writeJournal(t, accounts, &txnManifest{
		Version: txnManifestVer,
		Entries: []txnEntry{{File: "x.json", Kind: "unknown-kind", Name: "x"}},
	}, map[string][]byte{"x.json": []byte("{}")}, true)

	s := NewJSONStore(accounts, WithGuildsPath(filepath.Join(root, "guilds.json")))
	if s.initErr == nil {
		t.Fatal("an unknown destination type should fail explicitly")
	}
}

// The destination comes from the TYPE, but the name still builds the account
// path: a name with a traversal must be refused before any write.
func TestRecoveryRejectsPathsOutsideRoot(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	for _, name := range []string{"../outside", "sub/account", ".."} {
		writeJournal(t, accounts, &txnManifest{
			Version: txnManifestVer,
			Entries: []txnEntry{{File: "x.json", Kind: txnKindAccount, Name: name}},
		}, map[string][]byte{"x.json": []byte("{}")}, true)

		s := NewJSONStore(accounts, WithGuildsPath(filepath.Join(root, "guilds.json")))
		if s.initErr == nil {
			t.Fatalf("account name %q should be refused", name)
		}
		if _, err := os.Stat(filepath.Join(root, "outside.json")); !os.IsNotExist(err) {
			t.Fatalf("wrote outside the root with name %q", name)
		}
		// clears the invalid journal for the next case
		entries, _ := os.ReadDir(accounts)
		for _, e := range entries {
			if e.IsDir() {
				_ = os.RemoveAll(filepath.Join(accounts, e.Name()))
			}
		}
	}
}

// An invalid entry in the middle of the manifest must not leave the earlier
// ones applied: resolution happens entirely before the first write.
func TestInvalidManifestDoesNotApplyPartialChanges(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	accBytes, _ := json.Marshal(&model.Account{Name: "account1", PasswordHash: "hash"})
	writeJournal(t, accounts, &txnManifest{
		Version: txnManifestVer,
		Entries: []txnEntry{
			{File: "account1.json", Kind: txnKindAccount, Name: "account1"},
			{File: "y.json", Kind: "invalid", Name: "y"},
		},
	}, map[string][]byte{"account1.json": accBytes, "y.json": []byte("{}")}, true)

	s := NewJSONStore(accounts, WithGuildsPath(filepath.Join(root, "guilds.json")))
	if s.initErr == nil {
		t.Fatal("a manifest with an invalid entry should fail")
	}
	if _, err := os.Stat(filepath.Join(accounts, "account1.json")); !os.IsNotExist(err) {
		t.Fatal("no entry could have been applied")
	}
}

// The 7.48 client reads "./Guilds.txt" locally (WYD.exe opens it "rb" and
// parses %d %d %d %s). Without that file no guild name shows, so the export
// must go out with every registry change.
func TestSaveGameStateExportsGuildsTxtInNativeFormat(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	txtPath := filepath.Join(root, "Guilds.txt")
	s := NewJSONStore(accounts,
		WithGuildsPath(filepath.Join(root, "guilds.json")),
		WithGuildsTxtPath(txtPath))

	if err := s.SaveGameState(sampleGuild()); err != nil {
		t.Fatalf("SaveGameState: %v", err)
	}
	data, err := os.ReadFile(txtPath)
	if err != nil {
		t.Fatalf("Guilds.txt not exported: %v", err)
	}
	// group=0 channel=0 index=1 name=Alfa
	if got := strings.TrimSpace(string(data)); got != "0 0 1 Alfa" {
		t.Fatalf("wrong native format: %q", got)
	}
}

// The export is a full rewrite, not an append: a dissolved guild must leave
// the file, or the client keeps showing the name of a guild that no longer exists.
func TestExportGuildsTxtRewritesEntireFile(t *testing.T) {
	root := t.TempDir()
	accounts := filepath.Join(root, "accounts")
	if err := os.MkdirAll(accounts, 0o755); err != nil {
		t.Fatal(err)
	}
	txtPath := filepath.Join(root, "Guilds.txt")
	s := NewJSONStore(accounts,
		WithGuildsPath(filepath.Join(root, "guilds.json")),
		WithGuildsTxtPath(txtPath))

	twoGuilds := &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{
		{ID: 1, Name: "Alfa", Members: []model.GuildMember{
			{Character: "A", Account: "c1", Rank: model.GuildRankLeader}}},
		{ID: 2, Name: "Beta", Members: []model.GuildMember{
			{Character: "B", Account: "c2", Rank: model.GuildRankLeader}}},
	}}
	if err := s.SaveGameState(twoGuilds); err != nil {
		t.Fatal(err)
	}
	// dissolves Beta
	twoGuilds.Guilds = twoGuilds.Guilds[:1]
	if err := s.SaveGameState(twoGuilds); err != nil {
		t.Fatal(err)
	}
	data, _ := os.ReadFile(txtPath)
	if strings.Contains(string(data), "Beta") {
		t.Fatalf("a dissolved guild stayed in Guilds.txt: %q", string(data))
	}
}

// A disabled export must not break authoritative persistence.
func TestSaveGameStateWorksWithoutExport(t *testing.T) {
	s, _, _ := newTestStore(t) // without WithGuildsTxtPath
	if err := s.SaveGameState(sampleGuild()); err != nil {
		t.Fatalf("a disabled export should not affect the save: %v", err)
	}
}

func TestSaveGameStateRejectsInvalidRegistry(t *testing.T) {
	s, _, _ := newTestStore(t)
	// two guilds with the same name differing only in case
	registry := &model.GuildRegistry{
		Version: model.GuildRegistryVersion,
		Guilds: []model.Guild{
			{ID: 1, Name: "Alfa", Members: []model.GuildMember{
				{Character: "A", Account: "c1", Rank: model.GuildRankLeader}}},
			{ID: 2, Name: "ALFA", Members: []model.GuildMember{
				{Character: "B", Account: "c2", Rank: model.GuildRankLeader}}},
		},
	}
	if err := s.SaveGameState(registry); err == nil {
		t.Fatal("a duplicate name ignoring case should be refused")
	}
}
