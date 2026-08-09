package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
)

// odinSecretStoneSequenceRNG permite controlar separadamente o roll de sucesso
// e o sorteio do elemento no fallback de sete runas. Os valores sao reduzidos
// pelo modulo do limite recebido, preservando o contrato RNG.Intn.
type odinSecretStoneSequenceRNG struct {
	values []int
	next   int
}

func (r *odinSecretStoneSequenceRNG) Intn(n int) int {
	if n <= 0 {
		return 0
	}
	if r == nil || r.next >= len(r.values) {
		return 0
	}
	value := r.values[r.next]
	r.next++
	if value < 0 {
		value = -value
	}
	return value % n
}

// odinSecretStoneDeferredAttempt prepara a tentativa sem executa-la. Isso
// permite injetar uma falha de persistencia depois do snapshot inicial e antes
// de o handler mutar o estado autoritativo.
func odinSecretStoneDeferredAttempt(t *testing.T, runes [7]uint16, gold uint32, rng RNG) (*Player, *craftStore, func()) {
	t.Helper()
	defs := map[uint16]model.ItemDef{
		5334: {Index: 5334}, 5335: {Index: 5335},
		5336: {Index: 5336}, 5337: {Index: 5337},
		3020: {Index: 3020},
	}
	w, p, session, st := newCraftWorld(t, "Odin", defs, gold)
	w.rng = rng
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	for i, index := range runes {
		items[i] = model.Item{Index: index}
		pos[i] = int8(i)
	}
	placeItems(p.Char, items, pos)
	pkt := buildCombinePacket(items, pos)
	return p, st, func() { w.onCombineOdin(session, pkt) }
}

func TestOdinRandomSevenRunesCanYieldEverySecretStone(t *testing.T) {
	// Validas para Odin, mas nao correspondem a nenhuma das quatro sequencias
	// exatas nem a receita da Fury.
	runes := [7]uint16{5110, 5111, 5112, 5113, 5114, 5115, 5116}
	for offset := 0; offset < 4; offset++ {
		rng := &odinSecretStoneSequenceRNG{values: []int{0, offset}}
		_, p, st := odinSecretStoneAttempt(t, runes, odinSecretStoneGoldCost, rng)
		want := uint16(5334 + offset)
		if st.saves != 1 {
			t.Fatalf("offset %d: saves=%d, want 1", offset, st.saves)
		}
		if got := p.Char.Inv[0].Index; got != want {
			t.Fatalf("offset %d: result=%d, want %d", offset, got, want)
		}
	}
}

func TestOdinExactSecretStonePersistenceFailureRestoresRunesAndGold(t *testing.T) {
	recipe := secretStoneRecipes[0]
	var runes [7]uint16
	copy(runes[:], recipe[:7])
	p, st, attempt := odinSecretStoneDeferredAttempt(t, runes, odinSecretStoneGoldCost+123, fixedRNG{value: 94})
	beforeInv, beforeGold := p.Char.Inv, p.Char.Gold
	st.err = errors.New("postgres unavailable")

	attempt()

	if st.saves != 1 {
		t.Fatalf("saves=%d, want failed persist attempt", st.saves)
	}
	if p.Char.Gold != beforeGold || p.Char.Inv != beforeInv {
		t.Fatalf("exact Secret Stone persist failure did not restore inventory/gold")
	}
}

func TestOdinRandomSecretStonePersistenceFailureRestoresRunesAndGold(t *testing.T) {
	runes := [7]uint16{5110, 5111, 5112, 5113, 5114, 5115, 5116}
	rng := &odinSecretStoneSequenceRNG{values: []int{0, 3}}
	p, st, attempt := odinSecretStoneDeferredAttempt(t, runes, odinSecretStoneGoldCost+123, rng)
	beforeInv, beforeGold := p.Char.Inv, p.Char.Gold
	st.err = errors.New("postgres unavailable")

	attempt()

	if st.saves != 1 {
		t.Fatalf("saves=%d, want failed persist attempt", st.saves)
	}
	if p.Char.Gold != beforeGold || p.Char.Inv != beforeInv {
		t.Fatalf("random Secret Stone persist failure did not restore inventory/gold")
	}
}
