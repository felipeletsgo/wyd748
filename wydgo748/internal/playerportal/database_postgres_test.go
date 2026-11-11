package playerportal

import (
	"context"
	"encoding/json"
	"fmt"
	"net/netip"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/account"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

func loopbackPortalTestURL(t *testing.T) string {
	t.Helper()
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL must name an isolated loopback database")
	}
	cfg, err := pgxpool.ParseConfig(url)
	if err != nil {
		t.Fatalf("parse test database URL: %v", err)
	}
	if address, err := netip.ParseAddr(cfg.ConnConfig.Host); err != nil || !address.IsLoopback() {
		if !strings.EqualFold(cfg.ConnConfig.Host, "localhost") {
			t.Skip("database integration tests require loopback")
		}
	}
	return url
}

func TestOpenDatabaseRejectsInvalidConfiguration(t *testing.T) {
	if _, err := OpenDatabase(context.Background(), "not a postgres URL"); err == nil {
		t.Fatal("OpenDatabase() error = nil, want configuration error")
	}
}

func TestPortalDatabaseReadsAndRegistersAccountsOnIsolatedPostgres(t *testing.T) {
	url := loopbackPortalTestURL(t)
	ctx, cancel := context.WithTimeout(context.Background(), 8*time.Second)
	defer cancel()
	writer, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: url, MaxConns: 2})
	if err != nil {
		t.Fatalf("open isolated writer store: %v", err)
	}
	defer writer.Close()

	db, err := OpenDatabase(ctx, url)
	if err != nil {
		t.Fatalf("OpenDatabase(loopback test DB): %v", err)
	}
	defer db.Close()
	if err := db.Ping(ctx); err != nil {
		t.Fatalf("Ping(): %v", err)
	}

	name := fmt.Sprintf("portaltest%d", time.Now().UTC().UnixNano())
	defer func() {
		_, _ = db.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(name))
	}()
	if exists, err := db.AccountNameExists(name); err != nil || exists {
		t.Fatalf("AccountNameExists(before)=(%v,%v), want (false,nil)", exists, err)
	}
	registeredAccount := &model.Account{Name: name, PasswordHash: "test-only-hash"}
	if err := db.CreateAccount(registeredAccount); err != nil {
		t.Fatalf("CreateAccount(): %v", err)
	}
	if err := db.CreateAccount(registeredAccount); err != store.ErrAccountExists {
		t.Fatalf("CreateAccount(duplicate)=%v, want %v", err, store.ErrAccountExists)
	}
	if exists, err := db.AccountNameExists(strings.ToUpper(name)); err != nil || !exists {
		t.Fatalf("AccountNameExists(after)=(%v,%v), want (true,nil)", exists, err)
	}
	loaded, err := db.LoadAccount(strings.ToUpper(name))
	if err != nil || loaded.Name != name || loaded.PasswordHash != registeredAccount.PasswordHash || len(loaded.Chars) != 0 {
		t.Fatalf("LoadAccount()=%+v err=%v", loaded, err)
	}
	profile, err := db.Profile(ctx, strings.ToUpper(name))
	if err != nil || profile.Username != name || len(profile.Characters) != 0 || profile.AsOf.IsZero() {
		t.Fatalf("Profile()=%+v err=%v", profile, err)
	}
	ranking, err := db.Ranking(ctx)
	if err != nil || len(ranking) > 100 {
		t.Fatalf("Ranking() returned %d rows, err=%v", len(ranking), err)
	}
	publicGuilds, err := db.PublicGuilds(ctx)
	if err != nil || publicGuilds.Version != model.GuildRegistryVersion || publicGuilds.AsOf.IsZero() {
		t.Fatalf("PublicGuilds()=%+v err=%v", publicGuilds, err)
	}

	characterName := fmt.Sprintf("p%010d", time.Now().UTC().UnixNano()%10_000_000_000)
	characterAccount, err := account.Create(db, characterName, "Safe1234", "Safe1234")
	if err != nil {
		t.Fatalf("create character detail fixture: %v", err)
	}
	defer func() {
		_, _ = db.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(characterName))
	}()
	characterUID, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	characterAccount.Chars = []model.Char{{
		UID: characterUID, Name: "PortalKnight", Class: 1, Evolution: "mortal",
		Score: &model.Score{Version: model.ScoreVersion, Level: 120},
		Exp:   12345, Gold: 67890, CP: -12,
	}}
	itemUID := strings.Repeat("a", 32)
	characterAccount.Chars[0].Equip[1] = model.Item{Index: 413, UID: itemUID, Eff: [6]byte{43, 9}}
	if err := writer.SaveAccount(characterAccount); err != nil {
		t.Fatalf("save character detail fixture: %v", err)
	}
	detail, err := db.CharacterDetail(ctx, strings.ToUpper(characterName), 0)
	if err != nil || detail.Slot != 0 || detail.Name != "PortalKnight" || detail.Level != 120 ||
		detail.Exp != 12345 || detail.Gold != 67890 || detail.CP != -12 || detail.AsOf.IsZero() ||
		len(detail.Equipment) != 1 || detail.Equipment[0] != (EquipmentItem{
		Slot: 1, ItemID: 413, Effects: [6]byte{43, 9},
	}) {
		t.Fatalf("CharacterDetail()=%+v err=%v", detail, err)
	}
	encodedDetail, err := json.Marshal(detail)
	if err != nil {
		t.Fatal(err)
	}
	for _, private := range []string{characterUID, itemUID, "password", "rawEvolution", "inventory"} {
		if strings.Contains(string(encodedDetail), private) {
			t.Fatalf("character detail exposed private field %q: %s", private, encodedDetail)
		}
	}
}

func TestOpenDatabaseClosesPartiallyOpenedPoolWhenReadOnlyStoreFails(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	if _, err := OpenDatabase(ctx, "postgres://test:test@127.0.0.1:5432/wydgo_test"); err == nil {
		t.Fatal("OpenDatabase(canceled startup) error = nil, want unavailable error")
	}
}
