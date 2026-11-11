package game

import (
	"errors"
	"reflect"
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
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
	w.items = map[uint16]model.ItemDef{4117: {Index: 4117, Name: "Wisdom Box"}}
	w.volatiles = model.VolatileCatalog{
		Items:     map[uint16]model.VolatileRule{4117: wisdomBoxRule()},
		ItemCodes: map[uint16]int{4117: 191},
	}

	w.onUseItem(p.Session, useItemPacket(0, 0))
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

func TestQuestRewardRejectsWrongLevelAndDisabledExperience(t *testing.T) {
	for _, tc := range []struct {
		name  string
		level uint32
		rate  uint32
	}{
		{name: "below minimum", level: 38, rate: 100},
		{name: "experience disabled", level: 39, rate: 0},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, st := questRewardTestPlayer(t, tc.level)
			w.gameplay.EXPRatePercent = tc.rate
			before := cloneCharacterState(p.Char)
			packets := p.Session.QueuedPacketsForTest()

			w.useQuestReward(p.Session, p, &p.Char.Inv[0], 0, wisdomBoxRule(), 191)

			if !reflect.DeepEqual(*p.Char, before) || st.saves != 0 {
				t.Fatal("a rejected quest reward changed the character or saved the account")
			}
			if got := p.Session.QueuedPacketsForTest() - packets; got != 2 {
				t.Fatalf("rejected quest reward sent %d packets, want item resync and explanation", got)
			}
		})
	}
}

func TestQuestRewardCriticalPathWaitsForPersistenceAcknowledgement(t *testing.T) {
	w, p, _, _, base, q, release := criticalTreasuryFixture(t, 0)
	p.Char.Score.Level = 39
	p.Char.Exp = mortalNextLevel[39]
	p.Char.Inv[0] = model.Item{Index: 4117, UID: "critical-wisdom-box"}
	w.gameplay.EXPRatePercent = 100
	beforeExp, beforeGold := p.Char.Exp, p.Char.Gold

	w.useQuestReward(p.Session, p, &p.Char.Inv[0], 0, wisdomBoxRule(), 191)
	proposal := awaitCriticalWrite(t, base.entered)
	if len(proposal.accounts) != 1 {
		t.Fatalf("critical reward wrote %d accounts, want one", len(proposal.accounts))
	}
	prepared := proposal.accounts[0].Chars[p.CharSlot]
	if prepared.Inv[0].Index != 0 || prepared.Exp <= beforeExp || prepared.Gold != beforeGold+wisdomBoxRule().Gold {
		t.Fatalf("critical reward proposal is incomplete: exp=%d gold=%d item=%d", prepared.Exp, prepared.Gold, prepared.Inv[0].Index)
	}
	if p.Char.Inv[0].Index != 4117 || p.Char.Exp != beforeExp || p.Char.Gold != beforeGold {
		t.Fatal("critical quest reward became visible before durable acknowledgement")
	}

	release()
	w.applyCriticalCompletion(awaitCriticalResult(t, q))
	if w.shuttingDown || p.Char.Inv[0].Index != 0 || p.Char.Exp != prepared.Exp || p.Char.Gold != prepared.Gold {
		t.Fatal("committed critical quest reward did not install its account proposal")
	}
}

func TestQuestRewardDoesNothingWhileDisconnectPersistenceOwnsPlayer(t *testing.T) {
	w, p, st := questRewardTestPlayer(t, 39)
	w.criticalDisconnects = map[*gameNet.Session]*criticalDisconnect{
		p.Session: {player: p},
	}
	before := cloneCharacterState(p.Char)
	packets := p.Session.QueuedPacketsForTest()

	w.useQuestReward(p.Session, p, &p.Char.Inv[0], 0, wisdomBoxRule(), 191)

	if !reflect.DeepEqual(*p.Char, before) || st.saves != 0 || p.Session.QueuedPacketsForTest() != packets {
		t.Fatal("reserved quest reward changed state, wrote the account, or emitted a result")
	}
}
