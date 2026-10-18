package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestPlayerAttackSpeedUsesPercentagePointsNotTemplateNibbles(t *testing.T) {
	now := time.Date(2026, 10, 14, 18, 0, 0, 0, time.UTC)
	for _, tc := range []struct {
		name       string
		dex        uint32
		itemSpeed  int
		sanc       byte
		affect     model.Affect
		learned    uint32
		wantPacked uint32
	}{
		{name: "plain character at 110 percent", dex: 50, wantPacked: 0x12},
		{name: "item adds ten percentage points", dex: 50, itemSpeed: 10, wantPacked: 0x22},
		{name: "legacy item value one means ten points", dex: 50, itemSpeed: 1, wantPacked: 0x22},
		{name: "refinement remainder combines before quantizing", dex: 55, itemSpeed: 10, sanc: 9, wantPacked: 0x32},
		{name: "dexterity debuff changes effective speed", dex: 300,
			affect: model.Affect{Type: 5, Value: 50}, wantPacked: 0x32},
		{name: "slow penalty uses percentage points", dex: 300,
			affect: model.Affect{Type: 7, Level: 120}, wantPacked: 0x22},
		{name: "transformation remainder combines before quantizing", dex: 75,
			affect: model.Affect{Type: 16, Value: 3}, learned: 1 << (69 - 48), wantPacked: 0x53},
		{name: "native nibble cap", dex: 1000, itemSpeed: 100, wantPacked: 0xF2},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w := &World{clock: newFakeClock(now), items: map[uint16]model.ItemDef{
				100: {Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_ATTSPEED", Value: tc.itemSpeed}}},
			}}
			ch := &model.Char{Class: 2, LearnedSkill: tc.learned, Score: testScore(model.Score{
				Level: 399, Str: 8, Int: 4, Dex: tc.dex, Con: 6, AttackRun: 0xC2,
			})}
			if tc.itemSpeed != 0 {
				ch.Equip[1] = model.Item{Index: 100, Eff: [6]byte{43, tc.sanc}}
			}
			if tc.affect.Type != 0 {
				tc.affect.ExpiresAt = now.Add(time.Minute)
				ch.Affects[0] = tc.affect
			}
			for recalculation := 0; recalculation < 2; recalculation++ {
				w.recalcPlayer(ch)
				if got := effectiveScore(ch).AttackRun; got != tc.wantPacked {
					t.Fatalf("recalculation %d: AttackRun=%#x, want %#x", recalculation, got, tc.wantPacked)
				}
				if ch.Score.AttackRun != 0xC2 {
					t.Fatal("runtime recalculation changed the persisted template")
				}
			}
		})
	}
}

func TestDoubleHitBandsUseBonusNibbleWithoutChangingAttackCadence(t *testing.T) {
	if doubleHitChance(nil) != 0 {
		t.Fatal("a missing character must not have a double-hit chance")
	}
	for _, tc := range []struct {
		percent int
		chance  int
	}{{100, 0}, {110, 0}, {150, 0}, {160, 10}, {200, 50}, {210, 100}, {250, 100}} {
		ch := &model.Char{Score: testScore(model.Score{AttackRun: packedAttackSpeed(tc.percent - 100)})}
		if got := attackSpeedPercent(ch); got != tc.percent {
			t.Fatalf("speed=%d, want %d", got, tc.percent)
		}
		if got := doubleHitChance(ch); got != tc.chance {
			t.Fatalf("speed %d%%: double chance=%d, want %d", tc.percent, got, tc.chance)
		}
		if got := attackIntervalFor(ch); got != 900*time.Millisecond {
			t.Fatalf("speed %d%% changed physical cadence to %v", tc.percent, got)
		}
		if tc.chance == 0 {
			var progress uint16
			for action := 0; action < len(hitRateProgression); action++ {
				double, _ := rollPhysicalHitFlags(ch, &progress, func(int) int { return 999 })
				if double {
					t.Fatalf("speed %d%% produced a double at action %d", tc.percent, action)
				}
			}
		}
	}
}

