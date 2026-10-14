package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func questTimerPackets(t *testing.T, p *Player) []uint32 {
	t.Helper()
	var values []uint32
	for {
		pkt, ok := p.Session.DequeuePacketForTest()
		if !ok {
			return values
		}
		if !wire.Decrypt(pkt) {
			t.Fatal("invalid encrypted outbound packet")
		}
		if binary.LittleEndian.Uint16(pkt[4:6]) != wire.OpInstanceTime {
			continue
		}
		if len(pkt) != 16 || binary.LittleEndian.Uint16(pkt[6:8]) != instanceSignalID {
			t.Fatalf("unexpected countdown contract: %x", pkt)
		}
		values = append(values, binary.LittleEndian.Uint32(pkt[12:16]))
	}
}

func TestQuestZoneTimerGlobalDeadline(t *testing.T) {
	for _, elapsed := range []time.Duration{0, 3*time.Minute + 500*time.Millisecond, 10 * time.Minute} {
		t.Run(elapsed.String(), func(t *testing.T) {
			w := newGuardTestWorld()
			start := time.Date(2026, 10, 12, 12, 0, 0, 0, time.UTC)
			clock := newFakeClock(start)
			w.clock = clock
			w.nextQuestZoneReset = start.Add(questZoneResetInterval)
			clock.Advance(elapsed)
			p := addZonePlayer(w, 1, 2400, 2100, 500)
			w.grantQuestZoneEntry(p, p.X, p.Y)
			values := questTimerPackets(t, p)
			want := uint32((questZoneResetInterval - elapsed + time.Second - 1) / time.Second)
			if elapsed == questZoneResetInterval {
				if len(values) != 0 || p.QuestZoneTimer {
					t.Fatalf("expired deadline started a timer: %v", values)
				}
			} else if len(values) != 1 || values[0] != want || !p.QuestZoneTimer {
				t.Fatalf("countdown = %v, active %v; want %d seconds", values, p.QuestZoneTimer, want)
			}
		})
	}
}

func TestQuestZoneTimerExitAndResetClearOnce(t *testing.T) {
	for _, reset := range []bool{false, true} {
		w := newGuardTestWorld()
		w.nextQuestZoneReset = w.now().Add(questZoneResetInterval)
		p := addZonePlayer(w, 1, 2400, 2100, 500)
		w.grantQuestZoneEntry(p, p.X, p.Y)
		questTimerPackets(t, p)
		if reset {
			w.tickQuestZoneReset(w.now())
			if !nearRecall(p) || p.QuestZoneEntry != "" {
				t.Fatal("global reset did not recall and revoke quest entry")
			}
		} else {
			p.X, p.Y = 100, 100
			w.tickQuestZoneGuard(w.now())
		}
		w.hideQuestZoneTimer(p)
		values := questTimerPackets(t, p)
		if len(values) != 1 || values[0] != 0 || p.QuestZoneTimer {
			t.Fatalf("clear must occur exactly once: %v", values)
		}
	}
}

func TestQuestZoneTimerDoesNotOverwriteInstance(t *testing.T) {
	for _, reset := range []bool{false, true} {
		w := newGuardTestWorld()
		p := addZonePlayer(w, 1, 2400, 2100, 500)
		p.QuestZoneTimer, p.QuestZoneEntry = true, "Gravedigger"
		w.itemInstances = map[string]*ItemInstance{"private": {RuntimeID: "private", MemberIDs: []uint16{p.ID}}}
		if reset {
			w.tickQuestZoneReset(w.now())
		} else {
			w.tickQuestZoneGuard(w.now())
		}
		w.grantQuestZoneEntry(p, p.X, p.Y)
		if p.X != 2400 || p.Y != 2100 || p.QuestZoneTimer || p.QuestZoneEntry != "" || len(questTimerPackets(t, p)) != 0 {
			t.Fatal("public quest lifecycle recalled an instance member or overwrote its HUD")
		}
	}
}

func TestQuestZoneTimerMissingDeadlineAndClosedSession(t *testing.T) {
	w := newGuardTestWorld()
	p := addZonePlayer(w, 1, 2400, 2100, 500)
	w.showQuestZoneTimer(p)
	w.grantQuestZoneEntry(p, 100, 100)
	if p.QuestZoneTimer || p.QuestZoneEntry != "" || len(questTimerPackets(t, p)) != 0 {
		t.Fatal("missing deadline or outside entry started a countdown")
	}
	w.nextQuestZoneReset = w.now().Add(questZoneResetInterval)
	p.Session.Close()
	w.showQuestZoneTimer(p)
	if p.QuestZoneTimer {
		t.Fatal("closed session acquired a countdown")
	}
	w.showQuestZoneTimer(nil)
	w.hideQuestZoneTimer(nil)
}

// Every timed public-map stay shows its remaining time: the Game Room clear
// area counts down to the shared reset, which returns its occupants to town.
func TestGameRoomClearAreaShowsResetCountdownAndRecalls(t *testing.T) {
	w := newGuardTestWorld()
	w.clearAreas = []model.QuestZone{{Name: "GameRoom", X1: 3570, Y1: 3446, X2: 3965, Y2: 3711}}
	start := time.Date(2026, 10, 13, 20, 40, 0, 0, time.UTC)
	clock := newFakeClock(start)
	w.clock = clock
	w.nextQuestZoneReset = start.Add(questZoneResetInterval)
	clock.Advance(4 * time.Minute)
	p := addZonePlayer(w, 1, 3639, 3639, 500)

	w.tickQuestZoneGuard(w.now())
	if values := questTimerPackets(t, p); len(values) != 1 || values[0] != 360 || !p.QuestZoneTimer {
		t.Fatalf("Game Room countdown = %v; want one 360-second timer", values)
	}
	w.tickQuestZoneGuard(w.now())
	if values := questTimerPackets(t, p); len(values) != 0 {
		t.Fatalf("unchanged deadline was resent: %v", values)
	}
	w.tickQuestZoneReset(w.now())
	if !nearRecall(p) {
		t.Fatalf("reset left the player in the Game Room at (%d,%d)", p.X, p.Y)
	}
	if values := questTimerPackets(t, p); len(values) != 1 || values[0] != 0 || p.QuestZoneTimer {
		t.Fatalf("reset must clear the countdown once: %v", values)
	}
}

// Hidden Valley access is timed by its ticket: the countdown shows the access
// left while inside and clears when the player leaves.
func TestHiddenValleyShowsAccessCountdown(t *testing.T) {
	w := newGuardTestWorld()
	start := time.Date(2026, 10, 13, 20, 0, 0, 0, time.UTC)
	w.clock = newFakeClock(start)
	w.nextQuestZoneReset = start.Add(questZoneResetInterval)
	p := addZonePlayer(w, 1, 2281, 3688, 500)
	p.SpecialCoins = map[string]uint32{hiddenValleyAccessCounter: uint32(start.Add(90 * time.Minute).Unix())}

	w.tickQuestZoneGuard(w.now())
	if values := questTimerPackets(t, p); len(values) != 1 || values[0] != 5400 {
		t.Fatalf("Hidden Valley countdown = %v; want 5400 seconds", values)
	}
	p.X, p.Y = 2100, 2100
	w.tickQuestZoneGuard(w.now())
	if values := questTimerPackets(t, p); len(values) != 1 || values[0] != 0 || p.QuestZoneTimer {
		t.Fatalf("leaving Hidden Valley must clear the countdown once: %v", values)
	}
}
