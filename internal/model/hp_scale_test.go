package model

import "testing"

// O prefixo nativo do STRUCT_SCORE guarda HP em 16 bits, entao MaxHP acima de
// 30000 e ESCALADO. O dano dos pacotes de ataque viaja num WORD que o client
// subtrai desse CurHP ja escalado -- por isso ele precisa da MESMA escala.
// Mandar o dano cru fazia a barra cair `scale` vezes mais rapido e saltar de
// volta quando chegava o 0x181.

func TestEscalaDeHP(t *testing.T) {
	casos := []struct {
		maxHP uint32
		quer  uint32
	}{
		{0, 1},       // sem alvo conhecido: sem projecao
		{10_000, 1},  // cabe direto
		{30_000, 1},  // limite exato
		{30_001, 2},  // primeiro que precisa escalar
		{200_000, 7}, // ceil(200000/30000)
		{1_000_000, 34},
	}
	for _, c := range casos {
		if got := CompatibilityHPScale(c.maxHP); got != c.quer {
			t.Errorf("MaxHP %d: escala %d, quer %d", c.maxHP, got, c.quer)
		}
	}
}

func TestProjecaoDeDanoAcompanhaAEscala(t *testing.T) {
	casos := []struct {
		nome        string
		dano, maxHP uint32
		quer        uint16
	}{
		{"HP baixo passa direto", 300, 10_000, 300},
		{"escala 7", 3_000, 200_000, 429}, // ceil(3000/7)
		{"escala 34", 100_000, 1_000_000, 2942},
		{"dano zero", 0, 1_000_000, 0},
		// Golpe menor que a escala nao pode desaparecer da barra: sem o
		// arredondamento para cima, todo hit abaixo de 34 seria invisivel.
		{"golpe menor que a escala", 1, 1_000_000, 1},
		{"golpe igual a escala", 34, 1_000_000, 1},
	}
	for _, c := range casos {
		if got := ProjectHPDelta(c.dano, c.maxHP); got != c.quer {
			t.Errorf("%s: dano %d com MaxHP %d projetou %d, quer %d",
				c.nome, c.dano, c.maxHP, got, c.quer)
		}
	}
}

// TestProjecaoBateComABarra e a propriedade que importa: o dano projetado tem
// de ser coerente com a diferenca que a barra do client vai mostrar. Se um
// golpe leva metade do HP, a barra tem de cair perto de metade.
func TestProjecaoBateComABarra(t *testing.T) {
	for _, maxHP := range []uint32{25_000, 200_000, 1_000_000, 2_000_000} {
		e := Score{Version: ScoreVersion, MaxHP: maxHP, CurHP: maxHP}
		antes := e.CompatibilityScore()

		dano := maxHP / 2
		e.CurHP = maxHP - dano
		depois := e.CompatibilityScore()

		queda := int(antes.CurHP) - int(depois.CurHP)
		projetado := int(ProjectHPDelta(dano, maxHP))

		// Tolerancia de 1 por causa dos dois arredondamentos para cima.
		if diff := queda - projetado; diff > 1 || diff < -1 {
			t.Errorf("MaxHP %d: a barra caiu %d mas o pacote mandou %d",
				maxHP, queda, projetado)
		}
	}
}
