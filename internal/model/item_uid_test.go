package model

import (
	"encoding/json"
	"strings"
	"testing"
)

func TestItemUIDIsCanonicalAndWireComparisonIgnoresIt(t *testing.T) {
	uid, err := NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	if len(uid) != ItemUIDHexLength {
		t.Fatalf("UID com %d caracteres", len(uid))
	}
	if normalized, err := NormalizeItemUID(strings.ToUpper(uid)); err != nil || normalized != uid {
		t.Fatalf("normalizacao=%q err=%v", normalized, err)
	}
	authoritative := Item{Index: 4011, UID: uid, Eff: [6]byte{61, 10}}
	client := Item{Index: 4011, Eff: [6]byte{61, 10}}
	if !authoritative.WireEqual(client) {
		t.Fatal("UID server-side alterou a igualdade dos oito bytes nativos")
	}
	if authoritative == client {
		t.Fatal("comparacao interna perdeu a identidade")
	}
}

func TestItemJSONPersistsUIDEvenWithoutEffects(t *testing.T) {
	uid, err := NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	original := Item{Index: 4011, UID: uid, ActivatedUnix: 1_700_000_000, ExpiresUnix: 1_702_592_000}
	raw, err := json.Marshal(original)
	if err != nil {
		t.Fatal(err)
	}
	if !strings.Contains(string(raw), uid) {
		t.Fatalf("UID ausente do JSON: %s", raw)
	}
	var decoded Item
	if err := json.Unmarshal(raw, &decoded); err != nil {
		t.Fatal(err)
	}
	if decoded != original {
		t.Fatalf("round-trip incorreto: %+v", decoded)
	}
}

func TestWireEqualIgnoresServerTimedMetadata(t *testing.T) {
	a := Item{Index: 4200, Eff: [6]byte{1, 2}, ActivatedUnix: 10, ExpiresUnix: 20}
	b := Item{Index: 4200, Eff: [6]byte{1, 2}}
	if !a.WireEqual(b) {
		t.Fatal("server-only item deadline leaked into the legacy wire contract")
	}
}

func TestAccountRejectsInvalidTimedItemMetadata(t *testing.T) {
	uid, err := NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	charUID, err := NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	base := Account{Name: "timed", PasswordHash: "hash", Chars: []Char{{
		Name: "Timed", UID: charUID,
		Score: &Score{Version: ScoreVersion},
	}}}
	for _, item := range []Item{
		{Index: 4200, UID: uid, ActivatedUnix: 10},
		{Index: 4200, UID: uid, ActivatedUnix: 20, ExpiresUnix: 10},
		{Index: 4200, ActivatedUnix: 10, ExpiresUnix: 20},
	} {
		acc := base
		acc.Chars = append([]Char(nil), base.Chars...)
		acc.Chars[0].Inv[0] = item
		if err := acc.Validate(); err == nil {
			t.Fatalf("invalid timed item accepted: %+v", item)
		}
	}
}

func TestNormalizeItemUIDRejectsMalformedValues(t *testing.T) {
	for _, uid := range []string{"short", strings.Repeat("z", ItemUIDHexLength), strings.Repeat("0", 31)} {
		if _, err := NormalizeItemUID(uid); err == nil {
			t.Fatalf("UID invalido aceito: %q", uid)
		}
	}
}
