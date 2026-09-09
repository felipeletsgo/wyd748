package game

import "testing"

func TestHeldExperienceUsesCurrentLevelIntervalAndNativeDivisors(t *testing.T) {
	ch := testChar()
	ch.Score.Level = 10
	wantInterval := mortalNextLevel[11] - mortalNextLevel[10]
	if got := currentExperienceInterval(&ch); got != wantInterval {
		t.Fatalf("intervalo do level=%d, esperado %d", got, wantInterval)
	}
	if got := heldExperienceLimit(&ch); got != wantInterval/10 {
		t.Fatalf("limite Hold=%d, esperado %d", got, wantInterval/10)
	}

	tests := []struct {
		level   uint32
		divisor uint32
	}{
		{0, 20}, {29, 20}, {30, 22}, {40, 25}, {50, 30}, {60, 35},
		{70, 40}, {80, 45}, {90, 50}, {100, 55}, {150, 70},
		{200, 85}, {250, 100}, {399, 100},
	}
	for _, test := range tests {
		if got := heldExperienceDeathDivisor(test.level); got != test.divisor {
			t.Fatalf("divisor do level %d=%d, esperado %d", test.level, got, test.divisor)
		}
	}
}

func TestHeldExperienceDeathDebtCapsAndSaturates(t *testing.T) {
	ch := testChar()
	ch.Score.Level = 100
	ch.Exp = mortalNextLevel[100]
	limit := heldExperienceLimit(&ch)
	if limit == 0 || heldExperienceDeathDebt(&ch) == 0 {
		t.Fatalf("pre-condicao invalida: limit=%d debt=%d", limit, heldExperienceDeathDebt(&ch))
	}
	for range 1000 {
		addHeldExperienceDeathDebt(&ch)
	}
	if ch.Hold != limit {
		t.Fatalf("mortes repetidas deixaram Hold=%d, esperado cap=%d", ch.Hold, limit)
	}

	ch.Hold = ^uint32(0)
	addHeldExperienceDeathDebt(&ch)
	if ch.Hold != limit {
		t.Fatalf("soma saturada escapou do cap: Hold=%d cap=%d", ch.Hold, limit)
	}
	if got := saturatingAddU32(^uint32(0)-2, 3); got != ^uint32(0) {
		t.Fatalf("saturatingAddU32 retornou %d", got)
	}

	ch.Hold = 0
	ch.Exp = 3
	if got := heldExperienceDeathDebt(&ch); got != 3 {
		t.Fatalf("divida excedeu EXP disponivel: %d", got)
	}
}

func TestCombatExperiencePaysHoldBeforeProgression(t *testing.T) {
	ch := testChar()
	ch.Score.Level = 10
	ch.Exp = mortalNextLevel[10]
	ch.Hold = 120
	originalEXP := ch.Exp

	levels, applied := grantCombatExp(&ch, 50)
	if levels != 0 || applied != 0 || ch.Hold != 70 || ch.Exp != originalEXP {
		t.Fatalf("pagamento parcial incorreto: levels=%d applied=%d hold=%d exp=%d",
			levels, applied, ch.Hold, ch.Exp)
	}

	levels, applied = grantCombatExp(&ch, 100)
	if levels != 0 || applied != 30 || ch.Hold != 0 || ch.Exp != originalEXP+30 {
		t.Fatalf("pagamento total incorreto: levels=%d applied=%d hold=%d exp=%d",
			levels, applied, ch.Hold, ch.Exp)
	}
}

func TestNonCombatExperienceDoesNotPayHold(t *testing.T) {
	ch := testChar()
	ch.Score.Level = 10
	ch.Exp = mortalNextLevel[10]
	ch.Hold = 80
	originalEXP := ch.Exp

	_, applied := grantExp(&ch, 50)
	if applied != 50 || ch.Exp != originalEXP+50 || ch.Hold != 80 {
		t.Fatalf("recompensa nao-combate alterou Hold: applied=%d exp=%d hold=%d",
			applied, ch.Exp, ch.Hold)
	}
}

func TestHeldExperiencePenaltyHalvesAndRestoresRuntimeMaxHP(t *testing.T) {
	w := &World{}
	ch := testChar()
	ch.Score.Level = 100
	ch.Exp = mortalNextLevel[100]
	w.recalcPlayer(&ch)
	normalMaxHP := playerMaxHP(&ch)
	limit := heldExperienceLimit(&ch)
	threshold := (limit*4 + 4) / 5
	if normalMaxHP == 0 || threshold == 0 {
		t.Fatalf("pre-condicao invalida: hp=%d limit=%d", normalMaxHP, limit)
	}

	ch.Hold = threshold
	w.recalcPlayer(&ch)
	if got := playerMaxHP(&ch); got != normalMaxHP/2 {
		t.Fatalf("penalidade MaxHP=%d, esperado %d", got, normalMaxHP/2)
	}

	ch.Hold = threshold - 1
	w.recalcPlayer(&ch)
	if got := playerMaxHP(&ch); got != normalMaxHP {
		t.Fatalf("MaxHP nao restaurou: %d, esperado %d", got, normalMaxHP)
	}
}
