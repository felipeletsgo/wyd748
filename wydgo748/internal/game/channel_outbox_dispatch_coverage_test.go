package game

import (
	"encoding/json"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

func deliverChannelMessageForTest(t *testing.T, w *World, kind string, payload outboxPayload) {
	t.Helper()
	raw, err := json.Marshal(payload)
	if err != nil {
		t.Fatal(err)
	}
	w.onChannelMessage(&store.ChannelMessage{Kind: kind, Payload: raw})
}

func TestOnChannelMessageRoutesGlobalAndCitizenshipAudience(t *testing.T) {
	north, _ := networkedTestPlayer(1, "North", 2100, 2100)
	south, _ := networkedTestPlayer(2, "South", 2100, 2100)
	offline, _ := networkedTestPlayer(3, "Offline", 2100, 2100)
	missingChar, _ := networkedTestPlayer(4, "MissingChar", 2100, 2100)
	nilEntry, _ := networkedTestPlayer(5, "NilEntry", 2100, 2100)
	offline.InWorld = false
	missingChar.Char = nil
	w := worldWithNetworkedPlayers(north, south, offline, missingChar)
	w.players[nil] = &Player{InWorld: true, Char: &model.Char{Citizenship: 1}}
	w.players[nilEntry.Session] = nil
	north.Char.Citizenship, south.Char.Citizenship = 1, 2
	for _, p := range []*Player{north, south, offline, missingChar} {
		drainPlayerPackets(p)
	}

	deliverChannelMessageForTest(t, w, outboxGlobalChat,
		outboxPayload{Sender: "Remote", Message: "global"})
	if north.Session.QueuedPacketsForTest() != 1 || south.Session.QueuedPacketsForTest() != 1 ||
		offline.Session.QueuedPacketsForTest() != 0 || missingChar.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("global message did not reach only connected in-world characters")
	}
	drainPlayerPackets(north)
	drainPlayerPackets(south)

	deliverChannelMessageForTest(t, w, outboxCitizenshipChat,
		outboxPayload{Sender: "Citizen", Message: "local", Citizenship: 1})
	if north.Session.QueuedPacketsForTest() != 1 || south.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("citizenship message crossed its citizenship audience")
	}
}

func TestOnChannelMessageRestrictsGuildChatToCanonicalMembers(t *testing.T) {
	alpha, _ := networkedTestPlayer(1, "Alpha", 2100, 2100)
	beta, _ := networkedTestPlayer(2, "Beta", 2100, 2100)
	forged, _ := networkedTestPlayer(3, "NotListed", 2100, 2100)
	w := worldWithNetworkedPlayers(alpha, beta, forged)
	alpha.Char.GuildID, beta.Char.GuildID, forged.Char.GuildID = 1, 1, 1
	w.guilds = &model.GuildRegistry{Guilds: []model.Guild{{
		ID:      1,
		Members: []model.GuildMember{{Character: alpha.Char.Name}, {Character: beta.Char.Name}},
	}}}
	for _, p := range []*Player{alpha, beta, forged} {
		drainPlayerPackets(p)
	}

	deliverChannelMessageForTest(t, w, outboxGuildChat,
		outboxPayload{Sender: "Guildie", Message: "guild", GuildID: 1})
	if alpha.Session.QueuedPacketsForTest() != 1 || beta.Session.QueuedPacketsForTest() != 1 ||
		forged.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("guild chat delivery trusted the character GuildID without registry membership")
	}
	drainPlayerPackets(alpha)
	drainPlayerPackets(beta)
	deliverChannelMessageForTest(t, w, outboxGuildChat,
		outboxPayload{Sender: "Guildie", Message: "guild", GuildID: 999})
	if alpha.Session.QueuedPacketsForTest() != 0 || beta.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("guild chat for a missing guild was delivered")
	}
}

func TestOnChannelMessageWhisperRepliesTrackDelivery(t *testing.T) {
	target, _ := networkedTestPlayer(1, "Target", 2100, 2100)
	w := worldWithNetworkedPlayers(target)
	outbox := &ChannelOutbox{process: "dispatch-test", channel: 1, queue: make(chan store.ChannelMessage, 4)}
	w.outbox = outbox
	drainPlayerPackets(target)

	deliverChannelMessageForTest(t, w, outboxWhisper, outboxPayload{
		Request: "missing-recipient", ReplyTo: 2, Sender: "Remote", Target: "Nobody", Message: "hello",
	})
	missingReply := <-outbox.queue
	var missing outboxPayload
	if missingReply.Kind != outboxReply || missingReply.Target != 2 || json.Unmarshal(missingReply.Payload, &missing) != nil ||
		missing.Request != "missing-recipient" || missing.Delivered {
		t.Fatalf("missing-recipient reply is incorrect: message=%+v payload=%+v", missingReply, missing)
	}

	deliverChannelMessageForTest(t, w, outboxWhisper, outboxPayload{
		Request: "delivered", ReplyTo: 2, Sender: "Remote", Target: "Target", Message: "hello",
	})
	deliveredReply := <-outbox.queue
	var delivered outboxPayload
	if deliveredReply.Kind != outboxReply || deliveredReply.Target != 2 || json.Unmarshal(deliveredReply.Payload, &delivered) != nil ||
		!delivered.Delivered || delivered.Request != "delivered" || target.Session.QueuedPacketsForTest() != 1 {
		t.Fatalf("delivered whisper reply is incorrect: message=%+v payload=%+v packets=%d",
			deliveredReply, delivered, target.Session.QueuedPacketsForTest())
	}

	deliverChannelMessageForTest(t, w, outboxWhisper, outboxPayload{
		Request: "command", ReplyTo: 2, Sender: "Remote", Target: "Target", Message: "!remote-command",
	})
	if commandReply := <-outbox.queue; commandReply.Kind != outboxReply || target.Session.QueuedPacketsForTest() != 2 {
		t.Fatal("whisper command was not delivered or replied to")
	}
}

func TestOnChannelMessageHandlesStaffAndInvalidPayloads(t *testing.T) {
	staff, _ := networkedTestPlayer(1, "Staff", 2100, 2100)
	w := worldWithNetworkedPlayers(staff)
	drainPlayerPackets(staff)

	deliverChannelMessageForTest(t, w, outboxStaffNotice, outboxPayload{Message: "maintenance"})
	if staff.Session.QueuedPacketsForTest() != 2 {
		t.Fatal("staff notice did not send both native notice packets")
	}
	drainPlayerPackets(staff)

	w.onChannelMessage(&store.ChannelMessage{Kind: outboxGlobalChat, Payload: []byte("{")})
	w.onChannelMessage(&store.ChannelMessage{Kind: "unknown", Payload: []byte(`{}`)})
	w.onChannelMessage(&store.ChannelMessage{Kind: outboxSanction, Payload: []byte(`{"account":"missing-backend"}`)})
	if staff.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("invalid, unknown or unsupported sanction message reached a player")
	}
}
