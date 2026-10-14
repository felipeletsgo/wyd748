package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCombatAccuracyStartsAtOneHundredAndFallsWithDefenderEvasion(t *testing.T) {
	if got := combatAccuracyPercent(0, 0, 0, false); got != 100 {
		t.Fatalf("base accuracy=%d want=100", got)
	}
	if got := combatEvasionRatingPercent(0, 0); got != 0 {
		t.Fatalf("base evasion=%d want=0", got)
	}
	if got := combatAccuracyPercent(4000, 0, 0, false); got != 20 {
		t.Fatalf("accuracy against 4000 DEX=%d want=20", got)
	}
	if got := combatEvasionRatingPercent(4000, 10_000); got != 80 {
		t.Fatalf("evasion cap=%d want=80", got)
	}
	if got := combatAccuracyPercent(2000, 100, 50, false); got != 65 {
		t.Fatalf("accuracy with bonuses=%d want=65", got)
	}
	if got := combatAccuracyPercent(4000, 0, 800, true); got != 35 {
		t.Fatalf("Concentration minimum=%d want=35", got)
	}
}

func TestPhysicalFlagsUseNative748DoubleProgressionAndIndependentCritical(t *testing.T) {
	ch := &model.Char{Score: testScore(model.Score{AttackRun: 0xF0, Critical: 255})}
	progress := uint16(0)
	double, critical := rollPhysicalHitFlags(ch, &progress, func(int) int { return 254 })
	if !double || !critical {
		t.Fatalf("flags at 250%%/255 double=%v critical=%v want both", double, critical)
	}
	if progress != 1 {
		t.Fatalf("progress=%d want=1", progress)
	}

	ch.Score.AttackRun = 0x50 // 150% Attack Speed: no Double Hit chance.
	ch.Score.Critical = 0
	double, critical = rollPhysicalHitFlags(ch, &progress, func(int) int { return 0 })
	if double || critical {
		t.Fatalf("flags at 150%%/0 double=%v critical=%v want neither", double, critical)
	}
}

func TestCriticalChanceMatchesCharacterPanelScale(t *testing.T) {
	// Critical 96 is 9.6%, the value the item tooltip and panel display.
	ch := &model.Char{Score: testScore(model.Score{AttackRun: 0x50, Critical: 96})}
	for _, tc := range []struct {
		roll int
		want bool
	}{{0, true}, {95, true}, {96, false}, {999, false}} {
		progress := uint16(0)
		if _, critical := rollPhysicalHitFlags(ch, &progress, func(n int) int {
			if n != 1000 {
				t.Fatalf("critical roll range=%d, want 1000", n)
			}
			return tc.roll
		}); critical != tc.want {
			t.Fatalf("roll %d/1000 at 9.6%%: critical=%v want %v", tc.roll, critical, tc.want)
		}
	}
}

func TestCriticalReachesOneHundredPercent(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_CRITICAL", Value: 600}}},
	}}
	ch := &model.Char{Class: 0, Score: testScore(model.Score{Str: 8, Int: 4, Dex: 7, Con: 6})}
	ch.Equip[1] = model.Item{Index: 100}
	ch.Equip[2] = model.Item{Index: 100}
	w.recalcPlayer(ch)
	if got := effectiveScore(ch).Critical; got != 1000 {
		t.Fatalf("critical=%d, want capped at 1000 (100%%)", got)
	}
	progress := uint16(0)
	if _, critical := rollPhysicalHitFlags(ch, &progress, func(int) int { return 999 }); !critical {
		t.Fatal("100% critical missed the highest roll")
	}
}

func TestHuntressAirBladeRequiresAttackTwoAndLearnedPassive(t *testing.T) {
	ch := &model.Char{Class: 3, Score: testScore(model.Score{Str: 1000, Mastery: [4]uint32{0, 0, 0, 500}})}
	hit := physicalHitResult{Damage: 1000, Hit: true}
	procRNG := func(int) int { return 0 }
	if got := applyHuntressAirBlade(hit, ch, 0, true, procRNG); got.Flank != 0 || got.Damage != hit.Damage {
		t.Fatalf("unlearned Air Blade changed hit: %+v", got)
	}
	ch.LearnedSkill = huntressAirBladeBit
	if got := applyHuntressAirBlade(hit, ch, 0, false, procRNG); got.Flank != 0 || got.Damage != hit.Damage {
		t.Fatalf("non-0x39E physical hit triggered Air Blade: %+v", got)
	}
	noProc := applyHuntressAirBlade(hit, ch, 0, true, func(int) int { return 1 })
	if noProc.Flank != 0 || noProc.Damage != hit.Damage {
		t.Fatalf("failed 25%% roll changed hit: %+v", noProc)
	}
}

