package store

import (
	"context"
	"os"
	"testing"
)

// W9 phase 5: a channel process fences only its own channel's previous
// process (and a previous whole-server process); the other channel's process
// keeps its backends, its registration and its account leases.
func TestChannelProcessFencesOnlyItsChannel(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated database")
	}
	ctx := context.Background()
	open := func(channel byte) *PostgresStore {
		st, err := NewPostgresStore(ctx, PostgresConfig{URL: url, MaxConns: 2, FenceGameInstance: true, FenceChannel: channel})
		if err != nil {
			t.Fatal(err)
		}
		t.Cleanup(st.Close)
		if _, err := st.RegisterGameProcess(); err != nil {
			t.Fatal(err)
		}
		return st
	}
	pid := func(st *PostgresStore) int {
		conn, err := st.pool.Acquire(ctx)
		if err != nil {
			t.Fatal(err)
		}
		defer conn.Release()
		return int(conn.Conn().PgConn().PID())
	}
	alive := func(observer *PostgresStore, pid int) bool {
		var exists bool
		if err := observer.pool.QueryRow(ctx, `SELECT EXISTS(SELECT 1 FROM pg_stat_activity WHERE pid=$1)`, pid).Scan(&exists); err != nil {
			t.Fatal(err)
		}
		return exists
	}
	const one, two = "w9fence1", "w9fence2"
	oldOne, channelTwo := open(1), open(2)
	oldPID, twoPID := pid(oldOne), pid(channelTwo)
	if owner, err := channelTwo.ClaimAccountLease(two, *channelTwo.leaseProcess.Load(), 2, 1); err != nil || owner != "" {
		t.Fatalf("channel 2 lease: owner=%q err=%v", owner, err)
	}
	if owner, err := oldOne.ClaimAccountLease(one, *oldOne.leaseProcess.Load(), 1, 1); err != nil || owner != "" {
		t.Fatalf("channel 1 lease: owner=%q err=%v", owner, err)
	}
	t.Cleanup(func() {
		_, _ = channelTwo.pool.Exec(ctx, `DELETE FROM account_leases WHERE account_key = ANY($1::text[])`, []string{one, two})
	})

	newOne := open(1) // restart of channel 1
	if alive(newOne, oldPID) {
		t.Fatal("the restarted channel 1 process left the previous channel 1 backend running")
	}
	if !alive(newOne, twoPID) {
		t.Fatal("the restarted channel 1 process ended a channel 2 backend")
	}
	var leases map[string]string = map[string]string{}
	rows, err := newOne.pool.Query(ctx, `SELECT account_key, process FROM account_leases WHERE account_key = ANY($1::text[])`, []string{one, two})
	if err != nil {
		t.Fatal(err)
	}
	for rows.Next() {
		var key, process string
		if err := rows.Scan(&key, &process); err != nil {
			t.Fatal(err)
		}
		leases[key] = process
	}
	rows.Close()
	if _, kept := leases[one]; kept {
		t.Fatal("the previous channel 1 process kept its lease after the restart")
	}
	if leases[two] != *channelTwo.leaseProcess.Load() {
		t.Fatalf("channel 2 lease changed: %q", leases[two])
	}
	if err := channelTwo.HeartbeatGameProcess(*channelTwo.leaseProcess.Load()); err != nil {
		t.Fatalf("channel 2 process lost its connection: %v", err)
	}
}
