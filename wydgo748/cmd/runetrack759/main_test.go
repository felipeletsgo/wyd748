package main

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func TestDecode759IndependentOffsets(t *testing.T) {
	b := make([]byte, 1024)
	put := func(offset int, value uint32) { binary.LittleEndian.PutUint32(b[offset:], value) }
	put(32, 8500)
	put(92, 499)
	put(96, 5350)
	put(100, 5600)
	put(108, 32000)
	put(112, 1200)
	put(44, 99) // BaseScore must not replace Status.
	put(24, 1234)
	b[104], b[105], b[794], b[801] = 0xb0, 2, 9, 17
	binary.LittleEndian.PutUint16(b[124:], 123)
	binary.LittleEndian.PutUint16(b[140:], 12)
	binary.LittleEndian.PutUint16(b[268+8*5:], 5110)
	copy(b[268+8*5+2:], []byte{1, 2, 3, 4, 5, 6})
	d, err := decode("RT_LichKing", b)
	if err != nil {
		t.Fatal(err)
	}
	if d.Tipo != model.TipoMonstro || d.ExpReward != 8500 || d.Gold != 1234 ||
		d.Score.Level != 499 || d.Score.Defense != 5350 || d.Score.Attack != 5600 ||
		d.Score.MaxHP != 32000 || d.Score.CurHP != 32000 || d.Score.MaxMP != 1200 ||
		d.Score.Str != 123 || d.Direction != 0xb0 || d.Score.AttackRun != 2 ||
		d.Score.Critical != 9 || d.Score.MagicAttack != 17 || d.Equip.Rosto.Index != 12 {
		t.Fatalf("decoded wrong layout: %+v / %+v", d, d.Score)
	}
	if len(d.Carry) != 6 || d.Carry[0].Index != 0 || d.Carry[5].Index != 5110 ||
		d.Carry[5].Eff != [6]byte{1, 2, 3, 4, 5, 6} {
		t.Fatalf("carry holes or effects changed: %+v", d.Carry)
	}
	b[17] = 9
	d, err = decode("RT_Freyja", b)
	if err != nil || d.Tipo != model.TipoNPC || len(d.Carry) != 0 || len(d.Vende) != 6 {
		t.Fatalf("merchant classification: %+v / %v", d, err)
	}
}

func TestDecode759RejectsLossyInputs(t *testing.T) {
	for _, offset := range []int{36, 784} {
		b := make([]byte, 1024)
		b[offset] = 1
		if _, err := decode("RT_Test", b); err == nil {
			t.Fatalf("accepted widened value at %d", offset)
		}
	}
	if _, err := decode("RT_Test", make([]byte, 960)); err == nil {
		t.Fatal("accepted obsolete layout")
	}
	seen := map[string]bool{}
	for _, alias := range aliases {
		if len(alias) > 12 || seen[alias] {
			t.Fatalf("invalid wire alias %q", alias)
		}
		seen[alias] = true
	}
}