func TestHuntressAirBladeAddsBonusBeforeDoubleHit(t *testing.T) {
	ch := &model.Char{
		Class:        3,
		LearnedSkill: huntressAirBladeBit,
		Score:        testScore(model.Score{Str: 1000, Mastery: [4]uint32{0, 0, 0, 500}}),
	}
	rng := func(int) int { return 0 }
	single := applyHuntressAirBlade(physicalHitResult{Damage: 1000, Hit: true}, ch, 0, true, rng)
	if single.Flank != 742 || single.Damage != 1742 || single.visualFlags()&4 == 0 {
		t.Fatalf("single Air Blade=%+v want flank=742 damage=1742 bit4", single)
	}
	double := applyHuntressAirBlade(physicalHitResult{Damage: 1000, Hit: true, Double: true}, ch, 0, true, rng)
	if double.Flank != 742 || double.Damage != 2484 {
		t.Fatalf("double Air Blade=%+v want (basePreDouble+742)*2=2484", double)
	}
}

func TestDoubleHitCountsFullServerProgression(t *testing.T) {
	// The native table reserves cursor zero as 512, so its nominal 50% band
	// contains 499 doubles in one complete 1,024-action cycle. At 250% every
	// table value (maximum 999) is below the 1,000 threshold.
	for speedNibble, want := range map[byte]int{0x50: 0, 0xA0: 499, 0xF0: 1024} {
		ch := &model.Char{Score: testScore(model.Score{AttackRun: uint32(speedNibble)})}
		progress := uint16(0)
		got := 0
		for range 1024 {
			double, _ := rollPhysicalHitFlags(ch, &progress, func(int) int { return 254 })
			if double {
				got++
			}
		}
		if got != want {
			t.Fatalf("AttackRun=%02X double hits=%d want=%d", speedNibble, got, want)
		}
	}
}

func TestAcceptClientAttackRejectsInternalAndFastTicks(t *testing.T) {
	pkt := make([]byte, 48)
	p := &Player{}
	now := time.Now()
	binary.LittleEndian.PutUint32(pkt[8:12], 0x0E0A1ACA)
	if acceptClientAttack(p, pkt, now) {
		t.Fatal("external SKIPCHECKTICK was accepted")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 1000)
	if !acceptClientAttack(p, pkt, now) {
		t.Fatal("first valid attack was rejected")
	}
	if acceptClientAttack(p, pkt, now.Add(time.Second)) {
		t.Fatal("replay of the same ClientTick was accepted")
	}
	// Physical cadence is fixed at 900 ms; Attack Speed feeds Double Hit.
	binary.LittleEndian.PutUint32(pkt[8:12], 1899)
	if acceptClientAttack(p, pkt, now.Add(time.Second)) {
		t.Fatal("tick below the fixed 900ms interval was accepted")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 1900)
	if acceptClientAttack(p, pkt, now.Add(899*time.Millisecond)) {
		t.Fatal("server time below the fixed 900ms interval was accepted")
	}
	if !acceptClientAttack(p, pkt, now.Add(time.Second)) {
		t.Fatal("attack at the 900ms interval was rejected")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 1300)
	if acceptClientAttack(p, pkt, now.Add(3*time.Second)) {
		t.Fatal("rewinding tick was accepted")
	}
}

func TestAcceptClientSkillIsIndependentFromPhysicalAttackCadence(t *testing.T) {
	attackPkt := make([]byte, 48)
	skillPkt := make([]byte, 48)
	now := time.Unix(10, 0)
	binary.LittleEndian.PutUint32(attackPkt[8:12], 1000)
	p := &Player{}
	if !acceptClientAttack(p, attackPkt, now) {
		t.Fatal("initial physical attack should be accepted")
	}
	// A spell arriving immediately after the swing is a separate action. The
	// old shared gate discarded it and produced one missing hit in short macro
	// rotations.
	binary.LittleEndian.PutUint32(skillPkt[8:12], 1200)
	if !acceptClientSkill(p, skillPkt, 33, now.Add(10*time.Millisecond)) {
		t.Fatal("valid skill was blocked by the last physical attack")
	}
}

func TestAcceptClientSkillRejectsReplayAndOnlyBusyLoops(t *testing.T) {
	p := &Player{}
	pkt := make([]byte, 48)
	now := time.Unix(20, 0)
	binary.LittleEndian.PutUint32(pkt[8:12], 2000)
	if !acceptClientSkill(p, pkt, 33, now) {
		t.Fatal("first cast should be accepted")
	}
	if acceptClientSkill(p, pkt, 33, now.Add(time.Second)) {
		t.Fatal("replay of the same tick was accepted")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 2100)
	if acceptClientSkill(p, pkt, 34, now.Add(50*time.Millisecond)) {
		t.Fatal("skill busy loop was accepted")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 2200)
	if !acceptClientSkill(p, pkt, 34, now.Add(skillPacketInterval)) {
		t.Fatal("cast after the flood floor should be accepted")
	}
}

