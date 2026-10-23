package model

import (
	"encoding/json"
	"testing"
	"time"
)

func TestFairyPauseMetadataRoundTripAndFrozenTooltip(t *testing.T) {
	now := time.Unix(1900000000, 0)
	for _, effects := range [][6]byte{{}, {43, 9}} {
		item := Item{Index: 3914, UID: "11111111111141118111111111111111", Eff: effects,
			ActivatedUnix: now.Add(-time.Hour).Unix(), PausedUnix: now.Unix(),
			ExpiresUnix: now.Add(51*time.Hour + 4*time.Minute).Unix()}
		encoded, err := json.Marshal(item)
		if err != nil {
			t.Fatal(err)
		}
		var restored Item
		if err := json.Unmarshal(encoded, &restored); err != nil || restored != item {
			t.Fatalf("paused item round trip changed metadata: item=%+v err=%v", restored, err)
		}
		before := ExpiryDisplayEffects(item, now)
		if after := ExpiryDisplayEffects(item, now.Add(30*24*time.Hour)); after != before {
			t.Fatalf("paused tooltip decreased: before=%v after=%v", before, after)
		}
		if effects == [6]byte{} && before != [6]byte{106, 2, 107, 3, 108, 4} {
			t.Fatalf("remaining tooltip=%v", before)
		}
		if !item.WireEqual(Item{Index: item.Index, Eff: before}) {
			t.Fatal("server-only pause metadata changed the client echo contract")
		}
	}
}

func TestFairyPauseMetadataValidation(t *testing.T) {
	valid := Item{Index: 3914, UID: "11111111111141118111111111111111", ActivatedUnix: 100, ExpiresUnix: 300, PausedUnix: 200}
	for _, scenario := range []string{"valid", "negative", "before-activation", "at-expiry", "unactivated", "non-fairy", "empty"} {
		t.Run(scenario, func(t *testing.T) {
			item := valid
			switch scenario {
			case "negative":
				item.PausedUnix = -1
			case "before-activation":
				item.PausedUnix = 99
			case "at-expiry":
				item.PausedUnix = 300
			case "unactivated":
				item.ActivatedUnix, item.ExpiresUnix = 0, 0
			case "non-fairy":
				item.Index = 3980
			case "empty":
				item.Index = 0
			}
			account := Account{Name: "test", PasswordHash: "hash", Cargo: [MaxCargo]Item{item}}
			if err := account.Validate(); (err == nil) != (scenario == "valid") {
				t.Fatalf("pause validation returned %v", err)
			}
		})
	}
}
