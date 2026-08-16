package model

import "testing"

func TestKingdomFromCapeAndNamesCoverAllFamilies(t *testing.T) {
	hekalotia := []uint16{543, 545, 734, 736, 1767, 3191, 3194, 3197, 3300, 3303, 3306}
	akelonia := []uint16{544, 546, 735, 737, 1770, 3192, 3195, 3198, 3301, 3304, 3307}
	for _, cape := range hekalotia {
		if got := KingdomFromCape(cape); got != KingdomHekalotia {
			t.Fatalf("cape %d => kingdom %d", cape, got)
		}
	}
	for _, cape := range akelonia {
		if got := KingdomFromCape(cape); got != KingdomAkelonia {
			t.Fatalf("cape %d => kingdom %d", cape, got)
		}
	}
	if KingdomFromCape(0) != KingdomNeutral || KingdomFromCape(3199) != KingdomNeutral {
		t.Fatal("capa neutra/desconhecida ganhou reino")
	}
	if KingdomName(KingdomHekalotia) != "Hekalotia" || KingdomName(KingdomAkelonia) != "Akelonia" ||
		KingdomName(99) != "Neutro" {
		t.Fatal("KingdomName divergiu")
	}
}

func TestKingdomCapeTierLookupAndProjectionCoverEveryTier(t *testing.T) {
	for tier := CapeTierBasic; tier <= CapeTierMaster; tier++ {
		for _, kingdom := range []byte{KingdomHekalotia, KingdomAkelonia, KingdomNeutral} {
			cape, ok := KingdomCapeAtTier(tier, kingdom)
			if !ok || cape == 0 {
				t.Fatalf("tier=%d kingdom=%d nao projetou", tier, kingdom)
			}
			gotTier, found := KingdomCapeTierOf(cape)
			if !found || gotTier != tier {
				t.Fatalf("cape=%d tier=%d/%v esperado=%d", cape, gotTier, found, tier)
			}
		}
	}
	if cape, ok := KingdomCapeAtTier(KingdomCapeTier(99), KingdomNeutral); ok || cape != 0 {
		t.Fatal("tier fora da matriz foi aceito")
	}
	if cape, ok := KingdomCapeAtTier(CapeTierBasic, 99); ok || cape != 0 {
		t.Fatal("reino invalido foi aceito")
	}
	if _, ok := KingdomCapeTierOf(9999); ok {
		t.Fatal("capa fora da familia recebeu tier")
	}
}

func TestKingdomCapeOnlyConvertsNeutralVariants(t *testing.T) {
	for tier := CapeTierBasic; tier <= CapeTierMaster; tier++ {
		neutral, _ := KingdomCapeAtTier(tier, KingdomNeutral)
		for _, kingdom := range []byte{KingdomHekalotia, KingdomAkelonia} {
			got, ok := KingdomCape(neutral, kingdom)
			want, _ := KingdomCapeAtTier(tier, kingdom)
			if !ok || got != want {
				t.Fatalf("neutral=%d kingdom=%d => %d/%v want=%d", neutral, kingdom, got, ok, want)
			}
		}
	}
	basic, ok := KingdomCape(0, KingdomHekalotia)
	wantBasic, _ := KingdomCapeAtTier(CapeTierBasic, KingdomHekalotia)
	if !ok || basic != wantBasic {
		t.Fatalf("capa vazia => %d/%v want=%d", basic, ok, wantBasic)
	}
	if _, ok := KingdomCape(545, KingdomAkelonia); ok {
		t.Fatal("capa ja alinhada a reino foi reconvertida")
	}
	if _, ok := KingdomCape(9999, KingdomAkelonia); ok {
		t.Fatal("capa desconhecida foi convertida")
	}
	if _, ok := KingdomCape(548, 99); ok {
		t.Fatal("reino invalido foi aceito")
	}
}

func TestNeutralCapeCoversNativeAndSpecialCelestialCases(t *testing.T) {
	for tier := CapeTierBasic; tier <= CapeTierMaster; tier++ {
		neutral, _ := KingdomCapeAtTier(tier, KingdomNeutral)
		for _, kingdom := range []byte{KingdomHekalotia, KingdomAkelonia} {
			aligned, _ := KingdomCapeAtTier(tier, kingdom)
			got, ok := NeutralCape(aligned)
			if !ok || got != neutral {
				t.Fatalf("aligned=%d => neutral=%d/%v want=%d", aligned, got, ok, neutral)
			}
		}
		if _, ok := NeutralCape(neutral); ok {
			t.Fatalf("capa ja neutra %d foi convertida", neutral)
		}
	}
	masterNeutral, _ := KingdomCapeAtTier(CapeTierMaster, KingdomNeutral)
	for _, special := range []uint16{1767, 1770} {
		got, ok := NeutralCape(special)
		if !ok || got != masterNeutral {
			t.Fatalf("special=%d => %d/%v want=%d", special, got, ok, masterNeutral)
		}
	}
	if _, ok := NeutralCape(9999); ok {
		t.Fatal("capa desconhecida neutralizou")
	}
}
