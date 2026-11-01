package game

import (
	"context"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

// W9 phase 4 on PostgreSQL: channel 1 and channel 2 run in separate
// processes. A whisper, a guild chat line, a global chat line and a staff
// notice from channel 1 reach the right players of channel 2 exactly once.
func TestChannelOutboxCarriesChatBetweenProcessesPostgres(t *testing.T) {
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
		_, _ = conn.Exec(ctx, `DELETE FROM channel_messages WHERE source LIKE 'w9-test/outbox-%'`)
	}
	cleanup()
	t.Cleanup(func() { cleanup(); _ = conn.Close(ctx) })

	world := func(channel byte) *World {
		w := newZoneTestWorld()
		w.channel = channel
		w.commands = make(chan command, 64)
		w.guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{{ID: 7, Name: "Remote",
			Members: []model.GuildMember{{Character: "Sender", Account: "sender", Rank: model.GuildRankLeader},
				{Character: "Member", Account: "member"}}}}}
		return w
	}
	player := func(w *World, id uint16, name string, guild uint16) *Player {
		p := addZonePlayer(w, id, 2100, 2100, 1000)
		p.Account.Name, p.Char.Name, p.Char.GuildID = strings.ToLower(name), name, guild
		return p
	}
	one, two := world(1), world(2)
	one.outbox = NewChannelOutbox(pg, "w9-test/outbox-1", 1)
	two.outbox = NewChannelOutbox(pg, "w9-test/outbox-2", 2)
	WithRemoteChannels(2)(one)
	WithRemoteChannels(1)(two)
	sender := player(one, 1, "Sender", 7)
	member := player(two, 2, "Member", 7)
	stranger := player(two, 3, "Stranger", 0)
	run, stop := context.WithCancel(ctx)
	defer stop()
	go one.outbox.Run(run, one)
	go two.outbox.Run(run, two)
	time.Sleep(300 * time.Millisecond) // readers take their start time

	one.deliverWhisper(sender.Session, sender, "Member", "hello across")
	one.sendGuildChat(sender, "-guild line")
	one.outbox.publish(outboxGlobalChat, outboxPayload{Sender: "Sender", Message: "=global line"})
	one.staffBroadcast("staff line")

	received := map[*Player][]string{}
	deadline := time.Now().Add(10 * time.Second)
	for len(received[member]) < 4 || len(received[stranger]) < 2 {
		if time.Now().After(deadline) {
			t.Fatalf("messages did not arrive: member=%q stranger=%q", received[member], received[stranger])
		}
		select {
		case cmd := <-two.commands:
			if cmd.channelMessage != nil {
				two.onChannelMessage(cmd.channelMessage)
			}
		case <-time.After(50 * time.Millisecond):
		}
		for _, p := range []*Player{member, stranger} {
			for {
				pkt, ok := p.Session.DequeuePacketForTest()
				if !ok {
					break
				}
				if wire.Decrypt(pkt) && wire.ParseHeader(pkt).Type == wire.OpMessageWhisper {
					received[p] = append(received[p], cstr(pkt[28:]))
				}
			}
		}
	}
	joined := strings.Join(received[member], "|")
	for _, want := range []string{"hello across", "-guild line", "=global line", "staff line"} {
		if !strings.Contains(joined, want) {
			t.Fatalf("member missed %q: %q", want, received[member])
		}
	}
	if other := strings.Join(received[stranger], "|"); strings.Contains(other, "hello across") || strings.Contains(other, "-guild line") {
		t.Fatalf("stranger received a private line: %q", received[stranger])
	}
	// Repeated reads do not deliver a message twice.
	time.Sleep(2500 * time.Millisecond)
	for {
		select {
		case cmd := <-two.commands:
			if cmd.channelMessage != nil {
				t.Fatalf("message delivered twice: %s", cmd.channelMessage.Kind)
			}
		default:
			if one.players[sender.Session] == nil {
				t.Fatal("sender left")
			}
			return
		}
	}
}
