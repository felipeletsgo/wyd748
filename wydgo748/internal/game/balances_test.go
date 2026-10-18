package game

import (
	"bytes"
	"encoding/json"
	"strings"
	"testing"

	"wydgo/internal/wire"
)

func TestBalanceCommandsThroughBothChatPaths(t *testing.T) {
	for _, alias := range []string{"saldo", "balance", "SALDO", "BALANCE", "medals", "medalhas", "MEDALS", "MEDALHAS"} {
		for _, whisper := range []bool{false, true} {
			t.Run(alias+map[bool]string{false: "/chat", true: "/whisper"}[whisper], func(t *testing.T) {
				p, s := networkedTestPlayer(1, "Viewer", 2100, 2100)
				other, observer := networkedTestPlayer(2, "Observer", 2101, 2100)
				w := worldWithNetworkedPlayers(p, other)
				p.Account.Donate = 1234
				p.Account.CargoGold, p.Char.Gold = 999, 888
				p.Char.CP = -10
				p.SpecialCoins = map[string]uint32{fameCounter: 20, "honor": 30,
					kefraTicketCounter: 49, "event_pass": 4, "game_medal_red": 7,
					"game_medal_gold": 2, "game_medal_blue": 3, "custom_coin": 8, donateCounter: 9999}
				before, err := json.Marshal([]any{p.Account, p.SpecialCoins})
				if err != nil {
					t.Fatal(err)
				}
				if whisper {
					pkt := wire.Build(wire.OpMessageWhisper, p.ID, 128)
					copy(pkt[12:28], alias)
					w.onMessageWhisper(s, pkt)
				} else {
					pkt := wire.Build(wire.OpMessageChat, p.ID, 108)
					copy(pkt[12:], "/"+alias)
					w.onMessageChat(s, pkt)
				}
				want := "Donate:1234 Fame:20 Honor:30 Pass:4 Kefra:49"
				if strings.HasPrefix(strings.ToLower(alias), "med") {
					want = "Medals - Gold:2 Red:7 Blue:3"
				}
				assertBalanceNotice(t, p, want)
				after, err := json.Marshal([]any{p.Account, p.SpecialCoins})
				if err != nil || !bytes.Equal(before, after) || observer.QueuedPacketsForTest() != 0 {
					t.Fatal("balance lookup mutated economic state or leaked to an observer")
				}
			})
		}
	}
}

func TestBalanceZeroAndCounterSeparation(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Viewer", 2100, 2100)
	want := "Donate:0 Fame:0 Honor:N/A Pass:0 Kefra:0"
	if got := balanceMessage(p); got != want {
		t.Fatalf("empty balances=%q, want %q", got, want)
	}
	if got := medalsMessage(p); got != "Medals - Gold:0 Red:0 Blue:0" {
		t.Fatalf("empty medals=%q", got)
	}
	// Persisted access deadlines and unknown identifiers are not currencies.
	p.SpecialCoins = map[string]uint32{strings.Repeat("x", 120) + "\n": ^uint32(0),
		"game_medal_red": ^uint32(0), "access_hidden_ruins": ^uint32(0), donateCounter: ^uint32(0)}
	p.Char.CP, p.Char.Gold, p.Account.CargoGold = -75, 123, 456
	if got := balanceMessage(p); got != want {
		t.Fatalf("CP, gold, medals, legacy donate or unrelated counters leaked into balances: %q", got)
	}
	p.SpecialCoins["honor"] = 0
	if !strings.Contains(balanceMessage(p), "Honor:0 ") {
		t.Fatal("existing zero Honor was reported as unavailable")
	}
}

func TestBalanceRejectsArgumentsAndInactiveSession(t *testing.T) {
	for _, command := range []string{"balance", "medals"} {
		for _, arg := range []string{"OtherPlayer", "2", "-1", "1 2"} {
			p, s := networkedTestPlayer(1, "Viewer", 2100, 2100)
			w := worldWithNetworkedPlayers(p)
			if !w.dispatchChatCommand(s, p, command, arg) {
				t.Fatal("command was not consumed")
			}
			expectMessage(t, p, "Usage:")
			account := p.Account
			p.InWorld = false
			w.sendBalanceNotice(s, p, "", command)
			p.InWorld, p.Account = true, nil
			w.sendBalanceNotice(s, p, "", command)
			w.sendBalanceNotice(s, nil, "", command)
			w.sendBalanceNotice(nil, p, "", command)
			p.Account, p.Char = account, nil
			w.sendBalanceNotice(s, p, "", command)
			if s.QueuedPacketsForTest() != 0 {
				t.Fatal("inactive or incomplete session received balances")
			}
		}
	}
}

func TestBalanceAliasesShareRateLimit(t *testing.T) {
	for _, aliases := range [][2]string{{"saldo", "balance"}, {"medalhas", "medals"}} {
		p, s := networkedTestPlayer(1, "Viewer", 2100, 2100)
		w := worldWithNetworkedPlayers(p)
		w.operational.ChatWhisperPer10Seconds = 1
		w.dispatchChatCommand(s, p, aliases[0], "")
		nextPacket(t, p)
		w.dispatchChatCommand(s, p, aliases[1], "")
		expectMessage(t, p, "too quickly")
		for _, alias := range aliases {
			if validCharacterName(alias) {
				t.Fatalf("command alias %q is available as a new character name", alias)
			}
		}
	}
}

func TestBalanceResponsesFitTopNotice(t *testing.T) {
	p, s := networkedTestPlayer(1, "Viewer", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	p.Account.Donate = ^uint32(0)
	p.SpecialCoins = map[string]uint32{
		fameCounter: ^uint32(0), "honor": ^uint32(0),
		"game_medal_blue": ^uint32(0), "game_medal_red": ^uint32(0), "game_medal_gold": ^uint32(0),
		"event_pass": ^uint32(0), kefraTicketCounter: ^uint32(0),
	}
	for command, want := range map[string]string{
		"balance": "Donate:4294967295 Fame:4294967295 Honor:4294967295 Pass:4294967295 Kefra:4294967295",
		"medals":  "Medals - Gold:4294967295 Red:4294967295 Blue:4294967295",
	} {
		if len(want) > balanceNoticeBytes {
			t.Fatalf("%s exceeds notice limit: %d", command, len(want))
		}
		w.dispatchChatCommand(s, p, command, "")
		assertBalanceNotice(t, p, want)
		t.Logf("%s: %d/%d text bytes, one 108-byte top notice", command, len(want), balanceNoticeBytes)
	}
}

func assertBalanceNotice(t *testing.T, p *Player, want string) {
	t.Helper()
	pkt := nextPacket(t, p)
	if wire.ParseHeader(pkt).Type != wire.OpMessagePanel || wire.ParseHeader(pkt).ID != 0 ||
		len(pkt) != 108 || pkt[107] != 0 || string(bytes.TrimRight(pkt[12:108], "\x00")) != want {
		t.Fatalf("unexpected top notice: %x; want %q", pkt, want)
	}
	if p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("command sent extra packets: chat copy or pagination")
	}
}
