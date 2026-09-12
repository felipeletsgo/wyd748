package model

import (
	"encoding/json"
	"testing"
)

func TestArchFieldsRoundTrip(t *testing.T) {
	for _, origin := range []string{
		"11111111111141118111111111111111",
		"22222222222242228222222222222222",
	} {
		original := Char{
			UID: origin, Name: "Arch", Evolution: "arch",
			ArchMortalUID: origin, ArchMortalLevel: 395,
			Score: &Score{Version: ScoreVersion, MaxHP: 1, CurHP: 1},
		}
		blob, err := json.Marshal(original)
		if err != nil {
			t.Fatal(err)
		}
		var back Char
		if err := json.Unmarshal(blob, &back); err != nil {
			t.Fatal(err)
		}
		if back.ArchMortalUID != origin {
			t.Errorf("origem %s nao sobreviveu ao round-trip: %s (json=%s)",
				origin, back.ArchMortalUID, blob)
		}
		if back.ArchMortalLevel != 395 {
			t.Errorf("nivel nao sobreviveu: %d", back.ArchMortalLevel)
		}
	}
}
