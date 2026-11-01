package store

import (
	"context"
	"expvar"
	"os"
	"testing"
)

func poolMetric(t *testing.T, name string) int64 {
	t.Helper()
	f, ok := expvar.Get(name).(expvar.Func)
	if !ok {
		t.Fatalf("%s is not published", name)
	}
	value, ok := f().(int64)
	if !ok {
		t.Fatalf("%s is not an integer", name)
	}
	return value
}

func TestPrimaryPoolMetricsFollowStoreLifetime(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 3})
	if err != nil {
		t.Fatal(err)
	}
	defer st.Close()
	if got := poolMetric(t, "postgres_pool_max_conns"); got != 3 {
		t.Fatalf("max conns = %d, want 3", got)
	}
	before := poolMetric(t, "postgres_pool_acquire_total")
	if err := st.Ping(context.Background()); err != nil {
		t.Fatal(err)
	}
	if poolMetric(t, "postgres_pool_acquire_total") <= before || poolMetric(t, "postgres_pool_total_conns") < 1 {
		t.Fatal("pool acquisition is not visible")
	}
	readOnly, err := NewPostgresReadOnlyStore(context.Background(), PostgresConfig{URL: url, MaxConns: 1})
	if err != nil {
		t.Fatal(err)
	}
	readOnly.Close()
	if got := poolMetric(t, "postgres_pool_max_conns"); got != 3 {
		t.Fatalf("a read-only pool replaced the game pool: max=%d", got)
	}
	st.Close()
	if got := poolMetric(t, "postgres_pool_max_conns"); got != 0 {
		t.Fatalf("closed pool still published: max=%d", got)
	}
}
