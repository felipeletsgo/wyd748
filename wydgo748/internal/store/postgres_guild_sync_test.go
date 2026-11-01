package store

import (
	"context"
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

// W9 phase 3: versioned guild rows and change notifications.

func TestVersionedGuildWritesRejectStaleCopies(t *testing.T) {
	tx, ctx := guildRowsTx(t)
	const a, b = 60011, 60012
	registry := func(fame uint32, withB bool) *model.GuildRegistry {
		r := &model.GuildRegistry{Version: model.GuildRegistryVersion,
			Guilds: []model.Guild{{ID: a, Name: "W9A", Fame: fame}}}
		if withB {
			r.Guilds = append(r.Guilds, model.Guild{ID: b, Name: "W9B"})
		}
		return r
	}
	// A new guild expects no row.
	if err := writeGuildRows(ctx, tx, registry(1, false), []GuildParticipant{{GuildID: a, Version: -1}}); err != nil {
		t.Fatal(err)
	}
	if err := writeGuildRows(ctx, tx, registry(1, false), []GuildParticipant{{GuildID: a, Version: -1}}); !errors.Is(err, ErrGuildRowConflict) {
		t.Fatalf("second creation of one guild: %v", err)
	}
	var version int64
	if err := tx.QueryRow(ctx, `SELECT version FROM guild_rows WHERE guild_id=$1`, a).Scan(&version); err != nil || version != 1 {
		t.Fatalf("new row version %d err=%v", version, err)
	}
	if err := writeGuildRows(ctx, tx, registry(2, false), []GuildParticipant{{GuildID: a, Version: 1}}); err != nil {
		t.Fatalf("write with the read version: %v", err)
	}
	// Another process read version 1 too; its write is now stale.
	if err := writeGuildRows(ctx, tx, registry(9, false), []GuildParticipant{{GuildID: a, Version: 1}}); !errors.Is(err, ErrGuildRowConflict) {
		t.Fatalf("stale write: %v", err)
	}
	if fame, _ := guildRowFame(t, tx, ctx, a); fame != 2 {
		t.Fatalf("stale write changed the row: fame %d", fame)
	}
	// A stale removal is refused; a current one removes the row.
	empty := &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{}}
	if err := writeGuildRows(ctx, tx, empty, []GuildParticipant{{GuildID: a, Version: 1}}); !errors.Is(err, ErrGuildRowConflict) {
		t.Fatalf("stale removal: %v", err)
	}
	if err := writeGuildRows(ctx, tx, empty, []GuildParticipant{{GuildID: a, Version: 2}}); err != nil {
		t.Fatalf("current removal: %v", err)
	}
	if _, present := guildRowFame(t, tx, ctx, a); present {
		t.Fatal("current removal kept the row")
	}
	// An absent guild that another process created is a conflict.
	if err := writeGuildRows(ctx, tx, registry(1, true), []GuildParticipant{{GuildID: b, Version: -1}}); err != nil {
		t.Fatal(err)
	}
	if err := writeGuildRows(ctx, tx, empty, []GuildParticipant{{GuildID: b, Version: -1}}); !errors.Is(err, ErrGuildRowConflict) {
		t.Fatalf("absent copy of a created guild: %v", err)
	}
}

func TestGuildRowStatesAndNotifications(t *testing.T) {
	st, conn := instanceRowsStore(t)
	ctx := context.Background()
	const a = 4093
	cleanup := func() { _, _ = conn.Exec(ctx, `DELETE FROM guild_rows WHERE guild_id=$1`, a) }
	cleanup()
	t.Cleanup(cleanup)
	name := "w9-test/guild-writer"
	st.leaseProcess.Store(&name)
	st.EnableGuildRowSync()
	listenCtx, stop := context.WithCancel(ctx)
	defer stop()
	notified := make(chan []uint16, 4)
	go st.ListenGuildRows(listenCtx, "w9-test/guild-reader", func(ids []uint16) { notified <- ids })
	time.Sleep(200 * time.Millisecond) // let LISTEN start
	registry, err := st.LoadGuilds()
	if err != nil {
		t.Fatal(err)
	}
	if registry == nil {
		registry = &model.GuildRegistry{Version: model.GuildRegistryVersion}
	}
	registry.Guilds = append(registry.Guilds, model.Guild{ID: a, Name: "W9Notify", Fame: 5,
		Members: []model.GuildMember{{Character: "W9NotifyLead", Account: "w9notify", Rank: model.GuildRankLeader}}})
	if err := st.SaveCriticalGameStateRows(registry, []GuildParticipant{{GuildID: a, Version: -1}}, nil, nil, "", nil); err != nil {
		t.Fatalf("guild write: %v", err)
	}
	select {
	case ids := <-notified:
		if len(ids) != 1 || ids[0] != a {
			t.Fatalf("notification ids %v", ids)
		}
	case <-time.After(5 * time.Second):
		t.Fatal("no guild row notification")
	}
	states, err := st.LoadGuildRowStates([]uint16{a, 4094, 0})
	if err != nil {
		t.Fatal(err)
	}
	found := map[uint16]GuildRowState{}
	for _, state := range states {
		found[state.GuildID] = state
	}
	if s := found[a]; !s.Present || s.Version != 1 || s.Guild.Fame != 5 {
		t.Fatalf("guild state %+v", s)
	}
	if s, ok := found[4094]; !ok || s.Present {
		t.Fatalf("missing guild state %+v", s)
	}
	if s := found[0]; !s.Present || s.Header == nil || s.Version < 1 {
		t.Fatalf("header state %+v", s)
	}
	// The writer ignores its own notifications.
	selfNotified := make(chan []uint16, 1)
	go st.ListenGuildRows(listenCtx, name, func(ids []uint16) { selfNotified <- ids })
	time.Sleep(200 * time.Millisecond)
	registry.Guilds[len(registry.Guilds)-1].Fame = 6
	if err := st.SaveCriticalGameStateRows(registry, []GuildParticipant{{GuildID: a, Version: 1}}, nil, nil, "", nil); err != nil {
		t.Fatalf("second guild write: %v", err)
	}
	<-notified
	select {
	case ids := <-selfNotified:
		t.Fatalf("writer received its own notification %v", ids)
	case <-time.After(300 * time.Millisecond):
	}
}

// W9 phase 5: a guild name taken by another process's commit refuses the
// second creation in the database.
func TestGuildNamesAreUniqueInTheDatabase(t *testing.T) {
	tx, ctx := guildRowsTx(t)
	first := &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{{ID: 4085, Name: "W9Same"}}}
	if err := writeGuildRows(ctx, tx, first, []GuildParticipant{{GuildID: 4085, Version: -1}}); err != nil {
		t.Fatal(err)
	}
	second := &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{{ID: 4086, Name: "w9same"}}}
	if err := writeGuildRows(ctx, tx, second, []GuildParticipant{{GuildID: 4086, Version: -1}}); !isUniqueViolation(err) {
		t.Fatalf("second guild with one name: %v", err)
	}
}
