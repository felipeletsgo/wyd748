package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

// TestCharStateRoundTripsActiveBuffs cobre o objetivo do felipe: buffs (e moedas)
// sobrevivem ao relog. buildCharState descarta expirados; applyCharState restaura
// os que ainda valem num personagem novo.
func TestCharStateRoundTripsActiveBuffs(t *testing.T) {
	now := time.Now()
	p := &Player{Char: &model.Char{}, SpecialCoins: map[string]uint32{"wyden": 5}}
	p.Char.Affects[0] = model.Affect{Type: 4, ClientType: 4, Value: 25, ExpiresAt: now.Add(10 * time.Minute)}
	p.Char.Affects[1] = model.Affect{Type: 30, ExpiresAt: now.Add(-time.Minute)} // ja expirado

	state := buildCharState(p, now)
	if len(state.Affects) != 1 || state.Affects[0].Type != 4 || state.Affects[0].Value != 25 {
		t.Fatalf("buildCharState nao filtrou o affect expirado: %+v", state.Affects)
	}
	if state.SpecialCoins["wyden"] != 5 {
		t.Fatalf("moeda especial nao persistida: %+v", state.SpecialCoins)
	}

	fresh := &Player{Char: &model.Char{}}
	(&World{}).applyCharState(fresh, state, now)
	if a := activePlayerAffect(fresh.Char, 4); a == nil || a.Value != 25 {
		t.Fatalf("buff nao restaurado no relog: %+v", fresh.Char.Affects)
	}
	if fresh.SpecialCoins["wyden"] != 5 {
		t.Fatalf("moeda nao restaurada: %+v", fresh.SpecialCoins)
	}
}

// TestApplyCharStateDropsExpiredOnLoad garante que um buff cujo tempo ja passou
// entre o logout e o login NAO volta ativo.
func TestApplyCharStateDropsExpiredOnLoad(t *testing.T) {
	now := time.Now()
	state := &model.CharState{Version: model.CharStateVersion, Affects: []model.PersistedAffect{
		{Type: 4, Value: 10, ExpiresUnix: now.Add(5 * time.Minute).Unix()},
		{Type: 30, ExpiresUnix: now.Add(-time.Minute).Unix()},
	}}
	p := &Player{Char: &model.Char{}}
	(&World{}).applyCharState(p, state, now)
	if activePlayerAffect(p.Char, 4) == nil {
		t.Fatal("buff valido nao foi restaurado")
	}
	if activePlayerAffect(p.Char, 30) != nil {
		t.Fatal("buff expirado foi restaurado indevidamente")
	}
}
