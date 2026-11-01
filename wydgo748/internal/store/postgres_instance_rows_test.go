package store

import (
	"context"
	"encoding/json"
	"os"
	"sync"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
)

// W8 instance rows. Tests use channel 250 or a rolled-back transaction, so
// concurrent packages writing channel 1 on the shared test database are safe.
const instanceRowsTestChannel byte = 250

func instanceRowsStore(t *testing.T) (*PostgresStore, *pgx.Conn) {
	t.Helper()
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	ctx := context.Background()
	st, err := NewPostgresStore(ctx, PostgresConfig{URL: url, MaxConns: 4})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	cleanup := func() {
		_, _ = conn.Exec(ctx, `DELETE FROM instance_runtime_state WHERE channel=$1`, int16(instanceRowsTestChannel))
		_, _ = conn.Exec(ctx, `DELETE FROM instance_channel_state WHERE channel=$1`, int16(instanceRowsTestChannel))
	}
	cleanup()
	t.Cleanup(func() { cleanup(); _ = conn.Close(ctx) })
	return st, conn
}

func instanceRowsSnapshot(runtimes ...model.InstanceRuntimeState) *model.InstanceStateSnapshot {
	return &model.InstanceStateSnapshot{Version: model.InstanceStateVersion,
		Channels: map[byte]*model.InstanceStateSnapshot{instanceRowsTestChannel: {
			Version: model.InstanceStateVersion, Instances: runtimes,
			NightmarePartyRuns: map[string]int{"window": 2}, GambleJackpot: 7,
		}}}
}

func instanceRowsRuntime(id string, stage int) model.InstanceRuntimeState {
	return model.InstanceRuntimeState{RuntimeID: id, ConfigID: "nightmare_normal", State: "combat",
		CurrentStage: stage, MemberCharacterUIDs: []string{"uid-" + id}, LeaderCharacterUID: "uid-" + id,
		HardDeadline: time.Date(2026, 11, 1, 12, 0, 0, 0, time.UTC)}
}

func channelRows(t *testing.T, st *PostgresStore) *model.InstanceStateSnapshot {
	t.Helper()
	state, err := st.LoadInstanceState()
	if err != nil {
		t.Fatal(err)
	}
	return state.Channels[instanceRowsTestChannel]
}

func TestInstanceRowsKeyedWriteTouchesOnlyDeclaredRows(t *testing.T) {
	st, _ := instanceRowsStore(t)
	a, b := InstanceParticipant{instanceRowsTestChannel, "run-a"}, InstanceParticipant{instanceRowsTestChannel, "run-b"}
	globals := InstanceParticipant{Channel: instanceRowsTestChannel}
	if err := st.SaveCriticalInstanceRows(instanceRowsSnapshot(instanceRowsRuntime("run-a", 1), instanceRowsRuntime("run-b", 1)),
		[]InstanceParticipant{a, b, globals}); err != nil {
		t.Fatal(err)
	}
	// A stale snapshot of run-b travels with a write that declares only run-a.
	if err := st.SaveCriticalInstanceRows(instanceRowsSnapshot(instanceRowsRuntime("run-a", 2), instanceRowsRuntime("run-b", 9)),
		[]InstanceParticipant{a}); err != nil {
		t.Fatal(err)
	}
	got := channelRows(t, st)
	if got == nil || len(got.Instances) != 2 || got.Instances[0].CurrentStage != 2 || got.Instances[1].CurrentStage != 1 ||
		got.NightmarePartyRuns["window"] != 2 || got.GambleJackpot != 7 {
		t.Fatalf("keyed write changed an undeclared row: %+v", got)
	}
	// Declaring a runtime that the snapshot no longer holds removes its row.
	if err := st.SaveCriticalInstanceRows(instanceRowsSnapshot(instanceRowsRuntime("run-a", 2)), []InstanceParticipant{b}); err != nil {
		t.Fatal(err)
	}
	if got := channelRows(t, st); len(got.Instances) != 1 || got.Instances[0].RuntimeID != "run-a" {
		t.Fatalf("declared removal kept the row: %+v", got)
	}
}

