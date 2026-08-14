package main

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

func putU16(b []byte, off int, value uint16) {
	binary.LittleEndian.PutUint16(b[off:off+2], value)
}

func TestConvertShopPreservesProductEffects(t *testing.T) {
	b := make([]byte, mobFileSize)
	putU16(b, offStatus, 20)
	b[17] = 1
	b[offStatus+6] = 1
	putU16(b, offStatus+8, 1000)
	putU16(b, offEquip, 202)

	putU16(b, offCarry+8, 685)
	copy(b[offCarry+10:offCarry+16], []byte{61, 100, 0, 0, 0, 0})

	got, report := convert("Mercador", b)
	if report.invalidItems != 0 {
		t.Fatalf("itens invalidos=%d, esperado 0", report.invalidItems)
	}
	if got.Tipo != model.TipoNPC || len(got.Carry) != 0 || len(got.Vende) != 2 {
		t.Fatalf("classificacao/estoque incorretos: tipo=%q carry=%d vende=%d",
			got.Tipo, len(got.Carry), len(got.Vende))
	}
	if got.Vende[1].Index != 685 || got.Vende[1].Eff != [6]byte{61, 100} {
		t.Fatalf("efeitos do produto foram perdidos: %+v", got.Vende[1])
	}
}

func TestConvertSeparatesFunctionalMerchantFromDirection(t *testing.T) {
	b := make([]byte, mobFileSize)
	b[17] = 43
	b[offStatus+6] = 0xFB
	putU16(b, offStatus+8, 100)

	got, _ := convert("Kibita", b)
	if got.Extended == nil || got.Extended.Merchant != 43 || got.Direction != 0xF0 {
		t.Fatalf("extended=%+v direction=0x%X, quer Merchant 43/0xF0", got.Extended, got.Direction)
	}
	if score := got.MakeExtendedScore(got.Extended.MaxHP).CompatibilityScore(); score.Merchant != 0xFB {
		t.Fatalf("merchant visual=0x%X, quer 0xFB", score.Merchant)
	}
}

func TestConvertNormalizesSkillMasterPages(t *testing.T) {
	b := make([]byte, mobFileSize)
	b[17] = 19
	b[offStatus+6] = 0x13
	putU16(b, offStatus+8, 100)
	for branch, source := range []int{0, 27, 54} {
		for i := 0; i < 8; i++ {
			if branch == 2 && i == 7 {
				continue // simula o Evilator ausente no binario Micronics
			}
			putU16(b, offCarry+(source+i)*8, uint16(5000+branch*8+i))
		}
	}

	got, _ := convert("Mestre", b)
	if len(got.Vende) != 27 {
		t.Fatalf("estoque=%d, quer 27", len(got.Vende))
	}
	for _, separator := range []int{8, 17, 26} {
		if got.Vende[separator].Index != 0 {
			t.Fatalf("separador %d contem %d", separator, got.Vende[separator].Index)
		}
	}
	for branch, slot := range []int{0, 9, 18} {
		if got.Vende[slot].Index != uint16(5000+branch*8) {
			t.Fatalf("ramo %d inicia em %d", branch, got.Vende[slot].Index)
		}
	}
	if got.Vende[25].Index != 5023 {
		t.Fatalf("24a skill nao foi completada: %d", got.Vende[25].Index)
	}
}

func TestConvertMonsterPreservesDropEffectsAndRejectsInvalidIndexes(t *testing.T) {
	b := make([]byte, mobFileSize)
	putU16(b, offStatus, 4)
	putU16(b, offStatus+8, 100)

	putU16(b, offCarry, 1115)
	copy(b[offCarry+2:offCarry+8], []byte{101, 1, 0, 0, 0, 0})
	putU16(b, offCarry+8, model.ItemListSize)
	putU16(b, offCarry+16, 405)

	got, report := convert("Gremlin", b)
	if report.invalidItems != 1 {
		t.Fatalf("itens invalidos=%d, esperado 1", report.invalidItems)
	}
	if got.Tipo != model.TipoMonstro || len(got.Vende) != 0 || len(got.Carry) != 3 {
		t.Fatalf("classificacao/drop incorretos: tipo=%q carry=%d vende=%d",
			got.Tipo, len(got.Carry), len(got.Vende))
	}
	if got.Carry[0] != (model.Item{Index: 1115, Eff: [6]byte{101, 1}}) ||
		got.Carry[1] != (model.Item{}) ||
		got.Carry[2] != (model.Item{Index: 405}) {
		t.Fatalf("drop convertido incorretamente: %+v", got.Carry)
	}
}

func TestConvertPreservesNativeFieldsAndSanitizesRewards(t *testing.T) {
	b := make([]byte, mobFileSize)
	b[20] = 3
	putU16(b, offStatus, 200)
	putU16(b, offStatus+8, 9000)
	copy(b[offStatus+24:offStatus+28], []byte{10, 20, 30, 40})
	binary.LittleEndian.PutUint32(b[24:28], maxSaneNPCGold+1)
	binary.LittleEndian.PutUint32(b[28:32], 250_000)
	binary.LittleEndian.PutUint32(b[732:736], 0x1234)
	b[742], b[743], b[749], b[750], b[751] = 7, 8, 9, 10, 11

	got, report := convert("Completo", b)
	if got.Extended == nil || got.ClassInfo != 3 ||
		got.Extended.Mastery != [4]uint32{10, 20, 30, 40} ||
		got.LearnedSkill != 0x1234 || got.Extended.Critical != 7 ||
		got.Extended.SaveMana != 8 || got.Extended.MagicAttack != 9 ||
		got.Extended.RegenHP != 10 || got.Extended.RegenMP != 11 {
		t.Fatalf("campos nativos perdidos: %+v", got)
	}
	if got.Gold != 0 || got.ExpReward != 250_000 ||
		report.discardedGold != 1 || report.discardedExp != 0 {
		t.Fatalf("saneamento de recompensa incorreto: gold=%d exp=%d report=%+v",
			got.Gold, got.ExpReward, report)
	}
}
