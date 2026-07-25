package model

import (
	"encoding/json"
	"testing"
)

func TestArchFieldsRoundTrip(t *testing.T) {
	for _, slot := range []int{0, 1, 3} {
		original := Char{
			Name: "Arch", Evolution: "arch",
			ArchMortalSlot: slot, ArchMortalLevel: 395,
			Extended: &ExtendedScore{Version: ExtendedScoreVersion, MaxHP: 1, CurHP: 1},
		}
		blob, err := json.Marshal(original)
		if err != nil {
			t.Fatal(err)
		}
		var back Char
		if err := json.Unmarshal(blob, &back); err != nil {
			t.Fatal(err)
		}
		if back.ArchMortalSlot != slot {
			t.Errorf("slot %d nao sobreviveu ao round-trip: %d (json=%s)", slot, back.ArchMortalSlot, blob)
		}
		if back.ArchMortalLevel != 395 {
			t.Errorf("nivel nao sobreviveu: %d", back.ArchMortalLevel)
		}
	}
}
