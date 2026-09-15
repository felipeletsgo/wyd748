package game

import (
	"encoding/binary"
	"errors"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
)

func buildExtracaoPacket(slot int) []byte {
	pkt := make([]byte, 20)
	binary.LittleEndian.PutUint32(pkt[16:20], uint32(slot))
	return pkt
}

func extracaoFixture(t *testing.T) (*World, *Player, *craftStore) {
	t.Helper()
	defs := map[uint16]model.ItemDef{
		500: {
			Index: 500,
			Pos:   4,
			StaticEffects: []model.StaticEffect{
				{Name: "EF_ITEMLEVEL", Value: 2},
				{Name: "EF_MOBTYPE", Value: 0},
				{Name: "EF_DAMAGE", Value: 10},
			},
			DynamicEffectNames: map[byte]string{2: "EF_DAMAGE"},
		},
	}
	w, p, _, st := newCraftWorld(t, "unused", defs, 0)
	p.Char.Class = 3
	p.Char.LearnedSkill = 1 << (83 - 72)
	p.Char.Score = testScore(model.Score{MaxHP: 100, CurHP: 100, Mastery: [4]uint32{0, 0, 500, 0}})
	p.Char.Inv[0] = model.Item{
		Index: 500,
		UID:   "11111111111141118111111111111111",
		Eff:   [6]byte{0, 0, 2, 5, 2, 6},
	}
	p.Char.Inv[1] = model.Item{Index: extracaoCatalyst, Eff: [6]byte{effectAmount, 5}}
	w.rng = fixedRNG{value: 0}
	return w, p, st
}

func TestExtracaoUsesStrictRollBoundary(t *testing.T) {
	w := &World{rng: fixedRNG{value: 38}} // dado visível 39
	if got := w.rollPercentStrict(40); !got.Success || got.Roll != 39 {
		t.Fatalf("39 < 40 deveria passar: %+v", got)
	}
	w.rng = fixedRNG{value: 39} // dado visível 40
	if got := w.rollPercentStrict(40); got.Success || got.Roll != 40 {
		t.Fatalf("40 < 40 deveria falhar: %+v", got)
	}
}

func TestSkillCraftRejectsUnauthorizedState(t *testing.T) {
	for _, skillIndex := range []int{83, 84} {
		for _, tc := range []struct {
			name   string
			mutate func(*Player)
		}{
			{"unlearned", func(p *Player) { p.Char.LearnedSkill = 0 }},
			{"wrong-class", func(p *Player) { p.Char.Class = 0 }},
			{"dead", func(p *Player) { setPlayerCurHP(p.Char, 0) }},
			{"offline", func(p *Player) { p.InWorld = false }},
			{"poisoned", func(p *Player) { p.PersistencePoisoned = true }},
			{"shop", func(p *Player) { p.GhostShop = &GhostShop{} }},
		} {
			t.Run(fmt.Sprintf("%d/%s", skillIndex, tc.name), func(t *testing.T) {
				w, p, st := extracaoFixture(t)
				pkt := buildExtracaoPacket(0)
				if skillIndex == 84 {
					var items [combineSlots]model.Item
					var pos [combineSlots]int8
					w, p, st, items, pos = alquimiaScenario(t)
					pkt = buildCombinePacket(items, pos)
				}
				tc.mutate(p)
				before := p.Char.Inv
				if skillIndex == 83 {
					w.onCombineExtracao(p.Session, pkt)
				} else {
					w.onCombineAlquimia(p.Session, pkt)
				}
				if st.saves != 0 || p.Char.Inv != before {
					t.Fatalf("estado recusado alterou inventario: saves=%d", st.saves)
				}
			})
		}
	}
}

func TestSkillCraftUsesRuntimeMasteryAndRejectsReplay(t *testing.T) {
	for _, skillIndex := range []int{83, 84} {
		w, p, st := extracaoFixture(t)
		pkt := buildExtracaoPacket(0)
		if skillIndex == 84 {
			var items [combineSlots]model.Item
			var pos [combineSlots]int8
			w, p, st, items, pos = alquimiaScenario(t)
			pkt = buildCombinePacket(items, pos)
		}
		p.Char.RuntimeScore = testScore(*p.Char.Score)
		p.Char.Score.Mastery[2] = 20 // base sozinho tem chance zero
		call := func() {
			if skillIndex == 83 {
				w.onCombineExtracao(p.Session, pkt)
			} else {
				w.onCombineAlquimia(p.Session, pkt)
			}
		}
		call()
		if st.saves != 1 || p.Char.Inv[0].Index == 0 {
			t.Fatalf("skill %d ignorou mastery runtime", skillIndex)
		}
		before := p.Char.Inv
		call() // replay imediato
		p.LastCraft = time.Time{}
		call() // mesmo pacote depois da janela; ingredientes ja consumidos
		if st.saves != 1 || p.Char.Inv != before {
			t.Fatalf("skill %d aceitou replay", skillIndex)
		}
	}
}

