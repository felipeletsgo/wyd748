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
	raw, err := json.Marshal(Item{Index: 4011, UID: uid})
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
	if decoded.UID != uid || decoded.Index != 4011 {
		t.Fatalf("round-trip incorreto: %+v", decoded)
	}
}

func TestNormalizeItemUIDRejectsMalformedValues(t *testing.T) {
	for _, uid := range []string{"short", strings.Repeat("z", ItemUIDHexLength), strings.Repeat("0", 31)} {
		if _, err := NormalizeItemUID(uid); err == nil {
			t.Fatalf("UID invalido aceito: %q", uid)
		}
	}
}
