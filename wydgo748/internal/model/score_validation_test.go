package model

import "testing"

func TestScoreRejectsMissingVersionAndOverflowFields(t *testing.T) {
	var absent *Score
	if err := absent.Validate(); err == nil {
		t.Fatal("nil score was accepted")
	}
	if err := absent.ValidatePlayerState(); err == nil {
		t.Fatal("nil player score was accepted")
	}
	if err := (&Score{}).Validate(); err == nil {
		t.Fatal("wrong score version was accepted")
	}

	fields := []struct {
		name string
		set  func(*Score)
	}{
		{"level", func(s *Score) { s.Level = MaxScoreValue + 1 }},
		{"attack", func(s *Score) { s.Attack = MaxScoreValue + 1 }},
		{"magicAttack", func(s *Score) { s.MagicAttack = MaxScoreValue + 1 }},
		{"defense", func(s *Score) { s.Defense = MaxScoreValue + 1 }},
		{"maxHP", func(s *Score) { s.MaxHP = MaxScoreValue + 1 }},
		{"maxMP", func(s *Score) { s.MaxMP = MaxScoreValue + 1 }},
		{"curHP", func(s *Score) { s.CurHP = MaxScoreValue + 1 }},
		{"curMP", func(s *Score) { s.CurMP = MaxScoreValue + 1 }},
		{"str", func(s *Score) { s.Str = MaxScoreValue + 1 }},
		{"int", func(s *Score) { s.Int = MaxScoreValue + 1 }},
		{"dex", func(s *Score) { s.Dex = MaxScoreValue + 1 }},
		{"con", func(s *Score) { s.Con = MaxScoreValue + 1 }},
		{"accuracy", func(s *Score) { s.Accuracy = MaxScoreValue + 1 }},
		{"evasion", func(s *Score) { s.Evasion = MaxScoreValue + 1 }},
		{"parry", func(s *Score) { s.Parry = MaxScoreValue + 1 }},
		{"critical", func(s *Score) { s.Critical = MaxScoreValue + 1 }},
		{"range", func(s *Score) { s.Range = MaxScoreValue + 1 }},
		{"resistFire", func(s *Score) { s.ResistFire = MaxScoreValue + 1 }},
		{"resistIce", func(s *Score) { s.ResistIce = MaxScoreValue + 1 }},
		{"resistHoly", func(s *Score) { s.ResistHoly = MaxScoreValue + 1 }},
		{"resistThunder", func(s *Score) { s.ResistThunder = MaxScoreValue + 1 }},
		{"saveMana", func(s *Score) { s.SaveMana = MaxScoreValue + 1 }},
		{"magicAmp", func(s *Score) { s.MagicAmp = MaxScoreValue + 1 }},
		{"regenHP", func(s *Score) { s.RegenHP = MaxScoreValue + 1 }},
		{"regenMP", func(s *Score) { s.RegenMP = MaxScoreValue + 1 }},
		{"statusPoints", func(s *Score) { s.StatusPts = MaxScoreValue + 1 }},
		{"masteryPoints", func(s *Score) { s.MasterPts = MaxScoreValue + 1 }},
		{"skillPoints", func(s *Score) { s.SkillPts = MaxScoreValue + 1 }},
	}
	for _, test := range fields {
		t.Run(test.name, func(t *testing.T) {
			score := &Score{Version: ScoreVersion}
			test.set(score)
			if err := score.Validate(); err == nil {
				t.Fatal("score field above the signed calculation limit was accepted")
			}
		})
	}
	for i := range [4]uint32{} {
		score := &Score{Version: ScoreVersion}
		score.Mastery[i] = MaxScoreValue + 1
		if err := score.Validate(); err == nil {
			t.Errorf("mastery[%d] above the limit was accepted", i)
		}
	}
	max := &Score{Version: ScoreVersion, Level: MaxScoreValue, Attack: MaxScoreValue, Mastery: [4]uint32{MaxScoreValue, MaxScoreValue, MaxScoreValue, MaxScoreValue}}
	if err := max.Validate(); err != nil {
		t.Fatalf("score at the signed calculation limit was rejected: %v", err)
	}
}

func TestScorePlayerStateRejectsFunctionalBounds(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*Score)
	}{
		{"fire resistance", func(s *Score) { s.ResistFire = 101 }},
		{"ice resistance", func(s *Score) { s.ResistIce = 101 }},
		{"holy resistance", func(s *Score) { s.ResistHoly = 101 }},
		{"thunder resistance", func(s *Score) { s.ResistThunder = 101 }},
		{"save mana", func(s *Score) { s.SaveMana = 100 }},
		{"HP regeneration", func(s *Score) { s.RegenHP = 256 }},
		{"MP regeneration", func(s *Score) { s.RegenMP = 256 }},
		{"current HP", func(s *Score) { s.CurHP, s.MaxHP = 2, 1 }},
		{"current MP", func(s *Score) { s.CurMP, s.MaxMP = 2, 1 }},
	} {
		t.Run(test.name, func(t *testing.T) {
			score := &Score{Version: ScoreVersion}
			test.mutate(score)
			if err := score.ValidatePlayerState(); err == nil {
				t.Fatal("out-of-range player state was accepted")
			}
		})
	}
	score := &Score{
		Version: ScoreVersion, ResistFire: 100, ResistIce: 100, ResistHoly: 100, ResistThunder: 100,
		SaveMana: 99, RegenHP: 255, RegenMP: 255, MaxHP: MaxScoreValue, CurHP: MaxScoreValue,
		MaxMP: MaxScoreValue, CurMP: MaxScoreValue,
	}
	if err := score.ValidatePlayerState(); err != nil {
		t.Fatalf("functional maximum values were rejected: %v", err)
	}
}

func TestCelestialFormValidationRules(t *testing.T) {
	if err := (*CelestialForm)(nil).Validate(); err != nil {
		t.Fatalf("nil alternate form should be allowed: %v", err)
	}
	valid := &CelestialForm{
		Evolution: "celestial", Class: 3, Face: Item{Index: 1},
		Score: &Score{Version: ScoreVersion, Level: 199},
	}
	if err := valid.Validate(); err != nil {
		t.Fatalf("valid alternate form was rejected: %v", err)
	}
	for _, test := range []struct {
		name   string
		mutate func(*CelestialForm)
	}{
		{"unknown evolution", func(f *CelestialForm) { f.Evolution = "arch" }},
		{"class above range", func(f *CelestialForm) { f.Class = 4 }},
		{"missing face", func(f *CelestialForm) { f.Face = Item{} }},
		{"missing score", func(f *CelestialForm) { f.Score = nil }},
		{"bad score version", func(f *CelestialForm) { f.Score.Version = 0 }},
		{"level above alternate cap", func(f *CelestialForm) { f.Score.Level = 200 }},
	} {
		t.Run(test.name, func(t *testing.T) {
			form := *valid
			form.Score = &Score{Version: ScoreVersion, Level: 199}
			test.mutate(&form)
			if err := form.Validate(); err == nil {
				t.Fatal("invalid alternate celestial form was accepted")
			}
		})
	}
}
