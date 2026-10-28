package store

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"os"
	"sync"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/model"
)

func TestPostgresNPCDefinitionsMigrationIsolationAndConcurrency(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	ctx, cancel := context.WithTimeout(context.Background(), 30*time.Second)
	defer cancel()
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	defer conn.Close(context.Background())
	schema := fmt.Sprintf("npc_test_%d", time.Now().UnixNano())
	if _, err := conn.Exec(ctx, "CREATE SCHEMA "+schema); err != nil {
		t.Fatal(err)
	}
	defer conn.Exec(context.Background(), "DROP SCHEMA "+schema+" CASCADE")
	if _, err := conn.Exec(ctx, "SET search_path TO "+schema); err != nil {
		t.Fatal(err)
	}
	if _, err := conn.Exec(ctx, "CREATE TABLE schema_migrations(version integer PRIMARY KEY)"); err != nil {
		t.Fatal(err)
	}
	if _, err := conn.Exec(ctx, postgresNPCDefinitionsSchema); err != nil {
		t.Fatal(err)
	}
	raw := []byte(`{"name":"Shop","gold":1}`)
	if _, err := conn.Exec(ctx, `INSERT INTO npc_definitions
 (channel_id,npc_key,npc_name,source_json,definition_json,source_hash,version)
 VALUES(1,'shop','Shop',$1::jsonb,$1::jsonb,$2,1)`, raw, npcDefinitionSourceHash(raw)); err != nil {
		t.Fatal(err)
	}
	if _, err := conn.Exec(ctx, `INSERT INTO npc_definition_revisions
 (channel_id,npc_key,npc_name,version,definition_json,actor,reason)
 VALUES(1,'shop','Shop',2,$1::jsonb,'legacy','Before migration')`, raw); err != nil {
		t.Fatal(err)
	}
	if _, err := conn.Exec(ctx, postgresNPCExactNamesSchema); err != nil {
		t.Fatal(err)
	}
	var key string
	if err := conn.QueryRow(ctx, "SELECT npc_key FROM npc_definition_revisions").Scan(&key); err != nil || key != "Shop" {
		t.Fatalf("migration lost audit identity: %q %v", key, err)
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
	st := &PostgresStore{pool: pool}
	seeds := []NPCDefinitionSeed{
		{Definition: model.NPCDef{Name: "Shop", Gold: 10}, Active: true},
		{Definition: model.NPCDef{Name: "shop", Gold: 20}, Active: true},
	}
	for _, channel := range []uint8{1, 2} {
		if err := st.SyncNPCDefinitions(ctx, channel, seeds); err != nil {
			t.Fatal(err)
		}
	}
	read := func(channel uint8, name string, gold uint32) NPCDefinitionRecord {
		t.Helper()
		record, err := st.LoadNPCDefinition(ctx, channel, name)
		if err != nil {
			t.Fatal(err)
		}
		var def model.NPCDef
		if err := json.Unmarshal(record.Definition, &def); err != nil || def.Name != name || def.Gold != gold {
			t.Fatalf("channel %d %q: got %+v: %v", channel, name, def, err)
		}
		return record
	}
	before := read(1, "Shop", 10)
	cursor, err := st.NPCDefinitionChangeCursor(ctx, 1)
	if err != nil {
		t.Fatal(err)
	}
	next := json.RawMessage(`{"name":"Shop","gold":30}`)
	var wg sync.WaitGroup
	results := make(chan error, 2)
	for i := 0; i < 2; i++ {
		wg.Add(1)
		go func() {
			defer wg.Done()
			_, err := st.SaveNPCDefinition(ctx, 1, "Shop", before.Version, next, false, "editor", "Update shop")
			results <- err
		}()
	}
	wg.Wait()
	close(results)
	success, stale := 0, 0
	for err := range results {
		if err == nil {
			success++
		} else if errors.Is(err, ErrNPCDefinitionStale) {
			stale++
		} else {
			t.Fatal(err)
		}
	}
	if success != 1 || stale != 1 {
		t.Fatalf("CAS results: success=%d stale=%d", success, stale)
	}
	changed := read(1, "Shop", 30)
	read(1, "shop", 20)
	read(2, "Shop", 10)
	revisions, err := st.LoadNPCDefinitionChanges(ctx, 1, cursor)
	if err != nil || len(revisions) != 1 || revisions[0].Version != changed.Version {
		t.Fatalf("rejected writer created an audit revision: %+v %v", revisions, err)
	}
	seeds[0].Definition.Gold = 40
	if err := st.SyncNPCDefinitions(ctx, 1, seeds); err != nil {
		t.Fatal(err)
	}
	read(1, "Shop", 30) // Source refresh must preserve staff overrides.
	restored, err := st.SaveNPCDefinition(ctx, 1, "Shop", changed.Version, nil, true, "editor", "Restore source")
	if err != nil || restored.Overridden {
		t.Fatalf("restore failed: %+v %v", restored, err)
	}
	read(1, "Shop", 40)
	read(2, "Shop", 10)
	t.Run("source selectors upgrade existing overrides", func(t *testing.T) {
		old := model.NPCDef{Name: "Shop", Gold: 99, Score: &model.Score{Merchant: 43, MaxHP: 250}, Vende: []model.Item{{Index: 411}}}
		raw, _ := json.Marshal(old)
		saved, err := st.SaveNPCDefinition(ctx, 1, "Shop", restored.Version, raw, false, "editor", "Staff stock")
		if err != nil {
			t.Fatal(err)
		}
		cursor := saved.ChangeID
		selector := byte(11)
		seeds[0].Definition.ClientMerchant = &selector
		seeds[0].Definition.Score = &model.Score{Merchant: 40, MaxHP: 100}
		if err := st.SyncNPCDefinitions(ctx, 1, seeds); err != nil {
			t.Fatal(err)
		}
		entry := read(1, "Shop", 99)
		var def model.NPCDef
		if err := json.Unmarshal(entry.Definition, &def); err != nil {
			t.Fatal(err)
		}
		if !entry.Overridden || entry.Version != saved.Version+1 || def.ClientMerchant == nil || *def.ClientMerchant != 11 || def.Score.Merchant != 40 || def.Score.MaxHP != 250 || def.Vende[0].Index != 411 {
			t.Fatalf("override was lost or incompatible after source upgrade: %+v %+v", entry, def)
		}
		changes, err := st.LoadNPCDefinitionChanges(ctx, 1, cursor)
		if err != nil || len(changes) != 1 || changes[0].Version != entry.Version {
			t.Fatalf("source upgrade missing from feed: %+v %v", changes, err)
		}
		if err := st.SyncNPCDefinitions(ctx, 1, seeds); err != nil {
			t.Fatal(err)
		}
		if again := read(1, "Shop", 99); again.Version != entry.Version {
			t.Fatal("unchanged source created another version")
		}
		if extra, err := st.LoadNPCDefinitionChanges(ctx, 1, changes[0].ChangeID); err != nil || len(extra) != 0 {
			t.Fatalf("unchanged source created revisions: %+v %v", extra, err)
		}
	})
	t.Run("source sync repairs legacy service faces without losing staff content", func(t *testing.T) {
		source := model.NPCDef{Name: "AirService", Tipo: model.TipoNPC, Score: &model.Score{Merchant: 7, MaxHP: 100}}
		source.Equip.Rosto.Index = 63
		airSeeds := []NPCDefinitionSeed{{Definition: source, Active: true}}
		if err := st.SyncNPCDefinitions(ctx, 3, airSeeds); err != nil {
			t.Fatal(err)
		}
		initial, err := st.LoadNPCDefinition(ctx, 3, source.Name)
		if err != nil {
			t.Fatal(err)
		}
		legacy := source
		legacy.Equip.Rosto.Index = 60
		legacy.Score = &model.Score{Merchant: 7, MaxHP: 250}
		legacy.DisplayName = "Staff label"
		legacy.Vende = []model.Item{{Index: 411}}
		raw, err := json.Marshal(legacy)
		if err != nil {
			t.Fatal(err)
		}
		saved, err := st.SaveNPCDefinition(ctx, 3, source.Name, initial.Version, raw, false, "editor", "Legacy appearance edit")
		if err != nil {
			t.Fatal(err)
		}
		if err := st.SyncNPCDefinitions(ctx, 3, airSeeds); err != nil {
			t.Fatal(err)
		}
		repaired, err := st.LoadNPCDefinition(ctx, 3, source.Name)
		if err != nil {
			t.Fatal(err)
		}
		var effective model.NPCDef
		if err := json.Unmarshal(repaired.Definition, &effective); err != nil {
			t.Fatal(err)
		}
		if effective.Equip.Rosto.Index != 63 || effective.Score.MaxHP != 250 || effective.DisplayName != "Staff label" || len(effective.Vende) != 1 || effective.Vende[0].Index != 411 || !repaired.Overridden || repaired.Version != saved.Version+1 {
			t.Fatalf("service repair lost selector or staff content: %+v %+v", repaired, effective)
		}
		changes, err := st.LoadNPCDefinitionChanges(ctx, 3, saved.ChangeID)
		if err != nil || len(changes) != 1 || changes[0].Version != repaired.Version {
			t.Fatalf("repair not published: %+v %v", changes, err)
		}
		if err := st.SyncNPCDefinitions(ctx, 3, airSeeds); err != nil {
			t.Fatal(err)
		}
		if changes, err := st.LoadNPCDefinitionChanges(ctx, 3, changes[0].ChangeID); err != nil || len(changes) != 0 {
			t.Fatalf("repair was not idempotent: %+v %v", changes, err)
		}
	})
	t.Run("writers commit in feed order per channel", func(t *testing.T) {
		// Hold the first NPC row so its writer owns the channel lock while
		// waiting. A different NPC writer must wait before allocating an ID.
		hold, err := conn.Begin(ctx)
		if err != nil {
			t.Fatal(err)
		}
		defer hold.Rollback(context.Background())
		if _, err := hold.Exec(ctx, `SELECT 1 FROM npc_definitions WHERE channel_id=1 AND npc_key='Shop' FOR UPDATE`); err != nil {
			t.Fatal(err)
		}
		first, _ := st.LoadNPCDefinition(ctx, 1, "Shop")
		second, _ := st.LoadNPCDefinition(ctx, 1, "shop")
		type result struct {
			record NPCDefinitionRecord
			err    error
		}
		start := func(name string, version uint32) <-chan result {
			done := make(chan result, 1)
			go func() {
				raw := json.RawMessage(fmt.Sprintf(`{"name":%q,"gold":123}`, name))
				r, e := st.SaveNPCDefinition(ctx, 1, name, version, raw, false, "editor", "Ordered update")
				done <- result{r, e}
			}()
			return done
		}
		waitLock := func(granted bool) {
			t.Helper()
			deadline := time.Now().Add(5 * time.Second)
			for time.Now().Before(deadline) {
				var found bool
				if err := pool.QueryRow(ctx, `SELECT EXISTS(SELECT 1 FROM pg_locks WHERE locktype='advisory' AND classid=748015 AND objid=1 AND granted=$1)`, granted).Scan(&found); err != nil {
					t.Fatal(err)
				}
				if found {
					return
				}
				time.Sleep(5 * time.Millisecond)
			}
			t.Fatal("writer did not reach the expected channel lock")
		}
		a := start("Shop", first.Version)
		waitLock(true)
		b := start("shop", second.Version)
		waitLock(false)
		// Another channel remains independent while both channel 1 writers wait.
		other := read(2, "Shop", 10)
		otherCtx, cancel := context.WithTimeout(ctx, 2*time.Second)
		defer cancel()
		if _, err := st.SaveNPCDefinition(otherCtx, 2, "Shop", other.Version, json.RawMessage(`{"name":"Shop","gold":11}`), false, "editor", "Other channel"); err != nil {
			t.Fatal(err)
		}
		if err := hold.Commit(ctx); err != nil {
			t.Fatal(err)
		}
		ra, rb := <-a, <-b
		if ra.err != nil || rb.err != nil || ra.record.ChangeID >= rb.record.ChangeID {
			t.Fatalf("commit order broke feed order: %+v %+v", ra, rb)
		}
		changes, err := st.LoadNPCDefinitionChanges(ctx, 1, ra.record.ChangeID)
		if err != nil || len(changes) != 1 || changes[0].Name != "shop" {
			t.Fatalf("later commit disappeared from feed: %+v %v", changes, err)
		}
	})
	if _, err := st.SaveNPCDefinition(ctx, 1, "Shop", restored.Version, json.RawMessage(`{"name":"shop"}`), false, "editor", "Wrong identity"); err == nil {
		t.Fatal("case-changing payload accepted")
	}
	if err := st.SyncNPCDefinitions(ctx, 1, seeds[1:]); err != nil {
		t.Fatal(err)
	}
	if _, err := st.SaveNPCDefinition(ctx, 1, "Shop", restored.Version, next, false, "editor", "Inactive shop"); !errors.Is(err, ErrNPCDefinitionStale) {
		t.Fatalf("inactive template accepted: %v", err)
	}
	t.Run("revision failure rolls back definition and audit", func(t *testing.T) {
		before, err := st.LoadNPCDefinition(ctx, 1, "shop")
		if err != nil {
			t.Fatal(err)
		}
		cursor, err := st.NPCDefinitionChangeCursor(ctx, 1)
		if err != nil {
			t.Fatal(err)
		}
		_, err = pool.Exec(ctx, `CREATE FUNCTION reject_npc_revision() RETURNS trigger LANGUAGE plpgsql AS $$
BEGIN IF NEW.actor='reject-writer' THEN RAISE EXCEPTION 'injected revision failure'; END IF; RETURN NEW; END; $$;
CREATE TRIGGER reject_npc_revision BEFORE INSERT ON npc_definition_revisions FOR EACH ROW EXECUTE FUNCTION reject_npc_revision()`)
		if err != nil {
			t.Fatal(err)
		}
		_, err = st.SaveNPCDefinition(ctx, 1, "shop", before.Version, json.RawMessage(`{"name":"shop","gold":999}`), false, "reject-writer", "Injected failure")
		if err == nil {
			t.Fatal("revision failure did not reject the transaction")
		}
		after, err := st.LoadNPCDefinition(ctx, 1, "shop")
		if err != nil {
			t.Fatal(err)
		}
		if after.Version != before.Version || string(after.Definition) != string(before.Definition) {
			t.Fatal("definition update escaped the failed revision transaction")
		}
		changes, err := st.LoadNPCDefinitionChanges(ctx, 1, cursor)
		if err != nil || len(changes) != 0 {
			t.Fatalf("failed transaction created feed entries: %+v %v", changes, err)
		}
		cancelled, cancelNow := context.WithCancel(ctx)
		cancelNow()
		if _, err = st.SaveNPCDefinition(cancelled, 1, "shop", before.Version, json.RawMessage(`{"name":"shop","gold":999}`), false, "editor", "Cancelled save"); err == nil {
			t.Fatal("cancelled transaction succeeded")
		}
		after, err = st.LoadNPCDefinition(ctx, 1, "shop")
		if err != nil || after.Version != before.Version {
			t.Fatalf("cancelled update changed version: %+v %v", after, err)
		}
	})
	t.Run("pagination retains channel gaps without losing revisions", func(t *testing.T) {
		paged := []NPCDefinitionSeed{{Definition: model.NPCDef{Name: "Paged"}, Active: true}}
		for _, channel := range []uint8{1, 2} {
			if err := st.SyncNPCDefinitions(ctx, channel, paged); err != nil {
				t.Fatal(err)
			}
		}
		cursor, err := st.NPCDefinitionChangeCursor(ctx, 1)
		if err != nil {
			t.Fatal(err)
		}
		versions := map[uint8]uint32{1: 1, 2: 1}
		for i := 0; i < 105; i++ {
			raw := json.RawMessage(fmt.Sprintf(`{"name":"Paged","gold":%d}`, i))
			r, err := st.SaveNPCDefinition(ctx, 1, "Paged", versions[1], raw, false, "editor", "Page update")
			if err != nil {
				t.Fatal(err)
			}
			versions[1] = r.Version
			if i%15 == 0 {
				r, err := st.SaveNPCDefinition(ctx, 2, "Paged", versions[2], raw, false, "editor", "Other channel")
				if err != nil {
					t.Fatal(err)
				}
				versions[2] = r.Version
			}
		}
		seen := 0
		for _, want := range []int{100, 5, 0} {
			batch, err := st.LoadNPCDefinitionChanges(ctx, 1, cursor)
			if err != nil {
				t.Fatal(err)
			}
			if len(batch) != want {
				t.Fatalf("page size=%d want=%d", len(batch), want)
			}
			for _, entry := range batch {
				seen++
				if entry.Channel != 1 || entry.ChangeID <= cursor || entry.Version != uint32(seen+1) {
					t.Fatalf("lost or crossed channel revision: %+v", entry)
				}
				cursor = entry.ChangeID
			}
		}
		if seen != 105 {
			t.Fatalf("read %d revisions", seen)
		}
	})
	t.Run("atomic multi-channel updates", func(t *testing.T) {
		seed := []NPCDefinitionSeed{{Definition: model.NPCDef{Name: "Batch", Gold: 10}, Active: true}}
		for _, channel := range []uint8{1, 2} {
			if err := st.SyncNPCDefinitions(ctx, channel, seed); err != nil {
				t.Fatal(err)
			}
		}
		changes := []NPCDefinitionUpdate{
			{Channel: 2, Name: "Batch", ExpectedVersion: 1, Definition: json.RawMessage(`{"name":"Batch","gold":42}`)},
			{Channel: 1, Name: "Batch", ExpectedVersion: 1, Definition: json.RawMessage(`{"name":"Batch","gold":42}`)},
		}
		sourceRaw, err := json.Marshal(seed[0].Definition)
		if err != nil {
			t.Fatal(err)
		}
		for i := range changes {
			changes[i].ExpectedSourceHash = npcDefinitionSourceHash(sourceRaw)
		}
		assertVersions := func(want uint32) {
			t.Helper()
			for _, channel := range []uint8{1, 2} {
				entry, err := st.LoadNPCDefinition(ctx, channel, "Batch")
				if err != nil || entry.Version != want {
					t.Fatalf("channel %d version=%d want=%d err=%v", channel, entry.Version, want, err)
				}
			}
		}
		stale := append([]NPCDefinitionUpdate(nil), changes...)
		stale[0].ExpectedVersion = 2
		if _, err := st.SaveNPCDefinitions(ctx, stale, "editor", "Stale peer"); !errors.Is(err, ErrNPCDefinitionStale) {
			t.Fatalf("stale peer: %v", err)
		}
		assertVersions(1)
		staleSource := append([]NPCDefinitionUpdate(nil), changes...)
		staleSource[0].ExpectedSourceHash = "different-source"
		if _, err := st.SaveNPCDefinitions(ctx, staleSource, "editor", "Changed source"); !errors.Is(err, ErrNPCDefinitionStale) {
			t.Fatalf("stale source: %v", err)
		}
		assertVersions(1)
		if _, err := st.SaveNPCDefinitions(ctx, []NPCDefinitionUpdate{changes[0], changes[0]}, "editor", "Duplicate channel"); err == nil {
			t.Fatal("duplicate accepted")
		}
		if _, err := st.pool.Exec(ctx, `CREATE FUNCTION reject_batch_peer() RETURNS trigger LANGUAGE plpgsql AS $$
		BEGIN IF NEW.actor='batch-reject' AND NEW.channel_id=2 THEN RAISE EXCEPTION 'injected second channel failure'; END IF; RETURN NEW; END; $$;
		CREATE TRIGGER reject_batch_peer BEFORE INSERT ON npc_definition_revisions FOR EACH ROW EXECUTE FUNCTION reject_batch_peer()`); err != nil {
			t.Fatal(err)
		}
		if _, err := st.SaveNPCDefinitions(ctx, changes, "batch-reject", "Fail second write"); err == nil {
			t.Fatal("second revision failure accepted")
		}
		assertVersions(1)
		var revisions int
		if err := st.pool.QueryRow(ctx, `SELECT count(*) FROM npc_definition_revisions WHERE npc_name='Batch'`).Scan(&revisions); err != nil || revisions != 0 {
			t.Fatalf("partial audit: %d %v", revisions, err)
		}
		cancelled, cancelBatch := context.WithCancel(ctx)
		cancelBatch()
		if _, err := st.SaveNPCDefinitions(cancelled, changes, "editor", "Cancelled batch"); err == nil {
			t.Fatal("cancelled batch accepted")
		}
		assertVersions(1)
		errs := make(chan error, 2)
		for i := 0; i < 2; i++ {
			ordered := append([]NPCDefinitionUpdate(nil), changes...)
			if i == 1 {
				ordered[0], ordered[1] = ordered[1], ordered[0]
			}
			go func() { _, err := st.SaveNPCDefinitions(ctx, ordered, "editor", "Concurrent batch"); errs <- err }()
		}
		var success, conflicts int
		for i := 0; i < 2; i++ {
			err := <-errs
			if err == nil {
				success++
			} else if errors.Is(err, ErrNPCDefinitionStale) {
				conflicts++
			} else {
				t.Fatal(err)
			}
		}
		if success != 1 || conflicts != 1 {
			t.Fatalf("writers: success=%d conflict=%d", success, conflicts)
		}
		assertVersions(2)
		if err := st.pool.QueryRow(ctx, `SELECT count(*) FROM npc_definition_revisions WHERE npc_name='Batch'`).Scan(&revisions); err != nil || revisions != 2 {
			t.Fatalf("audit lost channel: %d %v", revisions, err)
		}
		for i := range changes {
			changes[i].ExpectedVersion = 2
			changes[i].RestoreSource = true
			changes[i].Definition = nil
		}
		entries, err := st.SaveNPCDefinitions(ctx, changes, "editor", "Restore both sources")
		if err != nil || len(entries) != 2 {
			t.Fatalf("restore: %v %v", entries, err)
		}
		for _, entry := range entries {
			var definition model.NPCDef
			if err := json.Unmarshal(entry.Definition, &definition); err != nil || definition.Gold != 10 || entry.Overridden {
				t.Fatalf("restore changed source: %+v %v", entry, err)
			}
		}
		assertVersions(3)
	})

}
