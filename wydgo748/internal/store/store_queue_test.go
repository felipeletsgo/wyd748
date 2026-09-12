package store

import (
	"sync"
	"testing"
	"time"
)

func TestJSONStoreAsyncQueueOverflowDoesNotBlockAndCoalesces(t *testing.T) {
	s := &JSONStore{writeQueue: make(chan writeJob, 1)}
	s.writeQueue <- writeJob{key: "occupied"}

	s.enqueueAsyncWrite("account:felipe", func() {})
	s.enqueueAsyncWrite("account:felipe", func() {})
	if len(s.overflow) != 1 || s.overflow[0].key != "account:felipe" {
		t.Fatalf("overflow deveria manter apenas o snapshot mais novo: %+v", s.overflow)
	}

	done := make(chan struct{})
	s.enqueueWrite(writeJob{done: done})
	if len(s.overflow) != 2 {
		t.Fatalf("barreira nao pode ser coalescida: overflow=%d", len(s.overflow))
	}

	if job, ok := s.nextWrite(); !ok || job.key != "occupied" {
		t.Fatalf("job FIFO perdido: %+v/%v", job, ok)
	}
	if job, ok := s.nextWrite(); !ok || job.key != "account:felipe" {
		t.Fatalf("snapshot do overflow fora de ordem: %+v/%v", job, ok)
	}
	if job, ok := s.nextWrite(); !ok || job.done != done {
		t.Fatalf("barreira nao foi preservada: %+v/%v", job, ok)
	}
}

func TestJSONStorePersistLoopPreservesWritesBeforeFlushBarrier(t *testing.T) {
	s := &JSONStore{writeQueue: make(chan writeJob, 1)}
	started := make(chan struct{})
	release := make(chan struct{})
	var mu sync.Mutex
	var order []int
	go s.persistLoop()

	s.enqueueWrite(writeJob{run: func() {
		close(started)
		<-release
		mu.Lock()
		order = append(order, 1)
		mu.Unlock()
	}})
	<-started
	s.enqueueAsyncWrite("account:felipe", func() {
		mu.Lock()
		order = append(order, 2)
		mu.Unlock()
	})

	flushed := make(chan struct{})
	go func() {
		s.Flush()
		close(flushed)
	}()
	select {
	case <-flushed:
		t.Fatal("Flush atravessou uma escrita ainda bloqueada")
	case <-time.After(20 * time.Millisecond):
	}
	close(release)
	select {
	case <-flushed:
	case <-time.After(time.Second):
		t.Fatal("Flush nao concluiu depois de drenar a fila")
	}
	mu.Lock()
	defer mu.Unlock()
	if len(order) != 2 || order[0] != 1 || order[1] != 2 {
		t.Fatalf("ordem de persistencia incorreta: %v", order)
	}
}
