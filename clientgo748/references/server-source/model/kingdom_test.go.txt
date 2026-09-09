package model

import "testing"

func TestKingdomIsDerivedFromCape(t *testing.T) {
	for _, index := range []uint16{543, 545, 734, 736, 3191, 3194, 3197, 3300} {
		if got := KingdomFromCape(index); got != KingdomHekalotia {
			t.Fatalf("capa %d resultou reino %d", index, got)
		}
	}
	for _, index := range []uint16{544, 546, 735, 737, 3192, 3195, 3198, 3301} {
		if got := KingdomFromCape(index); got != KingdomAkelonia {
			t.Fatalf("capa %d resultou reino %d", index, got)
		}
	}
	if got := KingdomFromCape(548); got != KingdomNeutral {
		t.Fatalf("Wanderer deveria ser neutra, veio %d", got)
	}
}

func TestKingdomCapeRoundTripPreservesTier(t *testing.T) {
	for neutral, hekalotia := range map[uint16]uint16{548: 545, 549: 543, 3193: 3191, 3196: 3194, 3199: 3197} {
		cape, ok := KingdomCape(neutral, KingdomHekalotia)
		if !ok || cape != hekalotia {
			t.Fatalf("neutra %d -> Hekalotia = %d,%v", neutral, cape, ok)
		}
		back, ok := NeutralCape(cape)
		if !ok || back != neutral {
			t.Fatalf("capa %d -> neutra = %d,%v", cape, back, ok)
		}
	}
}

func TestAllNativeKingdomCapeTiersAreExplicit(t *testing.T) {
	want := [][3]uint16{
		{545, 546, 548}, {543, 544, 549}, {3191, 3192, 3193},
		{3194, 3195, 3196}, {3197, 3198, 3199},
	}
	for tier, family := range want {
		for column, kingdom := range []byte{KingdomHekalotia, KingdomAkelonia, KingdomNeutral} {
			got, ok := KingdomCapeAtTier(KingdomCapeTier(tier), kingdom)
			if !ok || got != family[column] {
				t.Fatalf("tier=%d reino=%d capa=%d,%v; quer %d", tier, kingdom, got, ok, family[column])
			}
			gotTier, ok := KingdomCapeTierOf(got)
			if !ok || gotTier != KingdomCapeTier(tier) {
				t.Fatalf("capa=%d tier=%d,%v; quer %d", got, gotTier, ok, tier)
			}
		}
	}
}
