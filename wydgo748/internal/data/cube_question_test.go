package data

import (
	"testing"

	"wydgo/internal/model"
)

func TestAllCubeVariantsConfigureTenSecondQuestions(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	volatiles := loadRealVolatiles(t, "../../data", catalog)
	seen := map[string]int{}
	check := func(rule model.VolatileRule) {
		t.Helper()
		if rule.Instance == nil {
			return
		}
		cfg := rule.Instance
		if cfg.ID != "cube" && cfg.ID != "cube-mystic" && cfg.ID != "cube-arcane" {
			return
		}
		questions := 0
		for _, stage := range cfg.Stages {
			if stage.Quiz != nil {
				questions++
				if stage.Quiz.DurationSeconds != 10 {
					t.Errorf("%s/%s: question duration=%d, want 10", cfg.ID, stage.Name, stage.Quiz.DurationSeconds)
				}
			}
		}
		if questions != 24 {
			t.Errorf("%s: found %d questions, want 24 before the final combat room", cfg.ID, questions)
		}
		seen[cfg.ID]++
	}
	for _, rule := range volatiles.Items {
		check(rule)
	}
	for _, rule := range volatiles.Rules {
		check(rule)
	}
	for _, id := range []string{"cube", "cube-mystic", "cube-arcane"} {
		if seen[id] == 0 {
			t.Errorf("no active ticket resolved %s", id)
		}
	}
}
