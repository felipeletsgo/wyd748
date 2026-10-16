package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

// Support buffs last from 5 minutes (mastery 0) to 10 minutes (mastery 320),
// whatever the catalog AffectTime, in units of 8 seconds.
func TestSupportBuffLastsFiveToTenMinutes(t *testing.T) {
	for _, tc := range []struct {
		mastery, affectTime int
		want                time.Duration
	}{
		{0, 150, 304 * time.Second},
		{0, 3, 304 * time.Second},
		{160, 150, 448 * time.Second},
		{320, 150, 600 * time.Second},
		{320, 3, 600 * time.Second},
		{400, 150, 600 * time.Second},
	} {
		w, p, _ := handlerTestWorld(t)
		clock := newFakeClock(time.Unix(100, 0))
		w.clock = clock
		skill := model.SkillDef{Index: 44, AffectType: 11, AffectValue: 10, AffectTime: tc.affectTime}

		w.applySupportSkillToTargets(p, []*Player{p}, skill, tc.mastery)

		found := false
		for _, affect := range p.Char.Affects {
			if affect.Type == 11 {
				found = true
				if got := affect.ExpiresAt.Sub(clock.Now()); got != tc.want {
					t.Fatalf("mastery %d AffectTime %d: lasts %v, want %v", tc.mastery, tc.affectTime, got, tc.want)
				}
			}
		}
		if !found {
			t.Fatal("the buff was not applied")
		}
	}
}
