package game

import (
	"testing"

	"wydgo/internal/model"
)

// TestArchStatusPointBudgetMatchesSource confere a formula do ramo ARCH de
// BASE_GetCurrentScore (Basedef.cpp:1043-1056) em valores calculados a mao.
func TestArchStatusPointBudgetMatchesSource(t *testing.T) {
	for _, tc := range []struct {
		nome               string
		level, mortalLevel int
		want               int
	}{
		// lvl*6 + (mortal-299)*5 + 28
		{"arch nivel 0, mortal 370", 0, 370, 0*6 + 71*5 + 28},
		{"arch nivel 100, mortal 370", 100, 370, 100*6 + 71*5 + 28},
		{"arch nivel 100, mortal 400", 100, 400, 100*6 + 101*5 + 28},
		// acima de 354 entram mais 6 por nivel
		{"arch nivel 400, mortal 400", 400, 400, 400*6 + 101*5 + 28 + (400-354)*6},
		// mortal abaixo do offset nao pode tirar pontos
		{"mortal 200 nao gera bonus negativo", 100, 200, 100*6 + 28},
		{"mortal exatamente 299", 100, 299, 100*6 + 28},
	} {
		t.Run(tc.nome, func(t *testing.T) {
			if got := archStatusPointBudget(tc.level, tc.mortalLevel); got != tc.want {
				t.Fatalf("orcamento=%d, quer %d", got, tc.want)
			}
		})
	}
}

// TestArchGetsMorePointsThanMortal e a razao de ser da mecanica: ascender de um
// Mortal mais alto entrega um Arch mais forte.
func TestArchGetsMorePointsThanMortal(t *testing.T) {
	de370 := archStatusPointBudget(50, 370)
	de400 := archStatusPointBudget(50, 400)
	if de400 <= de370 {
		t.Fatalf("mortal 400 (%d) deveria render mais que mortal 370 (%d)", de400, de370)
	}
	if diff := de400 - de370; diff != 30*archPointsPerMortalLevel {
		t.Fatalf("diferenca=%d, quer %d (30 niveis x %d)", diff, 30*archPointsPerMortalLevel, archPointsPerMortalLevel)
	}
}

// TestStatusPointBudgetPicksFormulaByEvolution garante que o Mortal continua na
// formula antiga: a mudanca nao pode alterar personagem nenhum ja existente.
func TestStatusPointBudgetPicksFormulaByEvolution(t *testing.T) {
	mortal := &model.Char{Score: &model.Score{Level: 100}}
	if got, want := statusPointBudget(mortal), mortalStatusPointBudget(100); got != want {
		t.Errorf("mortal: orcamento=%d, quer %d", got, want)
	}
	arch := &model.Char{
		Evolution: archEvolution, ArchMortalLevel: 370,
		Score: &model.Score{Level: 100},
	}
	if got, want := statusPointBudget(arch), archStatusPointBudget(100, 370); got != want {
		t.Errorf("arch: orcamento=%d, quer %d", got, want)
	}
}

// TestRefreshArchMortalLevelFollowsMortal cobre o comportamento dinamico: subir
// o Mortal DEPOIS da ascensao continua fortalecendo o Arch.
func TestRefreshArchMortalLevelFollowsMortal(t *testing.T) {
	acc := &model.Account{Chars: []model.Char{
		{Name: "Mortal", UID: "11111111111141118111111111111111", Score: &model.Score{Level: 370}},
		{Name: "Mortal", UID: "22222222222242228222222222222222", Evolution: archEvolution,
			ArchMortalUID: "11111111111141118111111111111111", ArchMortalLevel: 370,
			Score: &model.Score{Level: 10}},
	}}

	// Mortal sobe de nivel; o proximo login precisa refletir isso.
	acc.Chars[0].Score.Level = 395
	if !refreshArchMortalLevel(acc) {
		t.Fatal("o refresh deveria detectar a mudanca de nivel do Mortal")
	}
	if got := acc.Chars[1].ArchMortalLevel; got != 395 {
		t.Fatalf("nivel do Mortal no Arch=%d, quer 395", got)
	}
	// Sem mudanca, nada a fazer.
	if refreshArchMortalLevel(acc) {
		t.Error("refresh sem mudanca nao deveria reportar alteracao")
	}
}

// TestRefreshArchMortalLevelKeepsValueWhenMortalGone: apagar o Mortal nao pode
// zerar pontos ja distribuidos no Arch.
func TestRefreshArchMortalLevelKeepsValueWhenMortalGone(t *testing.T) {
	acc := &model.Account{Chars: []model.Char{
		{}, // slot do Mortal, agora vazio
		{Name: "Orfao", UID: "22222222222242228222222222222222", Evolution: archEvolution,
			ArchMortalUID: "11111111111141118111111111111111", ArchMortalLevel: 380,
			Score: &model.Score{Level: 10}},
	}}
	refreshArchMortalLevel(acc)
	if got := acc.Chars[1].ArchMortalLevel; got != 380 {
		t.Fatalf("nivel=%d, quer manter 380 com o Mortal apagado", got)
	}
}

// TestRefreshArchMortalLevelIgnoresSelfReference protege contra um slot torto
// que apontasse para o proprio Arch.
func TestRefreshArchMortalLevelIgnoresSelfReference(t *testing.T) {
	acc := &model.Account{Chars: []model.Char{
		{Name: "Arch", UID: "11111111111141118111111111111111", Evolution: archEvolution,
			ArchMortalUID: "11111111111141118111111111111111", ArchMortalLevel: 380,
			Score: &model.Score{Level: 10}},
	}}
	refreshArchMortalLevel(acc)
	if got := acc.Chars[0].ArchMortalLevel; got != 380 {
		t.Fatalf("auto-referencia alterou o nivel: %d", got)
	}
}
