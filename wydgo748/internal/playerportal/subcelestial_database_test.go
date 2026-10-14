package playerportal

import (
	"context"
	"encoding/json"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

// Invoked only by the production-refusing, disposable PostgreSQL fixture.
func testSavedSubCelestialDatabase(t *testing.T, ctx context.Context, writer *store.PostgresStore, owner *pgxpool.Pool, db *Database, account *model.Account) {
	t.Helper()
	character := &account.Chars[0]
	character.Class, character.Score.Level = 3, model.SubCelestialMinimumLevel
	character.Equip[0] = model.Item{Index: 3}
	character.Equip[11] = model.Item{Index: 1761}
	character.Inv = [64]model.Item{}
	character.Inv[62] = model.Item{Index: 5338}
	save := func() {
		t.Helper()
		if err := writer.SaveAccount(account); err != nil {
			t.Fatal(err)
		}
	}
	read := func() CharacterDetail {
		t.Helper()
		detail, err := db.CharacterDetail(ctx, account.Name, 0)
		if err != nil {
			t.Fatal(err)
		}
		return detail
	}
	state := func(uid string, fame uint32) {
		t.Helper()
		if err := writer.SaveCharState(uid, &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": fame, "private-counter": 999}}); err != nil {
			t.Fatal(err)
		}
	}
	save()
	state(account.Chars[1].UID, 100)
	if r := read().SubCelestialAscension; !r.Applicable || r.RequirementsMet || r.Checks[2].Met {
		t.Fatal("another character's Fame satisfied the saved gate")
	}
	state(character.UID, 100)
	detail := read()
	if detail.Fame != 100 || detail.CP == int16(detail.Fame) {
		t.Fatal("character Fame was omitted or confused with CP")
	}
	if r := detail.SubCelestialAscension; !r.RequirementsMet || r.RequiredLevel != 121 || r.RequiredFame != 100 || len(r.Checks) != 8 {
		t.Fatalf("valid saved SubCelestial gates: %+v", r)
	}
	var stateTime time.Time
	if err := owner.QueryRow(ctx, `SELECT updated_at FROM character_states WHERE character_uid=$1::uuid`, character.UID).Scan(&stateTime); err != nil {
		t.Fatal(err)
	}
	if !detail.AsOf.Equal(stateTime) {
		t.Fatal("Fame snapshot timestamp omitted")
	}
	encoded, _ := json.Marshal(detail)
	for _, private := range []string{character.UID, `"inventory":`, "private-counter", "specialCoins", "affects", "validFace", "noAlternate" + `":true`} {
		if strings.Contains(string(encoded), private) {
			t.Fatalf("saved advice leaked %s", private)
		}
	}
	state(character.UID, 99)
	if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[2].Met {
		t.Fatal("Fame threshold bypassed")
	}
	state(character.UID, 100)
	character.Score.Level--
	save()
	if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[1].Met {
		t.Fatal("level threshold bypassed")
	}
	character.Score.Level++
	character.Equip[0].Index = 2
	save()
	if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[6].Met {
		t.Fatal("invalid persisted face accepted")
	}
	character.Equip[0].Index = 3
	character.Inv[1] = character.Equip[11]
	character.Equip[11] = model.Item{}
	save()
	if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[3].Met {
		t.Fatal("inventory Sefirot counted as equipped")
	}
	character.Equip[11] = character.Inv[1]
	character.Inv[1] = model.Item{}
	save()
	// Corrupt only this disposable snapshot; persisted alternate forms are
	// deliberately not constructed through the game's transformation flow.
	if _, err := owner.Exec(ctx, `UPDATE accounts SET payload=jsonb_set(payload,'{chars,0,alternateCelestial}','{}') WHERE name_key='archadvisor'`); err != nil {
		t.Fatal(err)
	}
	if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[5].Met {
		t.Fatal("existing alternate form ignored")
	}
	save()
	if _, err := owner.Exec(ctx, `UPDATE accounts SET payload=jsonb_set(jsonb_set(payload,'{chars,0,inv,62}','{"index":0}'),'{chars,0,inv,63}','{"index":5338}') WHERE name_key='archadvisor'`); err != nil {
		t.Fatal(err)
	}
	if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[4].Met {
		t.Fatal("reserved Ideal Stone accepted")
	}
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		character.Inv[slot] = model.Item{Index: 4148, Eff: [6]byte{61, 10}}
	}
	character.Inv[62] = model.Item{Index: 5338, Eff: [6]byte{61, 2}}
	save()
	if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[7].Met {
		t.Fatal("existing reward stack treated as empty reward space")
	}
	character.Inv[62].Eff[1] = 1
	save()
	if r := read().SubCelestialAscension; !r.RequirementsMet {
		t.Fatal("consumed Ideal Stone did not free its slot")
	}
	// The read model rejects invalid/unsupported sidecars, not just invalid
	// Fame values. No private sidecar payload is granted to the website role.
	for _, payload := range []string{
		`{"version":2,"specialCoins":{"fame":100}}`,
		`{"version":"1","specialCoins":{"fame":100}}`,
		`{"specialCoins":{"fame":100}}`,
		`{"version":1,"specialCoins":[]}`,
		`{"version":1,"specialCoins":{"fame":-1}}`,
		`{"version":1,"specialCoins":{"fame":"100"}}`,
		`{"version":1,"specialCoins":{"fame":100.5}}`,
		`{"version":1,"specialCoins":{"fame":4294967296}}`,
	} {
		if _, err := owner.Exec(ctx, `UPDATE character_states SET payload=$2::jsonb WHERE character_uid=$1::uuid`, character.UID, payload); err != nil {
			t.Fatal(err)
		}
		if _, err := db.CharacterDetail(ctx, account.Name, 0); err == nil {
			t.Fatalf("invalid saved counter accepted: %s", payload)
		}
	}
	for _, payload := range []string{`{"version":1}`, `{"version":1,"specialCoins":null}`, `{"version":1,"specialCoins":{}}`, `{"version":1,"specialCoins":{"fame":null}}`} {
		if _, err := owner.Exec(ctx, `UPDATE character_states SET payload=$2::jsonb WHERE character_uid=$1::uuid`, character.UID, payload); err != nil {
			t.Fatal(err)
		}
		if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[2].Met {
			t.Fatal("absent Fame not treated as zero")
		}
	}
	state(character.UID, ^uint32(0))
	if read().Fame != ^uint32(0) {
		t.Fatal("saved Fame narrowed below uint32")
	}
	if r := read().SubCelestialAscension; !r.RequirementsMet {
		t.Fatal("valid uint32 Fame maximum rejected")
	}
	// The owner binding is checked even if a historical account snapshot carries
	// another account's UID. This mutation never touches production.
	if _, err := owner.Exec(ctx, `UPDATE accounts SET payload=jsonb_set(payload,'{chars,0,uid}',to_jsonb($1::text)) WHERE name_key='archadvisor'`, accountUIDForTest(t, ctx, owner)); err != nil {
		t.Fatal(err)
	}
	if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[2].Met {
		t.Fatal("Fame crossed account ownership")
	}
	if read().Fame != 0 {
		t.Fatal("displayed Fame crossed account ownership")
	}
	save()
	if err := writer.SaveCharState(character.UID, nil); err != nil {
		t.Fatal(err)
	}
	if r := read().SubCelestialAscension; r.RequirementsMet || r.Checks[2].Met {
		t.Fatal("deleted sidecar retained Fame")
	}
}

func accountUIDForTest(t *testing.T, ctx context.Context, pool *pgxpool.Pool) string {
	t.Helper()
	var uid string
	if err := pool.QueryRow(ctx, `SELECT payload->'chars'->0->>'uid' FROM accounts WHERE name_key='webplayer'`).Scan(&uid); err != nil {
		t.Fatal(err)
	}
	// Give the other account enough Fame before corrupting the tested snapshot.
	if _, err := pool.Exec(ctx, `INSERT INTO character_states(character_uid,payload) VALUES($1::uuid,'{"version":1,"specialCoins":{"fame":100}}') ON CONFLICT(character_uid) DO UPDATE SET payload=EXCLUDED.payload`, uid); err != nil {
		t.Fatal(err)
	}
	return uid
}
