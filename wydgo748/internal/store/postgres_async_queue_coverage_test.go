package store

import (
	"strings"
	"testing"
)

func TestPostgresEnqueueRejectsClosedStoreAndFullBarrier(t *testing.T) {
	closed := &PostgresStore{closed: true}
	if err := closed.enqueue(postgresWriteJob{run: func() error { return nil }}); err == nil || !strings.Contains(err.Error(), "closed") {
		t.Fatalf("closed store enqueue error=%v", err)
	}

	full := &PostgresStore{writeQueue: make(chan postgresWriteJob, 1)}
	full.writeQueue <- postgresWriteJob{label: "occupy queue", run: func() error { return nil }}
	if err := full.enqueue(postgresWriteJob{label: "flush barrier", done: make(chan struct{})}); err == nil ||
		!strings.Contains(err.Error(), "queue full") {
		t.Fatalf("full barrier enqueue error=%v", err)
	}
}

func TestPostgresEnqueueCoalescesCharacterStateWhenQueueIsFull(t *testing.T) {
	st := &PostgresStore{
		writeQueue:    make(chan postgresWriteJob, 1),
		overflowState: make(map[string]postgresWriteJob),
	}
	st.writeQueue <- postgresWriteJob{label: "occupy queue", run: func() error { return nil }}
	for _, payload := range []string{"older", "newer"} {
		if err := st.enqueue(postgresWriteJob{
			charState: &postgresCharStateSnapshot{uid: "character-uid", payload: []byte(payload)},
		}); err != nil {
			t.Fatal(err)
		}
	}
	got, ok := st.overflowState["character-uid"]
	if !ok || string(got.charState.payload) != "newer" || got.seq == 0 {
		t.Fatalf("coalesced character state=%+v ok=%t", got, ok)
	}
	if got := st.coalesced.Load(); got != 1 {
		t.Fatalf("coalesced count=%d, want 1", got)
	}
}
