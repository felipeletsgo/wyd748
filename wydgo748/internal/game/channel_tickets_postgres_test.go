package game

import (
	"context"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// W9 phase 2 on PostgreSQL: channel 1 and channel 2 run in separate
// processes. The source stores a durable ticket, reveals it only after the
// final write, and the target consumes it with the account lease. The ticket
// cannot be replayed, and a plain login waits while the transfer is pending.
func TestDurableChannelTicketMovesThePlayerBetweenProcessesPostgres(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	ctx := context.Background()
	pg, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: url, MaxConns: 4})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(pg.Close)
	conn, err := pgx.Connect(ctx, url)
	if err != nil {
		t.Fatal(err)
	}
	const account, key = "W9Transfer", "w9transfer"
	cleanup := func() {
		_, _ = conn.Exec(ctx, `DELETE FROM channel_tickets WHERE account_key=$1`, key)
		_, _ = conn.Exec(ctx, `DELETE FROM account_leases WHERE account_key=$1`, key)
		_, _ = conn.Exec(ctx, `DELETE FROM game_processes WHERE process LIKE 'w9-test/channel-%'`)
	}
	cleanup()
	t.Cleanup(func() { cleanup(); _ = conn.Close(ctx) })
	for _, process := range []string{"w9-test/channel-1", "w9-test/channel-2"} {
		if err := pg.HeartbeatGameProcess(process); err != nil {
			t.Fatal(err)
		}
	}

	// Source process: channel 1 with the player in the world.
	one, p, source := newEnterWorldPlayer(t, 742, 1000)
	p.Account.Name, p.Account.Chars[0].Name, p.Account.Chars[0].UID = account, "TransferHero", "w9-transfer-hero"
	p.Account.Chars[0].Gold = 4321
	one.onEnterWorld(source, enterWorldPacket(0))
	if !p.InWorld {
		t.Fatal("source did not enter")
	}
	for source.QueuedPacketsForTest() > 0 {
		source.DequeuePacketForTest()
	}
	base := &criticalTestStore{entered: make(chan criticalWrite, 4)}
	backend := &channelTransferStore{criticalStateTestStore: &criticalStateTestStore{criticalBatchTestStore: &criticalBatchTestStore{base}}}
	one.store, one.channel = backend, 1
	one.commands = make(chan command, 4)
	one.leases = &AccountLeases{store: pg, tickets: pg, process: "w9-test/channel-1"}
	WithRemoteChannels(2)(one)
	one.accountSessions = map[string]*gameNet.Session{key: source}
	if err := one.leases.claim(account, 1, source.ID); err != nil {
		t.Fatalf("source lease: %v", err)
	}
	one.holdAccountLease(source, account, true)

	one.requestChannelTransfer(p, "2")
	if one.remoteTransfers[source] == nil || source.QueuedPacketsForTest() != 0 {
		t.Fatal("remote transfer did not wait for the durable ticket")
	}
	cmd := <-one.commands
	// A plain login of the account is refused while the ticket waits.
	if err := (&AccountLeases{store: pg, process: "w9-test/channel-2"}).claim(account, 2, 99); err != errAccountTransferPending {
		t.Fatalf("plain login during the transfer: %v", err)
	}
	one.onChannelTicketIssued(cmd.s, cmd.channelTicket)
	if write := awaitCriticalWrite(t, base.entered); write.character == nil || write.character.uid != "w9-transfer-hero" {
		t.Fatal("transfer did not save the final account and character state")
	}
	packet, ok := source.DequeuePacketForTest()
	if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != 0x52A {
		t.Fatal("missing migration response")
	}
	token := cstr(packet[28:80])
	if !strings.HasPrefix(token, "*2.") || one.players[source] != nil {
		t.Fatalf("source did not detach after the ticket: token=%q", token)
	}

	// Target process: channel 2 consumes the ticket with the account lease.
	destination := gameNet.NewTestSessionWithRemoteIP(20, 256, "192.0.2.20")
	two := newZoneTestWorld()
	two.gameplay = one.gameplay
	two.store, two.channel = backend, 2
	two.accountSessions = make(map[string]*gameNet.Session)
	two.authPending = map[*gameNet.Session]bool{destination: true}
	two.leases = &AccountLeases{store: pg, tickets: pg, process: "w9-test/channel-2"}
	hash := channelTicketHash(token)
	if valid, err := pg.ChannelTicketValid(hash, account, 2); err != nil || !valid {
		t.Fatalf("target ticket check: valid=%t err=%v", valid, err)
	}
	two.onLoginResult(destination, &loginResult{accountName: account, account: accountStateSnapshot(backend.account), durableTicket: hash})
	arrived := two.players[destination]
	if arrived == nil || !arrived.InWorld || arrived.Char.UID != "w9-transfer-hero" || arrived.Char.Gold != 4321 {
		t.Fatalf("target did not enter the committed character: %+v", arrived)
	}
	var owner string
	var session int64
	if err := conn.QueryRow(ctx, `SELECT process, session FROM account_leases WHERE account_key=$1`, key).Scan(&owner, &session); err != nil ||
		owner != "w9-test/channel-2" || session != destination.ID {
		t.Fatalf("lease after transfer: %q/%d err=%v", owner, session, err)
	}

	// The same token cannot admit another session.
	replay := gameNet.NewTestSessionWithRemoteIP(21, 64, "192.0.2.21")
	three := newZoneTestWorld()
	three.store, three.channel = backend, 2
	three.accountSessions = make(map[string]*gameNet.Session)
	three.authPending = map[*gameNet.Session]bool{replay: true}
	three.leases = &AccountLeases{store: pg, tickets: pg, process: "w9-test/channel-2b"}
	three.onLoginResult(replay, &loginResult{accountName: account, account: accountStateSnapshot(backend.account), durableTicket: hash})
	pkt, ok := replay.DequeuePacketForTest()
	if !ok || !wire.Decrypt(pkt) || !strings.Contains(string(pkt[12:]), three.msg("_WG_Channel_Transfer_Expired_Sign_Again")) ||
		three.players[replay] != nil {
		t.Fatal("a replayed ticket admitted another session")
	}
	// The source's lease release does not remove the target's lease.
	waitFor(t, func() bool {
		var count int
		_ = conn.QueryRow(ctx, `SELECT count(*) FROM account_leases WHERE account_key=$1 AND process='w9-test/channel-2'`, key).Scan(&count)
		return count == 1
	})
	time.Sleep(50 * time.Millisecond)
	if err := conn.QueryRow(ctx, `SELECT process FROM account_leases WHERE account_key=$1`, key).Scan(&owner); err != nil || owner != "w9-test/channel-2" {
		t.Fatalf("source release changed the target lease: %q err=%v", owner, err)
	}
}
