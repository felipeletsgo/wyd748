package playerportal

import (
	"context"
	"encoding/json"
	"errors"
	"os"
	"strings"
	"sync"
	"sync/atomic"
	"testing"

	"github.com/jackc/pgx/v5/pgconn"
	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/account"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

// This gate intentionally refuses the production database. It exercises the
// real registration role and the same privilege script used in deployment.
func TestPlayerDatabaseIntegration(t *testing.T) {
	ownerURL, playerURL := os.Getenv("WYD_TEST_PLAYER_OWNER_URL"), os.Getenv("WYD_TEST_PLAYER_DATABASE_URL")
	if ownerURL == "" || playerURL == "" {
		t.Skip("dedicated player database fixture is not configured")
	}
	for _, raw := range []string{ownerURL, playerURL} {
		cfg, err := pgxpool.ParseConfig(raw)
		if err != nil || cfg.ConnConfig.Database != "wyd_portal_validation" {
			t.Fatal("integration requires the dedicated wyd_portal_validation database")
		}
	}
	ctx := context.Background()
	writer, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: ownerURL, MaxConns: 2})
	if err != nil {
		t.Fatal(err)
	}
	defer writer.Close()
	owner, err := pgxpool.New(ctx, ownerURL)
	if err != nil {
		t.Fatal(err)
	}
	defer owner.Close()
	privileges, err := os.ReadFile(os.Getenv("WYD_TEST_PLAYER_PRIVILEGES"))
	if err != nil {
		t.Fatal(err)
	}
	if _, err := owner.Exec(ctx, string(privileges)); err != nil {
		t.Fatal(err)
	}
	db, err := OpenDatabase(ctx, playerURL)
	if err != nil {
		t.Fatal(err)
	}
	defer db.Close()
	if err := db.Ping(ctx); err != nil {
		t.Fatal(err)
	}

	a, err := account.Create(db, "WebPlayer", "Safe1234", "Safe1234")
	if err != nil {
		t.Fatal(err)
	}
	if _, err := account.Authenticate(writer, "webplayer", "Safe1234"); err != nil {
		t.Fatalf("native game authentication rejected web registration: %v", err)
	}
	for _, name := range []string{"WebPlayer", "WEBPLAYER"} {
		if _, err := account.Create(db, name, "Safe1234", "Safe1234"); !errors.Is(err, account.ErrUsernameUnavailable) {
			t.Fatalf("duplicate was not rejected: %v", err)
		}
	}
	if _, err := account.Authenticate(db, "WebPlayer", "Bad1234"); !errors.Is(err, account.ErrInvalidCredentials) {
		t.Fatal("wrong password accepted")
	}
	p, err := db.Profile(ctx, "WebPlayer")
	if err != nil || len(p.Characters) != 0 || p.Donate != 0 {
		t.Fatalf("new account is not empty: %+v %v", p, err)
	}

	// A compromised web database credential cannot modify the economy or schema.
	for _, query := range []string{
		`UPDATE accounts SET payload=payload WHERE name_key='webplayer'`,
		`DELETE FROM accounts WHERE name_key='webplayer'`,
		`SELECT * FROM characters`,
		`CREATE TABLE public.player_escape(id int)`,
		`SET ROLE wyd_game`,
	} {
		if _, err := db.pool.Exec(ctx, query); err == nil {
			t.Fatalf("registration role accepted forbidden operation: %s", query)
		}
	}
	for _, mutation := range []string{
		`jsonb_set($1::jsonb,'{donate}','1'::jsonb)`,
		`jsonb_set($1::jsonb,'{cargoGold}','1'::jsonb)`,
		`jsonb_set($1::jsonb,'{chars}','[{}]'::jsonb)`,
		`jsonb_set($1::jsonb,'{cargo,0,index}','4011'::jsonb)`,
		`jsonb_set($1::jsonb,'{cargo,0,uid}','"forged"'::jsonb)`,
		`jsonb_set($1::jsonb,'{staff}','true'::jsonb)`,
	} {
		invalid := *a
		invalid.Name = "BadPlayer"
		payload, _ := json.Marshal(&invalid)
		_, err := db.pool.Exec(ctx, "INSERT INTO accounts(name_key,payload) VALUES('badplayer',"+mutation+")", string(payload))
		var pgerr *pgconn.PgError
		if !errors.As(err, &pgerr) || pgerr.Code != "23514" {
			t.Fatalf("database did not reject forged registration: %s %v", mutation, err)
		}
	}

	// Duplicate registration remains atomic when workers race.
	concurrent := *a
	concurrent.Name = "RacePlayer"
	var successes, duplicates atomic.Int32
	var wg sync.WaitGroup
	for i := 0; i < 8; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			err := db.CreateAccount(&concurrent)
			if err == nil {
				successes.Add(1)
			} else if errors.Is(err, store.ErrAccountExists) {
				duplicates.Add(1)
			} else {
				t.Error(err)
			}
		}()
	}
	wg.Wait()
	if successes.Load() != 1 || duplicates.Load() != 7 {
		t.Fatalf("duplicate race: successes=%d duplicates=%d", successes.Load(), duplicates.Load())
	}

	uid, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	a.Chars = []model.Char{{UID: uid, Name: "Knight", Class: 0, Score: &model.Score{Version: model.ScoreVersion, Level: 49, MaxHP: 100, CurHP: 100}, Exp: 120}}
	if err := writer.SaveAccount(a); err != nil {
		t.Fatal(err)
	}
	if _, err := account.Authenticate(db, "WebPlayer", "Safe1234"); err != nil {
		t.Fatal(err)
	}
	p, err = db.Profile(ctx, "WebPlayer")
	if err != nil || len(p.Characters) != 1 || p.Characters[0].Level != 49 || p.Characters[0].Evolution != "mortal" {
		t.Fatalf("game snapshot projection: %+v %v", p, err)
	}
	second := *a
	second.Name = "OtherPlayer"
	second.Chars = append([]model.Char(nil), a.Chars...)
	second.Chars[0].UID, err = model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	second.Chars[0].Name = "Ascended"
	second.Chars[0].Evolution = "arch"
	second.Chars[0].Score = &model.Score{Version: model.ScoreVersion, Level: 0, MaxHP: 100, CurHP: 100}
	if err := writer.CreateAccount(&second); err != nil {
		t.Fatal(err)
	}
	ranking, err := db.Ranking(ctx)
	if err != nil || len(ranking) != 2 || ranking[0].Name != "Ascended" || ranking[1].Name != "Knight" {
		t.Fatalf("ranking order: %+v %v", ranking, err)
	}
	encoded, _ := json.Marshal(ranking)
	if strings.Contains(string(encoded), "passwordHash") || strings.Contains(string(encoded), "WebPlayer") {
		t.Fatal("ranking leaked private account data")
	}
}
