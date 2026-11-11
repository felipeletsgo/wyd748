package store

import (
	"testing"
	"time"
)

func TestPostgresRetryDerivedSkipsEmptyJob(t *testing.T) {
	st := &PostgresStore{writeQueue: make(chan postgresWriteJob, 1)}
	st.retryDerived(postgresWriteJob{})
	if len(st.writeQueue) != 0 {
		t.Fatal("empty derived job was scheduled")
	}
}

func TestPostgresRetryDerivedRequeuesFailedJobWithIncrementedAttempt(t *testing.T) {
	st := &PostgresStore{writeQueue: make(chan postgresWriteJob, 1)}
	job := postgresWriteJob{label: "test derived export", run: func() error { return nil }}
	st.retryDerived(job)
	select {
	case got := <-st.writeQueue:
		if got.label != job.label || got.retry != job.retry+1 || got.run == nil {
			t.Fatalf("retry job=%+v, want same job with attempt %d", got, job.retry+1)
		}
	case <-time.After(4 * time.Second):
		t.Fatal("derived job was not requeued after its backoff")
	}
}
