package data

import (
	"testing"

	"wydgo/internal/model"
)

func TestRuneTrackAcceptsSingleGeneratorAndReportsMissingConfiguration(t *testing.T) {
	cache := &instanceGeneratorCache{
		dir:   t.TempDir(),
		files: map[string][]model.NPCGener{"combined.txt": {{Label: 5662}}},
	}
	cfg := &model.VolatileUxmal{Version: 759, Generator: "combined.txt"}
	if err := cache.expandRuneTrack("Rune Track", cfg); err != nil {
		t.Fatal(err)
	}
	if len(cfg.Generators) != 1 || cfg.Generators[0].Label != 5662 {
		t.Fatalf("single-file generator was not copied: %+v", cfg.Generators)
	}

	cfg = &model.VolatileUxmal{Version: 759}
	if err := cache.expandRuneTrack("Rune Track", cfg); err == nil {
		t.Fatal("Rune Track accepted a configuration without generator files")
	}

	cfg = &model.VolatileUxmal{Version: 759, Generator: "missing.txt"}
	if err := cache.expandRuneTrack("Rune Track", cfg); err == nil {
		t.Fatal("Rune Track accepted a missing combined generator")
	}
}
