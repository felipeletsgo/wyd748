package model

import "testing"

// A aura de vida do TransKnight MULTIPLICA o MaxHP. Quando o resultado cruza os
// 30.000 do STRUCT_SCORE nativo, a escala de compatibilidade salta de 1 para 2
// e TODO WORD que o client ja tinha em maos muda de significado de uma vez.
//
// Estes testes fixam a propriedade que importa para a barra: a PROPORCAO
// projetada tem de acompanhar a proporcao real atraves do salto de escala. Se
// ela se mantiver e a barra ainda cair no client, o erro nao esta na projecao
// -- esta em quem mistura um campo escalado com um campo wide.

func racaoProjetada(cur, max uint32) float64 {
	c, m := projectCompatibilityPair(cur, max)
	if m == 0 {
		return 0
	}
	return float64(c) / float64(m)
}

func TestAuraQueCruzaOTetoNaoDerrubaABarra(t *testing.T) {
	casos := []struct {
		nome            string
		baseMax, bufMax uint32
		fracao          float64
	}{
		// O caso do felipe: a aura empurra o MaxHP por cima do teto e a escala
		// vira 2. Antes do buff a escala era 1.
		{"cheio cruzando o teto", 29_000, 34_800, 1.0},
		{"metade cruzando o teto", 29_000, 34_800, 0.5},
		// Ja acima do teto dos dois lados: escala 2 -> 3.
		{"cheio de 2 para 3", 45_000, 63_000, 1.0},
		{"metade de 2 para 3", 45_000, 63_000, 0.5},
		// Inteiramente abaixo do teto: escala 1 nos dois lados, sem salto.
		{"cheio sem salto", 10_000, 12_000, 1.0},
		// Os numeros REAIS do felipe: 500 mil de HP, escala 17, e a aura sobe
		// para 600 mil (escala 20). Nunca passa perto do teto de 30.000 -- foi
		// isso que derrubou a hipotese de que o salto 1->2 explicava a barra.
		{"aura do felipe cheio", 500_000, 600_000, 1.0},
		{"aura do felipe metade", 500_000, 600_000, 0.5},
	}
	for _, caso := range casos {
		t.Run(caso.nome, func(t *testing.T) {
			curAntes := uint32(float64(caso.baseMax) * caso.fracao)
			// O servidor soma a variacao do maximo ao atual (affects.go), entao
			// quem estava cheio continua cheio e quem estava na metade sobe o
			// mesmo tanto que o maximo subiu.
			curDepois := curAntes + (caso.bufMax - caso.baseMax)

			antes := racaoProjetada(curAntes, caso.baseMax)
			depois := racaoProjetada(curDepois, caso.bufMax)
			realDepois := float64(curDepois) / float64(caso.bufMax)

			// A barra NAO pode cair: depois do buff a proporcao projetada tem de
			// bater com a real, com folga de meio ponto percentual para o
			// arredondamento para cima da projecao.
			if diferenca := depois - realDepois; diferenca < -0.005 || diferenca > 0.005 {
				t.Errorf("proporcao projetada=%.4f, real=%.4f (antes do buff era %.4f) "+
					"-- escala %d->%d", depois, realDepois, antes,
					CompatibilityHPScale(caso.baseMax), CompatibilityHPScale(caso.bufMax))
			}
			if depois < antes-0.005 {
				t.Errorf("a barra CAIU de %.4f para %.4f ao aplicar o buff", antes, depois)
			}
		})
	}
}

// TestSaltoDeEscalaEhDocumentado deixa explicito o ponto exato onde o
// significado dos WORDs muda. Quem depurar a barra no client precisa deste
// numero: e o unico lugar onde um mesmo HP real vira dois WORDs diferentes.
func TestSaltoDeEscalaEhDocumentado(t *testing.T) {
	if CompatibilityHPScale(30_000) != 1 {
		t.Error("30000 ainda cabe na escala 1")
	}
	if CompatibilityHPScale(30_001) != 2 {
		t.Error("30001 tem de virar escala 2")
	}
	// A prova do risco: o MESMO HP real de 29.000 vale 29.000 no WORD quando o
	// maximo e 30.000, e 14.500 quando o maximo e 30.001. Um client que guarde
	// o MaxHP wide (30.001) e receba o CurHP escalado (14.500) desenha 48% da
	// barra em vez de 96%.
	curBaixo, _ := projectCompatibilityPair(29_000, 30_000)
	curAlto, _ := projectCompatibilityPair(29_000, 30_001)
	if curBaixo != 29_000 || curAlto != 14_500 {
		t.Fatalf("projecao mudou: %d e %d", curBaixo, curAlto)
	}
}
