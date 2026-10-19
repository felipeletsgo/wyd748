package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func guildDisablePackets(t *testing.T, p *Player) [][]byte {
	t.Helper()
	return labelPackets(t, p.Session)
}

func guildDisablePacketTypes(packets [][]byte) []uint16 {
	types := make([]uint16, 0, len(packets))
	for _, packet := range packets {
		types = append(types, wire.ParseHeader(packet).Type)
	}
	return types
}

func TestGuildDisableMasksPublicGuildStateWithoutChangingMembership(t *testing.T) {
	actor, _ := networkedTestPlayer(1, "GuildLeader", 2100, 2100)
	actor.Char.GuildID = 0x1234
	actor.Char.GuildRank = model.GuildRankLeader
	actor.GuildDisable = true

	maskedCreate := playerCreatePacket(actor)
	if got := binary.LittleEndian.Uint16(maskedCreate[134:136]); got != 0 {
		t.Fatalf("masked CreateMob guild=%#x, want zero", got)
	}
	if maskedCreate[136] != 0 {
		t.Fatalf("masked CreateMob rank=%d, want zero", maskedCreate[136])
	}

	maskedScore := playerScorePacket(actor)
	if got := binary.LittleEndian.Uint16(maskedScore[216:218]); got != 0 {
		t.Fatalf("masked UpdateScore guild=%#x, want zero", got)
	}
	if got := binary.LittleEndian.Uint16(maskedScore[218:220]); got != 0 {
		t.Fatalf("masked UpdateScore rank=%d, want zero", got)
	}
	if actor.Char.GuildID != 0x1234 || actor.Char.GuildRank != model.GuildRankLeader {
		t.Fatal("guild membership was modified by the public projection")
	}

	actor.GuildDisable = false
	unmasked := playerCreatePacket(actor)
	if got := binary.LittleEndian.Uint16(unmasked[134:136]); got != wire.GuildWireID(0x1234) ||
		unmasked[136] != model.GuildRankLeader {
		t.Fatalf("unmasked CreateMob guild/rank=%#x/%d, want %#x/%d", binary.LittleEndian.Uint16(unmasked[134:136]), unmasked[136], wire.GuildWireID(0x1234), model.GuildRankLeader)
	}
}

func TestGuildDisableUsesAuthenticatedSessionAndPreservesGuildAuthorization(t *testing.T) {
	actor, _ := networkedTestPlayer(1, "GuildLeader", 2100, 2100)
	actor.Char.GuildID = 7
	actor.Char.GuildRank = model.GuildRankLeader
	other, _ := networkedTestPlayer(2, "Other", 2101, 2100)
	w := worldWithNetworkedPlayers(actor, other)
	w.guilds.Guilds = []model.Guild{{ID: 7, Name: "Guild", Members: []model.GuildMember{{Character: actor.Char.Name, Rank: model.GuildRankLeader}}}}

	// The forged packet header names another player, but the authenticated
	// session remains the only source of actor identity.
	w.onMessageChat(actor.Session, wire.MessageChat(other.ID, "guildoff"))
	if !actor.GuildDisable || other.GuildDisable {
		t.Fatalf("forged header changed wrong actor: actor=%v other=%v", actor.GuildDisable, other.GuildDisable)
	}
	if actor.Char.GuildID != 7 || actor.Char.GuildRank != model.GuildRankLeader {
		t.Fatal("guild membership changed while toggling public affiliation")
	}
	if _, err := w.requireGuildLeader(actor.Char); err != nil {
		t.Fatalf("disabled actor lost guild-management authorization: %v", err)
	}
}