func TestInstanceRowsConcurrentWritesOfDifferentRuntimes(t *testing.T) {
	st, _ := instanceRowsStore(t)
	const runtimes = 8
	var wg sync.WaitGroup
	errs := make(chan error, runtimes*5)
	for i := range runtimes {
		wg.Add(1)
		go func() {
			defer wg.Done()
			id := string(rune('a'+i)) + "-run"
			for stage := 1; stage <= 5; stage++ {
				errs <- st.SaveCriticalInstanceRows(instanceRowsSnapshot(instanceRowsRuntime(id, stage)),
					[]InstanceParticipant{{instanceRowsTestChannel, id}})
			}
		}()
	}
	wg.Wait()
	close(errs)
	for err := range errs {
		if err != nil {
			t.Fatalf("concurrent participant write failed: %v", err)
		}
	}
	got := channelRows(t, st)
	if got == nil || len(got.Instances) != runtimes {
		t.Fatalf("rows after concurrent writes: %+v", got)
	}
	for _, runtime := range got.Instances {
		if runtime.CurrentStage != 5 {
			t.Fatalf("runtime %s ended at stage %d", runtime.RuntimeID, runtime.CurrentStage)
		}
	}
}

func TestInstanceRowsMigrationRebuildsSingleton(t *testing.T) {
	st, conn := instanceRowsStore(t)
	_ = st
	ctx := context.Background()
	tx, err := conn.BeginTx(ctx, pgx.TxOptions{IsoLevel: pgx.ReadCommitted})
	if err != nil {
		t.Fatal(err)
	}
	defer func() { _ = tx.Rollback(ctx) }()
	original := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion,
		Instances:          []model.InstanceRuntimeState{instanceRowsRuntime("water-1", 3)},
		NightmarePartyRuns: map[string]int{"nm": 1}, GamblePool: 99,
		Channels: map[byte]*model.InstanceStateSnapshot{2: {Version: model.InstanceStateVersion,
			Instances: []model.InstanceRuntimeState{instanceRowsRuntime("hg-2", 1), instanceRowsRuntime("hg-1", 2)}}}}
	payload, err := jsonMarshalForTest(original)
	if err != nil {
		t.Fatal(err)
	}
	for _, statement := range []string{`DELETE FROM instance_runtime_state`, `DELETE FROM instance_channel_state`} {
		if _, err := tx.Exec(ctx, statement); err != nil {
			t.Fatal(err)
		}
	}
	if _, err := tx.Exec(ctx, `INSERT INTO instance_state(singleton,payload) VALUES(true,$1)
		ON CONFLICT(singleton) DO UPDATE SET payload=EXCLUDED.payload`, payload); err != nil {
		t.Fatal(err)
	}
	if err := migrateInstanceRows(ctx, tx); err != nil {
		t.Fatalf("migration: %v", err)
	}
	rebuilt, err := loadInstanceRows(ctx, tx)
	if err != nil {
		t.Fatal(err)
	}
	if !equalInstanceStates(original, rebuilt) || len(rebuilt.Channels[2].Instances) != 2 || rebuilt.GamblePool != 99 {
		t.Fatalf("rebuilt state differs: %+v", rebuilt)
	}
	if err := migrateInstanceRows(ctx, tx); err == nil {
		t.Fatal("a second migration merged into existing rows")
	}
}

func TestInstanceRowsExportKeepsRollbackSingletonCurrent(t *testing.T) {
	st, conn := instanceRowsStore(t)
	if err := st.SaveCriticalInstanceRows(instanceRowsSnapshot(instanceRowsRuntime("export-run", 4)),
		[]InstanceParticipant{{instanceRowsTestChannel, "export-run"}, {Channel: instanceRowsTestChannel}}); err != nil {
		t.Fatal(err)
	}
	st.Flush()
	var payload []byte
	if err := conn.QueryRow(context.Background(), `SELECT payload FROM instance_state WHERE singleton=true`).Scan(&payload); err != nil {
		t.Fatal(err)
	}
	var exported model.InstanceStateSnapshot
	if err := jsonUnmarshalForTest(payload, &exported); err != nil {
		t.Fatal(err)
	}
	channel := exported.Channels[instanceRowsTestChannel]
	if channel == nil || len(channel.Instances) != 1 || channel.Instances[0].CurrentStage != 4 || channel.GambleJackpot != 7 {
		t.Fatalf("rollback singleton not exported: %+v", channel)
	}
}

func jsonMarshalForTest(v any) ([]byte, error)      { return json.Marshal(v) }
func jsonUnmarshalForTest(data []byte, v any) error { return json.Unmarshal(data, v) }
