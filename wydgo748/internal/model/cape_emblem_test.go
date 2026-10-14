package model

import (
	"encoding/json"
	"testing"
)

func TestCapeEmblemPersistenceAndLegacyProjection(t *testing.T) {
	for symbol := byte(1); symbol <= CapeEmblemCount; symbol++ {
		ch := Char{Name: "Citizen", Score: &Score{Version: ScoreVersion}, Citizenship: 1, CapeEmblem: symbol}
		data, err := json.Marshal(ch)
		if err != nil {
			t.Fatal(err)
		}
		var restored Char
		if err := json.Unmarshal(data, &restored); err != nil {
			t.Fatal(err)
		}
		if restored.CapeEmblem != symbol || restored.Citizenship != 1 || AppearanceCapeEmblem(restored) != symbol {
			t.Fatal("JSON reload changed the cosmetic draw or domain citizenship")
		}
	}
	for _, symbol := range []byte{0, 11, 255} {
		if AppearanceCapeEmblem(Char{CapeEmblem: symbol, Citizenship: 3}) != 3 ||
			AppearanceCapeEmblem(Char{CapeEmblem: symbol, Citizenship: 11}) != 0 {
			t.Fatal("legacy/invalid metadata did not use a bounded fallback")
		}
	}
	ch := Char{CapeEmblem: 1}
	if ch.IsEmpty() {
		t.Fatal("cosmetic state was discarded as an empty slot")
	}
	if _, err := json.Marshal(ch); err == nil {
		t.Fatal("nameless cosmetic state was serialized")
	}
}
