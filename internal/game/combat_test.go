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

func TestPhysicalFlagsUseW2PPDoubleProgressionAndIndependentCritical(t *testing.T) {
	ch := &model.Char{Score: testExtended(model.Score{AttackRun: 0xF0, Critical: 255})}
	progress := uint16(0)
	double, critical := rollPhysicalHitFlags(ch, &progress, func(int) int { return 254 })
	if !double || !critical {
		t.Fatalf("flags at 200%%/255 double=%v critical=%v want both", double, critical)
	}
	if progress != 1 {
		t.Fatalf("progress=%d want=1", progress)
	}

	ch.Score.AttackRun = 0x50 // 100% Attack Speed: no Double Hit chance.
	ch.Score.Critical = 0
	double, critical = rollPhysicalHitFlags(ch, &progress, func(int) int { return 0 })
	if double || critical {
		t.Fatalf("flags at 100%%/0 double=%v critical=%v want neither", double, critical)
	}
}

func TestDoubleHitCountsFullServerProgression(t *testing.T) {
	// The native table reserves cursor zero as 512, so its nominal 50% band
	// contains 499 doubles in one complete 1,024-action cycle. At 200% every
	// table value (maximum 999) is below the 1,000 threshold.
	for speedNibble, want := range map[byte]int{0x50: 0, 0xA0: 499, 0xF0: 1024} {
		ch := &model.Char{Score: testExtended(model.Score{AttackRun: uint32(speedNibble)})}
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
		t.Fatal("SKIPCHECKTICK externo foi aceito")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 1000)
	if !acceptClientAttack(p, pkt, now) {
		t.Fatal("primeiro ataque valido foi rejeitado")
	}
	if acceptClientAttack(p, pkt, now.Add(time.Second)) {
		t.Fatal("replay do mesmo ClientTick foi aceito")
	}
	// Physical cadence is fixed at 400 ms; Attack Speed feeds Double Hit.
	binary.LittleEndian.PutUint32(pkt[8:12], 1399)
	if acceptClientAttack(p, pkt, now.Add(time.Second)) {
		t.Fatal("tick abaixo do intervalo fixo de 400ms foi aceito")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 1400)
	if !acceptClientAttack(p, pkt, now.Add(time.Second)) {
		t.Fatal("ataque no intervalo de 400ms foi rejeitado")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 1300)
	if acceptClientAttack(p, pkt, now.Add(2*time.Second)) {
		t.Fatal("tick retrocedendo foi aceito")
	}
}

func TestAcceptClientSkillIsIndependentFromPhysicalAttackCadence(t *testing.T) {
	attackPkt := make([]byte, 48)
	skillPkt := make([]byte, 48)
	now := time.Unix(10, 0)
	binary.LittleEndian.PutUint32(attackPkt[8:12], 1000)
	p := &Player{}
	if !acceptClientAttack(p, attackPkt, now) {
		t.Fatal("ataque fisico inicial deveria ser aceito")
	}
	// A spell arriving immediately after the swing is a separate action. The
	// old shared gate discarded it and produced one missing hit in short macro
	// rotations.
	binary.LittleEndian.PutUint32(skillPkt[8:12], 1200)
	if !acceptClientSkill(p, skillPkt, 33, now.Add(10*time.Millisecond)) {
		t.Fatal("skill valida foi bloqueada pelo ultimo ataque fisico")
	}
}

func TestAcceptClientSkillRejectsReplayAndOnlyBusyLoops(t *testing.T) {
	p := &Player{}
	pkt := make([]byte, 48)
	now := time.Unix(20, 0)
	binary.LittleEndian.PutUint32(pkt[8:12], 2000)
	if !acceptClientSkill(p, pkt, 33, now) {
		t.Fatal("primeiro cast deveria ser aceito")
	}
	if acceptClientSkill(p, pkt, 33, now.Add(time.Second)) {
		t.Fatal("replay do mesmo tick foi aceito")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 2100)
	if acceptClientSkill(p, pkt, 34, now.Add(50*time.Millisecond)) {
		t.Fatal("busy-loop de skills foi aceito")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 2200)
	if !acceptClientSkill(p, pkt, 34, now.Add(skillPacketInterval)) {
		t.Fatal("cast apos o piso de flood deveria ser aceito")
	}
}

