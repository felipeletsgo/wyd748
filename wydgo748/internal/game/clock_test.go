package game

import (
	"testing"
	"time"
)

func TestWorldClockFallsBackToRealWhenUnset(t *testing.T) {
	// A World built directly in a test (without NewWorld) has no clock; now()
	// must still work, otherwise every older test would start to panic.
	w := &World{}
	if w.now().IsZero() {
		t.Fatal("now() without a clock should fall back to the real clock")
	}
	if got := w.intn(0); got != 0 {
		t.Fatalf("intn(0)=%d, want 0", got)
	}
	if got := w.intn(5); got < 0 || got >= 5 {
		t.Fatalf("intn(5)=%d outside [0,5)", got)
	}
}

func TestWorldUsesInjectedClockAndRNG(t *testing.T) {
	start := time.Date(2026, 7, 24, 12, 0, 0, 0, time.UTC)
	clock := newFakeClock(start)
	w := &World{clock: clock, rng: fixedRNG{value: 3}}

	if !w.now().Equal(start) {
		t.Fatalf("now()=%v, want %v", w.now(), start)
	}
	clock.Advance(90 * time.Second)
	if want := start.Add(90 * time.Second); !w.now().Equal(want) {
		t.Fatalf("after Advance now()=%v, want %v", w.now(), want)
	}
	if got := w.intn(10); got != 3 {
		t.Fatalf("intn(10)=%d, want 3 (fixed RNG)", got)
	}
	// The fixed RNG must never return a value outside the requested range.
	if got := w.intn(2); got != 1 {
		t.Fatalf("intn(2)=%d, want 1 (clamped to n-1)", got)
	}
}

// TestQuestZoneResetHonorsDeadlineWithFakeClock proves what clock injection
// buys: the ten-minute window is checkable without time.Sleep. Before, the only
// way to exercise the deadline was to actually wait.
func TestQuestZoneResetHonorsDeadlineWithFakeClock(t *testing.T) {
	start := time.Date(2026, 7, 24, 12, 0, 0, 0, time.UTC)
	clock := newFakeClock(start)
	w := newZoneTestWorld()
	w.clock = clock
	w.nextQuestZoneReset = start.Add(questZoneResetInterval)
	// Keep autosave out of this test: only the zone reset matters here.
	w.nextAutoSave = start.Add(time.Hour)

	inside := addZonePlayer(w, 1, 2400, 2100, 500)
	// A legitimate stay: the per-second entry guard must not expel this player.
	w.grantQuestZoneEntry(inside, inside.X, inside.Y)

	// One minute later the deadline has not expired.
	clock.Advance(time.Minute)
	w.tick()
	if inside.X != 2400 || inside.Y != 2100 {
		t.Fatalf("recalled before the deadline: (%d,%d)", inside.X, inside.Y)
	}

	// After ten minutes, the next tick recalls.
	clock.Advance(questZoneResetInterval)
	w.tick()
	if !nearRecall(inside) {
		t.Fatalf("not recalled after the deadline: (%d,%d)", inside.X, inside.Y)
	}
	if want := clock.Now().Add(questZoneResetInterval); !w.nextQuestZoneReset.Equal(want) {
		t.Fatalf("next reset=%v, want %v", w.nextQuestZoneReset, want)
	}
}

// TestWorldOptionsInjectClockAndRNG covers the public injection options. No
// other test exercised them (the rest set the fields directly), so a break in
// them would go unnoticed.
func TestWorldOptionsInjectClockAndRNG(t *testing.T) {
	start := time.Date(2026, 1, 1, 0, 0, 0, 0, time.UTC)
	fake := newFakeClock(start)
	w := &World{clock: realClock{}, rng: realRNG{}}

	WithClock(fake)(w)
	WithRNG(fixedRNG{value: 7})(w)
	if !w.now().Equal(start) {
		t.Errorf("WithClock not applied: now()=%v", w.now())
	}
	if got := w.intn(100); got != 7 {
		t.Errorf("WithRNG not applied: intn=%d", got)
	}

	// nil is ignored on purpose: an empty option must not reset the clock and
	// fire every deadline in the world at once.
	WithClock(nil)(w)
	WithRNG(nil)(w)
	if !w.now().Equal(start) {
		t.Error("WithClock(nil) replaced the injected clock")
	}
	if got := w.intn(100); got != 7 {
		t.Errorf("WithRNG(nil) replaced the injected RNG: %d", got)
	}
}
