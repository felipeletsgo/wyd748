package model

import "testing"

func TestKingdomNamingAndSpecialCapeClassification(t *testing.T) {
	for _, test := range []struct {
		kingdom byte
		name    string
	}{
		{KingdomHekalotia, "Hekalotia"},
		{KingdomAkelonia, "Akelonia"},
		{KingdomNeutral, "Neutral"},
		{99, "Neutral"},
	} {
		if got := KingdomName(test.kingdom); got != test.name {
			t.Errorf("KingdomName(%d) = %q, want %q", test.kingdom, got, test.name)
		}
	}
	for _, index := range []uint16{734, 736, 1767, 3300, 3303, 3306} {
		if got := KingdomFromCape(index); got != KingdomHekalotia {
			t.Errorf("Hekalotia special cape %d mapped to kingdom %d", index, got)
		}
	}
	for _, index := range []uint16{735, 737, 1770, 3301, 3304, 3307} {
		if got := KingdomFromCape(index); got != KingdomAkelonia {
			t.Errorf("Akelonia special cape %d mapped to kingdom %d", index, got)
		}
	}
}

func TestKingdomCapeConversionRejectsUnsupportedInputs(t *testing.T) {
	if tier, ok := KingdomCapeTierOf(999); ok || tier != 0 {
		t.Fatalf("unknown cape tier = (%d, %t), want (0, false)", tier, ok)
	}
	if cape, ok := KingdomCapeAtTier(KingdomCapeTier(len(kingdomCapeFamilies)), KingdomHekalotia); ok || cape != 0 {
		t.Fatalf("out-of-range cape tier = (%d, %t), want (0, false)", cape, ok)
	}
	if cape, ok := KingdomCapeAtTier(CapeTierBasic, 99); ok || cape != 0 {
		t.Fatalf("unknown kingdom cape = (%d, %t), want (0, false)", cape, ok)
	}
	if cape, ok := KingdomCape(0, 99); ok || cape != 0 {
		t.Fatalf("empty cape for invalid kingdom = (%d, %t), want (0, false)", cape, ok)
	}
	for _, test := range []struct {
		kingdom byte
		want    uint16
	}{{KingdomHekalotia, 545}, {KingdomAkelonia, 546}, {KingdomNeutral, 548}} {
		if got, ok := KingdomCape(0, test.kingdom); !ok || got != test.want {
			t.Errorf("empty cape for kingdom %d = (%d, %t), want %d", test.kingdom, got, ok, test.want)
		}
	}
	for _, cape := range []uint16{543, 544, 1767, 1770, 999} {
		if converted, ok := KingdomCape(cape, KingdomHekalotia); ok || converted != 0 {
			t.Errorf("KingdomCape(%d) converted a non-neutral source to %d", cape, converted)
		}
	}
	for _, cape := range []uint16{548, 999} {
		if converted, ok := NeutralCape(cape); ok || converted != 0 {
			t.Errorf("NeutralCape(%d) converted unsupported source to %d", cape, converted)
		}
	}
	if got, ok := NeutralCape(1767); !ok || got != 3199 {
		t.Fatalf("special celestial cape 1767 -> (%d, %t), want master neutral cape", got, ok)
	}
	if got, ok := NeutralCape(1770); !ok || got != 3199 {
		t.Fatalf("special celestial cape 1770 -> (%d, %t), want master neutral cape", got, ok)
	}
}
