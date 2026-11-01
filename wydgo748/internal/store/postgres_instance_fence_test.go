package store

import (
	"context"
	"os"
	"testing"
	"time"
)

// W4: a new game instance ends the previous instance's backends before use.
// An open transaction of the old instance aborts; other pools are untouched.
func TestNewGameInstanceFencesPreviousInstanceBackends(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	tools, table := criticalFaultStore(t, 2*time.Second) // unfenced, like a panel or tool
	old, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 2, FenceGameInstance: true})
	if err != nil {
		t.Fatal(err)
	}
	defer old.Close()
	tx, err := old.pool.Begin(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	if _, err := tx.Exec(context.Background(), `INSERT INTO `+table+`(id, v) VALUES('old-instance', 1)`); err != nil {
		t.Fatal(err)
	}
	toolConn, err := tools.pool.Acquire(context.Background())
	if err != nil {
		t.Fatal(err)
	}
	defer toolConn.Release()
	toolPID := toolConn.Conn().PgConn().PID()

	fresh, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 2, FenceGameInstance: true})
	if err != nil {
		t.Fatalf("new instance did not start after fencing: %v", err)
	}
	defer fresh.Close()
	if err := tx.Commit(context.Background()); err == nil {
		t.Fatal("the previous instance committed after it was fenced")
	}
	if n := faultRowCount(t, fresh, table, "old-instance"); n != 0 {
		t.Fatal("the fenced instance's open transaction became durable")
	}
	var alive bool
	if err := fresh.pool.QueryRow(context.Background(),
		`SELECT EXISTS(SELECT 1 FROM pg_stat_activity WHERE pid=$1)`, toolPID).Scan(&alive); err != nil || !alive {
		t.Fatalf("fencing ended an unrelated pool backend: alive=%t err=%v", alive, err)
	}
}
