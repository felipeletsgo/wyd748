package game

import (
	"context"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/control"
	"wydgo/internal/net"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// W9 phase 5 on PostgreSQL: the staff panel of channel 1 lists a player of
// the channel 2 process, sees the account online, and kicks it; a whisper
// to a character that no process holds reports "not connected".
func TestStaffOperationsReachOtherChannelProcessesPostgres(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	ctx := context.Background()
	pg, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: url, MaxConns: 6})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(pg.Close)
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	cleanup := func() {
		_, _ = conn.Exec(ctx, `DELETE FROM channel_messages WHERE source LIKE 'w9-test/staff-%'`)
	}
	cleanup()
	t.Cleanup(func() { cleanup(); _ = conn.Close(ctx) })

	run, stop := context.WithCancel(ctx)
	defer stop()
	world := func(channel, peer byte) *World {
		w := newZoneTestWorld()
		w.channel = channel
		w.commands = make(chan command, 64)
		w.accountSessions = make(map[string]*net.Session)
		w.outbox = NewChannelOutbox(pg, "w9-test/staff-"+string('0'+rune(channel)), channel)
		WithRemoteChannels(peer)(w)
		go w.outbox.Run(run, w)
		go func() {
			for {
				select {
				case <-run.Done():
					return
				case cmd := <-w.commands:
					w.handle(cmd)
				}
			}
		}()
		return w
	}
	one, two := world(1, 2), world(2, 1)
	// Players are added before the actors receive any command.
	sender := addZonePlayer(one, 1, 2100, 2100, 1000)
	sender.Account.Name, sender.Char.Name = "staffsender", "StaffSender"
	target := addZonePlayer(two, 2, 2100, 2100, 1000)
	target.Account.Name, target.Char.Name, target.Char.UID = "staffacc", "StaffTarget", "w9-staff-uid"
	time.Sleep(300 * time.Millisecond) // readers take their start time

	overview, err := one.Overview(ctx, control.Query{Search: "stafftarget"})
	if err != nil {
		t.Fatal(err)
	}
	if len(overview.Players) != 1 || overview.Players[0].UID != "w9-staff-uid" || overview.Players[0].SessionID != target.Session.ID {
		t.Fatalf("channel 1 overview does not list the channel 2 player: %+v", overview.Players)
	}
	if all, err := one.Overview(ctx, control.Query{}); err != nil || all.Online != 2 {
		t.Fatalf("combined online count %d err=%v", all.Online, err)
	}
	presence, err := one.AccountPresence(ctx, []string{"staffacc", "nobody"})
	if err != nil || !presence["staffacc"] || presence["nobody"] {
		t.Fatalf("presence %v err=%v", presence, err)
	}

	// A whisper to a character that no process holds reports "not connected".
	one.commands <- command{apply: func() { one.deliverWhisper(sender.Session, sender, "Nobody", "hello?") }}
	deadline := time.Now().Add(10 * time.Second)
	for notice := false; !notice; {
		if time.Now().After(deadline) {
			t.Fatal("no offline notice for a whisper that no process delivered")
		}
		if pkt, ok := sender.Session.DequeuePacketForTest(); ok && wire.Decrypt(pkt) && wire.ParseHeader(pkt).Type == wire.OpMessagePanel &&
			strings.Contains(string(pkt[12:]), one.msg("_NN_Not_Connected")) {
			notice = true
		}
		time.Sleep(10 * time.Millisecond)
	}

	result, err := one.KickCommand(ctx, "staff", control.KickCommand{Version: 1, OperationID: "w9-remote-kick-0001",
		Epoch: overview.ModerationEpoch, UID: "w9-staff-uid", Account: "staffacc", ExpectedSessionID: target.Session.ID,
		Reason: "remote kick test"}, func() bool { return true })
	if err != nil || (result.Code != "ok" && result.Code != "disconnected_persistence_failed") {
		t.Fatalf("remote kick: %+v err=%v", result, err)
	}
	if !target.Session.IsClosed() {
		t.Fatal("the channel 2 session is still open after the kick")
	}
}
