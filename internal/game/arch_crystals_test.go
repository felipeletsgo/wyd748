package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

// Os quatro cristais de Kefra: feitos como Arch, EM ORDEM, cada um cobrando
// 100 milhoes de EXP. O nivel e recalculado a partir da EXP reduzida, entao o
// personagem DESCE de nivel -- e assim no nativo.

func archComExp(exp uint32, cristais byte) (*World, *Player, *net.Session) {
	s := net.NewTestSession(1, 64)
	ch := model.Char{
		Name: "God", Evolution: "arch", Exp: exp, ArchCrystals: cristais,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion,
			MaxHP: 1000, CurHP: 1000},
	}
	ch.Extended.Level = levelForExp(exp)
	acc := &model.Account{Name: "c", Chars: []model.Char{ch}}
	p := &Player{ID: 1, Session: s, Account: acc, Char: &acc.Chars[0], InWorld: true}
	w := &World{store: &craftStore{}, players: map[*net.Session]*Player{s: p},
		items: map[uint16]model.ItemDef{}}
	for i := uint16(0); i < archCrystalCount; i++ {
		w.items[firstArchCrystal+i] = model.ItemDef{Index: firstArchCrystal + i}
	}
	return w, p, s
}

func usarCristal(w *World, p *Player, s *net.Session, indice uint16) {
	p.Char.Inv[0] = model.Item{Index: indice}
	w.useArchCrystal(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 187)
}

func TestCristalCobraCemMilhoesERebaixaNivel(t *testing.T) {
	const exp = uint32(2_600_000_000)
	w, p, s := archComExp(exp, 0)
	nivelAntes := p.Char.Extended.Level

	usarCristal(w, p, s, firstArchCrystal)

	if p.Char.ArchCrystals != 1 {
		t.Fatalf("cristais=%d, quer 1", p.Char.ArchCrystals)
	}
	if p.Char.Exp != exp-archCrystalExpCost {
		t.Errorf("exp=%d, quer %d", p.Char.Exp, exp-archCrystalExpCost)
	}
	// O nivel acompanha a EXP reduzida: nao pode ficar parado.
	if p.Char.Extended.Level >= nivelAntes {
		t.Errorf("nivel %d -> %d; deveria cair com a EXP", nivelAntes, p.Char.Extended.Level)
	}
	if p.Char.Extended.Level != levelForExp(p.Char.Exp) {
		t.Errorf("nivel %d nao corresponde a EXP %d", p.Char.Extended.Level, p.Char.Exp)
	}
	if p.Char.Inv[0].Index != 0 {
		t.Error("o cristal nao foi consumido")
	}
}

// TestCristalExigeOrdem: o cristal N so vale depois dos N-1 anteriores.
func TestCristalExigeOrdem(t *testing.T) {
	w, p, s := archComExp(2_600_000_000, 0)

	// Pular direto para o terceiro nao pode funcionar.
	usarCristal(w, p, s, firstArchCrystal+2)
	if p.Char.ArchCrystals != 0 {
		t.Errorf("aceitou fora de ordem: cristais=%d", p.Char.ArchCrystals)
	}
	if p.Char.Inv[0].Index != firstArchCrystal+2 {
		t.Error("consumiu o cristal recusado")
	}

	// Na ordem, avanca um a um.
	for passo := byte(1); passo <= archCrystalCount; passo++ {
		usarCristal(w, p, s, firstArchCrystal+uint16(passo)-1)
		if p.Char.ArchCrystals != passo {
			t.Fatalf("passo %d: cristais=%d", passo, p.Char.ArchCrystals)
		}
	}
}

func TestCristalNaoRepete(t *testing.T) {
	w, p, s := archComExp(2_600_000_000, 2)
	expAntes := p.Char.Exp

	usarCristal(w, p, s, firstArchCrystal+1) // o segundo, ja feito
	if p.Char.ArchCrystals != 2 || p.Char.Exp != expAntes {
		t.Errorf("refez um cristal ja concluido: cristais=%d exp=%d",
			p.Char.ArchCrystals, p.Char.Exp)
	}
}

// TestCristalExigeMortalNao: so Arch. Mortal usando o cristal nao pode perder
// EXP nem o item.
func TestCristalSoParaArch(t *testing.T) {
	w, p, s := archComExp(2_600_000_000, 0)
	p.Char.Evolution = "" // vira Mortal
	expAntes := p.Char.Exp

	usarCristal(w, p, s, firstArchCrystal)

	if p.Char.ArchCrystals != 0 || p.Char.Exp != expAntes {
		t.Error("Mortal conseguiu usar o cristal")
	}
	if p.Char.Inv[0].Index != firstArchCrystal {
		t.Error("consumiu o cristal de um Mortal")
	}
}

// TestCristalNaoDaVoltaNoUnderflow: Exp e uint32. Num estado inconsistente
// (nivel alto com EXP baixa -- edicao manual, tabela alterada), descontar sem
// checar faria `Exp -= 100M` dar a volta para ~4 bilhoes, PRESENTEANDO o
// jogador em vez de cobra-lo.
func TestCristalNaoDaVoltaNoUnderflow(t *testing.T) {
	w, p, s := archComExp(2_600_000_000, 0)
	// Estado torto de proposito: passa no piso de nivel, mas sem EXP.
	p.Char.Exp = archCrystalExpCost - 1

	usarCristal(w, p, s, firstArchCrystal)

	if p.Char.Exp >= archCrystalExpCost {
		t.Fatalf("EXP deu a volta: %d", p.Char.Exp)
	}
	if p.Char.ArchCrystals != 0 {
		t.Error("marcou o cristal a partir de estado inconsistente")
	}
}

// TestCristalExigeNivelMinimo: o piso e por cristal. Como cada um custa ~2,5
// niveis nessa faixa, quem esta no limite faz UM e precisa recuperar nivel.
func TestCristalExigeNivelMinimo(t *testing.T) {
	w, p, s := archComExp(2_600_000_000, 0)
	p.Char.Extended.Level = archCrystalMinLevel - 1
	expAntes := p.Char.Exp

	usarCristal(w, p, s, firstArchCrystal)

	if p.Char.ArchCrystals != 0 || p.Char.Exp != expAntes {
		t.Errorf("aceitou abaixo do nivel %d: cristais=%d exp=%d",
			archCrystalMinLevel, p.Char.ArchCrystals, p.Char.Exp)
	}
	if p.Char.Inv[0].Index != firstArchCrystal {
		t.Error("consumiu o cristal de quem nao alcancou o nivel")
	}
}

// TestCristalNaoCobraSeOSaveFalhar: 100 milhoes de EXP e caro demais para sair
// sem estar em disco.
func TestCristalNaoCobraSeOSaveFalhar(t *testing.T) {
	const exp = uint32(2_600_000_000)
	w, p, s := archComExp(exp, 0)
	nivelAntes := p.Char.Extended.Level
	w.store = &craftStore{err: errors.New("disco cheio")}

	usarCristal(w, p, s, firstArchCrystal)

	if p.Char.Exp != exp {
		t.Errorf("cobrou %d de EXP apesar da falha de disco", exp-p.Char.Exp)
	}
	if p.Char.ArchCrystals != 0 {
		t.Error("marcou o cristal apesar da falha")
	}
	if p.Char.Extended.Level != nivelAntes {
		t.Errorf("nivel mudou apesar da falha: %d -> %d", nivelAntes, p.Char.Extended.Level)
	}
	if p.Char.Inv[0].Index != firstArchCrystal {
		t.Error("o cristal sumiu apesar da falha")
	}
}
