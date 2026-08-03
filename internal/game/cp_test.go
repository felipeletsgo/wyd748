package game

import (
	"errors"
	"testing"
	"time"
)

func TestNativeCPKillDeltaAndClamp(t *testing.T) {
	if got := nativeCPKillDelta(0); got != -3 {
		t.Fatalf("matar neutro deveria custar 3 CP, veio %d", got)
	}
	if got := nativeCPKillDelta(-75); got != 0 {
		t.Fatalf("matar CP -75 nao deveria custar CP, veio %d", got)
	}
	if got := nativeCPKillDelta(75); got != -3 {
		t.Fatalf("matar CP +75 deveria custar 3 CP, veio %d", got)
	}
}

func TestPvPKillPersistsSignedCPAtomically(t *testing.T) {
	killer, _ := networkedTestPlayer(1, "Killer", 2100, 2100)
	victim, _ := networkedTestPlayer(2, "Victim", 2101, 2100)
	killer.Char.CP = 75
	victim.Char.CP = -10
	w, st := guildFlowWorld(killer, victim)

	w.applyPvPKills(killer, victim)
	if killer.Char.CP != 72 || victim.Char.CP != -9 || st.gameSaves != 1 {
		t.Fatalf("CP incorreto killer=%d victim=%d saves=%d",
			killer.Char.CP, victim.Char.CP, st.gameSaves)
	}
}

func TestPvPKillRollsBackCPWhenPersistenceFails(t *testing.T) {
	killer, _ := networkedTestPlayer(1, "Killer", 2100, 2100)
	victim, _ := networkedTestPlayer(2, "Victim", 2101, 2100)
	killer.Char.CP = 75
	victim.Char.CP = -10
	w, st := guildFlowWorld(killer, victim)
	st.err = errors.New("database unavailable")

	w.applyPvPKills(killer, victim)
	if killer.Char.CP != 75 || victim.Char.CP != -10 {
		t.Fatalf("rollback falhou killer=%d victim=%d", killer.Char.CP, victim.Char.CP)
	}
}

func TestChaosRecoveryRaisesOnlyNegativeCP(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Recover", 2100, 2100)
	w, st := guildFlowWorld(p)
	now := time.Unix(100, 0)
	p.NextCPRecovery = now
	p.Char.CP = -2
	w.tickChaosRecovery(now)
	if p.Char.CP != -1 || st.saves != 1 {
		t.Fatalf("recuperacao incorreta cp=%d saves=%d", p.Char.CP, st.saves)
	}

	p.NextCPRecovery = now
	p.Char.CP = 0
	w.tickChaosRecovery(now)
	if p.Char.CP != 0 || st.saves != 1 {
		t.Fatalf("CP neutro foi alterado cp=%d saves=%d", p.Char.CP, st.saves)
	}
}
