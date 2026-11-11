package store

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/authlimit"
	"wydgo/internal/model"
)

func TestJSONStoreAccountPathHandlesCaseAndMissingDirectory(t *testing.T) {
	dir := t.TempDir()
	legacy := &model.Account{Name: "Player", PasswordHash: "test-hash", Chars: []model.Char{}}
	body, err := json.Marshal(legacy)
	if err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(dir, "Player.JSON"), body, 0o600); err != nil {
		t.Fatal(err)
	}
	loaded, err := (&JSONStore{dir: dir}).LoadAccount("player")
	if err != nil || loaded.Name != "Player" {
		t.Fatalf("case-insensitive account lookup=%+v err=%v", loaded, err)
	}
	if err := os.Mkdir(filepath.Join(dir, "Directory.json"), 0o700); err != nil {
		t.Fatal(err)
	}
	if _, err := (&JSONStore{dir: dir}).LoadAccount("directory"); err == nil {
		t.Fatal("directory was accepted as an account file")
	}
	missing := filepath.Join(dir, "missing")
	if _, err := (&JSONStore{dir: missing}).LoadAccount("player"); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("missing account directory error=%v", err)
	}
	if _, err := (&JSONStore{dir: dir}).accountPath("bad\x00name"); err == nil || errors.Is(err, os.ErrNotExist) {
		t.Fatalf("invalid path stat error=%v", err)
	}
	blocker := filepath.Join(dir, "not-a-directory")
	if err := os.WriteFile(blocker, []byte("file"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := (&JSONStore{dir: blocker}).accountPath("missing"); err == nil {
		t.Fatal("account lookup ignored a directory read failure")
	}
}

func TestDurableFileHelpersCleanFailedRenames(t *testing.T) {
	root := t.TempDir()
	atomicTarget := filepath.Join(root, "atomic-target")
	if err := os.Mkdir(atomicTarget, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := writeFileAtomic(atomicTarget, []byte("replacement")); err == nil {
		t.Fatal("atomic replacement of a directory succeeded")
	}
	atomicEntries, err := os.ReadDir(root)
	if err != nil || len(atomicEntries) != 1 || atomicEntries[0].Name() != "atomic-target" {
		t.Fatalf("atomic rename failure left a temporary file: entries=%v err=%v", atomicEntries, err)
	}

	accountDir := filepath.Join(root, "accounts")
	if err := os.Mkdir(accountDir, 0o700); err != nil {
		t.Fatal(err)
	}
	accountTarget := filepath.Join(accountDir, "player.json")
	if err := os.Mkdir(accountTarget, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := (&JSONStore{dir: accountDir}).writeAccountFile(accountTarget, []byte("replacement")); err == nil {
		t.Fatal("account replacement of a directory succeeded")
	}
	accountEntries, err := os.ReadDir(accountDir)
	if err != nil || len(accountEntries) != 1 || accountEntries[0].Name() != "player.json" {
		t.Fatalf("account rename failure left a temporary file: entries=%v err=%v", accountEntries, err)
	}

	syncedPath := filepath.Join(root, "synced.json")
	if err := writeSyncedFile(syncedPath, []byte("durable"), 0o600); err != nil {
		t.Fatal(err)
	}
	if got, err := os.ReadFile(syncedPath); err != nil || string(got) != "durable" {
		t.Fatalf("synced file=%q err=%v", got, err)
	}
}

func TestPostgresAuthenticationLimiterRetainsItsPool(t *testing.T) {
	pool := &pgxpool.Pool{}
	limiter, ok := (&PostgresStore{pool: pool}).AuthenticationLimiter().(authlimit.Postgres)
	if !ok || limiter.Pool != pool {
		t.Fatalf("authentication limiter=%T pool=%p", limiter, limiter.Pool)
	}
}

func TestResetLoadtestAccountsDeletesOnlyExactBotNames(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	ctx, cancel := context.WithTimeout(context.Background(), 10*time.Second)
	defer cancel()
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	schema := fmt.Sprintf("loadtest_reset_%d", time.Now().UnixNano())
	if _, err := conn.Exec(ctx, "CREATE SCHEMA "+schema); err != nil {
		conn.Close(context.Background())
		t.Fatal(err)
	}
	defer func() {
		_, _ = conn.Exec(context.Background(), "DROP SCHEMA "+schema+" CASCADE")
		_ = conn.Close(context.Background())
	}()
	if _, err := conn.Exec(ctx, "SET search_path TO "+schema); err != nil {
		t.Fatal(err)
	}
	if _, err := conn.Exec(ctx, `CREATE TABLE accounts(name_key text PRIMARY KEY)`); err != nil {
		t.Fatal(err)
	}
	cfg, err := pgxpool.ParseConfig(url)
	if err != nil {
		t.Fatal(err)
	}
	cfg.ConnConfig.RuntimeParams["search_path"] = schema
	pool, err := pgxpool.NewWithConfig(ctx, cfg)
	if err != nil {
		t.Fatal(err)
	}
	defer pool.Close()
	for _, name := range []string{"bot0001", "bot0990", "bot01000", "bot0001x", "other"} {
		if _, err := pool.Exec(ctx, "INSERT INTO accounts(name_key) VALUES($1)", name); err != nil {
			t.Fatal(err)
		}
	}
	store := &PostgresStore{pool: pool}
	if _, err := (&PostgresStore{readOnly: true}).ResetLoadtestAccounts(ctx, "bot"); err == nil {
		t.Fatal("read-only account reset was accepted")
	}
	for _, prefix := range []string{"", "user", "boat", "bot-evil", "123456789"} {
		if _, err := store.ResetLoadtestAccounts(ctx, prefix); err == nil {
			t.Errorf("unsafe reset prefix %q was accepted", prefix)
		}
	}
	removed, err := store.ResetLoadtestAccounts(ctx, " BOT ")
	if err != nil || removed != 2 {
		t.Fatalf("reset removed=%d err=%v", removed, err)
	}
	var preserved int
	if err := pool.QueryRow(ctx, `SELECT count(*) FROM accounts`).Scan(&preserved); err != nil || preserved != 3 {
		t.Fatalf("non-exact bot names were removed: count=%d err=%v", preserved, err)
	}
	if _, err := pool.Exec(ctx, `INSERT INTO accounts(name_key) VALUES('bot0002')`); err != nil {
		t.Fatal(err)
	}
	removed, err = store.ResetLoadtestAccounts(nil, "bot")
	if err != nil || removed != 1 {
		t.Fatalf("reset with nil context removed=%d err=%v", removed, err)
	}
}
