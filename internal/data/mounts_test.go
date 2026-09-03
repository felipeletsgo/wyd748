package data

import (
	"path/filepath"
	"testing"
)

func TestLoadMountsPreservesNativeRiderDamageRates(t *testing.T) {
	catalog, err := LoadMounts(filepath.Join("..", "..", "data", "mounts.json"))
	if err != nil {
		t.Fatalf("LoadMounts: %v", err)
	}

	for mountType, want := range map[int]int{10: 73, 16: 66, 19: 65, 22: 69, 25: 65} {
		stats, ok := catalog.Stats(mountType)
		if !ok {
			t.Fatalf("tipo %d ausente", mountType)
		}
		if stats.RiderDamagePct != want {
			t.Errorf("tipo %d: riderDamagePct=%d, quer %d", mountType, stats.RiderDamagePct, want)
		}
	}
}