func TestPhysicalAttackIntervalIsFixedAndSpeedFeedsDoubleChance(t *testing.T) {
	slow := &model.Char{Score: testScore(model.Score{AttackRun: 0x00})} // speed 0
	fast := &model.Char{Score: testScore(model.Score{AttackRun: 0xF0})} // speed 15
	slowInterval := attackIntervalFor(slow)
	fastInterval := attackIntervalFor(fast)
	if slowInterval != 900*time.Millisecond || fastInterval != slowInterval {
		t.Fatalf("physical intervals=%v/%v want fixed 900ms", slowInterval, fastInterval)
	}
	if got := doubleHitChance(slow); got != 0 {
		t.Fatalf("slow double chance=%d want=0", got)
	}
	if got := doubleHitChance(fast); got != 100 {
		t.Fatalf("250%% attack speed double chance=%d want=100", got)
	}
}

func TestAttackClockValidationDoesNotConsumeRejectedTargetIntent(t *testing.T) {
	pkt := make([]byte, 48)
	binary.LittleEndian.PutUint32(pkt[8:12], 1_000)
	p := &Player{}
	now := time.Unix(30, 0)
	clock, ok := validateClientAttackClock(p, pkt, now)
	if !ok {
		t.Fatal("initial clock should validate")
	}
	// Target validation happens between these two calls. If that target is dead,
	// absent, out of range or behind terrain, the clock is deliberately not
	// committed and the next legitimate target may reuse this action window.
	if p.LastAttackTick != 0 || !p.LastAttackAt.IsZero() {
		t.Fatalf("pure validation mutated player: tick=%d at=%v", p.LastAttackTick, p.LastAttackAt)
	}
	clock2, ok := validateClientAttackClock(p, pkt, now.Add(10*time.Millisecond))
	if !ok || clock2.tick != clock.tick {
		t.Fatal("uncommitted rejected intent consumed the physical action window")
	}
	commitClientAttackClock(p, clock2, now.Add(10*time.Millisecond))
	if p.LastAttackTick != 1_000 {
		t.Fatalf("committed tick=%d want=1000", p.LastAttackTick)
	}
}

func TestCourageAppliesFixedPvEBonusWithoutChangingScore(t *testing.T) {
	ch := &model.Char{
		Score:        testScore(model.Score{Attack: 500, MagicAttack: 700, Accuracy: 25}),
		RuntimeScore: testScore(model.Score{Attack: 500, MagicAttack: 700, Accuracy: 25}),
	}
	ch.Affects[0] = model.Affect{
		Type: affectCourage, ClientType: affectCourage, ExpiresAt: time.Now().Add(time.Hour),
	}

	if got := applyCouragePvEDamage(ch, 300, false); got != 1_300 {
		t.Fatalf("physical Courage=%d, want 1300", got)
	}
	if got := applyCouragePvEDamage(ch, 300, true); got != 2_300 {
		t.Fatalf("magic Courage=%d, want 2300", got)
	}
	if got := applyCouragePvEDamage(ch, 0, true); got != 0 {
		t.Fatalf("Courage turned a miss into a hit: %d", got)
	}

	w := &World{}
	w.applyExtendedAffectStats(ch)
	if ch.RuntimeScore.Attack != 500 || ch.RuntimeScore.MagicAttack != 700 ||
		ch.RuntimeScore.Accuracy != 25 {
		t.Fatalf("Courage changed the score: %+v", *ch.RuntimeScore)
	}
}

func TestCourageExpiredDoesNotApplyAndDamageIsClamped(t *testing.T) {
	ch := &model.Char{Score: testScore(model.Score{})}
	ch.Affects[0] = model.Affect{Type: affectCourage, ExpiresAt: time.Now().Add(-time.Second)}
	if got := applyCouragePvEDamage(ch, 300, true); got != 300 {
		t.Fatalf("expired Courage applied its bonus: %d", got)
	}

	ch.Affects[0].ExpiresAt = time.Now().Add(time.Hour)
	if got := applyCouragePvEDamage(ch, int(maxScoreValue)-500, true); got != int(maxScoreValue) {
		t.Fatalf("Courage exceeded the wide clamp: %d", got)
	}
}

func TestCourageIsNotAppliedByPvPDamagePipeline(t *testing.T) {
	attacker := &Player{Char: &model.Char{
		Score:        testScore(model.Score{Attack: 1_000, Dex: 1_000, Accuracy: 10_000}),
		RuntimeScore: testScore(model.Score{Attack: 1_000, Dex: 1_000, Accuracy: 10_000}),
	}}
	attacker.Char.Affects[0] = model.Affect{
		Type: affectCourage, ExpiresAt: time.Now().Add(time.Hour),
	}
	target := &Player{Char: &model.Char{
		Score:        testScore(model.Score{Defense: 100, Dex: 1}),
		RuntimeScore: testScore(model.Score{Defense: 100, Dex: 1}),
	}}

	// playerHitsPlayer is the PvP pipeline. Courage does not appear in it.
	for i := 0; i < 32; i++ {
		if damage := playerHitsPlayer(attacker, target); damage >= 2_000 {
			t.Fatalf("Courage leaked into PvP: damage=%d", damage)
		}
	}
}