func TestDoubleHitGuaranteeStartsAt201PercentWithUnpackedPrecision(t *testing.T) {
	w := &World{}
	for _, percent := range []int{110, 159, 160, 199, 200, 201, 209, 210, 250, 300} {
		ch := &model.Char{Class: 2, Score: testScore(model.Score{
			Level: 399, Str: 8, Int: 4, Con: 6,
			Dex: uint32((percent - 100) * 5), AttackRun: 2,
		})}
		for recalculation := 0; recalculation < 2; recalculation++ {
			w.recalcPlayer(ch)
			if got := attackSpeedPercent(ch); got != percent {
				t.Fatalf("recalculation %d: speed=%d, want %d", recalculation, got, percent)
			}
			wantChance := clampInt((percent-100)/10*10-50, 0, 100)
			if percent > 200 {
				wantChance = 100
			}
			if got := doubleHitChance(ch); got != wantChance {
				t.Fatalf("speed %d: chance=%d, want %d", percent, got, wantChance)
			}
			misses := 0
			var progress uint16
			for action := range hitRateProgression {
				double, critical := rollPhysicalHitFlags(ch, &progress, func(int) int { return 999 })
				if critical {
					t.Fatal("double-hit guarantee changed the independent critical roll")
				}
				if !double {
					misses++
					if percent > 200 {
						t.Fatalf("speed %d: action %d was not a double", percent, action)
					}
				}
			}
			if percent == 200 && misses == 0 {
				t.Fatal("exactly 200 percent must not guarantee double hits")
			}
			if attackIntervalFor(ch) != 900*time.Millisecond {
				t.Fatal("double-hit rule changed the action interval")
			}
		}
		// Losing DEX must invalidate the previous guarantee on recalculation.
		ch.Score.Dex = 500
		w.recalcPlayer(ch)
		if attackSpeedPercent(ch) != 200 || doubleHitChance(ch) != 50 {
			t.Fatal("recalculation retained a stale guarantee after speed decreased")
		}
		applyScore(ch)
		if ch.RuntimeAttackSpeedPercent != 0 {
			t.Fatal("base-score reset retained a stale exact-speed cache")
		}
	}
}

func TestPhysicalAttackHandlerEnforces900msAt110Percent(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Date(2026, 10, 14, 18, 0, 0, 0, time.UTC))
	w.clock = clock
	w.rng = fixedRNG{value: 0}
	p.Char.Score.Attack = 1000
	p.Char.Score.Dex = 50
	p.Char.Score.Accuracy = 1000
	p.Char.Score.AttackRun = 0x12
	applyScore(p.Char)
	if attackSpeedPercent(p.Char) != 110 || doubleHitChance(p.Char) != 0 {
		t.Fatal("handler fixture must represent 110 percent without double hits")
	}
	mob := &Mob{ID: 1000, X: 2101, Y: 2100, HP: 1_000_000,
		Def: &model.NPCDef{Name: "CadenceTarget", Tipo: model.TipoMonstro,
			Score: testScore(model.Score{MaxHP: 1_000_000, CurHP: 1_000_000})}}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	p.show(mob.ID)
	attack := func(tick uint32) {
		w.onAttack(p.Session, physicalAttackPacket(tick, mob.ID, mob.X, mob.Y))
	}
	attack(1000)
	if mob.HP >= 1_000_000 || p.LastAttackTick != 1000 {
		t.Fatal("first physical attack was not applied")
	}
	after := mob.HP
	for elapsed := uint32(1); elapsed < 900; elapsed++ {
		clock.Advance(time.Millisecond)
		attack(1000 + elapsed)
		attack(100_000 + elapsed) // A forged client clock cannot bypass server time.
		if mob.HP != after || p.LastAttackTick != 1000 {
			t.Fatalf("burst bypassed the physical limiter at %d ms", elapsed)
		}
	}
	clock.Advance(time.Millisecond)
	attack(1899)
	if mob.HP != after {
		t.Fatal("899 ms of client time bypassed the limiter after server time elapsed")
	}
	attack(1900)
	if mob.HP >= after || p.LastAttackTick != 1900 {
		t.Fatal("the next physical attack was not accepted at exactly 900 ms")
	}
	after = mob.HP
	attack(2800)
	clock.Advance(899 * time.Millisecond)
	attack(2800)
	if mob.HP != after || p.LastAttackTick != 1900 {
		t.Fatal("a future client tick bypassed the second server-time interval")
	}
	clock.Advance(time.Millisecond)
	attack(2800)
	if mob.HP >= after || p.LastAttackTick != 2800 {
		t.Fatal("rejected bursts consumed the next legitimate physical attack")
	}
}