func TestExtracaoRejectsReservedCarrySlot(t *testing.T) {
	w, p, st := extracaoFixture(t)
	p.Char.Inv[model.PlayerCarrySlots] = p.Char.Inv[0]
	before := p.Char.Inv
	w.onCombineExtracao(p.Session, buildExtracaoPacket(model.PlayerCarrySlots))
	if st.saves != 0 || p.Char.Inv != before {
		t.Fatal("slot reservado foi consumido")
	}
}

func TestCombineExtracaoTransformsTargetAndClearsCatalystSlot(t *testing.T) {
	w, p, st := extracaoFixture(t)
	originalUID := p.Char.Inv[0].UID

	w.onCombineExtracao(p.Session, buildExtracaoPacket(0))

	got := p.Char.Inv[0]
	if st.saves != 1 || got.Index != 3022 || got.UID != originalUID {
		t.Fatalf("Extração incompleta: saves=%d item=%+v", st.saves, got)
	}
	if got.Eff != [6]byte{effectItemLevel, 2, 2, 15, 2, 16} {
		t.Fatalf("efeitos da Extração divergentes: %+v", got.Eff)
	}
	if p.Char.Inv[1].Index != 0 {
		t.Fatalf("Extração não limpou o slot 1774 inteiro: %+v", p.Char.Inv[1])
	}
}

func TestCombineExtracaoRollsBackOnSaveFailure(t *testing.T) {
	w, p, st := extracaoFixture(t)
	before := p.Char.Inv
	st.err = errors.New("postgres indisponivel")

	w.onCombineExtracao(p.Session, buildExtracaoPacket(0))

	if st.saves != 1 || p.Char.Inv != before {
		t.Fatalf("rollback Extração incompleto: saves=%d invMudou=%t", st.saves, p.Char.Inv != before)
	}
}

func TestCombineExtracaoRejectsMissingScore(t *testing.T) {
	w, p, st := extracaoFixture(t)
	p.Char.Score = nil
	before := p.Char.Inv

	w.onCombineExtracao(p.Session, buildExtracaoPacket(0))

	if st.saves != 0 || p.Char.Inv != before {
		t.Fatalf("Extração sem score alterou estado: saves=%d invMudou=%t", st.saves, p.Char.Inv != before)
	}
}

func alquimiaScenario(t *testing.T) (*World, *Player, *craftStore, [combineSlots]model.Item, [combineSlots]int8) {
	t.Helper()
	w, p, _, st := newCraftWorld(t, "unused", nil, 0)
	p.Char.Class = 3
	p.Char.LearnedSkill = 1 << (84 - 72)
	p.Char.Score = testScore(model.Score{MaxHP: 100, CurHP: 100, Mastery: [4]uint32{0, 0, 500, 0}})
	p.Char.SecondaryLearnedSkill = 0x10
	w.rng = fixedRNG{value: 0}
	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	items[0] = model.Item{Index: 413, UID: "11111111111141118111111111111111"}
	items[1] = model.Item{Index: 2441, UID: "22222222222242228222222222222222"}
	items[2] = model.Item{Index: 2442, UID: "33333333333343338333333333333333"}
	for i := range pos {
		pos[i] = int8(i)
	}
	placeItems(p.Char, items, pos)
	return w, p, st, items, pos
}

func TestCombineAlquimiaAllowsSagacidadeAndPreservesFirstUID(t *testing.T) {
	w, p, st, items, pos := alquimiaScenario(t)
	// O W2PP casa a receita mesmo com material extra e depois consome todos
	// os slots não vazios. Preservamos esse comportamento.
	items[3] = model.Item{Index: 999, UID: "44444444444444448444444444444444"}
	placeItems(p.Char, items, pos)

	w.onCombineAlquimia(p.Session, buildCombinePacket(items, pos))

	got := p.Char.Inv[0]
	if st.saves != 1 || got.Index != 3200 || got.UID != items[0].UID {
		t.Fatalf("Sagacidade não foi criada corretamente: saves=%d item=%+v", st.saves, got)
	}
	if got.Eff != [6]byte{effectAmount, 5} {
		t.Fatalf("Ponto Mestre da Alquimia divergente: %+v", got.Eff)
	}
	for _, slot := range []int{1, 2, 3} {
		if p.Char.Inv[slot].Index != 0 {
			t.Fatalf("Alquimia não consumiu ingrediente extra no slot %d: %+v", slot, p.Char.Inv[slot])
		}
	}
}

