package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestRegenAmountMatchesNativePulse(t *testing.T) {
	for _, tc := range []struct {
		attribute byte
		want      uint32
	}{{0, 468}, {0x01, 768}, {0x20, 1268}, {0x21, 1268}} {
		if got := regenAmount(naturalRecovery(tc.attribute), 270); got != tc.want {
			t.Errorf("attribute=%#x: recovery=%d, want %d", tc.attribute, got, tc.want)
		}
	}
}

func TestMithrilBootsRegenDoesNotRefillManaControlDamage(t *testing.T) {
	w, _, p, _ := manaControlCombatFixture(t, true)
	w.items = reportTestCatalog(t).Items
	ch := p.Char
	ch.Equip[5] = model.Item{Index: 1353, Eff: [6]byte{120, 9}}
	w.recalcPlayer(ch)
	if index := effectiveScore(ch).RegenMP; index != 270 {
		t.Fatalf("Mithril boots +9 index=%d, want 270", index)
	}

	start, maxMP := w.now(), playerMaxMP(ch)
	p.NextRegen = start.Add(8 * time.Second)
	setPlayerCurMP(ch, maxMP)
	for i := 1; i <= 2; i++ {
		if hp := absorbManaControlDamageAt(ch, 500, start.Add(time.Duration(i)*time.Second)); hp != 50 {
			t.Fatalf("hit %d: HP damage=%d, want 50", i, hp)
		}
	}
	w.tickPlayerRegen(start.Add(7 * time.Second))
	if got := playerCurMP(ch); got != maxMP-900 {
		t.Fatalf("early recovery: MP=%d, want %d", got, maxMP-900)
	}
	w.tickPlayerRegen(start.Add(8 * time.Second))
	if got := playerCurMP(ch); got != maxMP-432 {
		t.Fatalf("due recovery: MP=%d, want %d", got, maxMP-432)
	}
	if !p.NextRegen.Equal(start.Add(16 * time.Second)) {
		t.Fatalf("next pulse scheduled for %s", p.NextRegen)
	}
	assertManaControlVitalsPacket(t, p, p)

	ch.Equip[5] = model.Item{}
	w.recalcPlayer(ch)
	w.terrain = guildCityTerrain()
	p.X, p.Y = 2092, 2144
	setPlayerCurMP(ch, playerMaxMP(ch)-2000)
	w.tickPlayerRegen(start.Add(16 * time.Second))
	if got, want := playerCurMP(ch), playerMaxMP(ch)-1000; got != want {
		t.Fatalf("guild area recovery: MP=%d, want %d", got, want)
	}
}

func TestApplyPlayerRegenRestoresAndCapsHPMP(t *testing.T) {
	ch := &model.Char{
		Score: testScore(model.Score{MaxHP: 600, MaxMP: 300, CurHP: 590, CurMP: 280}),
	}
	ch.Score.RegenHP, ch.Score.RegenMP = 2, 4
	if !applyPlayerRegen(ch, 200) {
		t.Fatal("pulso nao alterou o personagem")
	}
	if playerCurHP(ch) != 600 || playerCurMP(ch) != 300 {
		t.Fatalf("HP/MP=%d/%d, quer 600/300", playerCurHP(ch), playerCurMP(ch))
	}
	if applyPlayerRegen(ch, 200) {
		t.Fatal("recovery reported a change at full vitals")
	}
}

func TestApplyPlayerRegenDoesNotRevive(t *testing.T) {
	ch := &model.Char{Score: testScore(model.Score{MaxHP: 100, MaxMP: 100})}
	if applyPlayerRegen(ch, 200) || playerCurHP(ch) != 0 {
		t.Fatal("regeneracao reviveu personagem morto")
	}
}
