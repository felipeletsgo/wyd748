package game

import (
	"testing"

	"wydgo/internal/model"
)

// O Arch PARA de receber EXP nos niveis internos 354 e 369 ate destravar na
// Lindy. Nao e um teto que ignora o excedente: o ganho e barrado por inteiro
// (GetFunc.cpp:1565 devolve 0; CMob.cpp:2079 impede o level-up).

func archNoNivel(level uint32, d355, d370 bool) *model.Char {
	return &model.Char{
		Name: "God", Evolution: "arch",
		Exp:          mortalNextLevel[level],
		ArchLevel355: d355, ArchLevel370: d370,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, Level: level},
	}
}

func TestArchTravaEmTrezentosECinquentaEQuatro(t *testing.T) {
	ch := archNoNivel(archLockLevel355, false, false)
	expAntes := ch.Exp

	if _, aplicado := grantExp(ch, 50_000_000); aplicado != 0 {
		t.Errorf("recebeu %d de EXP com a trava de pe", aplicado)
	}
	if ch.Exp != expAntes {
		t.Errorf("EXP mudou de %d para %d", expAntes, ch.Exp)
	}
}

func TestArchVoltaAGanharDepoisDeDestravar(t *testing.T) {
	ch := archNoNivel(archLockLevel355, true, false) // 355 destravado
	if _, aplicado := grantExp(ch, 50_000_000); aplicado == 0 {
		t.Fatal("continuou travado depois de destravar o 355")
	}
	// ...e trava de novo no 369, que ainda nao foi destravado.
	ch = archNoNivel(archLockLevel370, true, false)
	if _, aplicado := grantExp(ch, 50_000_000); aplicado != 0 {
		t.Errorf("passou da trava de 370 sem destravar: aplicado=%d", aplicado)
	}
	ch = archNoNivel(archLockLevel370, true, true)
	if _, aplicado := grantExp(ch, 50_000_000); aplicado == 0 {
		t.Error("continuou travado com as duas travas liberadas")
	}
}

// TestMortalNaoSofreATrava: as travas sao do Arch. Um Mortal passando por 354
// nao pode parar de evoluir.
func TestMortalNaoSofreATrava(t *testing.T) {
	ch := archNoNivel(archLockLevel355, false, false)
	ch.Evolution = "" // Mortal
	if _, aplicado := grantExp(ch, 50_000_000); aplicado == 0 {
		t.Error("Mortal foi barrado por uma trava de Arch")
	}
}

// TestDestraveSobreviveAoRebaixamentoDoCristal e a razao de as flags viverem na
// CONTA: o cristal derruba o personagem para baixo de 354, e sem o registro
// permanente ele travaria DE NOVO ao subir -- teria de destravar na Lindy a
// cada cristal.
func TestDestraveSobreviveAoRebaixamentoDoCristal(t *testing.T) {
	w, p, s := archComExp(2_600_000_000, 0)
	p.Char.ArchLevel355, p.Char.ArchLevel370 = true, true

	usarCristal(w, p, s, firstArchCrystal)

	// O desconto rebaixou o nivel...
	if p.Char.Extended.Level >= 2_600_000_000 {
		t.Fatal("cenario invalido")
	}
	// ...mas o destrave persiste na conta.
	if !p.Account.Chars[0].ArchLevel355 || !p.Account.Chars[0].ArchLevel370 {
		t.Fatal("o destrave se perdeu; o personagem travaria de novo ao subir")
	}
	// E ele continua ganhando EXP em qualquer nivel.
	if archExperienceLocked(p.Char) {
		t.Error("travou de novo depois do cristal")
	}
}

// --- destrave na Lindy -------------------------------------------------------

func TestLindyDestraveSoNasTravas(t *testing.T) {
	casos := []struct {
		nome     string
		nivel    uint32
		arch     bool
		d355     bool
		quer     bool
		querQual uint32
	}{
		{"Arch parado em 354", archLockLevel355, true, false, true, archLockLevel355},
		{"Arch parado em 369", archLockLevel370, true, true, true, archLockLevel370},
		{"Arch fora da trava", 360, true, true, false, 0},
		{"trava ja liberada", archLockLevel355, true, true, false, 0},
		{"Mortal na mesma receita", archLockLevel355, false, false, false, 0},
	}
	for _, c := range casos {
		ch := archNoNivel(c.nivel, c.d355, false)
		if !c.arch {
			ch.Evolution = ""
		}
		qual, ok := lindyLevelUnlock(ch)
		if ok != c.quer || (ok && qual != c.querQual) {
			t.Errorf("%s: destrava=%v trava=%d; queria %v/%d",
				c.nome, ok, qual, c.quer, c.querQual)
		}
	}
}

// A cobrança de um ponto de Fame ocorre somente no 370, dentro do handler da
// Lindy. O destrave 355 não possui custo de Fame.
func TestLindyDestraveDe370ConsomeFame(t *testing.T) {
	ch := archNoNivel(archLockLevel370, true, false)
	trava, ok := lindyLevelUnlock(ch)
	if !ok || trava != archLockLevel370 {
		t.Fatal("nao reconheceu a trava de 370")
	}
	// O consumo em si acontece no handler.
}