func TestCombineAlquimiaAllW2PPRecipes(t *testing.T) {
	// GetFunc.cpp::GetMatchCombineAlquimia, resultados 3200..3209.
	recipes := [][4]uint16{
		{413, 2441, 2442}, {413, 2443, 2442}, {4127, 4127, 4127},
		{4127, 4127, 697}, {412, 2441, 2444}, {412, 2444, 2443},
		{612, 2441, 2442}, {612, 613, 614, 615}, {614, 2443, 2444},
		{615, 697, 697, 697},
	}
	for recipe, ingredients := range recipes {
		t.Run(fmt.Sprint(recipe), func(t *testing.T) {
			w, p, st, items, pos := alquimiaScenario(t)
			p.Char.SecondaryLearnedSkill = 0
			items = [combineSlots]model.Item{}
			for i, index := range ingredients {
				if index != 0 {
					items[i] = model.Item{Index: index}
				}
			}
			if recipe == 6 || recipe == 8 || recipe == 9 {
				items[0].Eff = [6]byte{43, 8}
				if _, valid := alquimiaRecipe(combineRequest{Items: items}); valid {
					t.Fatal("receita que exige +9 aceitou +8")
				}
				items[0].Eff[1] = 9
			}
			p.Char.Inv = [model.MaxCarry]model.Item{}
			placeItems(p.Char, items, pos)
			w.onCombineAlquimia(p.Session, buildCombinePacket(items, pos))
			if st.saves != 1 || p.Char.Inv[0].Index != uint16(3200+recipe) ||
				p.Char.Inv[0].Eff != [6]byte{} {
				t.Fatalf("receita %d: saves=%d resultado=%+v", recipe, st.saves, p.Char.Inv[0])
			}
			for i := 1; i < len(ingredients); i++ {
				if p.Char.Inv[i].Index != 0 {
					t.Fatalf("ingrediente %d nao consumido", i)
				}
			}
		})
	}
}

func TestCombineAlquimiaRejectsPackedIngredient(t *testing.T) {
	w, p, st, items, pos := alquimiaScenario(t)
	items[1].Eff = [6]byte{effectAmount, 2}
	p.Char.Inv = [model.MaxCarry]model.Item{}
	placeItems(p.Char, items, pos)
	before := p.Char.Inv

	w.onCombineAlquimia(p.Session, buildCombinePacket(items, pos))

	if st.saves != 0 || p.Char.Inv != before {
		t.Fatalf("Alquimia aceitou item em pack: saves=%d invMudou=%t", st.saves, p.Char.Inv != before)
	}
}

func TestCombineAlquimiaConsumesIngredientsOnFailedRoll(t *testing.T) {
	w, p, st, items, pos := alquimiaScenario(t)
	p.Char.Score.Mastery[2] = 220 // chance 40
	w.rng = fixedRNG{value: 40}   // dado visível 41

	w.onCombineAlquimia(p.Session, buildCombinePacket(items, pos))

	if st.saves != 1 {
		t.Fatalf("falha da Alquimia não persistiu: saves=%d", st.saves)
	}
	for slot := 0; slot < 3; slot++ {
		if p.Char.Inv[slot].Index != 0 {
			t.Fatalf("falha da Alquimia preservou ingrediente no slot %d: %+v", slot, p.Char.Inv[slot])
		}
	}
}

func TestCombineAlquimiaRollsBackOnSaveFailure(t *testing.T) {
	w, p, st, items, pos := alquimiaScenario(t)
	before := p.Char.Inv
	st.err = errors.New("postgres indisponivel")

	w.onCombineAlquimia(p.Session, buildCombinePacket(items, pos))

	if st.saves != 1 || p.Char.Inv != before {
		t.Fatalf("rollback Alquimia incompleto: saves=%d invMudou=%t", st.saves, p.Char.Inv != before)
	}
}
