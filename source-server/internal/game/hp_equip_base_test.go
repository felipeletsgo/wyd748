package game

import (
	"testing"

	"wydgo/internal/model"
)

// A metade que importa da correcao: limitar o base NAO pode custar HP em jogo.
// Se custasse, todo personagem com equipamento de HP passaria a aparecer abaixo
// de 100% depois de qualquer recalculo -- trocaria um bug por outro.
//
// O valor vivo e o do runtime, e ele sobrevive porque recalcExtendedPlayer
// captura oldHP/wasFullHP a partir do RUNTIME, nao do base.
func TestClampDoBaseNaoReduzOHPEmJogo(t *testing.T) {
	w := &World{}
	const baseMax, efetivoMax = uint32(1_000_000), uint32(1_202_183)

	ch := &model.Char{
		Name: "A",
		Score: &model.Score{
			Version: model.ScoreVersion,
			MaxHP:   baseMax, CurHP: baseMax,
			MaxMP: baseMax, CurMP: baseMax,
		},
		RuntimeScore: &model.Score{
			Version: model.ScoreVersion,
			MaxHP:   efetivoMax, CurHP: efetivoMax,
			MaxMP: efetivoMax, CurMP: efetivoMax,
		},
	}

	// Estava cheio no teto efetivo; um heal redundante nao pode rebaixa-lo.
	restorePlayerHP(ch, 1)

	if got := playerCurHP(ch); got != efetivoMax {
		t.Errorf("HP em jogo caiu para %d, esperava %d -- o clamp do base vazou "+
			"para o runtime", got, efetivoMax)
	}
	if ch.Score.CurHP > ch.Score.MaxHP {
		t.Errorf("base gravou %d/%d, um estado impossivel",
			ch.Score.CurHP, ch.Score.MaxHP)
	}
	_ = w
}

// TestBaseNuncaExcedeOTetoEmNenhumRecurso trava a invariante nos DOIS recursos.
// O MP foi o que denunciou a causa: nenhum affect aumenta MaxMP, entao o
// curMP > maxMP so podia vir do teto efetivo do equipamento.
func TestBaseNuncaExcedeOTetoEmNenhumRecurso(t *testing.T) {
	const baseMax, efetivoMax = uint32(1_000_000), uint32(1_202_183)
	novo := func() *model.Char {
		return &model.Char{
			Name: "A",
			Score: &model.Score{
				Version: model.ScoreVersion,
				MaxHP:   baseMax, CurHP: baseMax / 2,
				MaxMP: baseMax, CurMP: baseMax / 2,
			},
			RuntimeScore: &model.Score{
				Version: model.ScoreVersion,
				MaxHP:   efetivoMax, CurHP: baseMax / 2,
				MaxMP: efetivoMax, CurMP: baseMax / 2,
			},
		}
	}
	ch := novo()
	restorePlayerHP(ch, efetivoMax)
	restorePlayerMP(ch, efetivoMax)

	if ch.Score.CurHP > ch.Score.MaxHP {
		t.Errorf("base HP %d/%d", ch.Score.CurHP, ch.Score.MaxHP)
	}
	if ch.Score.CurMP > ch.Score.MaxMP {
		t.Errorf("base MP %d/%d", ch.Score.CurMP, ch.Score.MaxMP)
	}
	// E o teto efetivo continua alcancavel em jogo.
	if playerCurHP(ch) != efetivoMax || playerCurMP(ch) != efetivoMax {
		t.Errorf("runtime nao encheu ate o teto efetivo: HP=%d MP=%d",
			playerCurHP(ch), playerCurMP(ch))
	}
}
