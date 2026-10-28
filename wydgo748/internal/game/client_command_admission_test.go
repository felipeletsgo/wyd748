package game

import (
	"bytes"
	"encoding/json"
	"strings"
	"testing"

	"wydgo/internal/wire"
)

func TestUnavailableClientCommandsDoNotBecomeWhispers(t *testing.T) {
	commands := []string{"getout", "waiver", "fimcidadao", "handover", "transferir", "war", "sitguerra", "item_lock", "item_unlock"}
	for _, command := range commands {
		for _, whisper := range []bool{false, true} {
			t.Run(command+map[bool]string{true: "/whisper", false: "/chat"}[whisper], func(t *testing.T) {
				p, session := networkedTestPlayer(1, "Viewer", 2100, 2100)
				// A matching online nickname must never receive the command argument.
				target, observer := networkedTestPlayer(2, command, 2101, 2100)
				w := worldWithNetworkedPlayers(p, target)
				p.Char.Citizenship, p.Char.Gold = 2, 12345
				p.SpecialCoins = map[string]uint32{fameCounter: 101}
				before, err := json.Marshal([]any{p.Account, p.SpecialCoins})
				if err != nil {
					t.Fatal(err)
				}
				if whisper {
					packet := wire.Build(wire.OpMessageWhisper, p.ID, 128)
					copy(packet[12:28], strings.ToUpper(command))
					copy(packet[28:], "Target")
					w.onMessageWhisper(session, packet)
				} else {
					packet := wire.Build(wire.OpMessageChat, p.ID, 108)
					copy(packet[12:], "/"+strings.ToUpper(command)+" Target")
					w.onMessageChat(session, packet)
				}
				packet, ok := session.DequeuePacketForTest()
				if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != wire.OpMessagePanel ||
					!bytes.Contains(packet[12:], []byte("unavailable")) || session.QueuedPacketsForTest() != 0 {
					t.Fatal("expected one explicit unavailable notice")
				}
				after, err := json.Marshal([]any{p.Account, p.SpecialCoins})
				if err != nil || !bytes.Equal(before, after) || observer.QueuedPacketsForTest() != 0 {
					t.Fatal("unsupported command changed state or reached the matching nickname")
				}
			})
		}
	}
}

func TestCitizenshipDepartureRejectsEveryGuildRankFirst(t *testing.T) {
	for rank := byte(0); rank <= 9; rank++ {
		p, session := networkedTestPlayer(1, "Member", 2100, 2100)
		w := worldWithNetworkedPlayers(p)
		p.Char.GuildID, p.Char.GuildRank, p.Char.Citizenship = 42, rank, 0
		if !w.dispatchChatCommand(session, p, "getout", "") {
			t.Fatal("getout not consumed")
		}
		packet, ok := session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) ||
			!bytes.Contains(packet[12:], []byte("Leave your guild before leaving citizenship.")) {
			t.Fatalf("rank %d did not receive the guild rejection first", rank)
		}
		if p.Char.GuildID != 42 || p.Char.GuildRank != rank || p.Char.Citizenship != 0 {
			t.Fatal("rejected departure changed membership")
		}
	}
}
