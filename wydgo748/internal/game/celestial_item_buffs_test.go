package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

// Potion buffs (affects with a source item) belong to the character: they
// survive the Celestial form change, while skill buffs stay with their form.
func TestCelestialFormChangeCarriesItemBuffs(t *testing.T) {
	expires := time.Now().Add(time.Hour)
	potion := model.Affect{Type: 35, Value: 10, ExpiresAt: expires, SourceItemIndex: 3314, SourceItemUID: "potion"}
	skill := model.Affect{Type: 24, Value: 200, ExpiresAt: expires}
	ch := &model.Char{Evolution: "celestial", Score: testScore(model.Score{Level: 100})}
	ch.Affects[2], ch.Affects[3] = potion, skill
	other := model.CelestialForm{Evolution: "subcelestial", Score: testScore(model.Score{Level: 50})}
	other.Affects[0] = model.Affect{Type: 26, ExpiresAt: expires}

	current := activeCelestialForm(ch)
	loadCelestialForm(ch, other)
	if current.Affects[2].Type != 0 || current.Affects[3] != skill {
		t.Fatalf("stored form kept the potion or lost its skill buff: %+v", current.Affects[:4])
	}
	if ch.Affects[0].Type != 26 || ch.Affects[2] != potion || ch.Affects[3].Type != 0 {
		t.Fatalf("loaded form did not carry only the potion: %+v", ch.Affects[:4])
	}
	if got := itemBuffsOnly(current.Affects); got != ([16]model.Affect{}) {
		t.Fatalf("skill buff treated as an item buff: %+v", got)
	}
}
