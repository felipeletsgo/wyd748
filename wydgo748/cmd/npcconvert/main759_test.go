package main

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func TestDecode759UsesWidenedStatusAndPreservesShopItems(t *testing.T) {
	b := make([]byte, 1024)
	b[17], b[20], b[104], b[105] = 1, 2, 0xe1, 3
	for offset, value := range map[int]uint32{92: 399, 96: 70000, 100: 80000, 108: 90000, 112: 10000, 780: 0x40000000} {
		binary.LittleEndian.PutUint32(b[offset:], value)
	}
	binary.LittleEndian.PutUint64(b[32:], 123456)
	putU16(b, 132, 2000)
	putU16(b, 140, 59)
	putU16(b, 268+8, 685)
	copy(b[278:], []byte{61, 100, 43, 9, 0, 0})
	b[794], b[795], b[796], b[801], b[804] = 7, 8, 41, 9, 10
	got, _, err := decodeNPC("OfficialShop", b)
	if err != nil {
		t.Fatal(err)
	}
	if got.Tipo != model.TipoNPC || got.ClassInfo != 2 || got.Direction != 0xe0 ||
		got.Score.Defense != 70000 || got.Score.Attack != 80000 || got.Score.MaxHP != 90000 ||
		got.Score.CurHP != 90000 || got.Score.Mastery[0] != 2000 || got.Score.Merchant != 1 ||
		got.Score.Critical != 7 || got.Score.SaveMana != 8 || got.Score.MagicAttack != 9 ||
		got.Score.ResistFire != 10 || got.LearnedSkill != 0x40000000 || got.ExpReward != 123456 ||
		got.Equip.Rosto.Index != 59 || got.SkillBar[0] != 41 {
		t.Fatalf("7.59 record was narrowed or read using the legacy layout: %+v score=%+v", got, got.Score)
	}
	if len(got.Vende) != 2 || got.Vende[0] != (model.Item{}) ||
		got.Vende[1] != (model.Item{Index: 685, Eff: [6]byte{61, 100, 43, 9}}) || len(got.Carry) != 0 {
		t.Fatalf("shop slots or product effects changed: %+v", got.Vende)
	}
}

func TestDecodeRejectsUnsupportedLayoutsAndUnrepresentable759Data(t *testing.T) {
	for _, size := range []int{0, 624, 755, 757, 1023, 1025} {
		if _, _, err := decodeNPC("Unknown", make([]byte, size)); err == nil {
			t.Errorf("accepted unsupported size %d", size)
		}
	}
	for _, tc := range []struct {
		name string
		edit func([]byte)
	}{
		{"skill mask", func(b []byte) { b[784] = 1 }},
		{"experience", func(b []byte) { b[36] = 1 }},
		{"equipment", func(b []byte) { putU16(b, 140, model.ItemListSize) }},
		{"stock", func(b []byte) { putU16(b, 268, model.ItemListSize) }},
	} {
		t.Run(tc.name, func(t *testing.T) {
			b := make([]byte, 1024)
			tc.edit(b)
			if _, _, err := decodeNPC("Unsupported", b); err == nil {
				t.Fatal("unsupported 7.59 data was silently narrowed or discarded")
			}
		})
	}
}
