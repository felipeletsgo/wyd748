package game

import (
	"context"
	"encoding/json"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// W9 phase 3 on PostgreSQL: two game processes (two pools, two process names)
// change one guild. A commit of process A reaches process B through the
// notification; a write of B from a stale copy is rejected, B reloads the row,
// and its retry commits and reaches A.
func TestGuildRowsStayConsistentAcrossProcessesPostgres(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	ctx := context.Background()
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	const id = 4089
	cleanup := func() { _, _ = conn.Exec(ctx, `DELETE FROM guild_rows WHERE guild_id=$1`, id) }
	cleanup()
	t.Cleanup(func() { cleanup(); _ = conn.Close(ctx) })

	open := func() (*store.PostgresStore, string) {
		st, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: url, MaxConns: 4})
		if err != nil {
			t.Fatal(err)
		}
		t.Cleanup(st.Close)
		name, err := st.RegisterGameProcess()
		if err != nil {
			t.Fatal(err)
		}
		st.EnableGuildRowSync()
		return st, name
	}
	storeA, nameA := open()
	storeB, nameB := open()
	// Seed the guild through process A's store before both processes start.
	seed, err := storeA.LoadGuilds()
	if err != nil {
		t.Fatal(err)
	}
	if seed == nil {
		seed = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	}
	seed.Guilds = append(seed.Guilds, model.Guild{ID: id, Name: "W9Sync", Fame: 10,
		Members: []model.GuildMember{{Character: "W9SyncLead", Account: "w9sync", Rank: model.GuildRankLeader}}})
	if err := storeA.SaveCriticalGameStateRows(seed, []store.GuildParticipant{{GuildID: id, Version: -1}}, nil, nil, "", nil); err != nil {
		t.Fatalf("seed: %v", err)
	}
	world := func(st *store.PostgresStore) *World {
		w, err := NewWorld(st, nil, nil, model.Catalog{}, [model.MaxCarry]int{}, model.VolatileCatalog{},
			model.CharacterTemplateFile{}, model.TerrainMap{}, WithCriticalPersistence(2, 16), WithGuildRowVersions())
		if err != nil {
			t.Fatal(err)
		}
		return w
	}
	a, b := world(storeA), world(storeB)
	listen, stop := context.WithCancel(ctx)
	defer stop()
	go storeB.ListenGuildRows(listen, nameB, b.NotifyGuildRows)
	go storeA.ListenGuildRows(listen, nameA, a.NotifyGuildRows)
	time.Sleep(300 * time.Millisecond) // let both LISTEN start

	fame := func(w *World) uint32 {
		guild, _ := guildByID(w.guilds, id)
		return guild.Fame
	}
	if fame(a) != 10 || fame(b) != 10 || a.guildVersions[id] != 1 || b.guildVersions[id] != 1 {
		t.Fatalf("boot state: a=%d/%d b=%d/%d", fame(a), a.guildVersions[id], fame(b), b.guildVersions[id])
	}
	// change submits a keyed plan through the owner and returns its result.
	change := func(w *World, value uint32) error {
		retained, proposed := w.criticalGuildSnapshot(), w.criticalGuildSnapshot()
		for i := range proposed.Guilds {
			if proposed.Guilds[i].ID == id {
				proposed.Guilds[i].Fame = value
			}
		}
		plan := newCriticalGuildPlan(w.guilds, retained, proposed, true)
		done := make(chan error, 1)
		_, err := w.criticalWrites.submitWrite(criticalWrite{guilds: plan.proposed, guildKeys: plan.keys()}, false,
			func(result criticalPersistenceResult) error {
				if result.err == nil {
					plan.install(w)
				}
				done <- result.err
				return nil
			}, nil, func(result criticalPersistenceResult) error {
				// A certified rejection settles the reservation, as producers do.
				done <- result.err
				return nil
			})
		if err != nil {
			return err
		}
		for end := time.Now().Add(10 * time.Second); time.Now().Before(end); time.Sleep(5 * time.Millisecond) {
			w.processCriticalCompletions(criticalCompletionBatchLimit)
			select {
			case err := <-done:
				return err
			default:
			}
		}
		t.Fatal("guild write did not complete")
		return nil
	}
	// pump runs the World's queued guild sync commands until cond holds.
	pump := func(w *World, cond func() bool) {
		t.Helper()
		for end := time.Now().Add(10 * time.Second); !cond(); {
			if time.Now().After(end) {
				t.Fatal("guild sync did not converge")
			}
			select {
			case cmd := <-w.commands:
				if cmd.guildSync != nil {
					w.onGuildSync(cmd.guildSync)
				}
			case <-time.After(20 * time.Millisecond):
				w.startGuildRefresh()
			}
		}
	}

	if err := change(a, 11); err != nil {
		t.Fatalf("process A write: %v", err)
	}
	if a.guildVersions[id] != 2 {
		t.Fatalf("process A version %d after its commit", a.guildVersions[id])
	}
	// Process B still holds version 1: its write is rejected and B reloads.
	if err := change(b, 50); !store.IsWriteRejected(err) {
		t.Fatalf("stale write of process B: %v", err)
	}
	pump(b, func() bool { return fame(b) == 11 && b.guildVersions[id] == 2 })
	var stored []byte
	if err := conn.QueryRow(ctx, `SELECT payload FROM guild_rows WHERE guild_id=$1`, id).Scan(&stored); err != nil ||
		!containsFame(stored, 11) {
		t.Fatalf("the stale write changed the row: %s err=%v", stored, err)
	}
	if err := change(b, 50); err != nil {
		t.Fatalf("process B retry: %v", err)
	}
	pump(a, func() bool { return fame(a) == 50 && a.guildVersions[id] == 3 })
}

func containsFame(payload []byte, fame int) bool {
	var guild model.Guild
	if err := json.Unmarshal(payload, &guild); err != nil {
		return false
	}
	return int(guild.Fame) == fame
}
