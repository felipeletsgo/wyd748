package wire

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
)

// Cenario de servidor com numeros grandes: personagem batendo 500.000 num
// monstro de 5.000.000 de HP. E o caso que quebrava a barra -- MaxHP muito
// acima dos 30.000 que o STRUCT_SCORE nativo comporta.
const (
	danoAlto  = uint32(500_000)
	hpDoChefe = uint32(5_000_000)
	// ceil(5.000.000 / 30.000)
	escalaEsperada = uint32(167)
	// ceil(500.000 / 167)
	danoProjetado = uint16(2_995)
)

func TestGolpeAltoEmChefeDeCincoMilhoes(t *testing.T) {
	if got := model.CompatibilityHPScale(hpDoChefe); got != escalaEsperada {
		t.Fatalf("escala %d, quer %d", got, escalaEsperada)
	}

	b := AttackHitExtended(1, 2000, 10, 10, 11, 10, danoAlto, hpDoChefe, 0, 0)
	if len(b) != 52 {
		t.Fatalf("pacote com %d bytes, quer 52", len(b))
	}
	// O WORD alimenta a BARRA e vai projetado.
	if got := binary.LittleEndian.Uint16(b[46:48]); got != danoProjetado {
		t.Errorf("WORD da barra = %d, quer %d", got, danoProjetado)
	}
	// A cauda alimenta o NUMERO na tela e vai real, sem perder nada.
	if got := binary.LittleEndian.Uint32(b[48:52]); got != danoAlto {
		t.Errorf("cauda wide = %d, quer %d (o numero flutuante nao pode encolher)", got, danoAlto)
	}
	// Antes da correcao o WORD saturava em 65535 (melee) ou 32767 (skill) --
	// os dois estouram o MaxHP projetado do alvo, que e 29941.
	maxProjetado := uint16(29_941)
	if danoProjetado > maxProjetado {
		t.Errorf("o dano projetado (%d) nao pode passar da barra cheia (%d)",
			danoProjetado, maxProjetado)
	}
}

// TestBarraAcompanhaALutaInteira: o client subtrai o WORD a cada golpe. Ao
// longo dos dez golpes que matam o chefe, o que ele acumula tem de seguir
// colado no HP real projetado -- senao a barra esvazia cedo (o bug) ou fica
// travada cheia.
func TestBarraAcompanhaALutaInteira(t *testing.T) {
	_, maxProjetado := projectPairForTest(hpDoChefe, hpDoChefe)
	barraDoClient := int(maxProjetado)
	hpReal := hpDoChefe

	for golpe := 1; hpReal > 0; golpe++ {
		aplicado := danoAlto
		if aplicado > hpReal {
			aplicado = hpReal
		}
		hpReal -= aplicado
		barraDoClient -= int(model.ProjectHPDelta(aplicado, hpDoChefe))

		curProjetado, _ := projectPairForTest(hpReal, hpDoChefe)
		desvio := barraDoClient - int(curProjetado)
		if desvio < 0 {
			desvio = -desvio
		}
		// Um por golpe, do arredondamento para cima dos dois lados. O 0x181
		// corrige de verdade; o que nao pode e a barra despencar.
		if desvio > golpe {
			t.Fatalf("golpe %d: client em %d, real em %d (desvio %d)",
				golpe, barraDoClient, curProjetado, desvio)
		}
		if golpe > 12 {
			t.Fatal("a luta nao terminou em 10 golpes; o dano nao esta sendo aplicado")
		}
	}
	if barraDoClient > 0 {
		t.Errorf("o chefe morreu mas a barra do client parou em %d", barraDoClient)
	}
}

// TestOverkillNaoEstouraOCampo: golpe maior que o HP restante e comum no ultimo
// hit. O WORD nao pode dar a volta nem passar da barra cheia.
func TestOverkillNaoEstouraOCampo(t *testing.T) {
	for _, dano := range []uint32{hpDoChefe, hpDoChefe * 2, 4_000_000_000} {
		got := model.ProjectHPDelta(dano, hpDoChefe)
		if got > 30_000 {
			t.Errorf("dano %d projetou %d, acima do teto do campo", dano, got)
		}
	}
}

// TestMonstroPequenoNaoEscala: mob comum continua com dano 1:1, sem regressao.
func TestMonstroPequenoNaoEscala(t *testing.T) {
	b := AttackHitExtended(1, 2000, 10, 10, 11, 10, 800, 2_000, 0, 0)
	if got := binary.LittleEndian.Uint16(b[46:48]); got != 800 {
		t.Errorf("mob de 2000 de HP: WORD = %d, quer 800 (sem projecao)", got)
	}
}

// projectPairForTest reproduz a projecao do STRUCT_SCORE via caminho publico,
// para o teste comparar com o mesmo numero que o client recebe.
func projectPairForTest(current, maximum uint32) (uint16, uint16) {
	e := model.Score{
		Version: model.ScoreVersion,
		MaxHP:   maximum, CurHP: current,
	}
	sc := e.CompatibilityScore()
	return sc.CurHP, sc.MaxHP
}

// TestDanoNuncaPassaDaBarraCheia e a invariante que o codigo antigo violava, e
// que explica o sintoma relatado: com um chefe de 5.000.000 de HP a barra cheia
// projetada vale 29.941, mas o WORD saia saturado em 65.535 (melee) ou 32.767
// (skill). Os dois passam da barra inteira, entao ela ZERAVA no primeiro golpe
// -- com o chefe ainda em 90% de vida -- e voltava quando chegava o 0x181.
//
// Nenhum dano projetado pode passar da barra cheia do alvo.
func TestDanoNuncaPassaDaBarraCheia(t *testing.T) {
	hps := []uint32{2_000, 30_000, 30_001, 200_000, 1_000_000, 5_000_000, 50_000_000}
	danos := []uint32{1, 500, 500_000, 5_000_000, 100_000_000}

	for _, maxHP := range hps {
		_, barraCheia := projectPairForTest(maxHP, maxHP)
		for _, dano := range danos {
			projetado := model.ProjectHPDelta(dano, maxHP)
			if projetado > barraCheia {
				t.Errorf("MaxHP %d (barra %d): dano %d projetou %d -- estoura a barra inteira",
					maxHP, barraCheia, dano, projetado)
			}
		}
	}
}

// TestGolpeProporcionalTiraFatiaProporcional: um golpe de 10% do HP tem de
// tirar ~10% da barra, em qualquer escala. E a checagem que amarra a projecao
// do dano a projecao do HP.
func TestGolpeProporcionalTiraFatiaProporcional(t *testing.T) {
	for _, maxHP := range []uint32{30_001, 200_000, 5_000_000, 50_000_000} {
		_, barraCheia := projectPairForTest(maxHP, maxHP)
		dano := maxHP / 10
		projetado := int(model.ProjectHPDelta(dano, maxHP))
		esperado := int(barraCheia) / 10

		// 2% de folga sobre a barra cheia absorve os arredondamentos.
		folga := int(barraCheia)/50 + 1
		if diff := projetado - esperado; diff > folga || diff < -folga {
			t.Errorf("MaxHP %d: golpe de 10%% projetou %d, esperado ~%d (barra %d)",
				maxHP, projetado, esperado, barraCheia)
		}
	}
}
