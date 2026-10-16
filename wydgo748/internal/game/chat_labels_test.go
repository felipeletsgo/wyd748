package game

import (
	"bytes"
	"fmt"
	"strings"
	"testing"
	"time"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

func labelPackets(t *testing.T, s *net.Session) [][]byte {
	t.Helper()
	var packets [][]byte
	for {
		packet, ok := s.DequeuePacketForTest()
		if !ok {
			return packets
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid outbound label packet")
		}
		packets = append(packets, packet)
	}
}

func submitLabel(w *World, p *Player, whisper bool, name, text string) {
	if whisper {
		packet := wire.Build(wire.OpMessageWhisper, p.ID, 128)
		copy(packet[12:28], name)
		copy(packet[28:124], text)
		w.onMessageWhisper(p.Session, packet)
	} else {
		w.onMessageChat(p.Session, wire.MessageChat(p.ID, "/"+name+" "+text))
	}
}

func TestTabCommandsRouteAndRefreshVisibleLabels(t *testing.T) {
	for _, whisper := range []bool{false, true} {
		t.Run(fmt.Sprintf("whisper=%v", whisper), func(t *testing.T) {
			owner, _ := networkedTestPlayer(1, "Owner", 2100, 2100)
			observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
			hidden, _ := networkedTestPlayer(3, "Hidden", 2101, 2100)
			isolated, _ := networkedTestPlayer(4, "Isolated", 2101, 2100)
			isolated.show(owner.ID)
			owner.Char.Score.Level = 69 // Displayed level 70, the first allowed Mortal level.
			observer.show(owner.ID)
			w := worldWithNetworkedPlayers(owner, observer, hidden, isolated)
			w.itemInstances = map[string]*ItemInstance{"isolated": privateTestInstance("isolated", isolated.ID)}
			clock := newFakeClock(time.Unix(1000, 0))
			w.clock = clock
			for color := byte(0); color < 9; color++ {
				command := "tab"
				if color > 0 {
					command = fmt.Sprintf("tab%d", color+1)
				}
				submitLabel(w, owner, whisper, command, "Same text")
				if owner.TabText != "Same text" || owner.TabColor != color {
					t.Fatalf("command %s did not update its label", command)
				}
				for _, viewer := range []*Player{owner, observer} {
					packets := labelPackets(t, viewer.Session)
					if len(packets) != 1 || wire.ParseHeader(packets[0]).Type != wire.OpCreateMob ||
						string(bytes.TrimRight(packets[0][300:326], "\x00")) != "Same text" || packets[0][327] != color {
						t.Fatal("visible label update missing, duplicated or accompanied by action/vitals packets")
					}
				}
				if hidden.Session.QueuedPacketsForTest() != 0 || isolated.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("tab update escaped visibility or gameplay-space isolation")
				}
				clock.Advance(11 * time.Second)
			}
			// Future entry and unrelated CP metadata refresh must retain the label.
			if packet := playerEnterViewPackets(owner)[0]; packet[327] != 8 || string(packet[300:309]) != "Same text" {
				t.Fatal("enter-view snapshot lost tab state")
			}
			w.syncPlayerChaos(owner)
			if packets := labelPackets(t, observer.Session); len(packets) != 1 || packets[0][327] != 8 {
				t.Fatal("chaos refresh lost tab color")
			}
			labelPackets(t, owner.Session)
			submitLabel(w, owner, whisper, "tab", "")
			packets := labelPackets(t, observer.Session)
			if owner.TabText != "" || owner.TabColor != 0 || len(packets) != 1 || packets[0][300] != 0 {
				t.Fatal("empty tab did not clear the visible label")
			}
		})
	}
}

func TestTabAdmissionBoundsAndFloodControl(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Owner", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	clock := newFakeClock(time.Unix(1000, 0))
	w.clock = clock
	w.operational.ChatWhisperPer10Seconds = 1
	submitLabel(w, p, true, "tab", "Too early")
	if p.TabText != "" || p.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("Mortal below level 70 bypassed admission or received no explanation")
	}
	labelPackets(t, p.Session)
	p.Char.Score.Level = 69
	submitLabel(w, p, true, "tab", "Flood")
	if p.TabText != "" {
		t.Fatal("command bypassed shared label rate limit")
	}
	labelPackets(t, p.Session)
	clock.Advance(11 * time.Second)
	submitLabel(w, p, true, "tab9", strings.Repeat("x", 95))
	if len(p.TabText) != 25 || p.TabColor != 8 {
		t.Fatal("oversized tab did not respect the 26-byte terminated field")
	}
	clock.Advance(11 * time.Second)
	p.Char.Evolution, p.Char.Score.Level = "arch", 0
	submitLabel(w, p, true, "tab2", "Arch tab")
	if p.TabText != "Arch tab" {
		t.Fatal("Mortal restriction leaked into advanced evolutions")
	}
	for _, name := range []string{"tab0", "tab1", "tab10", "tab2x", "table"} {
		if _, ok := tabCommandColor(name); ok {
			t.Fatalf("near-match %q accepted as tab", name)
		}
	}
	if boundedChatLabel("hello\tworld\x00hidden", 25) != "hello world" {
		t.Fatal("control/NUL bytes leaked into labels")
	}
}

func TestSndStatusLookupClearAndCharacterReset(t *testing.T) {
	for _, whisper := range []bool{false, true} {
		t.Run(fmt.Sprintf("whisper=%v", whisper), func(t *testing.T) {
			owner, _ := networkedTestPlayer(1, "Owner", 2100, 2100)
			reader, _ := networkedTestPlayer(2, "Reader", 2101, 2100)
			w := worldWithNetworkedPlayers(owner, reader)
			submitLabel(w, owner, whisper, "snd", "!!123456 "+strings.Repeat("x", 85))
			if len(owner.StatusMessage) != 87 || owner.TabText != "" || reader.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("status length, private storage or independence from tab is wrong")
			}
			confirmation := labelPackets(t, owner.Session)
			if len(confirmation) != 1 || !bytes.Contains(confirmation[0], []byte("Status message saved")) {
				t.Fatal("SND update did not confirm instead of looking up a player named snd")
			}
			submitLabel(w, reader, true, "Owner", "")
			packets := labelPackets(t, reader.Session)
			if len(packets) != 2 || string(bytes.TrimRight(packets[1][12:], "\x00")) != "Status: "+owner.StatusMessage {
				t.Fatal("empty-body character lookup omitted or reinterpreted the status")
			}
			submitLabel(w, reader, true, "Owner", "An ordinary whisper")
			if reader.Session.QueuedPacketsForTest() != 0 || owner.Session.QueuedPacketsForTest() != 1 {
				t.Fatal("status was incorrectly used as an automatic whisper reply")
			}
			labelPackets(t, owner.Session)
			submitLabel(w, owner, whisper, "snd", "")
			if owner.StatusMessage != "" {
				t.Fatal("empty SND did not clear the status")
			}
			owner.TabText, owner.TabColor, owner.StatusMessage = "Old tab", 8, "Old status"
			resetCharacterRuntime(owner)
			if owner.TabText != "" || owner.TabColor != 0 || owner.StatusMessage != "" {
				t.Fatal("labels leaked across character logout")
			}
		})
	}
}