func TestPhysicalAttackIntervalIsFixedAndSpeedFeedsDoubleChance(t *testing.T) {
	slow := &model.Char{Score: testExtended(model.Score{AttackRun: 0x00})} // speed 0
	fast := &model.Char{Score: testExtended(model.Score{AttackRun: 0xF0})} // speed 15
	slowInterval := attackIntervalFor(slow)
	fastInterval := attackIntervalFor(fast)
	if slowInterval != 400*time.Millisecond || fastInterval != slowInterval {
		t.Fatalf("physical intervals=%v/%v want fixed 400ms", slowInterval, fastInterval)
	}
	if got := doubleHitChance(slow); got != 0 {
		t.Fatalf("slow double chance=%d want=0", got)
	}
	if got := doubleHitChance(fast); got != 100 {
		t.Fatalf("200%% attack speed double chance=%d want=100", got)
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
		Score:        testExtended(model.Score{Attack: 500, MagicAttack: 700, Accuracy: 25}),
		RuntimeScore: testExtended(model.Score{Attack: 500, MagicAttack: 700, Accuracy: 25}),
	}
	ch.Affects[0] = model.Affect{
		Type: affectCourage, ClientType: affectCourage, ExpiresAt: time.Now().Add(time.Hour),
	}

	if got := applyCouragePvEDamage(ch, 300, false); got != 1_300 {
		t.Fatalf("Courage fisico=%d, quer 1300", got)
	}
	if got := applyCouragePvEDamage(ch, 300, true); got != 2_300 {
		t.Fatalf("Courage magico=%d, quer 2300", got)
	}
	if got := applyCouragePvEDamage(ch, 0, true); got != 0 {
		t.Fatalf("Courage transformou erro em hit: %d", got)
	}

	w := &World{}
	w.applyExtendedAffectStats(ch)
	if ch.RuntimeScore.Attack != 500 || ch.RuntimeScore.MagicAttack != 700 ||
		ch.RuntimeScore.Accuracy != 25 {
		t.Fatalf("Courage alterou score: %+v", *ch.RuntimeScore)
	}
}

func TestCourageExpiredDoesNotApplyAndDamageIsClamped(t *testing.T) {
	ch := &model.Char{Score: testExtended(model.Score{})}
	ch.Affects[0] = model.Affect{Type: affectCourage, ExpiresAt: time.Now().Add(-time.Second)}
	if got := applyCouragePvEDamage(ch, 300, true); got != 300 {
		t.Fatalf("Courage expirado aplicou bonus: %d", got)
	}

	ch.Affects[0].ExpiresAt = time.Now().Add(time.Hour)
	if got := applyCouragePvEDamage(ch, int(maxExtendedStat)-500, true); got != int(maxExtendedStat) {
		t.Fatalf("Courage ultrapassou clamp wide: %d", got)
	}
}

func TestCourageIsNotAppliedByPvPDamagePipeline(t *testing.T) {
	attacker := &Player{Char: &model.Char{
		Score:        testExtended(model.Score{Attack: 1_000, Dex: 1_000, Accuracy: 10_000}),
		RuntimeScore: testExtended(model.Score{Attack: 1_000, Dex: 1_000, Accuracy: 10_000}),
	}}
	attacker.Char.Affects[0] = model.Affect{
		Type: affectCourage, ExpiresAt: time.Now().Add(time.Hour),
	}
	target := &Player{Char: &model.Char{
		Score:        testExtended(model.Score{Defense: 100, Dex: 1}),
		RuntimeScore: testExtended(model.Score{Defense: 100, Dex: 1}),
	}}

	// playerHitsPlayer e o pipeline PvP. Courage nao aparece nele.
	for i := 0; i < 32; i++ {
		if damage := playerHitsPlayer(attacker, target); damage >= 2_000 {
			t.Fatalf("Courage vazou para PvP: dano=%d", damage)
		}
	}
}
