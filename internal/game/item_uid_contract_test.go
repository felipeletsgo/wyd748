package game

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestMaterializeItemCoversEmptyExistingAndGeneratedIdentities(t *testing.T) {
	empty, err := materializeItem(model.Item{})
	if err != nil || empty != (model.Item{}) {
		t.Fatalf("item vazio=%+v err=%v", empty, err)
	}
	for _, invalid := range []model.Item{
		{UID: "11111111111141118111111111111111"},
		{ActivatedUnix: 1},
		{ExpiresUnix: 1},
	} {
		if _, err := materializeItem(invalid); err == nil {
			t.Fatalf("slot vazio com metadata foi aceito: %+v", invalid)
		}
	}

	upper := "AAAAAAAAAAAA4AAA8AAAAAAAAAAAAAAA"
	existing := model.Item{Index: 4011, UID: upper, Eff: [6]byte{1, 2, 3, 4, 5, 6}, ActivatedUnix: 2, ExpiresUnix: 3}
	got, err := materializeItem(existing)
	if err != nil {
		t.Fatal(err)
	}
	if got.UID != strings.ToLower(upper) || got.Index != existing.Index || got.Eff != existing.Eff ||
		got.ActivatedUnix != 2 || got.ExpiresUnix != 3 {
		t.Fatalf("item existente perdeu estado: %+v", got)
	}
	if _, err := materializeItem(model.Item{Index: 1, UID: "bad"}); err == nil {
		t.Fatal("UID invalido foi aceito")
	}

	generated, err := materializeItem(model.Item{Index: 4012, Eff: [6]byte{9, 8, 7, 6, 5, 4}})
	if err != nil {
		t.Fatal(err)
	}
	if generated.UID == "" || generated.Index != 4012 || generated.Eff != [6]byte{9, 8, 7, 6, 5, 4} {
		t.Fatalf("materializacao incompleta: %+v", generated)
	}
	if normalized, err := model.NormalizeItemUID(generated.UID); err != nil || normalized != generated.UID {
		t.Fatalf("UID gerado nao e canonico: %q err=%v", generated.UID, err)
	}
}
