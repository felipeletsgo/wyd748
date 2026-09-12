package game

import (
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func questRewardTestPlayer(t *testing.T, level uint32) (*World, *Player, *craftStore) {
	t.Helper()
	w := newZoneTestWorld()
	st := &craftStore{}
	w.store = st
	w.gameplay = model.DefaultGameplayConfig()
	p := addZonePlayer(w, 1, 2100, 2100, 800)
	p.Char.Score.Level = level
	p.Char.Score.Version = model.ScoreVersion
	p.Char.Score.MaxHP, p.Char.Score.CurHP = 1000, 800
	p.Char.Score.MaxMP, p.Char.Score.CurMP = 700, 600
	p.Char.Inv[0] = model.Item{Index: 4117}
	return w, p, st
}

func wisdomBoxRule() model.VolatileRule {
	return model.VolatileRule{Action: "quest_reward", Consume: true, MortalOnly: true,
		MinLevel: 39, MaxLevelExclusive: 115, Exp: 5000, Gold: 2000}
}

func TestQuestRewardUsaMaximoExclusivoEMortal(t *testing.T) {
	rule := wisdomBoxRule()
	for _, tc := range []struct {
		level uint32
		want  bool
	}{{38, false}, {39, true}, {114, true}, {115, false}} {
		ch := &model.Char{Score: &model.Score{Level: tc.level}}
		if got := questRewardAllowed(ch, rule); got != tc.want {
			t.Fatalf("level %d: allowed=%v, quer %v", tc.level, got, tc.want)
		}
	}
	ch := &model.Char{Evolution: "arch", Score: &model.Score{Level: 39}}
	if questRewardAllowed(ch, rule) {
		t.Fatal("Arch nao pode usar recompensa Mortal")
	}
}

func TestQuestRewardAplicaRateSemPisoNemParty(t *testing.T) {
	cfg := model.DefaultGameplayConfig()
	cfg.EXPMinimum = 99_999
	cfg.EXPRatePercent = 150
	if got := scaledQuestExperience(5000, cfg); got != 7500 {
		t.Fatalf("EXP de quest=%d, quer 7500", got)
	}
}

func TestQuestRewardConsomeDepoisDePersistir(t *testing.T) {
	w, p, st := questRewardTestPlayer(t, 39)
	w.gameplay.EXPRatePercent = 200

	w.useQuestReward(p.Session, p, &p.Char.Inv[0], 0, wisdomBoxRule(), 191)
	if st.saves != 1 {
		t.Fatalf("esperava um save sincrono, veio %d", st.saves)
	}
	if p.Char.Inv[0].Index != 0 {
		t.Fatalf("caixa nao foi consumida: %+v", p.Char.Inv[0])
	}
	if p.Char.Exp != 10_000 || p.Char.Gold != 2000 {
		t.Fatalf("recompensa incorreta: exp=%d gold=%d", p.Char.Exp, p.Char.Gold)
	}
	if playerCurHP(p.Char) != 800 || playerCurMP(p.Char) != 600 {
		t.Fatalf("HP/MP mudaram ao usar caixa: %d/%d", playerCurHP(p.Char), playerCurMP(p.Char))
	}
}

func TestQuestRewardRollbackRestauraPersonagemCompleto(t *testing.T) {
	w, p, st := questRewardTestPlayer(t, 39)
	before := cloneCharacterState(p.Char)
	st.err = errors.New("disco indisponivel")

	w.useQuestReward(p.Session, p, &p.Char.Inv[0], 0, wisdomBoxRule(), 191)
	if !reflect.DeepEqual(*p.Char, before) {
		t.Fatalf("rollback incompleto:\nantes=%+v\ndepois=%+v", before, *p.Char)
	}
}
