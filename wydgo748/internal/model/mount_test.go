package model

import "testing"

func TestMountCatalogLookups(t *testing.T) {
	var empty MountCatalog
	if got, ok := empty.Stats(1); ok || got != (MountStats{}) {
		t.Fatalf("empty Stats = (%+v, %t)", got, ok)
	}
	if got, ok := empty.PremiumStats(3980); ok || got != (PremiumMountStats{}) {
		t.Fatalf("empty PremiumStats = (%+v, %t)", got, ok)
	}

	want := MountStats{Attack: 17, Speed: 8, Name: "Pig"}
	premium := PremiumMountStats{Name: "Timed Horse", DurationDays: 30}
	catalog := MountCatalog{
		Types:   map[int]MountStats{0: want},
		Premium: map[uint16]PremiumMountStats{3980: premium},
	}
	if got, ok := catalog.Stats(0); !ok || got != want {
		t.Fatalf("Stats(0) = (%+v, %t), want (%+v, true)", got, ok, want)
	}
	if _, ok := catalog.Stats(30); ok {
		t.Fatal("missing mount type was found")
	}
	if got, ok := catalog.PremiumStats(3980); !ok || got != premium {
		t.Fatalf("PremiumStats(3980) = (%+v, %t), want (%+v, true)", got, ok, premium)
	}
	if _, ok := catalog.PremiumStats(3999); ok {
		t.Fatal("missing premium mount was found")
	}
}

func TestMountStagesAndTypeBoundaries(t *testing.T) {
	for _, test := range []struct {
		index     uint16
		mountType int
		egg       bool
		baby      bool
		adult     bool
		isMount   bool
	}{
		{2299, -1, false, false, false, false},
		{2300, 0, true, false, false, false},
		{2329, 29, true, false, false, false},
		{2330, 0, false, true, false, true},
		{2359, 29, false, true, false, true},
		{2360, 0, false, false, true, true},
		{2389, 29, false, false, true, true},
		{2390, -1, false, false, false, false},
	} {
		if got := MountType(test.index); got != test.mountType {
			t.Errorf("MountType(%d) = %d, want %d", test.index, got, test.mountType)
		}
		if got := IsMountEgg(test.index); got != test.egg {
			t.Errorf("IsMountEgg(%d) = %t, want %t", test.index, got, test.egg)
		}
		if got := IsMountBaby(test.index); got != test.baby {
			t.Errorf("IsMountBaby(%d) = %t, want %t", test.index, got, test.baby)
		}
		if got := IsMountAdult(test.index); got != test.adult {
			t.Errorf("IsMountAdult(%d) = %t, want %t", test.index, got, test.adult)
		}
		if got := IsMount(test.index); got != test.isMount {
			t.Errorf("IsMount(%d) = %t, want %t", test.index, got, test.isMount)
		}
	}
}

func TestMountItemEffectAccessorsAndClamps(t *testing.T) {
	item := Item{}
	item.SetMountHP(0xABCD)
	item.SetMountLevel(42)
	item.SetMountLongev(13)
	item.SetMountFood(77)
	if item.MountHP() != 0xABCD || item.MountLevel() != 42 || item.MountLongev() != 13 || item.MountFood() != 77 {
		t.Fatalf("mount effects decoded as HP=%d level=%d longevity=%d food=%d", item.MountHP(), item.MountLevel(), item.MountLongev(), item.MountFood())
	}

	item.SetMountHP(-1)
	if item.MountHP() != 0 {
		t.Fatalf("negative HP was not clamped to zero: %d", item.MountHP())
	}
	item.SetMountHP(70_000)
	if item.MountHP() != 65_535 {
		t.Fatalf("oversized HP was not clamped to uint16: %d", item.MountHP())
	}
	item.SetMountLevel(-1)
	item.SetMountLongev(-1)
	item.SetMountFood(-1)
	if item.MountLevel() != 0 || item.MountLongev() != 0 || item.MountFood() != 0 {
		t.Fatalf("negative mount state was not clamped: level=%d longevity=%d food=%d", item.MountLevel(), item.MountLongev(), item.MountFood())
	}
	item.SetMountLevel(300)
	item.SetMountLongev(300)
	item.SetMountFood(300)
	if item.MountLevel() != 255 || item.MountLongev() != 255 || item.MountFood() != 255 {
		t.Fatalf("oversized byte state was not clamped: level=%d longevity=%d food=%d", item.MountLevel(), item.MountLongev(), item.MountFood())
	}
}
