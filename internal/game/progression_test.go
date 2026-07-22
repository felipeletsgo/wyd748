package game

import "testing"

func TestGrantExpLevelsAndAwardsPoints(t *testing.T) {
	ch := testChar()
	ch.Extended.Level = 10
	ch.Exp = mortalNextLevel[11] - 10
	ch.Extended.SkillPts = 0
	syncProgression(&ch)
	beforeStatus := ch.Extended.StatusPts
	gained, applied := grantExp(&ch, 10)
	if gained != 1 || applied != 10 {
		t.Fatalf("gained=%d applied=%d", gained, applied)
	}
	if playerLevel(&ch) != 11 {
		t.Fatalf("level wide=%d", playerLevel(&ch))
	}
	if ch.Extended.StatusPts != beforeStatus+5 || ch.Extended.SkillPts != 33 {
		t.Fatalf("status=%d (antes %d), skill=%d", ch.Extended.StatusPts, beforeStatus, ch.Extended.SkillPts)
	}
	if ch.NextExp != mortalNextLevel[12] {
		t.Fatalf("nextExp=%d", ch.NextExp)
	}
}

func TestGrantExpCanCrossMultipleLevels(t *testing.T) {
	ch := testChar()
	ch.Extended.Level = 1
	ch.Exp = 0
	gained, applied := grantExp(&ch, mortalNextLevel[4])
	if gained != 3 || applied != mortalNextLevel[4] || playerLevel(&ch) != 4 {
		t.Fatalf("gained=%d applied=%d level=%d", gained, applied, playerLevel(&ch))
	}
}

func TestMortalExpTableIsCompleteAndIncreasing(t *testing.T) {
	if len(mortalNextLevel) != 401 {
		t.Fatalf("tabela possui %d marcos, esperado 401", len(mortalNextLevel))
	}
	if mortalNextLevel[0] != 0 || mortalNextLevel[400] != 4100000000 {
		t.Fatalf("limites incorretos: first=%d last=%d", mortalNextLevel[0], mortalNextLevel[400])
	}
	for i := 1; i < len(mortalNextLevel); i++ {
		if mortalNextLevel[i] <= mortalNextLevel[i-1] {
			t.Fatalf("tabela nao crescente em %d: %d <= %d", i, mortalNextLevel[i], mortalNextLevel[i-1])
		}
	}
}

func TestGrantExpReachesInternal399DisplayedAsLevel400(t *testing.T) {
	ch := testChar()
	ch.Extended.Level = 398
	ch.Exp = mortalNextLevel[399] - 1
	gained, applied := grantExp(&ch, 1)
	if gained != 1 || applied != 1 || playerLevel(&ch) != maxMortalLevel {
		t.Fatalf("gained=%d applied=%d level interno=%d", gained, applied, playerLevel(&ch))
	}
	if ch.NextExp != mortalNextLevel[400] {
		t.Fatalf("level maximo perdeu marco final: nextExp=%d", ch.NextExp)
	}
}

func TestGrantExpCapsAtFinalMarkerWithoutCreatingInternalLevel400(t *testing.T) {
	ch := testChar()
	ch.Extended.Level = maxMortalLevel
	ch.Exp = mortalNextLevel[400] - 50

	gained, applied := grantExp(&ch, 100)
	if gained != 0 || applied != 50 {
		t.Fatalf("gained=%d applied=%d, quer 0/50", gained, applied)
	}
	if ch.Extended.Level != maxMortalLevel || ch.Exp != mortalNextLevel[400] {
		t.Fatalf("progressao final incorreta: level=%d exp=%d", ch.Extended.Level, ch.Exp)
	}

	gained, applied = grantExp(&ch, 10_000)
	if gained != 0 || applied != 0 || ch.Extended.Level != maxMortalLevel {
		t.Fatalf("nivel interno 400 criado: gained=%d applied=%d level=%d",
			gained, applied, ch.Extended.Level)
	}
}
