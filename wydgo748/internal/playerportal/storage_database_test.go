package playerportal

import (
	"context"
	"errors"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

func TestStorageDatabaseReadsIsolatedAccountSnapshots(t *testing.T) {
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
	schema := fmt.Sprintf("portal_storage_%d", time.Now().UnixNano())
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
	if _, err := conn.Exec(ctx, `CREATE TABLE accounts(
		name_key text PRIMARY KEY,
		payload jsonb NOT NULL,
		created_at timestamptz NOT NULL DEFAULT statement_timestamp(),
		updated_at timestamptz NOT NULL DEFAULT statement_timestamp())`); err != nil {
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
	db := &Database{pool: pool}
	created := &model.Account{Name: "PortalCreated", PasswordHash: "opaque-test-hash", Chars: []model.Char{}}
	if err := db.CreateAccount(nil); err == nil {
		t.Fatal("nil registration was persisted")
	}
	if err := db.CreateAccount(&model.Account{Name: "PortalCreated", PasswordHash: "opaque-test-hash", Chars: []model.Char{{Name: "Knight"}}}); err == nil {
		t.Fatal("non-empty registration was persisted")
	}
	if err := db.CreateAccount(created); err != nil {
		t.Fatal(err)
	}
	if err := db.CreateAccount(created); !errors.Is(err, store.ErrAccountExists) {
		t.Fatalf("duplicate registration error=%v", err)
	}
	profile, err := db.Profile(ctx, "PORTALCREATED")
	if err != nil || profile.Username != created.Name || profile.Donate != 0 || profile.Characters == nil || len(profile.Characters) != 0 || profile.CreatedAt.IsZero() || profile.AsOf.Before(profile.CreatedAt) {
		t.Fatalf("new-account profile=%+v err=%v", profile, err)
	}
	if _, err := db.Profile(ctx, "Missing"); err == nil {
		t.Fatal("missing account produced a profile")
	}
	asOf := time.Date(2026, 11, 11, 12, 30, 0, 123000, time.UTC)
	payload := `{"name":"PortalStorage","cargoGold":4000000000,"cargo":[{"index":413,"eff":[43,9,61,8,0,0]},{"index":0}],"chars":[{"name":"Knight","score":{},"inv":[{"index":412,"eff":[61,7,0,0,0,0]},{"index":0}]}]}`
	if _, err := pool.Exec(ctx, "INSERT INTO accounts(name_key,payload,updated_at) VALUES('portalstorage',$1::jsonb,$2)", payload, asOf); err != nil {
		t.Fatal(err)
	}

	inventory, err := db.Inventory(ctx, "PORTALSTORAGE", 0)
	if err != nil {
		t.Fatal(err)
	}
	if inventory.Capacity != model.PlayerCarrySlots || inventory.CharacterSlot == nil || *inventory.CharacterSlot != 0 ||
		!inventory.AsOf.Equal(asOf) || len(inventory.Items) != 1 || inventory.Items[0] != (StoredItem{Slot: 0, ItemID: 412, Effects: [6]byte{61, 7}}) {
		t.Fatalf("inventory projection=%+v", inventory)
	}

	cargo, err := db.Cargo(ctx, "PortalStorage")
	if err != nil {
		t.Fatal(err)
	}
	if cargo.Capacity != model.PlayerCargoSlots || cargo.Gold == nil || *cargo.Gold != 4000000000 ||
		!cargo.AsOf.Equal(asOf) || len(cargo.Items) != 1 || cargo.Items[0] != (StoredItem{Slot: 0, ItemID: 413, Effects: [6]byte{43, 9, 61, 8}}) {
		t.Fatalf("cargo projection=%+v", cargo)
	}
	if _, err := db.Inventory(ctx, "PortalStorage", -1); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("negative slot error=%v", err)
	}
	if _, err := db.Inventory(ctx, "Missing", 0); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("missing inventory error=%v", err)
	}
	if _, err := db.Cargo(ctx, "Missing"); !errors.Is(err, os.ErrNotExist) {
		t.Fatalf("missing cargo error=%v", err)
	}
	cancelled, stop := context.WithCancel(ctx)
	stop()
	if _, err := db.Cargo(cancelled, "PortalStorage"); err == nil {
		t.Fatal("cancelled storage read succeeded")
	}
}
