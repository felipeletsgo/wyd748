package model

import (
	"testing"
	"time"
)

func TestExpiryDisplayEffectsProjectsNativeTooltipFields(t *testing.T) {
	previous := time.Local
	time.Local = time.UTC
	defer func() { time.Local = previous }()
	now := time.Date(2026, time.October, 12, 1, 55, 0, 0, time.UTC)
	expires := now.Add(72 * time.Hour).Unix()

	mount := Item{Index: 3980, UID: "m", ActivatedUnix: 1, ExpiresUnix: expires}
	if got := ExpiryDisplayEffects(mount, now); got != [6]byte{106, 15, 109, 26, 110, 10} {
		t.Fatalf("mount/costume expiry date = %v", got)
	}
	// A 3-day Angel shows its remaining lifetime, not the expiry day of month.
	angel := Item{Index: 3900, UID: "a", ActivatedUnix: 1, ExpiresUnix: expires}
	if got := ExpiryDisplayEffects(angel, now); got != [6]byte{106, 3, 107, 0, 108, 0} {
		t.Fatalf("fresh angel remaining = %v", got)
	}
	if got := ExpiryDisplayEffects(angel, now.Add(25*time.Hour+30*time.Minute)); got != [6]byte{106, 1, 107, 22, 108, 30} {
		t.Fatalf("angel remaining after 25h30m = %v", got)
	}
	if got := ExpiryDisplayEffects(angel, now.Add(80*time.Hour)); got != [6]byte{106, 0, 107, 0, 108, 0} {
		t.Fatalf("expired angel remaining = %v", got)
	}
	refined := Item{Index: 4150, Eff: [6]byte{43, 9}, UID: "c", ActivatedUnix: 1, ExpiresUnix: expires}
	if got := ExpiryDisplayEffects(refined, now); got != [6]byte{43, 9, 106, 15, 109, 26} {
		t.Fatalf("occupied effect pair was not preserved: %v", got)
	}
	plain := Item{Index: 4150, Eff: [6]byte{43, 9}}
	if got := ExpiryDisplayEffects(plain, now); got != plain.Eff {
		t.Fatalf("inactive item gained a projection: %v", got)
	}
}

func TestWireEqualAcceptsEchoedExpiryProjection(t *testing.T) {
	now := time.Unix(1_900_000_000, 0)
	item := Item{Index: 3900, UID: "a", ActivatedUnix: 1, ExpiresUnix: now.Add(72 * time.Hour).Unix()}
	// The client may echo a projection taken minutes or hours earlier.
	for _, sent := range []time.Time{now, now.Add(-90 * time.Minute), now.Add(30 * time.Hour)} {
		echoed := Item{Index: 3900, Eff: ExpiryDisplayEffects(item, sent)}
		if !item.WireEqual(echoed) || !echoed.WireEqual(item) {
			t.Fatalf("authoritative and echoed timed item did not match: %v", echoed.Eff)
		}
	}
	if !item.WireEqual(Item{Index: 3900}) {
		t.Fatal("an echo without projection did not match")
	}
	refined := Item{Index: 4150, Eff: [6]byte{43, 9}, UID: "c", ActivatedUnix: 1, ExpiresUnix: now.Unix()}
	for _, forged := range []Item{
		{Index: 4150, Eff: [6]byte{43, 8, 106, 1, 109, 26}}, // refinement changed
		{Index: 4150, Eff: [6]byte{43, 9, 61, 5}},           // non-expiry effect added
		{Index: 4151, Eff: [6]byte{43, 9}},                  // different item
	} {
		if refined.WireEqual(forged) || forged.WireEqual(refined) {
			t.Fatalf("forged item %v matched", forged)
		}
	}
	untimed := Item{Index: 4150, Eff: [6]byte{43, 9}}
	if untimed.WireEqual(Item{Index: 4150, Eff: [6]byte{43, 9, 106, 1}}) {
		t.Fatal("an untimed item accepted projected fields")
	}
}
