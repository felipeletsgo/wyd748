package game

import (
	"context"
	"os"
	"testing"

	"github.com/jackc/pgx/v5"

	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// W9 phase 1 on PostgreSQL: two game processes share one database. An account
// signed in on one process is refused on the other until the first session
// ends, and is then admitted there.
func TestAccountLeaseKeepsLoginSingleAcrossProcessesPostgres(t *testing.T) {
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
	const account = "W9Lease8"
	cleanup := func() {
		_, _ = conn.Exec(ctx, `DELETE FROM account_leases WHERE account_key=$1`, "w9lease8")
		_, _ = conn.Exec(ctx, `DELETE FROM game_processes WHERE process LIKE 'w9-test/game-%'`)
	}
	cleanup()
	t.Cleanup(func() { cleanup(); _ = conn.Close(ctx) })
	process := func(name string, id int64) (*World, *gameNet.Session, func()) {
		if err := st.HeartbeatGameProcess(name); err != nil {
			t.Fatal(err)
		}
		s := newLoginTestSession(id, 64)
		t.Cleanup(s.Close)
		acc := loginTestAccount()
		acc.Name = account
		w := loginResultWorld(&loginReloadStore{account: acc}, s)
		w.channel = byte(id)
		w.leases = &AccountLeases{store: st, process: name}
		login := func() {
			w.authPending[s] = true
			w.onLoginResult(s, &loginResult{accountName: account, account: acc})
		}
		return w, s, login
	}
	a, sessionA, loginA := process("w9-test/game-a", 1)
	b, sessionB, loginB := process("w9-test/game-b", 2)
	loginA()
	if a.players[sessionA] == nil {
		t.Fatal("first process did not admit the account")
	}
	loginB()
	pkt, ok := sessionB.DequeuePacketForTest()
	if !ok || !wire.Decrypt(pkt) || wire.ParseHeader(pkt).Type != wire.OpAlreadyPlaying || len(b.players) != 0 {
		t.Fatal("second process admitted an account owned by the first process")
	}
	// The first process ends the session; its lease release runs off the actor.
	a.releaseAccountSession(sessionA, a.players[sessionA].Account)
	waitFor(t, func() bool {
		var count int
		_ = conn.QueryRow(ctx, `SELECT count(*) FROM account_leases WHERE account_key='w9lease8'`).Scan(&count)
		return count == 0
	})
	loginB()
	if b.players[sessionB] == nil {
		t.Fatal("second process refused the account after the first released it")
	}
	var owner string
	if err := conn.QueryRow(ctx, `SELECT process FROM account_leases WHERE account_key='w9lease8'`).Scan(&owner); err != nil || owner != "w9-test/game-b" {
		t.Fatalf("lease owner %q err=%v", owner, err)
	}
}