func TestGuildDisableBareTokenDoesNotLeakToLocalChat(t *testing.T) {
	actor, _ := networkedTestPlayer(1, "Actor", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	observer.show(actor.ID)
	w := worldWithNetworkedPlayers(actor, observer)

	w.onMessageChat(actor.Session, wire.MessageChat(actor.ID, "guildoff"))
	actorPackets := guildDisablePackets(t, actor)
	observerPackets := guildDisablePackets(t, observer)
	for _, packet := range actorPackets {
		if wire.ParseHeader(packet).Type == wire.OpMessageChat {
			t.Fatal("guildoff was relayed as local chat to the actor")
		}
	}
	for _, packet := range observerPackets {
		if wire.ParseHeader(packet).Type == wire.OpMessageChat {
			t.Fatal("guildoff leaked to a local-chat observer")
		}
	}
	if len(actorPackets) != 2 || wire.ParseHeader(actorPackets[0]).Type != wire.OpUpdateScore ||
		wire.ParseHeader(actorPackets[1]).Type != wire.OpGuildDisable {
		t.Fatalf("unexpected actor transition packets: %v", guildDisablePacketTypes(actorPackets))
	}
	if len(observerPackets) != 1 || wire.ParseHeader(observerPackets[0]).Type != wire.OpUpdateScore {
		t.Fatalf("unexpected observer transition packets: %v", guildDisablePacketTypes(observerPackets))
	}
}

func TestGuildDisableAcknowledgesTransitionsOnly(t *testing.T) {
	actor, _ := networkedTestPlayer(1, "Actor", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	observer.show(actor.ID)
	w := worldWithNetworkedPlayers(actor, observer)

	w.onMessageChat(actor.Session, wire.MessageChat(actor.ID, "guildoff"))
	firstActor := guildDisablePackets(t, actor)
	firstObserver := guildDisablePackets(t, observer)
	if len(firstActor) != 2 || wire.ParseHeader(firstActor[1]).Type != wire.OpGuildDisable ||
		binary.LittleEndian.Uint32(firstActor[1][12:16]) != 1 {
		t.Fatalf("disable transition ACK=%v, want one 0x3A4 Parm=1", firstActor)
	}
	if len(firstObserver) != 1 {
		t.Fatalf("disable transition observer packets=%d, want one score refresh", len(firstObserver))
	}

	w.onMessageChat(actor.Session, wire.MessageChat(actor.ID, "guildoff"))
	if actor.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("repeating guildoff emitted packets")
	}

	w.onMessageChat(actor.Session, wire.MessageChat(actor.ID, "guildon"))
	secondActor := guildDisablePackets(t, actor)
	secondObserver := guildDisablePackets(t, observer)
	if len(secondActor) != 2 || wire.ParseHeader(secondActor[1]).Type != wire.OpGuildDisable ||
		binary.LittleEndian.Uint32(secondActor[1][12:16]) != 0 {
		t.Fatalf("enable transition ACK=%v, want one 0x3A4 Parm=0", secondActor)
	}
	if len(secondObserver) != 1 {
		t.Fatalf("enable transition observer packets=%d, want one score refresh", len(secondObserver))
	}

	w.onMessageChat(actor.Session, wire.MessageChat(actor.ID, "guildon"))
	if actor.Session.QueuedPacketsForTest() != 0 || observer.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("repeating guildon emitted packets")
	}
}

func TestGuildDisableResetRestoresDefaultAndReconnectProjection(t *testing.T) {
	actor, _ := networkedTestPlayer(1, "Actor", 2100, 2100)
	actor.Char.GuildID = 9
	actor.Char.GuildRank = model.GuildRankLeader
	actor.GuildDisable = true

	enter := wire.EnterWorld(actor.ID, 0, clientGuildProjection(actor, *actor.Char))
	if got := binary.LittleEndian.Uint16(enter[34:36]); got != 0 || enter[1008] != 0 {
		t.Fatalf("masked EnterWorld guild/rank=%#x/%d, want zero", got, enter[1008])
	}

	resetCharacterRuntime(actor)
	if actor.GuildDisable {
		t.Fatal("GuildDisable survived character/session reset")
	}
	if actor.Char != nil {
		t.Fatal("reset fixture did not clear the selected character")
	}

	actor.Char = &actor.Account.Chars[0]
	actor.Char.GuildID = 9
	actor.Char.GuildRank = model.GuildRankLeader
	actor.InWorld = true
	create := playerCreatePacket(actor)
	if got := binary.LittleEndian.Uint16(create[134:136]); got != wire.GuildWireID(9) ||
		create[136] != model.GuildRankLeader {
		t.Fatalf("reconnect default CreateMob guild/rank=%#x/%d, want %#x/%d", got, create[136], wire.GuildWireID(9), model.GuildRankLeader)
	}
}

func TestGuildDisableRequiresExactBareToken(t *testing.T) {
	for _, message := range []string{"/guildoff", "guildoff extra", "GuildOff"} {
		if _, ok := guildDisableCommand(message); ok {
			t.Fatalf("non-bare token %q was accepted", message)
		}
	}
	for _, test := range []struct {
		message  string
		disabled bool
	}{
		{message: "guildon", disabled: false},
		{message: "guildoff", disabled: true},
	} {
		got, ok := guildDisableCommand(test.message)
		if !ok || got != test.disabled {
			t.Fatalf("token %q -> (%v,%v), want (%v,true)", test.message, got, ok, test.disabled)
		}
	}
}
