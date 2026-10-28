package data

import (
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestLoadMountsPreservesNativeRiderDamageRates(t *testing.T) {
	catalog, err := LoadMounts(filepath.Join("..", "..", "data", "content", "mounts.json"))
	if err != nil {
		t.Fatalf("LoadMounts: %v", err)
	}

	for mountType, want := range map[int]int{10: 73, 16: 66, 19: 65, 22: 69, 25: 65} {
		stats, ok := catalog.Stats(mountType)
		if !ok {
			t.Fatalf("type %d missing", mountType)
		}
		if stats.RiderDamagePct != want {
			t.Errorf("type %d: riderDamagePct=%d, want %d", mountType, stats.RiderDamagePct, want)
		}
	}
}

// The timed horses mirror the client g_pMountBonus2 rows and name-encoded
// lifetimes; without them they had no speed, bonus or expiry.
func TestLoadMountsPremiumHorsesMirrorClientTable(t *testing.T) {
	catalog, err := LoadMounts(filepath.Join("..", "..", "data", "content", "mounts.json"))
	if err != nil {
		t.Fatal(err)
	}
	want := map[uint16]model.PremiumMountStats{
		3980: {Attack: 35, MagicPercent: 7, Speed: 6, DurationDays: 3},
		3984: {Attack: 450, MagicPercent: 72, Evasion: 10, Resist: 28, Speed: 6, DurationDays: 15},
		3986: {Attack: 120, MagicPercent: 45, Speed: 6, DurationDays: 30},
		3989: {Attack: 325, MagicPercent: 35, Evasion: 16, Resist: 28, Speed: 6, DurationDays: 30},
	}
	for index, expected := range want {
		got, ok := catalog.PremiumStats(index)
		got.Name = ""
		if !ok || got != expected {
			t.Errorf("premium horse %d = %+v ok=%v, want %+v", index, got, ok, expected)
		}
	}
	if len(catalog.Premium) != 10 {
		t.Fatalf("premium horses = %d, want 10", len(catalog.Premium))
	}
}

func TestLoadMountsRejectsInvalidPremiumHorses(t *testing.T) {
	for _, body := range []string{
		`{"premium":{"2360":{"speed":6,"durationDays":3}}}`,
		`{"premium":{"3980":{"speed":7,"durationDays":3}}}`,
		`{"premium":{"3980":{"speed":6,"durationDays":0}}}`,
		`{"premium":{"3980":{"speed":6,"durationDays":3,"attack":-1}}}`,
		`{"premium":{"x":{"speed":6,"durationDays":3}}}`,
	} {
		path := filepath.Join(t.TempDir(), "mounts.json")
		if err := os.WriteFile(path, []byte(body), 0o600); err != nil {
			t.Fatal(err)
		}
		if _, err := LoadMounts(path); err == nil {
			t.Errorf("invalid premium horse accepted: %s", body)
		}
	}
}
