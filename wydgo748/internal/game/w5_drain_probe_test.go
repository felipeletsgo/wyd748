package game

import (
	"context"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// W5/D11 measurement probe: terminal saves of populated accounts against the
// local PostgreSQL test database. Run with WYD_DRAIN_PROBE=1. It records
// durations only; VPS timing remains a separate measurement.
func TestTerminalDrainProbePostgres(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" || os.Getenv("WYD_DRAIN_PROBE") != "1" {
		t.Skip("set WYD_DRAIN_PROBE=1 and WYD_TEST_POSTGRES_URL")
	}
	ctx := context.Background()
	st, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: url, MaxConns: 8})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() { _ = conn.Close(ctx) })
	prefix := "drain" + time.Now().UTC().Format("150405")
	t.Cleanup(func() { _, _ = conn.Exec(ctx, `DELETE FROM accounts WHERE name_key LIKE $1`, prefix+"%") })

	players := make([]*Player, 0, 300)
	for i := range 300 {
		p, _ := networkedTestPlayer(uint16(i+1), fmt.Sprintf("Dr%s%03d", prefix[len(prefix)-4:], i), 2100, 2100)
		p.Account.Name, p.Char.UID = fmt.Sprintf("%s%03d", prefix, i), ""
		if err := st.CreateAccount(p.Account); err != nil {
			t.Fatal(err)
		}
		for slot := range 30 {
			// Live items carry their identity, as materializeItem gives them.
			item, err := materializeItem(model.Item{Index: uint16(400 + slot)})
			if err != nil {
				t.Fatal(err)
			}
			p.Char.Inv[slot] = item
		}
		p.Char.Gold = uint32(1000 + i)
		players = append(players, p)
	}
	for _, count := range []int{50, 150, 300} {
		for _, workers := range []int{1, 4} {
			w := worldWithNetworkedPlayers(players[:count]...)
			w.store, w.shutdownSaveWorkers = st, workers
			for _, p := range players[:count] {
				p.Char.Gold++ // every final snapshot differs from the stored row
			}
			started := time.Now()
			w.persistShutdownState()
			elapsed := time.Since(started)
			if w.shutdownErr != nil {
				t.Fatalf("%d accounts, %d workers: %v", count, workers, w.shutdownErr)
			}
			t.Logf("D11 drain: %d populated accounts, %d worker(s): %s (%.1f ms/account)",
				count, workers, elapsed.Round(time.Millisecond), float64(elapsed.Microseconds())/1000/float64(count))
		}
	}
}
