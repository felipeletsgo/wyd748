package model

import (
	"encoding/json"
	"testing"
)

func TestMarshalChaos(t *testing.T) {
	c := Char{
		Name:     "Test",
		Chaos:    150,
		Gold:     1000,
		Extended: &ExtendedScore{Version: ExtendedScoreVersion},
	}

	b, err := json.Marshal(c)
	if err != nil {
		t.Fatal(err)
	}

	t.Logf("Marshaled: %s", string(b))

	var c2 Char
	if err := json.Unmarshal(b, &c2); err != nil {
		t.Fatal(err)
	}

	if c2.Chaos != 150 {
		t.Fatalf("Expected Chaos 150, got %d", c2.Chaos)
	}
}
