package game

import (
	"testing"

	netpkg "wydgo/internal/net"
)

func TestPendingCommandQueueDrainKeepsCapacityAndDropsReferences(t *testing.T) {
	session := &netpkg.Session{ID: 1}
	w := &World{pendingCommands: make([]command, 0, 8)}
	for i := 0; i < 4; i++ {
		w.pendingCommands = append(w.pendingCommands, command{s: session, pkt: []byte{byte(i + 1)}})
	}
	originalCap := cap(w.pendingCommands)

	for i := 0; i < 4; i++ {
		cmd, ok := w.popPendingCommand()
		if !ok || cmd.s != session || len(cmd.pkt) != 1 || cmd.pkt[0] != byte(i+1) {
			t.Fatalf("pop %d invalido: ok=%v cmd=%+v", i, ok, cmd)
		}
	}
	if len(w.pendingCommands) != 0 || w.pendingCommandHead != 0 {
		t.Fatalf("fila nao voltou ao inicio: len=%d head=%d", len(w.pendingCommands), w.pendingCommandHead)
	}
	if cap(w.pendingCommands) != originalCap {
		t.Fatalf("capacidade caiu de %d para %d", originalCap, cap(w.pendingCommands))
	}
	backing := w.pendingCommands[:cap(w.pendingCommands)]
	for i := range backing {
		if backing[i].s != nil || backing[i].pkt != nil {
			t.Fatalf("slot %d reteve referencias depois do drain", i)
		}
	}
}

func TestPendingCommandPrependCompactsLiveTailInOrder(t *testing.T) {
	session := &netpkg.Session{ID: 1}
	w := &World{pendingCommands: make([]command, 0, 8)}
	for seq := byte(1); seq <= 3; seq++ {
		w.pendingCommands = append(w.pendingCommands, command{s: session, pkt: []byte{seq}})
	}
	first, ok := w.popPendingCommand()
	if !ok || first.pkt[0] != 1 {
		t.Fatalf("primeiro pop=%+v ok=%v", first, ok)
	}
	if w.pendingCommandHead != 1 {
		t.Fatalf("head=%d; esperado=1", w.pendingCommandHead)
	}

	w.prependPendingCommands([]command{
		{s: session, pkt: []byte{8}},
		{s: session, pkt: []byte{9}},
	})
	if w.pendingCommandHead != 0 {
		t.Fatalf("prepend deixou head=%d", w.pendingCommandHead)
	}
	want := []byte{8, 9, 2, 3}
	if len(w.pendingCommands) != len(want) {
		t.Fatalf("len=%d; esperado=%d", len(w.pendingCommands), len(want))
	}
	for i := range want {
		if got := w.pendingCommands[i].pkt[0]; got != want[i] {
			t.Fatalf("pending[%d]=%d; esperado=%d", i, got, want[i])
		}
	}
}

func TestPendingCommandDrainReusesBackingWithoutAllocations(t *testing.T) {
	session := &netpkg.Session{ID: 1}
	w := &World{pendingCommands: make([]command, 0, 32)}

	run := func() {
		for i := 0; i < 16; i++ {
			w.pendingCommands = append(w.pendingCommands, command{s: session})
		}
		for i := 0; i < 16; i++ {
			if _, ok := w.popPendingCommand(); !ok {
				panic("pending command ausente")
			}
		}
	}
	run()
	allocs := testing.AllocsPerRun(1000, run)
	if allocs != 0 {
		t.Fatalf("drain da fila pendente alocou %.2f objeto(s) por ciclo", allocs)
	}
}

func BenchmarkPendingCommandDrainReuse(b *testing.B) {
	session := &netpkg.Session{ID: 1}
	w := &World{pendingCommands: make([]command, 0, worldCommandBatchLimit)}
	b.ReportAllocs()
	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for j := 0; j < worldCommandBatchLimit; j++ {
			w.pendingCommands = append(w.pendingCommands, command{s: session})
		}
		for j := 0; j < worldCommandBatchLimit; j++ {
			_, _ = w.popPendingCommand()
		}
	}
}
