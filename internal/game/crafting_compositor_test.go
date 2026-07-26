package game

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

// A chance do Compositor e base + bonus de cada um dos QUATRO materiais, com o
// bonus escolhido pelo refino. Com os padroes (10 / 3 / 5 / 8) o teto e 42.
//
// Estes testes batem em compositorChance direto: e a funcao que decide, e assim
// nao dependem de montar o pacote de 84 bytes nem do estado do mundo.

// compositorTestWorld devolve um mundo com o catalogo minimo: para cada
// combinacao de nivel (4=set D, 5=set E) existe um item equipavel.
func compositorTestWorld() *World {
	w := &World{
		items:    map[uint16]model.ItemDef{},
		gameplay: model.DefaultGameplayConfig(),
	}
	for level := 4; level <= 5; level++ {
		w.items[compositorItemIndex(level)] = model.ItemDef{
			Index: compositorItemIndex(level),
			Pos:   2, // equipavel: o nativo exige EF_POS != 0
			Grade: 2,
			StaticEffects: []model.StaticEffect{
				{Name: "EF_ITEMLEVEL", Value: level},
			},
		}
	}
	return w
}

func compositorItemIndex(level int) uint16 { return uint16(9000 + level) }

// compositorMaterial monta um material do set indicado com o refino pedido.
// itemSanc le o par (43, valor) e cai no `valor % 10` para +0..+9.
func compositorMaterial(level, sanc int) model.Item {
	return model.Item{
		Index: compositorItemIndex(level),
		Eff:   [6]byte{43, byte(sanc)},
	}
}

// compositorRequest coloca os materiais a partir do slot 2; os slots 0 e 1 sao
// a arma-base e a joia, que compositorChance nao le.
func compositorRequest(materials ...model.Item) combineRequest {
	var req combineRequest
	for i, item := range materials {
		req.Items[2+i] = item
	}
	return req
}

func TestCompositorNativeChance(t *testing.T) {
	w := compositorTestWorld()
	tests := []struct {
		name  string
		sancs []int
		want  int
	}{
		{"quatro +7", []int{7, 7, 7, 7}, 22},        // 10 + 3*4
		{"quatro +8", []int{8, 8, 8, 8}, 30},        // 10 + 5*4
		{"quatro +9 (teto)", []int{9, 9, 9, 9}, 42}, // 10 + 8*4
		{"misto +7+8+9+9", []int{7, 8, 9, 9}, 34},   // 10 + 3 + 5 + 8 + 8
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			materials := make([]model.Item, 0, len(tt.sancs))
			for _, sanc := range tt.sancs {
				materials = append(materials, compositorMaterial(5, sanc))
			}
			got, _, ok := w.compositorChance(compositorRequest(materials...))
			if !ok {
				t.Fatal("composicao valida recusada")
			}
			if got != tt.want {
				t.Errorf("chance=%d, quer %d", got, tt.want)
			}
		})
	}
}

// TestCompositorChanceNeverExceedsCeiling: o teto de 42 e a regra do servidor.
// Nenhuma combinacao de quatro materiais validos pode passar disso.
func TestCompositorChanceNeverExceedsCeiling(t *testing.T) {
	w := compositorTestWorld()
	const ceiling = 42
	for _, a := range []int{7, 8, 9} {
		for _, b := range []int{7, 8, 9} {
			for _, c := range []int{7, 8, 9} {
				for _, d := range []int{7, 8, 9} {
					got, _, ok := w.compositorChance(compositorRequest(
						compositorMaterial(4, a), compositorMaterial(5, b),
						compositorMaterial(4, c), compositorMaterial(5, d)))
					if !ok {
						t.Fatalf("+%d+%d+%d+%d recusado", a, b, c, d)
					}
					if got > ceiling {
						t.Fatalf("+%d+%d+%d+%d deu %d, acima do teto %d", a, b, c, d, got, ceiling)
					}
				}
			}
		}
	}
}

// TestCompositorRequiresExactlyFourMaterials: nao se compoe com 3, 2, 1 ou
// nenhum material -- nem com 5 ou 6, que os seis slots permitiriam.
func TestCompositorRequiresExactlyFourMaterials(t *testing.T) {
	w := compositorTestWorld()
	for count := 0; count <= 6; count++ {
		materials := make([]model.Item, 0, count)
		for i := 0; i < count; i++ {
			materials = append(materials, compositorMaterial(5, 9))
		}
		_, _, ok := w.compositorChance(compositorRequest(materials...))
		if want := count == compositorMaterials; ok != want {
			t.Errorf("%d materiais: aceito=%t, quer %t", count, ok, want)
		}
	}
}

