package store

import (
	"expvar"
	"sync"
	"testing"
	"time"
)

func TestSetPostgresMetricMaxNeverRegresses(t *testing.T) {
	var metric expvar.Int
	setPostgresMetricMax(&metric, 12)
	setPostgresMetricMax(&metric, 3)
	if got := metric.Value(); got != 12 {
		t.Fatalf("max regrediu para %d; esperado 12", got)
	}
}

func TestSetPostgresMetricMaxIsConcurrentSafe(t *testing.T) {
	var metric expvar.Int
	var wg sync.WaitGroup
	for i := int64(1); i <= 64; i++ {
		wg.Add(1)
		go func(value int64) {
			defer wg.Done()
			setPostgresMetricMax(&metric, value)
		}(i)
	}
	wg.Wait()
	if got := metric.Value(); got != 64 {
		t.Fatalf("max concorrente=%d; esperado 64", got)
	}
}

func TestDurationMicrosClampsNegativeDuration(t *testing.T) {
	if got := durationMicros(-time.Microsecond); got != 0 {
		t.Fatalf("duracao negativa=%d; esperado 0", got)
	}
	if got := durationMicros(37 * time.Microsecond); got != 37 {
		t.Fatalf("duracao positiva=%d; esperado 37", got)
	}
}
