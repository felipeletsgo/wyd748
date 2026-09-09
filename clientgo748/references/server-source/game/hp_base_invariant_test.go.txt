package game

import (
	"testing"

	"wydgo/internal/model"
)

// Flagrado por /hpdebug com a Aura da Vida ativa:
//
//	base=1202183/1000000  wide=1202183/1202183(100.0%)
//
// O CurHP BASE ficou 20% acima do MaxHP BASE. O base e o score persistido e sem
// buff -- CurHP maior que MaxHP la e um estado impossivel, e ele vai para o
// disco no autosave.
//
// A origem e setPlayerCurHP: ele limita o valor pelo MaxHP EFETIVO (com buff) e
// grava o resultado no base, que nao tem o buff.
func jogadorComBuffDeMaxHP(baseMax, buffMax uint32) *model.Char {
	ch := &model.Char{
		Name: "A",
		Score: &model.Score{
			Version: model.ScoreVersion,
			MaxHP:   baseMax, CurHP: baseMax,
		},
		RuntimeScore: &model.Score{
			Version: model.ScoreVersion,
			MaxHP:   buffMax, CurHP: baseMax,
		},
	}
	return ch
}

func TestBaseNuncaGuardaCurHPAcimaDoMaxHPBase(t *testing.T) {
	// Numeros reais do felipe: 1.000.000 de base, aura levando a 1.202.183.
	const baseMax, buffMax = uint32(1_000_000), uint32(1_202_183)
	ch := jogadorComBuffDeMaxHP(baseMax, buffMax)

	// E o que applySupportSkill faz quando o buff sobe o teto: soma a diferenca
	// ao HP corrente para quem estava cheio continuar cheio.
	restorePlayerHP(ch, buffMax-baseMax)

	if ch.RuntimeScore.CurHP != buffMax {
		t.Errorf("runtime CurHP=%d, quer %d (o buff precisa valer em jogo)",
			ch.RuntimeScore.CurHP, buffMax)
	}
	if ch.Score.CurHP > ch.Score.MaxHP {
		t.Errorf("base guardou CurHP=%d acima do MaxHP base=%d -- estado impossivel "+
			"e persistido no autosave", ch.Score.CurHP, ch.Score.MaxHP)
	}
}

// TestBuffDeMaxHPNaoInflaOBasePorRepeticao: reaplicar a aura nao pode empilhar.
// Com o clamp errado, cada recast somava a diferenca de novo e o base crescia
// sem teto.
func TestBuffDeMaxHPNaoInflaOBasePorRepeticao(t *testing.T) {
	const baseMax, buffMax = uint32(1_000_000), uint32(1_202_183)
	ch := jogadorComBuffDeMaxHP(baseMax, buffMax)
	for i := 0; i < 5; i++ {
		restorePlayerHP(ch, buffMax-baseMax)
	}
	if ch.Score.CurHP > ch.Score.MaxHP {
		t.Errorf("apos 5 recasts o base ficou em %d/%d",
			ch.Score.CurHP, ch.Score.MaxHP)
	}
	if ch.RuntimeScore.CurHP > ch.RuntimeScore.MaxHP {
		t.Errorf("apos 5 recasts o runtime ficou em %d/%d",
			ch.RuntimeScore.CurHP, ch.RuntimeScore.MaxHP)
	}
}
