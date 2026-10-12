package game

import (
	"encoding/binary"
	"testing"
	"time"

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
