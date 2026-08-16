package game

import (
	"testing"

	"wydgo/internal/model"
)

func passiveChar(class byte, learned uint32, ext model.ExtendedScore) *model.Char {
	ext.Version = model.ExtendedScoreVersion
	return &model.Char{Class: class, LearnedSkill: learned, Extended: &ext}
}

func TestLearnedLocalBoundsAndNilContracts(t *testing.T) {
	if learnedLocal(nil, 0) {
		t.Fatal("nil char foi tratado como skill aprendida")
	}
	ch := passiveChar(0, 1<<3, model.ExtendedScore{})
	if learnedLocal(ch, -1) || learnedLocal(ch, 24) || !learnedLocal(ch, 3) || learnedLocal(ch, 4) {
		t.Fatal("learnedLocal divergiu dos 24 bits locais")
	}
}

func TestPassiveSkillsCoverEveryClassBranch(t *testing.T) {
	w := &World{}

	tk := passiveChar(0, 1<<15, model.ExtendedScore{Defense: 100, Critical: 7})
	w.applyPassiveSkills(tk)
	if tk.Extended.Defense != 160 || tk.Extended.Critical != 31 {
		t.Fatalf("TK passive divergente: %+v", tk.Extended)
	}

	bm := passiveChar(2, (1<<(65-48))|(1<<(67-48)), model.ExtendedScore{
		Defense: 100, ResistFire: 90, ResistIce: 10, ResistHoly: 20, ResistThunder: 30,
		Mastery: [4]uint32{0, 0, 120, 0},
	})
	w.applyPassiveSkills(bm)
	if bm.Extended.Defense != 105 || bm.Extended.ResistFire != 100 || bm.Extended.ResistIce != 30 ||
		bm.Extended.ResistHoly != 40 || bm.Extended.ResistThunder != 50 {
		t.Fatalf("BM passives divergentes: %+v", bm.Extended)
	}

	ht := passiveChar(3, (1<<(82-72))|(1<<(91-72))|(1<<(92-72)), model.ExtendedScore{
		Attack: 100, Dex: 300, Range: 1,
		Mastery: [4]uint32{0, 40, 0, 90},
	})
	w.applyPassiveSkills(ht)
	if ht.Extended.Attack != 150 {
		t.Fatalf("Huntress agressividade=%d, esperado 150", ht.Extended.Attack)
	}
	// (90+1)/10 + 300/75 = 9 + 4 = 13.
	if ht.Extended.Critical != 13 || ht.Extended.Range != 2 {
		t.Fatalf("Huntress passives divergentes: crit=%d range=%d", ht.Extended.Critical, ht.Extended.Range)
	}
}

func TestPassiveSkillsLeaveUnlearnedAndUnsupportedClassesUntouched(t *testing.T) {
	w := &World{}
	base := model.ExtendedScore{Version: model.ExtendedScoreVersion, Attack: 10, Defense: 20, Critical: 3, Range: 1}
	for _, class := range []byte{0, 1, 2, 3, 9} {
		ch := &model.Char{Class: class, Extended: &base}
		copyBefore := *ch.Extended
		w.applyPassiveSkills(ch)
		if *ch.Extended != copyBefore {
			t.Fatalf("classe %d sem passiva mudou score: before=%+v after=%+v", class, copyBefore, *ch.Extended)
		}
	}
	w.applyPassiveSkills(nil)

	ch := &model.Char{Class: 0, LearnedSkill: 1 << 15, Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion}}
	ch.ExtendedRuntime = nil
	w.applyPassiveSkills(ch)
	if ch.Extended.Defense != 50 || ch.Extended.Critical != 24 {
		t.Fatalf("fallback para Extended persistido falhou: %+v", ch.Extended)
	}
}
