package game

import (
	"testing"
	"time"

	netpkg "wydgo/internal/net"
)

func commandBatchTestCommand(session *netpkg.Session, sequence int64) command {
	return command{s: session, queuedAt: time.Unix(0, sequence)}
}

func commandBatchSequence(cmd command) int64 {
	return cmd.queuedAt.UnixNano()
}

func TestPreparedCommandBatchPreservesRoundRobinFIFO(t *testing.T) {
	a := &netpkg.Session{ID: 1}
	b := &netpkg.Session{ID: 2}
	c := &netpkg.Session{ID: 3}
	batch := []command{
		commandBatchTestCommand(a, 1),
		commandBatchTestCommand(a, 2),
		commandBatchTestCommand(b, 3),
		commandBatchTestCommand(c, 4),
		commandBatchTestCommand(b, 5),
		commandBatchTestCommand(a, 6),
	}
	w := &World{}
	order := w.prepareCommandBatchQueues(batch)
	if len(order) != 3 || order[0] != a || order[1] != b || order[2] != c {
		t.Fatalf("ordem de sessoes inesperada: %#v", order)
	}

	var got []int64
	for {
		progress := false
		for _, session := range order {
			cmd, ok := w.popPreparedCommand(batch, session)
			if !ok {
				continue
			}
			got = append(got, commandBatchSequence(cmd))
			progress = true
		}
		if !progress {
			break
		}
	}
	want := []int64{1, 3, 4, 2, 5, 6}
	if len(got) != len(want) {
		t.Fatalf("comandos executados=%v; esperado=%v", got, want)
	}
	for i := range want {
		if got[i] != want[i] {
			t.Fatalf("round-robin[%d]=%d; esperado=%d (todos=%v)", i, got[i], want[i], got)
		}
	}
	w.releaseCommandBatchScratch(batch, order)
}

func TestPreparedCommandBatchRequeueMatchesLegacyOrdering(t *testing.T) {
	a := &netpkg.Session{ID: 1}
	b := &netpkg.Session{ID: 2}
	c := &netpkg.Session{ID: 3}
	oldPending := &netpkg.Session{ID: 9}
	batch := []command{
		commandBatchTestCommand(a, 1),
		commandBatchTestCommand(a, 2),
		commandBatchTestCommand(b, 3),
		commandBatchTestCommand(c, 4),
		commandBatchTestCommand(b, 5),
		commandBatchTestCommand(a, 6),
	}
	w := &World{pendingCommands: []command{commandBatchTestCommand(oldPending, 99)}}
	order := w.prepareCommandBatchQueues(batch)

	// Simula uma rodada completa antes de o budget/tick interromper o lote.
	for _, session := range order {
		if _, ok := w.popPreparedCommand(batch, session); !ok {
			t.Fatalf("sessao %d sem primeiro comando", session.ID)
		}
	}
	w.requeuePreparedCommandBatch(batch, order)

	want := []int64{2, 6, 5, 99}
	if len(w.pendingCommands) != len(want) {
		t.Fatalf("pending=%d; esperado=%d", len(w.pendingCommands), len(want))
	}
	for i := range want {
		if got := commandBatchSequence(w.pendingCommands[i]); got != want[i] {
			t.Fatalf("pending[%d]=%d; esperado=%d", i, got, want[i])
		}
	}
	w.releaseCommandBatchScratch(batch, order)
}

func TestReleaseCommandBatchScratchDropsReferences(t *testing.T) {
	session := &netpkg.Session{ID: 1}
	batch := []command{{s: session, pkt: []byte{1, 2, 3}}}
	w := &World{}
	order := w.prepareCommandBatchQueues(batch)
	w.releaseCommandBatchScratch(batch, order)

	if len(w.commandBatchScratch) != 0 || len(w.commandBatchOrderScratch) != 0 || len(w.commandBatchQueues) != 0 {
		t.Fatalf("scratch nao foi zerado: batch=%d order=%d queues=%d",
			len(w.commandBatchScratch), len(w.commandBatchOrderScratch), len(w.commandBatchQueues))
	}
	if batch[0].s != nil || batch[0].pkt != nil {
		t.Fatal("scratch reteve referencias do comando processado")
	}
}

func TestPrepareCommandBatchQueuesReusesSchedulerScratch(t *testing.T) {
	var sessions [16]*netpkg.Session
	for i := range sessions {
		sessions[i] = &netpkg.Session{ID: int64(i + 1)}
	}
	batch := make([]command, worldCommandBatchLimit)
	w := &World{}

	fill := func() {
		for i := range batch {
			batch[i] = command{s: sessions[i%len(sessions)]}
		}
	}
	fill()
	order := w.prepareCommandBatchQueues(batch)
	w.releaseCommandBatchScratch(batch, order)

	allocs := testing.AllocsPerRun(1000, func() {
		fill()
		order := w.prepareCommandBatchQueues(batch)
		w.releaseCommandBatchScratch(batch, order)
	})
	if allocs != 0 {
		t.Fatalf("scheduler do lote alocou %.2f objeto(s) por preparacao", allocs)
	}
}

func BenchmarkPrepareCommandBatchQueues(b *testing.B) {
	var sessions [16]*netpkg.Session
	for i := range sessions {
		sessions[i] = &netpkg.Session{ID: int64(i + 1)}
	}
	batch := make([]command, worldCommandBatchLimit)
	w := &World{}
	fill := func() {
		for i := range batch {
			batch[i] = command{s: sessions[i%len(sessions)]}
		}
	}
	fill()
	order := w.prepareCommandBatchQueues(batch)
	w.releaseCommandBatchScratch(batch, order)

	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		fill()
		order := w.prepareCommandBatchQueues(batch)
		w.releaseCommandBatchScratch(batch, order)
	}
}
