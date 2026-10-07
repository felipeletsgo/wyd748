package model

import (
	"encoding/json"
	"strings"
	"testing"
)

func TestAccountDonateRoundTripAndCap(t *testing.T) {
	acc := Account{Name: "donor", PasswordHash: "hash", Donate: 1500}
	raw, err := json.Marshal(acc)
	if err != nil {
		t.Fatal(err)
	}
	if !strings.Contains(string(raw), `"donate":1500`) {
		t.Fatalf("donate balance not serialized: %s", raw)
	}
	var loaded Account
	if err := json.Unmarshal(raw, &loaded); err != nil || loaded.Donate != 1500 {
		t.Fatalf("donate balance not loaded: %v %d", err, loaded.Donate)
	}
	if err := loaded.Validate(); err != nil {
		t.Fatalf("valid balance rejected: %v", err)
	}
	loaded.Donate = MaxDonateBalance + 1
	if err := loaded.Validate(); err == nil {
		t.Fatal("balance above the cap was accepted")
	}
	empty, _ := json.Marshal(Account{Name: "zero", PasswordHash: "hash"})
	if strings.Contains(string(empty), "donate") {
		t.Fatalf("zero balance must be omitted: %s", empty)
	}
}
