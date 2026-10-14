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
	"time"

	"github.com/jackc/pgx/v5/pgconn"
	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/account"
	"wydgo/internal/authlimit"
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
	// The restricted role can update security counters, but no game-state table.
	quota := []authlimit.Limit{{Key: "validation:player-role", Window: time.Hour, Maximum: 1}}
	if wait, err := db.AuthenticationLimiter().Admit(ctx, quota); err != nil || wait != 0 {
		t.Fatalf("restricted security counter reservation: %v %v", wait, err)
	}
	if wait, err := db.AuthenticationLimiter().Admit(ctx, quota); err != nil || wait <= 0 {
		t.Fatalf("restricted security counter rejection: %v %v", wait, err)
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
	if p.CreatedAt.IsZero() || p.AsOf.Before(p.CreatedAt) || p.Username != "WebPlayer" {
		t.Fatalf("invalid profile timestamps or name: %+v", p)
	}
	createdAt := p.CreatedAt
	if _, err := db.Profile(ctx, "MissingPlayer"); err == nil {
		t.Fatal("missing account produced a profile")
	}
	cancelled, cancel := context.WithCancel(ctx)
	cancel()
	if _, err := db.Profile(cancelled, "WebPlayer"); err == nil {
		t.Fatal("cancelled profile query succeeded")
	}

	// A compromised web database credential cannot modify the economy or schema.
	for _, query := range []string{
		`UPDATE accounts SET payload=payload WHERE name_key='webplayer'`,
		`DELETE FROM accounts WHERE name_key='webplayer'`,
		`SELECT * FROM characters`,
		`SELECT * FROM character_states`,
		`UPDATE public.player_character_progression SET fame=100`,
		`DELETE FROM public.player_character_progression`,
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
	a.Chars[0].Gold, a.Chars[0].CP = 4000000000, -15
	itemUID, err := model.NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	a.Chars[0].Equip[1] = model.Item{Index: 100, UID: itemUID, Eff: [6]byte{43, 9}}
	a.Chars[0].Equip[15] = model.Item{Index: 101}
	a.Chars[0].Inv[0] = model.Item{Index: 412, Eff: [6]byte{61, 255}}
	a.Chars[0].Inv[62] = model.Item{Index: 2330, Eff: [6]byte{61, 200}}
	a.Cargo[119] = model.Item{Index: 413, Eff: [6]byte{43, 9, 61, 10}}
	a.CargoGold = 4000000000
	if err := writer.SaveAccount(a); err != nil {
		t.Fatal(err)
	}
	testProgressionSchemaUpgrade(t, owner, ownerURL)
	testSavedPublicGuildsDatabase(t, ctx, writer, owner, db, ownerURL)
	if _, err := account.Authenticate(db, "WebPlayer", "Safe1234"); err != nil {
		t.Fatal(err)
	}
	p, err = db.Profile(ctx, "WebPlayer")
	if err != nil || len(p.Characters) != 1 || p.Characters[0].Level != 49 || p.Characters[0].Evolution != "mortal" {
		t.Fatalf("game snapshot projection: %+v %v", p, err)
	}
	var savedAt time.Time
	if err := owner.QueryRow(ctx, `SELECT updated_at FROM accounts WHERE name_key='webplayer'`).Scan(&savedAt); err != nil {
		t.Fatal(err)
	}
	if !p.AsOf.Equal(savedAt) || !p.CreatedAt.Equal(createdAt) || p.Characters[0].Slot != 0 {
		t.Fatalf("profile is not the persisted snapshot: %+v", p)
	}
	detail, err := db.CharacterDetail(ctx, "WEBPLAYER", 0)
	if err != nil || detail.Name != "Knight" || detail.Exp != 120 || detail.Gold != 4000000000 || detail.CP != -15 || detail.Level != 49 || !detail.AsOf.Equal(savedAt) || len(detail.Equipment) != 2 || detail.Equipment[0].Slot != 1 || detail.Equipment[0].Effects != [6]byte{43, 9} || detail.Equipment[1].Slot != 15 {
		t.Fatalf("character snapshot: %+v %v", detail, err)
	}
	encodedDetail, _ := json.Marshal(detail)
	for _, private := range []string{uid, itemUID, "uid", "password", "attack", "cargo", "rawEvolution"} {
		if strings.Contains(string(encodedDetail), private) {
			t.Fatalf("character projection leaked %s", private)
		}
	}
	if !detail.Ascension.Applicable || detail.Ascension.RequirementsMet || detail.Ascension.RequiredLevel != 371 || len(detail.Ascension.Checks) != 5 || !detail.Ascension.Checks[4].Met {
		t.Fatalf("saved low-level Mortal advisor: %+v", detail.Ascension)
	}
	// Use a separate fixture so economy/ranking tests keep their original state.
	advisor, err := account.Create(writer, "ArchAdvisor", "Safe1234", "Safe1234")
	if err != nil {
		t.Fatal(err)
	}
	advisorAccount := *advisor
	advisorAccount.Chars = make([]model.Char, 4)
	for slot, name := range []string{"MortalOne", "SlotTwo", "SlotThree", "SlotFour"} {
		uid, err := model.NewCharacterUID()
		if err != nil {
			t.Fatal(err)
		}
		advisorAccount.Chars[slot] = model.Char{UID: uid, Name: name, Score: &model.Score{Version: model.ScoreVersion, Level: 370}}
	}
	advisorAccount.Chars[0].Equip[10] = model.Item{Index: 1742}
	advisorAccount.Chars[0].Equip[11] = model.Item{Index: 1763}
	if err := writer.SaveAccount(&advisorAccount); err != nil {
		t.Fatal(err)
	}
	readAdvice := func() ArchAscension {
		t.Helper()
		result, err := db.CharacterDetail(ctx, "ARCHADVISOR", 0)
		if err != nil {
			t.Fatal(err)
		}
		return result.Ascension
	}
	if advice := readAdvice(); advice.RequirementsMet || advice.Checks[4].Met {
		t.Fatalf("full account bypassed slot gate: %+v", advice)
	}
	advisorAccount.Chars[3] = model.Char{}
	if err := writer.SaveAccount(&advisorAccount); err != nil {
		t.Fatal(err)
	}
	if advice := readAdvice(); !advice.RequirementsMet {
		t.Fatalf("available fourth slot rejected: %+v", advice)
	}
	advisorAccount.Chars[0].Inv[0] = advisorAccount.Chars[0].Equip[10]
	advisorAccount.Chars[0].Equip[10] = model.Item{}
	if err := writer.SaveAccount(&advisorAccount); err != nil {
		t.Fatal(err)
	}
	if advice := readAdvice(); advice.RequirementsMet || advice.Checks[2].Met {
		t.Fatalf("inventory stone satisfied equipment gate: %+v", advice)
	}
	advisorAccount.Chars[0].Evolution = "arch"
	if err := writer.SaveAccount(&advisorAccount); err != nil {
		t.Fatal(err)
	}
	if advice := readAdvice(); advice.Applicable || advice.RequirementsMet {
		t.Fatalf("Arch offered Mortal ascension: %+v", advice)
	}
	readCelestial := func() CelestialAscension {
		t.Helper()
		result, err := db.CharacterDetail(ctx, "ARCHADVISOR", 0)
		if err != nil {
			t.Fatal(err)
		}
		return result.CelestialAscension
	}
	advisorAccount.Chars[0].Score.Level = 355
	advisorAccount.Chars[0].ArchMortalLevel = 399
	advisorAccount.Chars[0].Inv[62] = model.Item{Index: 5338}
	if err := writer.SaveAccount(&advisorAccount); err != nil {
		t.Fatal(err)
	}
	if r := readCelestial(); !r.Applicable || !r.RequirementsMet || r.RequiredLevel != 356 || r.RequiredMortalLevel != 400 {
		t.Fatalf("saved Celestial gates rejected: %+v", r)
	}
	// Corrupt only this disposable fixture to prove the read projection excludes
	// ABI-reserved inventory even when historical data bypassed store validation.
	if _, err := owner.Exec(ctx, `UPDATE accounts SET payload=jsonb_set(jsonb_set(payload,'{chars,0,inv,62}','{"index":0}'),'{chars,0,inv,63}','{"index":5338}') WHERE name_key='archadvisor'`); err != nil {
		t.Fatal(err)
	}
	if r := readCelestial(); r.RequirementsMet || r.Checks[3].Met {
		t.Fatal("SQL counted reserved Ideal Stone")
	}
	advisorAccount.Chars[0].ArchMortalLevel = 398
	if err := writer.SaveAccount(&advisorAccount); err != nil {
		t.Fatal(err)
	}
	if r := readCelestial(); r.RequirementsMet || r.Checks[2].Met {
		t.Fatal("SQL bypassed saved origin level")
	}
	advisorAccount.Chars[0].ArchMortalLevel = 399
	advisorAccount.Chars[0].Score.Level = 354
	if err := writer.SaveAccount(&advisorAccount); err != nil {
		t.Fatal(err)
	}
	if r := readCelestial(); r.RequirementsMet || r.Checks[1].Met {
		t.Fatal("SQL bypassed Arch level")
	}
	advisorAccount.Chars[0].Score.Level = 355
	advisorAccount.Chars[0].Evolution = "celestial"
	if err := writer.SaveAccount(&advisorAccount); err != nil {
		t.Fatal(err)
	}
	if r := readCelestial(); r.Applicable || r.RequirementsMet {
		t.Fatal("SQL offered Arch advice to Celestial")
	}
	if _, err := db.CharacterDetail(ctx, "OtherAccount", 0); !errors.Is(err, os.ErrNotExist) {
		t.Fatal("advisor crossed account ownership")
	}
	testSavedSubCelestialDatabase(t, ctx, writer, owner, db, &advisorAccount)
	// Remove only this fixture's ranking entries before the original assertions.
	advisorAccount.Chars = nil
	if err := writer.SaveAccount(&advisorAccount); err != nil {
		t.Fatal(err)
	}
	emptyProfile, err := db.Profile(ctx, "ArchAdvisor")
	if err != nil || emptyProfile.Characters == nil || len(emptyProfile.Characters) != 0 {
		t.Fatalf("null character list is not an empty profile: %+v %v", emptyProfile, err)
	}
	for _, slot := range []int{-1, 1, 4} {
		if _, err := db.CharacterDetail(ctx, "WebPlayer", slot); !errors.Is(err, os.ErrNotExist) {
			t.Fatalf("invalid/missing slot %d: %v", slot, err)
		}
	}
	if _, err := db.CharacterDetail(ctx, "OtherAccount", 0); !errors.Is(err, os.ErrNotExist) {
		t.Fatal("other account was not isolated")
	}
	if _, err := db.CharacterDetail(cancelled, "WebPlayer", 0); err == nil {
		t.Fatal("cancelled character query succeeded")
	}
	inventory, err := db.Inventory(ctx, "WEBPLAYER", 0)
	if err != nil || inventory.Capacity != 63 || inventory.CharacterSlot == nil || *inventory.CharacterSlot != 0 || inventory.Gold != nil || len(inventory.Items) != 2 || inventory.Items[0].Slot != 0 || inventory.Items[1].Slot != 62 || !inventory.AsOf.Equal(savedAt) || inventory.Items[0].Effects != [6]byte{61, 255} {
		t.Fatalf("inventory snapshot: %+v %v", inventory, err)
	}
	cargo, err := db.Cargo(ctx, "WEBPLAYER")
	if err != nil || cargo.Capacity != 120 || cargo.Gold == nil || *cargo.Gold != 4000000000 || cargo.CharacterSlot != nil || len(cargo.Items) != 1 || cargo.Items[0].Slot != 119 || !cargo.AsOf.Equal(savedAt) {
		t.Fatalf("Cargo snapshot: %+v %v", cargo, err)
	}
	for _, slot := range []int{-1, 1, 4} {
		if _, err := db.Inventory(ctx, "WebPlayer", slot); !errors.Is(err, os.ErrNotExist) {
			t.Fatalf("missing inventory selector %d: %v", slot, err)
		}
	}
	if _, err := db.Cargo(ctx, "MissingPlayer"); !errors.Is(err, os.ErrNotExist) {
		t.Fatal("missing Cargo account accepted")
	}
	if _, err := db.Inventory(cancelled, "WebPlayer", 0); err == nil {
		t.Fatal("cancelled inventory query succeeded")
	}
	if _, err := db.Cargo(cancelled, "WebPlayer"); err == nil {
		t.Fatal("cancelled Cargo query succeeded")
	}
	emptyCargo, err := db.Cargo(ctx, "RacePlayer")
	if err != nil || emptyCargo.Items == nil || len(emptyCargo.Items) != 0 || emptyCargo.Gold == nil || *emptyCargo.Gold != 0 {
		t.Fatalf("empty Cargo is not a zero balance and empty list: %+v %v", emptyCargo, err)
	}
	// Deliberately corrupt only this disposable fixture to prove SQL excludes
	// reserved ABI cells even when persisted data bypasses model validation.
	if _, err := owner.Exec(ctx, `UPDATE accounts SET payload=jsonb_set(jsonb_set(payload,'{chars,0,inv,63}','{"index":999,"uid":"reserved-secret"}'),'{cargo,120}','{"index":999,"uid":"reserved-secret"}') WHERE name_key='webplayer'`); err != nil {
		t.Fatal(err)
	}
	for _, read := range []func() (StorageSnapshot, error){
		func() (StorageSnapshot, error) { return db.Inventory(ctx, "WebPlayer", 0) },
		func() (StorageSnapshot, error) { return db.Cargo(ctx, "WebPlayer") },
	} {
		result, err := read()
		if err != nil {
			t.Fatal(err)
		}
		encoded, _ := json.Marshal(result)
		if strings.Contains(string(encoded), "999") || strings.Contains(string(encoded), "uid") || strings.Contains(string(encoded), "secret") {
			t.Fatal("SQL storage projection leaked reserved cells or identity")
		}
	}
	if err := writer.SaveAccount(a); err != nil {
		t.Fatal(err)
	}
	upper, err := db.Profile(ctx, "WEBPLAYER")
	if err != nil || upper.AsOf.Before(p.AsOf) || upper.Username != p.Username {
		t.Fatalf("case-insensitive profile lookup: %+v %v", upper, err)
	}
	profileJSON, _ := json.Marshal(p)
	for _, private := range []string{"password", "cargo", "inventory", "uid", "Safe1234"} {
		if strings.Contains(strings.ToLower(string(profileJSON)), strings.ToLower(private)) {
			t.Fatalf("profile leaked private field %s", private)
		}
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
	// This account owns no equipment; copying item identities would violate
	// the store's cross-account item ownership constraint.
	second.Chars[0].Equip = [16]model.Item{}
	second.Chars[0].Inv = [64]model.Item{}
	second.Cargo = [128]model.Item{}
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
