package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestCleansePlayerRemovesOnlyNegativeAffects(t *testing.T) {
	now := time.Now().Add(time.Minute)
	ch := &model.Char{}
	for i, typ := range []byte{1, 3, 5, 7, 10, 12, 20, 22, 4, 17} {
		ch.Affects[i] = model.Affect{Type: typ, ExpiresAt: now}
	}
	if !cleansePlayer(ch) {
		t.Fatal("cleanse nao informou alteracao")
	}
	for i := 0; i < 8; i++ {
		if ch.Affects[i].Type != 0 {
			t.Fatalf("debuff no slot %d sobreviveu: %d", i, ch.Affects[i].Type)
		}
	}
	if ch.Affects[8].Type != 4 || ch.Affects[9].Type != 17 {
		t.Fatal("cleanse removeu buffs positivos")
	}
	if cleansePlayer(&model.Char{}) {
		t.Fatal("cleanse vazio informou alteracao")
	}
}

func TestMobSkillEffectsChangeEffectiveCombatStats(t *testing.T) {
	w, owner, _ := handlerTestWorld(t)
	mob := &Mob{
		ID: 1400, X: owner.X + 1, Y: owner.Y, HP: 1000,
		Def: testNPCDef(model.Score{
			Attack: 500, Defense: 400, AttackRun: 0x64,
			MaxHP: 1000, CurHP: 1000,
			ResistFire: 50, ResistIce: 40, ResistHoly: 30, ResistThunder: 20,
		}),
	}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	owner.show(mob.ID)

	w.applySkillMobEffects(owner, mob, model.SkillDef{
		AffectType: 3, AffectValue: 15, AffectTime: 5,
		TickType: 12, TickValue: 25,
	}, 40)
	if activeMobAffect(mob, 3) == nil || activeMobAffect(mob, 12) == nil {
		t.Fatalf("affects nao aplicados: %+v", mob.Affects)
	}
	resists := effectiveMobResistances(mob)
	if resists.Fire != 35 || resists.Ice != 25 || resists.Sacred != 15 || resists.Thunder != 5 {
		t.Fatalf("resistencias efetivas incorretas: %+v", resists)
	}
	if got := effectiveMobDefense(mob); got != 300 {
		t.Fatalf("defesa efetiva=%d, quer 300", got)
	}

	setPlayerOwnedMobAffect(mob, owner, 10, 20, 40, 5)
	setPlayerOwnedMobAffect(mob, owner, 1, 2, 40, 5)
	if got := effectiveMobAttack(mob); got != 472 {
		t.Fatalf("ataque efetivo=%d, quer 472", got)
	}
	if got := effectiveMobAttackRun(mob) & 0x0f; got != 2 {
		t.Fatalf("movimento efetivo=%d, quer 2", got)
	}
}

func TestTickMobAffectsDamagesAndExpires(t *testing.T) {
	w, owner, _ := handlerTestWorld(t)
	now := time.Now()
	mob := &Mob{
		ID: 1401, X: owner.X + 1, Y: owner.Y, HP: 1000,
		Def: testNPCDef(model.Score{MaxHP: 1000, CurHP: 1000, MaxMP: 100}),
	}
	mob.Affects[0] = model.Affect{
		Type: 20, Level: 20, Value: 5,
		ExpiresAt: now.Add(time.Minute), NextTick: now.Add(-time.Second),
	}
	mob.Affects[0].OwnerID = owner.ID
	mob.Affects[0].OwnerCharacterUID = owner.Char.UID
	mob.Affects[1] = model.Affect{Type: 3, ExpiresAt: now.Add(-time.Second)}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	w.activeMobs[mob.ID] = mob
	owner.show(mob.ID)

	w.tickMobAffects(now, 0, 1)
	if mob.HP != 985 {
		t.Fatalf("tick de veneno causou HP=%d, quer 985", mob.HP)
	}
	if mob.Affects[1].Type != 0 || mob.Affects[1].OwnerID != 0 || mob.Affects[1].OwnerCharacterUID != "" {
		t.Fatal("affect expirado do mob nao foi limpo")
	}
	if !mob.Affects[0].NextTick.After(now) {
		t.Fatal("proximo tick do veneno nao foi reagendado")
	}
}

func TestTickPlayerAffectsRegenPoisonAreaAndExpiration(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	// Expiracao e estado economico: o sidecar deve ser salvo imediatamente para
	// que um relog rapido nao ressuscite o buff removido pelo tick.
	stateStore := &charStateMemoryStore{}
	w.store = stateStore
	now := time.Now()
	p.Char.Score.CurHP = 500
	p.Char.Score.MaxHP = 1000
	applyScore(p.Char)
	p.Char.Affects[0] = model.Affect{
		Type: 17, Level: 40, Value: 10,
		ExpiresAt: now.Add(time.Minute), NextTick: now.Add(-time.Second),
	}
	p.Char.Affects[1] = model.Affect{
		Type: 20, Level: 10, Value: 5,
		ExpiresAt: now.Add(time.Minute), NextTick: now.Add(-time.Second), OwnerID: p.ID,
		OwnerCharacterUID: p.Char.UID,
	}
	p.Char.Affects[2] = model.Affect{
		Type: 22, Level: 20, Value: 10,
		ExpiresAt: now.Add(time.Minute), NextTick: now.Add(-time.Second),
	}
	p.Char.Affects[3] = model.Affect{Type: 4, ExpiresAt: now.Add(-time.Second)}
	mob := &Mob{
		ID: 1402, X: p.X + 1, Y: p.Y, HP: 1000,
		Def: testNPCDef(model.Score{MaxHP: 1000, CurHP: 1000, MaxMP: 100}),
	}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	p.show(mob.ID)

	w.tickPlayerAffects(now)
	// Aura +30 e veneno -10 no mesmo ciclo.
	if playerCurHP(p.Char) != 520 {
		t.Fatalf("ticks de HP resultaram em %d, quer 520", playerCurHP(p.Char))
	}
	if mob.HP != 970 {
		t.Fatalf("area persistente causou HP=%d, quer 970", mob.HP)
	}
	if p.Char.Affects[3].Type != 0 {
		t.Fatal("affect expirado do jogador nao foi removido")
	}
	if stateStore.asyncSaves != 1 || stateStore.state == nil {
		t.Fatalf("expiracao nao enfileirou charstate: saves=%d state=%+v",
			stateStore.asyncSaves, stateStore.state)
	}
	for _, persisted := range stateStore.state.Affects {
		if persisted.Type == 4 {
			t.Fatal("affect expirado foi gravado novamente no sidecar")
		}
	}
}

func TestTickAreaDamageKillsAtMostSixTargets(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	affect := &model.Affect{Level: 100, Value: 100}
	all := make([]*Mob, 0, 8)
	for i := 0; i < 8; i++ {
		mob := &Mob{
			ID: uint16(1500 + i), X: p.X + uint16(i%3), Y: p.Y + uint16(i/3),
			HP: 100, GenerIndex: -1,
			Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100, MaxMP: 10}),
		}
		all = append(all, mob)
		w.mobs = append(w.mobs, mob)
		w.registerMobSpatial(mob)
		p.show(mob.ID)
	}

	w.tickAreaDamageAffect(p, affect, 37)
	dead := 0
	for _, mob := range all {
		if mob.Dead {
			dead++
		}
	}
	if dead != 6 {
		t.Fatalf("tick de area matou %d alvos, quer limite 6", dead)
	}
	if st.saves == 0 {
		t.Fatal("lote de mortes do affect nao foi persistido")
	}
}
