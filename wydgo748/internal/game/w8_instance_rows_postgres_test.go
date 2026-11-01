package game

import (
	"context"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// W8 on PostgreSQL: a runtime whose row write is held by a lock does not stop
// another runtime's write. Each flush writes only its dirty rows, and the
// rebuilt aggregate holds both runtimes' latest committed state.
func TestInstanceRuntimeWritesProgressIndependentlyPostgres(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	ctx := context.Background()
	st, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: url, MaxConns: 4})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	suffix := time.Now().UTC().Format("150405000")
	idA, idB := "w8-a-"+suffix, "w8-b-"+suffix
	t.Cleanup(func() {
		_, _ = conn.Exec(ctx, `DELETE FROM instance_runtime_state WHERE runtime_id IN ($1,$2)`, idA, idB)
		_ = conn.Close(ctx)
	})
	w, err := NewWorld(st, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
		model.CharacterTemplateFile{}, model.TerrainMap{}, WithCriticalPersistence(2, 16), WithCriticalAccountContainment())
	if err != nil {
		t.Fatal(err)
	}
	deadline := w.now().Add(time.Hour)
	room := func(id string) *ItemInstance {
		return &ItemInstance{RuntimeID: id, State: "combat", CurrentStage: 1, HardDeadline: deadline,
			MemberCharacterUIDs: []string{"uid-" + id},
			Config:              model.VolatileInstance{ID: "hg-w8", StateMachine: "hell_gate"}}
	}
	a, b := room(idA), room(idB)
	w.itemInstances[idA], w.itemInstances[idB] = a, b
	settle := func() {
		t.Helper()
		for end := time.Now().Add(10 * time.Second); time.Now().Before(end); time.Sleep(5 * time.Millisecond) {
			w.processCriticalCompletions(criticalCompletionBatchLimit)
			if len(w.criticalWrites.queue.pending) == 0 {
				return
			}
		}
		t.Fatal("instance writes did not settle")
	}
	stage := func(id string) int {
		t.Helper()
		state, err := st.LoadInstanceState()
		if err != nil {
			t.Fatal(err)
		}
		for _, runtime := range state.Instances {
			if runtime.RuntimeID == id {
				return runtime.CurrentStage
			}
		}
		return 0
	}
	w.markInstanceRuntimeDirty(idA)
	w.markInstanceRuntimeDirty(idB)
	w.flushInstanceStateIfDirty()
	settle()
	if stage(idA) != 1 || stage(idB) != 1 || w.instanceStateDirtyAny() {
		t.Fatalf("initial rows: a=%d b=%d dirty=%t", stage(idA), stage(idB), w.instanceStateDirtyAny())
	}

	lock, err := conn.Begin(ctx)
	if err != nil {
		t.Fatal(err)
	}
	defer func() { _ = lock.Rollback(ctx) }()
	if _, err := lock.Exec(ctx, `SELECT 1 FROM instance_runtime_state WHERE runtime_id=$1 FOR UPDATE`, idA); err != nil {
		t.Fatal(err)
	}
	a.CurrentStage = 2
	w.markInstanceRuntimeDirty(idA)
	w.flushInstanceStateIfDirty() // held by the row lock
	time.Sleep(100 * time.Millisecond)
	if !w.criticalInstanceRuntimeReserved(idA) || w.criticalInstanceRuntimeReserved(idB) || w.criticalInstanceWholeReserved() {
		t.Fatal("the held write does not own exactly its runtime")
	}
	b.CurrentStage = 3
	w.markInstanceRuntimeDirty(idB)
	w.flushInstanceStateIfDirty() // must not wait for runtime A
	for end := time.Now().Add(5 * time.Second); w.criticalInstanceRuntimeReserved(idB) && time.Now().Before(end); time.Sleep(5 * time.Millisecond) {
		w.processCriticalCompletions(criticalCompletionBatchLimit)
	}
	if w.criticalInstanceRuntimeReserved(idB) || stage(idB) != 3 || stage(idA) != 1 {
		t.Fatalf("runtime B waited for A: reservedB=%t b=%d a=%d", w.criticalInstanceRuntimeReserved(idB), stage(idB), stage(idA))
	}
	if err := lock.Rollback(ctx); err != nil {
		t.Fatal(err)
	}
	settle()
	if stage(idA) != 2 || stage(idB) != 3 || w.instanceStateDirtyAny() {
		t.Fatalf("final rows: a=%d b=%d dirty=%t", stage(idA), stage(idB), w.instanceStateDirtyAny())
	}
}