// TestCompositorIgnoresItemGrade: o Grade do item nao entra na chance. Importa
// porque 114 dos 366 materiais de nivel 4/5 do itemlist.csv tem Grade 0 -- a
// formula antiga os rejeitava e derrubava a composicao inteira.
func TestCompositorIgnoresItemGrade(t *testing.T) {
	w := compositorTestWorld()
	baseline, _, ok := w.compositorChance(compositorRequest(
		compositorMaterial(4, 8), compositorMaterial(4, 8),
		compositorMaterial(4, 8), compositorMaterial(4, 8)))
	if !ok {
		t.Fatal("composicao de referencia recusada")
	}

	for _, grade := range []int{0, 1, 5, 12} {
		def := w.items[compositorItemIndex(4)]
		def.Grade = grade
		w.items[compositorItemIndex(4)] = def

		got, _, ok := w.compositorChance(compositorRequest(
			compositorMaterial(4, 8), compositorMaterial(4, 8),
			compositorMaterial(4, 8), compositorMaterial(4, 8)))
		if !ok {
			t.Errorf("Grade %d recusado; o grade nao deveria influir", grade)
			continue
		}
		if got != baseline {
			t.Errorf("Grade %d mudou a chance para %d, quer %d", grade, got, baseline)
		}
	}
}

// TestCompositorRejectsInvalidMaterial: fora do set D/E, fora de +7..+9, ou nao
// equipavel, a composicao inteira e recusada.
func TestCompositorRejectsInvalidMaterial(t *testing.T) {
	valid := func() []model.Item {
		return []model.Item{
			compositorMaterial(5, 9), compositorMaterial(5, 9),
			compositorMaterial(5, 9), compositorMaterial(5, 9),
		}
	}

	t.Run("refino abaixo de +7", func(t *testing.T) {
		w := compositorTestWorld()
		materials := valid()
		materials[2] = compositorMaterial(5, 6)
		if _, _, ok := w.compositorChance(compositorRequest(materials...)); ok {
			t.Error("material +6 aceito")
		}
	})

	t.Run("refino acima de +9", func(t *testing.T) {
		w := compositorTestWorld()
		materials := valid()
		// valor 230 e refino +10 em itemSanc.
		materials[0] = model.Item{Index: compositorItemIndex(5), Eff: [6]byte{43, 230}}
		if _, _, ok := w.compositorChance(compositorRequest(materials...)); ok {
			t.Error("material +10 aceito")
		}
	})

	t.Run("fora do set D/E", func(t *testing.T) {
		w := compositorTestWorld()
		w.items[7777] = model.ItemDef{Index: 7777, Pos: 2,
			StaticEffects: []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: 3}}}
		materials := valid()
		materials[1] = model.Item{Index: 7777, Eff: [6]byte{43, 9}}
		if _, _, ok := w.compositorChance(compositorRequest(materials...)); ok {
			t.Error("material de set C aceito")
		}
	})

	t.Run("nao equipavel", func(t *testing.T) {
		w := compositorTestWorld()
		def := w.items[compositorItemIndex(5)]
		def.Pos = 0
		w.items[compositorItemIndex(5)] = def
		if _, _, ok := w.compositorChance(compositorRequest(valid()...)); ok {
			t.Error("material sem EF_POS aceito")
		}
	})

	t.Run("fora do catalogo", func(t *testing.T) {
		w := compositorTestWorld()
		materials := valid()
		materials[3] = model.Item{Index: 31337, Eff: [6]byte{43, 9}}
		if _, _, ok := w.compositorChance(compositorRequest(materials...)); ok {
			t.Error("material inexistente aceito")
		}
	})
}

// TestCompositorUsesServerConfig: a config precisa chegar ao calculo. E
// exatamente o que falha na W2PP -- la o CompRate.txt e lido para um array que
// nenhuma funcao consulta, entao ajustar o arquivo nao muda nada no jogo.
func TestCompositorUsesServerConfig(t *testing.T) {
	w := compositorTestWorld()
	w.gameplay.CompositorBaseChance = 20
	w.gameplay.CompositorRefineChance = [model.CompositorRefineLevels]uint32{1, 2, 3}

	got, _, ok := w.compositorChance(compositorRequest(
		compositorMaterial(5, 8), compositorMaterial(5, 8),
		compositorMaterial(5, 8), compositorMaterial(5, 9)))
	if !ok {
		t.Fatal("composicao valida recusada")
	}
	if want := 20 + 2 + 2 + 2 + 3; got != want {
		t.Errorf("chance=%d, quer %d -- a config nao chegou ao calculo", got, want)
	}
}

