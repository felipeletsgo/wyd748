package model

import (
	"encoding/json"
	"testing"
)

func validCharacterJSON(t *testing.T) []byte {
	t.Helper()
	encoded, err := json.Marshal(Char{
		UID: "character-uid", Name: "Tester", Score: &Score{Version: ScoreVersion},
	})
	if err != nil {
		t.Fatal(err)
	}
	return encoded
}

func editCharacterJSON(t *testing.T, mutate func(map[string]json.RawMessage)) []byte {
	t.Helper()
	var fields map[string]json.RawMessage
	if err := json.Unmarshal(validCharacterJSON(t), &fields); err != nil {
		t.Fatal(err)
	}
	mutate(fields)
	encoded, err := json.Marshal(fields)
	if err != nil {
		t.Fatal(err)
	}
	return encoded
}

func setCharacterJSONField(t *testing.T, fields map[string]json.RawMessage, key string, value any) {
	t.Helper()
	encoded, err := json.Marshal(value)
	if err != nil {
		t.Fatal(err)
	}
	fields[key] = encoded
}

func setCharacterScoreField(t *testing.T, fields map[string]json.RawMessage, key string, value any) {
	t.Helper()
	var score map[string]json.RawMessage
	if err := json.Unmarshal(fields["score"], &score); err != nil {
		t.Fatal(err)
	}
	setCharacterJSONField(t, score, key, value)
	setCharacterJSONField(t, fields, "score", score)
}

func decodeCharacterJSON(data []byte) (Char, error) {
	var character Char
	err := json.Unmarshal(data, &character)
	return character, err
}

func TestCharacterJSONNullUnknownAndTrailingInput(t *testing.T) {
	character := Char{Name: "before"}
	if err := json.Unmarshal([]byte(" null "), &character); err != nil || !character.IsEmpty() {
		t.Fatalf("null did not clear the character slot: %+v, %v", character, err)
	}
	if _, err := decodeCharacterJSON([]byte("{")); err == nil {
		t.Fatal("malformed character JSON was accepted")
	}
	if _, err := decodeCharacterJSON(append(validCharacterJSON(t), []byte(" {}")...)); err == nil {
		t.Fatal("trailing JSON value was accepted")
	}
	unknown := editCharacterJSON(t, func(fields map[string]json.RawMessage) {
		setCharacterJSONField(t, fields, "unexpected", true)
	})
	if _, err := decodeCharacterJSON(unknown); err == nil {
		t.Fatal("unknown character field was accepted")
	}
}

func TestCharacterJSONMigratesLegacyChaosExactlyOnce(t *testing.T) {
	for _, test := range []struct {
		name   string
		legacy uint32
		wantCP int16
	}{
		{"zero legacy chaos", 0, 0},
		{"neutral legacy byte", 75, 0},
		{"negative legacy byte", 1, -74},
		{"upper clamp", 200, 75},
	} {
		t.Run(test.name, func(t *testing.T) {
			data := editCharacterJSON(t, func(fields map[string]json.RawMessage) {
				delete(fields, "cp")
				delete(fields, "chaosVersion")
				setCharacterJSONField(t, fields, "chaos", test.legacy)
			})
			character, err := decodeCharacterJSON(data)
			if err != nil || character.CP != test.wantCP || character.ChaosVersion != ChaosFormatVersion {
				t.Fatalf("legacy chaos decoded as CP=%d version=%d err=%v", character.CP, character.ChaosVersion, err)
			}
		})
	}

	for _, test := range []struct {
		cp   int16
		want int16
	}{{75, 0}, {74, 74}} {
		data := editCharacterJSON(t, func(fields map[string]json.RawMessage) {
			delete(fields, "chaosVersion")
			setCharacterJSONField(t, fields, "cp", test.cp)
		})
		character, err := decodeCharacterJSON(data)
		if err != nil || character.CP != test.want || character.ChaosVersion != ChaosFormatVersion {
			t.Fatalf("unversioned CP=%d migrated as %d, version=%d err=%v", test.cp, character.CP, character.ChaosVersion, err)
		}
	}
}

