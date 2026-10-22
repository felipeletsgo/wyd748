package game

import (
	"bytes"
	"encoding/binary"
	"fmt"
	"reflect"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

type colosseumOutbound struct {
	panels, chat []string
	timers       []uint32
}

func readColosseumOutbound(t *testing.T, p *Player) colosseumOutbound {
	t.Helper()
	var out colosseumOutbound
	for {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok {
			return out
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid encrypted outbound packet")
		}
		header := wire.ParseHeader(packet)
		switch header.Type {
		case wire.OpMessagePanel:
			if len(packet) != 108 || header.ID != 0 {
				t.Fatalf("unexpected notice contract: %x", packet)
			}
			out.panels = append(out.panels, string(bytes.TrimRight(packet[12:108], "\x00")))
		case wire.OpMessageWhisper:
			if len(packet) != 128 || header.ID != 0 {
				t.Fatalf("unexpected chat contract: %x", packet)
			}
			if string(bytes.TrimRight(packet[12:28], "\x00")) == "[Colosseum]" {
				out.chat = append(out.chat, string(bytes.TrimRight(packet[28:124], "\x00")))
			}
		case wire.OpInstanceTime:
			if len(packet) != 16 || header.ID != instanceSignalID {
				t.Fatalf("unexpected countdown contract: %x", packet)
			}
			out.timers = append(out.timers, binary.LittleEndian.Uint32(packet[12:16]))
		}
	}
}

func colosseumNoticeWorld(start time.Time) (*World, *fakeClock) {
	w := newZoneTestWorld()
	w.questZones = nil
	w.warConfig.Timezone = "UTC"
	clock := newFakeClock(start)
	w.clock = clock
	w.colosseum = &colosseumRuntime{config: model.DefaultColosseumConfig()}
	return w, clock
}

func assertColosseumNotice(t *testing.T, out colosseumOutbound, want string) {
	t.Helper()
	if !reflect.DeepEqual(out.panels, []string{want}) || !reflect.DeepEqual(out.chat, []string{want}) {
		t.Fatalf("global notice = %+v; want panel and chat %q", out, want)
	}
	if len(want) > 95 {
		t.Fatalf("notice exceeds the existing packet text limit: %q", want)
	}
}

func TestColosseumAdvanceNoticesGlobalOncePerTierAndDay(t *testing.T) {
	for tier, label := range "NMA" {
		t.Run(string(label), func(t *testing.T) {
			w, clock := colosseumNoticeWorld(time.Date(2026, 10, 21, 18+tier, 0, 0, 0, time.UTC))
			inside := addZonePlayer(w, 1, 2630, 1726, 500)
			outside := addZonePlayer(w, 2, 100, 100, 500)
			offline := addZonePlayer(w, 3, 100, 100, 500)
			offline.InWorld = false
			w.tickColosseum(clock.Now())
			want := fmt.Sprintf("Colosseum %c starts in 5 minutes. Entrance gates close at %02d:05.", label, 18+tier)
			assertColosseumNotice(t, readColosseumOutbound(t, inside), want)
			assertColosseumNotice(t, readColosseumOutbound(t, outside), want)
			if out := readColosseumOutbound(t, offline); len(out.panels)+len(out.chat) != 0 {
				t.Fatalf("offline player received notices: %+v", out)
			}
			clock.Advance(time.Second)
			w.tickColosseum(clock.Now())
			if out := readColosseumOutbound(t, inside); len(out.panels)+len(out.chat)+len(out.timers) != 0 {
				t.Fatalf("duplicate advance notice: %+v", out)
			}
			clock.Advance(4*time.Minute - time.Second)
			w.tickColosseum(clock.Now())
			want = fmt.Sprintf("Colosseum %c starts in 1 minute. Entrance gates close at %02d:05.", label, 18+tier)
			assertColosseumNotice(t, readColosseumOutbound(t, inside), want)
			readColosseumOutbound(t, outside)
			clock.Advance(time.Second)
			w.tickColosseum(clock.Now())
			if out := readColosseumOutbound(t, inside); len(out.panels)+len(out.chat) != 0 {
				t.Fatalf("duplicate one-minute notice: %+v", out)
			}
			clock.Advance(24*time.Hour - 4*time.Minute - time.Second)
			w.tickColosseum(clock.Now())
			want = fmt.Sprintf("Colosseum %c starts in 5 minutes. Entrance gates close at %02d:05.", label, 18+tier)
			assertColosseumNotice(t, readColosseumOutbound(t, inside), want)
		})
	}
}

func TestColosseumAdvanceNoticeWindowTimezoneAndLateTick(t *testing.T) {
	w, clock := colosseumNoticeWorld(time.Date(2026, 10, 21, 20, 59, 59, 0, time.UTC))
	w.warConfig.Timezone = "America/Sao_Paulo"
	p := addZonePlayer(w, 1, 100, 100, 500)
	w.tickColosseum(clock.Now())
	if out := readColosseumOutbound(t, p); len(out.panels) != 0 {
		t.Fatal("notice sent before the five-minute window")
	}
	clock.Advance(4*time.Minute + time.Second)
	w.tickColosseum(clock.Now())
	assertColosseumNotice(t, readColosseumOutbound(t, p), "Colosseum N starts in 1 minute. Entrance gates close at 18:05.")
	clock.Advance(30 * time.Second)
	w.tickColosseum(clock.Now())
	if out := readColosseumOutbound(t, p); len(out.panels) != 0 {
		t.Fatal("late tick replayed an earlier notice")
	}
	w.colosseum.claimed[model.ColosseumTierN] = "2026-10-21"
	w.colosseum.notices = [model.ColosseumTiers]colosseumNoticeState{}
	clock.Advance(time.Second)
	w.tickColosseum(clock.Now())
	if out := readColosseumOutbound(t, p); len(out.panels) != 0 {
		t.Fatal("a manually claimed tier announced another scheduled start")
	}
}

func TestColosseumScheduledStartClosesGatesAndStartsCountdown(t *testing.T) {
	w, clock := colosseumNoticeWorld(time.Date(2026, 10, 21, 18, 4, 59, 0, time.UTC))
	p := addZonePlayer(w, 1, 2630, 1726, 500)
	clock.Advance(time.Second)
	w.tickColosseum(clock.Now())
	out := readColosseumOutbound(t, p)
	if !reflect.DeepEqual(out.panels, []string{"Colosseum N has started. Entrance gates are now closed."}) ||
		!reflect.DeepEqual(out.timers, []uint32{60}) || len(out.chat) != 2 || w.colosseum.stage != 1 {
		t.Fatalf("scheduled start notice and countdown: %+v, stage %d", out, w.colosseum.stage)
	}
	w.tickColosseum(clock.Now())
	clock.Advance(time.Second)
	w.tickColosseum(clock.Now())
	if out := readColosseumOutbound(t, p); len(out.panels)+len(out.chat)+len(out.timers) != 0 {
		t.Fatalf("scheduled start repeated: %+v", out)
	}
}

func TestColosseumManualStartAndWaveAnnouncements(t *testing.T) {
	w, clock := colosseumWorld(t, time.Date(2026, 10, 21, 17, 30, 0, 0, time.UTC))
	inside := addZonePlayer(w, 1, 2630, 1726, 500)
	// Fixture/mob broadcasts share this test queue with notices.
	delete(w.players, inside.Session)
	inside.Session = net.NewTestSession(1, 4096)
	w.players[inside.Session] = inside
	outside := addZonePlayer(w, 2, 100, 100, 500)
	cmd := colosseumStartCommand(w, "N")
	if result := runColosseumCommand(w, cmd, true); result.Code != "ok" {
		t.Fatal(result.Code)
	}
	assertColosseumNotice(t, readColosseumOutbound(t, outside), "Colosseum N has started. Entrance gates are now closed.")
	out := readColosseumOutbound(t, inside)
	if !reflect.DeepEqual(out.timers, []uint32{60}) || len(out.panels) != 1 || len(out.chat) != 2 {
		t.Fatalf("initial arena notice and countdown: %+v", out)
	}
	if result := runColosseumCommand(w, cmd, true); !result.Replayed {
		t.Fatal("manual start was not replayed idempotently")
	}
	if out := readColosseumOutbound(t, outside); len(out.panels)+len(out.chat) != 0 {
		t.Fatal("manual replay duplicated the start notice")
	}
	clock.Advance(time.Minute)
	w.tickColosseum(clock.Now())
	assertColosseumNotice(t, readColosseumOutbound(t, outside), "Colosseum N: combat has begun! Wave 1/5 is now active.")
	if out := readColosseumOutbound(t, inside); !reflect.DeepEqual(out.timers, []uint32{180}) {
		t.Fatalf("next wave after combat starts: %+v", out)
	}
	clock.Advance(time.Minute)
	w.tickColosseum(clock.Now())
	readColosseumOutbound(t, inside)
	clock.Advance(2 * time.Minute)
	w.tickColosseum(clock.Now())
	out = readColosseumOutbound(t, inside)
	if !reflect.DeepEqual(out.panels, []string{"Colosseum N: wave 2/5 has begun."}) || !reflect.DeepEqual(out.timers, []uint32{120}) {
		t.Fatalf("second wave notice and countdown: %+v", out)
	}
	if out := readColosseumOutbound(t, outside); len(out.panels)+len(out.chat)+len(out.timers) != 0 {
		t.Fatalf("subsequent wave was not arena-only: %+v", out)
	}
}

func TestColosseumCountdownDeadlinesAndOwnership(t *testing.T) {
	start := time.Date(2026, 10, 21, 18, 5, 0, 0, time.UTC)
	w, _ := colosseumNoticeWorld(start)
	w.colosseum.startedAt = start
	inside := addZonePlayer(w, 1, 2630, 1726, 500)
	outside := addZonePlayer(w, 2, 100, 100, 500)
	private := addZonePlayer(w, 3, 2630, 1726, 500)
	w.itemInstances = map[string]*ItemInstance{"private": {RuntimeID: "private", MemberIDs: []uint16{private.ID}}}
	for _, step := range []struct {
		stage, elapsed, seconds, wave int
	}{
		{1, 0, 60, 1}, {2, 60, 180, 2}, {3, 120, 0, 2},
		{4, 240, 120, 3}, {5, 360, 120, 4}, {6, 480, 120, 5}, {7, 600, 0, 0},
	} {
		w.colosseum.stage = step.stage
		now := start.Add(time.Duration(step.elapsed) * time.Second)
		w.syncColosseumCountdowns(now)
		out := readColosseumOutbound(t, inside)
		if step.stage == 3 {
			if len(out.timers)+len(out.chat) != 0 {
				t.Fatalf("gate-opening stage restarted an unchanged timer: %+v", out)
			}
		} else if !reflect.DeepEqual(out.timers, []uint32{uint32(step.seconds)}) {
			t.Fatalf("stage %d timer = %+v", step.stage, out)
		} else if step.wave != 0 && (len(out.chat) != 1 || !strings.Contains(out.chat[0], fmt.Sprintf("Next wave: %d/5.", step.wave))) {
			t.Fatalf("stage %d wave label = %+v", step.stage, out)
		}
		w.syncColosseumCountdowns(now.Add(time.Millisecond))
		if out := readColosseumOutbound(t, inside); len(out.timers)+len(out.chat) != 0 {
			t.Fatalf("unchanged timer was resent: %+v", out)
		}
		if inside.QuestZoneTimer {
			t.Fatal("Colosseum acquired the unrelated quest timer lifecycle")
		}
		for _, p := range []*Player{outside, private} {
			if out := readColosseumOutbound(t, p); len(out.timers)+len(out.chat) != 0 {
				t.Fatalf("nonparticipant received wave timer: %+v", out)
			}
		}
	}
}

func TestColosseumCountdownLateJoinLeaveRelogAndTimerHandoff(t *testing.T) {
	for _, action := range []string{"leave", "private", "public timer", "relog", "finish", "overdue", "disabled", "quest guard"} {
		t.Run(action, func(t *testing.T) {
			start := time.Date(2026, 10, 21, 18, 5, 0, 0, time.UTC)
			w, clock := colosseumNoticeWorld(start)
			w.colosseum.stage, w.colosseum.startedAt = 1, start
			p := addZonePlayer(w, 1, 2630, 1726, 500)
			clock.Advance(20*time.Second + 100*time.Millisecond)
			w.syncColosseumCountdowns(clock.Now())
			if values := questTimerPackets(t, p); !reflect.DeepEqual(values, []uint32{40}) {
				t.Fatalf("late join must receive rounded-up remaining seconds: %v", values)
			}
			want := []uint32{0}
			switch action {
			case "leave":
				p.X, p.Y = 100, 100
				w.updatePlayerSpatial(p)
			case "private":
				w.itemInstances = map[string]*ItemInstance{"private": {RuntimeID: "private", MemberIDs: []uint16{p.ID}}}
				want = nil
			case "public timer":
				p.X, p.Y = 100, 100
				w.updatePlayerSpatial(p)
				w.clearAreas = []model.QuestZone{{Name: "Other timer", X1: 90, Y1: 90, X2: 110, Y2: 110}}
				w.nextQuestZoneReset = clock.Now().Add(5 * time.Minute)
				w.syncAreaTimer(p, clock.Now())
				questTimerPackets(t, p)
				want = nil
			case "relog":
				w.unregisterPlayerSpatial(p)
				delete(w.players, p.Session)
				other := addZonePlayer(w, p.ID, p.X, p.Y, 500)
				w.syncColosseumCountdowns(clock.Now())
				if values := questTimerPackets(t, other); !reflect.DeepEqual(values, []uint32{40}) {
					t.Fatalf("new session did not acquire the timer: %v", values)
				}
				want = nil
			case "finish":
				w.colosseum.stage = 8
			case "overdue":
				clock.Advance(time.Minute)
			case "disabled":
				w.colosseum.config.Enabled = false
				w.tickColosseum(clock.Now())
			case "quest guard":
				// Exercise the reset sweep, not its empty-configuration shortcut.
				w.clearAreas = []model.QuestZone{{Name: "Other area", X1: 90, Y1: 90, X2: 110, Y2: 110}}
				w.tickQuestZoneGuard(clock.Now())
				w.tickQuestZoneReset(clock.Now())
				want = nil
			}
			w.syncColosseumCountdowns(clock.Now())
			w.syncColosseumCountdowns(clock.Now())
			if values := questTimerPackets(t, p); !reflect.DeepEqual(values, want) {
				t.Fatalf("%s timer handoff = %v; want %v", action, values, want)
			}
			if action != "quest guard" {
				if _, tracked := w.colosseum.countdowns[p]; tracked {
					t.Fatal("stale timer ownership retained")
				}
			}
		})
	}
}