// TestCompositorBreakdownExplainsTheChance: o log precisa mostrar a conta item a
// item. Sem isso, um "chance=24" no log nao da para diagnosticar.
func TestCompositorBreakdownExplainsTheChance(t *testing.T) {
	w := compositorTestWorld()
	_, breakdown, ok := w.compositorChance(compositorRequest(
		compositorMaterial(4, 7), compositorMaterial(5, 9),
		compositorMaterial(5, 9), compositorMaterial(5, 9)))
	if !ok {
		t.Fatal("composicao valida recusada")
	}
	if n := len(strings.Fields(breakdown)); n != compositorMaterials {
		t.Fatalf("quebra com %d entradas, quer %d: %q", n, compositorMaterials, breakdown)
	}
	if !strings.Contains(breakdown, "setD+7:3") {
		t.Errorf("a quebra nao identifica o set e o bonus do material: %q", breakdown)
	}
	if !strings.Contains(breakdown, "setE+9:8") {
		t.Errorf("a quebra nao identifica o material de set E: %q", breakdown)
	}
}

// compositorScenario monta a composicao completa: arma-base (Unique 41..49 com
// Extra apontando para o resultado), joia 2441 e os materiais pedidos.
func compositorScenario(materials ...model.Item) ([combineSlots]model.Item, [combineSlots]int8, map[uint16]model.ItemDef) {
	defs := map[uint16]model.ItemDef{
		500: {Index: 500, Unique: 45, Extra: 600, Pos: 64,
			StaticEffects: []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: 4}}},
		600:  {Index: 600, Unique: 45, Pos: 64},
		2441: {Index: 2441},
	}
	for level := 4; level <= 5; level++ {
		defs[compositorItemIndex(level)] = model.ItemDef{
			Index: compositorItemIndex(level), Pos: 2, Grade: 2,
			StaticEffects: []model.StaticEffect{{Name: "EF_ITEMLEVEL", Value: level}},
		}
	}

	var items [combineSlots]model.Item
	var pos [combineSlots]int8
	// A arma-base precisa de slot de refino: setItemSanc grava o resultado nele.
	items[0] = model.Item{Index: 500, Eff: [6]byte{43, 0}}
	items[1] = model.Item{Index: 2441}
	for i, material := range materials {
		items[2+i] = material
	}
	for i := range pos {
		pos[i] = int8(i)
	}
	return items, pos, defs
}

// TestCompositorHandlerRunsWithFourMaterials cobre o handler inteiro, nao so a
// formula: com os quatro materiais a composicao roda e consome tudo (o sorteio
// decide apenas se a arma nova aparece).
func TestCompositorHandlerRunsWithFourMaterials(t *testing.T) {
	material := compositorMaterial(5, 9)
	items, pos, defs := compositorScenario(material, material, material, material)

	w, p, session, st := newCraftWorld(t, "Compositor", defs, 0)
	w.gameplay = model.DefaultGameplayConfig()
	placeItems(p.Char, items, pos)

	w.onCombineCompositor(session, buildCombinePacket(items, pos))

	if st.saves != 1 {
		t.Fatalf("deveria persistir uma vez, saves=%d", st.saves)
	}
	for slot := 2; slot < 6; slot++ {
		if p.Char.Inv[slot].Index != 0 {
			t.Errorf("material do slot %d nao foi consumido", slot)
		}
	}
	if p.Char.Inv[1].Index != 0 {
		t.Error("a joia nao foi consumida")
	}
}

// TestCompositorHandlerRefusesThreeMaterials: recusa nao pode consumir nada.
func TestCompositorHandlerRefusesThreeMaterials(t *testing.T) {
	material := compositorMaterial(5, 9)
	items, pos, defs := compositorScenario(material, material, material)

	w, p, session, st := newCraftWorld(t, "Compositor", defs, 0)
	w.gameplay = model.DefaultGameplayConfig()
	placeItems(p.Char, items, pos)
	before := p.Char.Inv

	w.onCombineCompositor(session, buildCombinePacket(items, pos))

	if st.saves != 0 {
		t.Errorf("composicao recusada nao deveria persistir, saves=%d", st.saves)
	}
	if p.Char.Inv != before {
		t.Error("composicao recusada consumiu itens do inventario")
	}
}
