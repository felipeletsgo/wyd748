package game

import (
	"context"
	"encoding/json"
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

func TestRemoteTeleportUsesPeerResultAndLocalFallback(t *testing.T) {
	for _, test := range []struct {
		name string
		body json.RawMessage
		want string
	}{
		{name: "peer success", body: json.RawMessage(`{"code":"ok"}`), want: "ok"},
		{name: "empty result", body: json.RawMessage(`{}`), want: "local"},
		{name: "stale target", body: json.RawMessage(`{"code":"stale_target"}`), want: "local"},
		{name: "invalid result", body: json.RawMessage(`[]`), want: "local"},
	} {
		t.Run(test.name, func(t *testing.T) {
			outbox := &ChannelOutbox{process: "staff-test", channel: 1, queue: make(chan store.ChannelMessage, 1)}
			w := &World{outbox: outbox, remoteChannels: map[byte]bool{2: true}}
			go func() {
				message := <-outbox.queue
				var request outboxPayload
				if json.Unmarshal(message.Payload, &request) != nil {
					return
				}
				raw, err := json.Marshal(outboxPayload{Request: request.Request, Body: test.body})
				if err == nil {
					outbox.deliverReply(raw)
				}
			}()
			local := control.TeleportResult{Code: "local"}
			got := w.remoteTeleport(context.Background(), "operator", control.TeleportCommand{}, local)
			if got.Code != test.want {
				t.Fatalf("remote teleport code=%q, want %q", got.Code, test.want)
			}
		})
	}
	local := control.TeleportResult{Code: "local"}
	if got := (&World{}).remoteTeleport(context.Background(), "operator", control.TeleportCommand{}, local); got != local {
		t.Fatalf("no-peer fallback=%+v, want %+v", got, local)
	}
}

func remoteStaffOutbox() *ChannelOutbox {
	return &ChannelOutbox{process: "remote-staff-test", channel: 1, queue: make(chan store.ChannelMessage, 1)}
}

func awaitRemoteStaffReply(t *testing.T, outbox *ChannelOutbox) store.ChannelMessage {
	t.Helper()
	select {
	case message := <-outbox.queue:
		if message.Kind != outboxReply || message.Target != 2 {
			t.Fatalf("unexpected remote staff reply message: %+v", message)
		}
		return message
	case <-time.After(time.Second):
		t.Fatal("remote staff request did not reply")
		return store.ChannelMessage{}
	}
}

func TestRemoteStaffRequestHandlesOverviewPresenceKickAndTeleport(t *testing.T) {
	t.Run("rejects malformed and oversized requests", func(t *testing.T) {
		outbox := remoteStaffOutbox()
		w := newZoneTestWorld()
		w.outbox = outbox
		payload := outboxPayload{Request: "invalid-request", ReplyTo: 2, Body: json.RawMessage("{")}
		w.onRemoteStaffRequest(outboxStaffOverview, payload)
		w.onRemoteStaffRequest(outboxStaffPresence, payload)
		tooMany, _ := json.Marshal(make([]string, 51))
		payload.Body = tooMany
		w.onRemoteStaffRequest(outboxStaffPresence, payload)
		payload.Body = json.RawMessage(`{}`)
		w.onRemoteStaffRequest(outboxStaffKick, payload)
		w.onRemoteStaffRequest(outboxStaffTeleport, payload)
		if len(outbox.queue) != 0 {
			t.Fatal("invalid remote staff command received a reply")
		}
	})

	t.Run("overview", func(t *testing.T) {
		outbox := remoteStaffOutbox()
		w := newZoneTestWorld()
		w.outbox = outbox
		body, _ := json.Marshal(control.Query{})
		w.onRemoteStaffRequest(outboxStaffOverview, outboxPayload{Request: "overview-request", ReplyTo: 2, Body: body})
		message := awaitRemoteStaffReply(t, outbox)
		var reply outboxPayload
		var part overviewPart
		if json.Unmarshal(message.Payload, &reply) != nil || json.Unmarshal(reply.Body, &part) != nil ||
			reply.Request != "overview-request" || part.Players == nil {
			t.Fatalf("overview reply=%+v part=%+v", reply, part)
		}
	})

	t.Run("presence", func(t *testing.T) {
		outbox := remoteStaffOutbox()
		w, p, _ := handlerTestWorld(t)
		w.outbox = outbox
		p.Account.Name = "player"
		body, _ := json.Marshal([]string{"player", "offline"})
		w.onRemoteStaffRequest(outboxStaffPresence, outboxPayload{Request: "presence-request", ReplyTo: 2, Body: body})
		message := awaitRemoteStaffReply(t, outbox)
		var reply outboxPayload
		var present map[string]bool
		if json.Unmarshal(message.Payload, &reply) != nil || json.Unmarshal(reply.Body, &present) != nil ||
			reply.Request != "presence-request" || !present["player"] || present["offline"] {
			t.Fatalf("presence reply=%+v presence=%v", reply, present)
		}
	})

	t.Run("kick", func(t *testing.T) {
		outbox := remoteStaffOutbox()
		w := newZoneTestWorld()
		w.outbox = outbox
		cmd := control.KickCommand{Version: 1, OperationID: "operation-0000001", Epoch: strings.Repeat("a", 32),
			UID: "target-uid", Account: "player", ExpectedSessionID: 1, Reason: "verified report"}
		body, _ := json.Marshal(remoteStaffCommand{Actor: "operator", Kick: cmd})
		w.onRemoteStaffRequest(outboxStaffKick, outboxPayload{Request: "kick-request", ReplyTo: 2, Body: body})
		message := awaitRemoteStaffReply(t, outbox)
		var reply outboxPayload
		var result control.KickResult
		if json.Unmarshal(message.Payload, &reply) != nil || json.Unmarshal(reply.Body, &result) != nil || result.Code != "stale_target" {
			t.Fatalf("remote kick result=%+v, reply=%+v", result, reply)
		}
	})

	t.Run("teleport", func(t *testing.T) {
		outbox := remoteStaffOutbox()
		w := newZoneTestWorld()
		w.outbox = outbox
		cmd := control.TeleportCommand{Version: 1, OperationID: "operation-0000002", Epoch: strings.Repeat("b", 32),
			UID: "target-uid", Account: "player", ExpectedSessionID: 1, Reason: "verified report", Destination: "armia"}
		body, _ := json.Marshal(remoteStaffCommand{Actor: "operator", Teleport: cmd})
		w.onRemoteStaffRequest(outboxStaffTeleport, outboxPayload{Request: "teleport-request", ReplyTo: 2, Body: body})
		message := awaitRemoteStaffReply(t, outbox)
		var reply outboxPayload
		var result control.TeleportResult
		if json.Unmarshal(message.Payload, &reply) != nil || json.Unmarshal(reply.Body, &result) != nil || result.Code != "stale_target" {
			t.Fatalf("remote teleport result=%+v, reply=%+v", result, reply)
		}
	})
}

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