func TestCharacterJSONRejectsInvalidPersistentState(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*testing.T, map[string]json.RawMessage)
	}{
		{"both chaos formats", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterJSONField(t, fields, "chaos", uint32(75))
		}},
		{"unknown chaos version", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterJSONField(t, fields, "chaosVersion", uint8(2))
		}},
		{"CP above range", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterJSONField(t, fields, "cp", 76)
		}},
		{"invalid score", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterScoreField(t, fields, "version", 0)
		}},
		{"invalid alternate score", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterJSONField(t, fields, "alternateCelestial", CelestialForm{
				Evolution: "subcelestial", Class: 1, Face: Item{Index: 1}, Score: &Score{Version: 0},
			})
		}},
		{"progress above range", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterJSONField(t, fields, "archCrystals", 5)
		}},
		{"active celestial above level", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterJSONField(t, fields, "evolution", "celestial")
			setCharacterScoreField(t, fields, "level", 200)
		}},
		{"alternate without active form", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterJSONField(t, fields, "alternateCelestial", CelestialForm{
				Evolution: "subcelestial", Class: 1, Face: Item{Index: 1}, Score: &Score{Version: ScoreVersion},
			})
		}},
		{"duplicate active form", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterJSONField(t, fields, "evolution", "celestial")
			setCharacterJSONField(t, fields, "alternateCelestial", CelestialForm{
				Evolution: "celestial", Class: 1, Face: Item{Index: 1}, Score: &Score{Version: ScoreVersion},
			})
		}},
		{"short inventory", func(t *testing.T, fields map[string]json.RawMessage) {
			setCharacterJSONField(t, fields, "inv", make([]Item, len(Char{}.Inv)-1))
		}},
	} {
		t.Run(test.name, func(t *testing.T) {
			data := editCharacterJSON(t, func(fields map[string]json.RawMessage) {
				test.mutate(t, fields)
			})
			if _, err := decodeCharacterJSON(data); err == nil {
				t.Fatal("invalid character state was accepted")
			}
		})
	}

	validAlternate := editCharacterJSON(t, func(fields map[string]json.RawMessage) {
		setCharacterJSONField(t, fields, "evolution", "celestial")
		setCharacterJSONField(t, fields, "alternateCelestial", CelestialForm{
			Evolution: "subcelestial", Class: 1, Face: Item{Index: 1}, Score: &Score{Version: ScoreVersion},
		})
	})
	if _, err := decodeCharacterJSON(validAlternate); err != nil {
		t.Fatalf("valid distinct forms were rejected: %v", err)
	}
}

func TestAccountJSONRequiresCompleteCargoAndNoTrailingInput(t *testing.T) {
	encoded, err := json.Marshal(Account{Name: "account", PasswordHash: "hash"})
	if err != nil {
		t.Fatal(err)
	}
	var loaded Account
	if err := json.Unmarshal(encoded, &loaded); err != nil || loaded.Name != "account" {
		t.Fatalf("valid account did not round-trip: %+v, %v", loaded, err)
	}

	var fields map[string]json.RawMessage
	if err := json.Unmarshal(encoded, &fields); err != nil {
		t.Fatal(err)
	}
	shortCargo, _ := json.Marshal(make([]Item, MaxCargo-1))
	fields["cargo"] = shortCargo
	short, _ := json.Marshal(fields)
	if err := json.Unmarshal(short, &Account{}); err == nil {
		t.Fatal("account with truncated structural cargo was accepted")
	}
	if err := json.Unmarshal(append(encoded, []byte(" {}")...), &Account{}); err == nil {
		t.Fatal("account with trailing JSON was accepted")
	}
	if err := json.Unmarshal([]byte(`{"name":"account","unknown":1}`), &Account{}); err == nil {
		t.Fatal("unknown account field was accepted")
	}
}
