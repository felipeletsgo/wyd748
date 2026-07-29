package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestParryRateUsesNativeDexTiersAndCaps(t *testing.T) {
	if got := parryRate(500, 200, 50); got != 300 { // equipamento limita em 100
		t.Fatalf("parry inicial=%d, quer 300", got)
	}
	if got := parryRate(2000, 20, 100); got != 650 { // 500 + 250 + 20 - 100
		t.Fatalf("parry intermediario=%d, quer limite 650", got)
	}
	if got := parryRate(10, 0, 999); got != 1 {
		t.Fatalf("parry minimo=%d, quer 1", got)
	}
}

func TestCriticalUsesServerOwned1024Progression(t *testing.T) {
	progress := uint16(0)
	for i := 0; i < 4; i++ {
		if criticalHit(128, &progress) {
			t.Fatalf("critico inesperado no cursor %d", i)
		}
	}
	if !criticalHit(128, &progress) { // tabela[4] = 1, threshold = 512
		t.Fatal("progressao nao produziu o critico esperado")
	}
	if progress != 5 {
		t.Fatalf("cursor=%d, quer 5", progress)
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
	// Char sem Extended = velocidade 0 = piso de 900ms (attackIntervalFor).
	binary.LittleEndian.PutUint32(pkt[8:12], 1500)
	if acceptClientAttack(p, pkt, now.Add(time.Second)) {
		t.Fatal("tick abaixo do intervalo (speed 0 = 900ms) foi aceito")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 1900)
	if !acceptClientAttack(p, pkt, now.Add(time.Second)) {
		t.Fatal("ataque no intervalo (900ms, speed 0) foi rejeitado")
	}
	binary.LittleEndian.PutUint32(pkt[8:12], 1400)
	if acceptClientAttack(p, pkt, now.Add(2*time.Second)) {
		t.Fatal("tick retrocedendo foi aceito")
	}
}

// TestAttackIntervalScalesWithSpeed garante que um char mais rapido tem um piso
// de ataque menor -- o bug do felipe (velocidade travada em ~1 golpe/s).
func TestAttackIntervalScalesWithSpeed(t *testing.T) {
	slow := &model.Char{Extended: testExtended(model.ExtendedScore{AttackRun: 0x00})} // speed 0
	fast := &model.Char{Extended: testExtended(model.ExtendedScore{AttackRun: 0xF0})} // speed 15
	slowMs := attackIntervalFor(slow) / time.Millisecond
	fastMs := attackIntervalFor(fast) / time.Millisecond
	if slowMs != 900 {
		t.Fatalf("velocidade 0 deveria dar 900ms, deu %d", slowMs)
	}
	if fastMs >= slowMs || fastMs > 500 {
		t.Fatalf("velocidade 15 deveria permitir ~2 golpes/s (<=500ms), deu %d", fastMs)
	}
}

func TestCourageAppliesFixedPvEBonusWithoutChangingScore(t *testing.T) {
	ch := &model.Char{
		Extended:        testExtended(model.ExtendedScore{Attack: 500, MagicAttack: 700, Accuracy: 25}),
		ExtendedRuntime: testExtended(model.ExtendedScore{Attack: 500, MagicAttack: 700, Accuracy: 25}),
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
	if ch.ExtendedRuntime.Attack != 500 || ch.ExtendedRuntime.MagicAttack != 700 ||
		ch.ExtendedRuntime.Accuracy != 25 {
		t.Fatalf("Courage alterou score: %+v", *ch.ExtendedRuntime)
	}
}

func TestCourageExpiredDoesNotApplyAndDamageIsClamped(t *testing.T) {
	ch := &model.Char{Extended: testExtended(model.ExtendedScore{})}
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
		Extended:        testExtended(model.ExtendedScore{Attack: 1_000, Dex: 1_000, Accuracy: 10_000}),
		ExtendedRuntime: testExtended(model.ExtendedScore{Attack: 1_000, Dex: 1_000, Accuracy: 10_000}),
	}}
	attacker.Char.Affects[0] = model.Affect{
		Type: affectCourage, ExpiresAt: time.Now().Add(time.Hour),
	}
	target := &Player{Char: &model.Char{
		Extended:        testExtended(model.ExtendedScore{Defense: 100, Dex: 1}),
		ExtendedRuntime: testExtended(model.ExtendedScore{Defense: 100, Dex: 1}),
	}}

	// playerHitsPlayer e o pipeline PvP. Courage nao aparece nele.
	for i := 0; i < 32; i++ {
		if damage := playerHitsPlayer(attacker, target); damage >= 2_000 {
			t.Fatalf("Courage vazou para PvP: dano=%d", damage)
		}
	}
}
