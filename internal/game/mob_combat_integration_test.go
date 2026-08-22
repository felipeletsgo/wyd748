package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func combatMob(id, x, y uint16) *Mob {
	def := testNPCDef(model.Score{
		Level: 10, MaxHP: 1000, CurHP: 1000,
		Attack: 500, Defense: 10, AttackRun: 4,
	})
	def.Name = "CombatMob"
	m := &Mob{ID: id, Def: def, X: x, Y: y, HP: 1000, Awake: true}
	m.Segments[0] = model.GenerSegment{X: x, Y: y}
	return m
}

func TestActiveMobAcquiresChasesAndAttacksNearbyPlayer(t *testing.T) {
	p, session := networkedTestPlayer(1, "Target", 103, 100)
	p.Char.Score.Defense = 0
	p.Char.Score.Dex = 0
	applyExtendedScore(p.Char)
	m := combatMob(1000, 100, 100)
	w := worldWithNetworkedPlayers(p)
	w.rng = fixedRNG{value: 0}
	w.registerMobSpatial(m)
	w.mobs = append(w.mobs, m)
	w.activeMobs[m.ID] = m
	p.show(m.ID)

	now := time.Date(2026, 7, 26, 18, 0, 0, 0, time.UTC)
	w.tickMobCombat(now, 0, 1, true)
	if m.TargetID != p.ID {
		t.Fatalf("mob adquiriu alvo=%d, quer %d", m.TargetID, p.ID)
	}
	if chebyshev(m.X, m.Y, p.X, p.Y) > mobAttackRange {
		t.Fatalf("mob nao perseguiu ate alcance: mob=(%d,%d) player=(%d,%d)", m.X, m.Y, p.X, p.Y)
	}

	hpBefore := playerCurHP(p.Char)
	w.tickActiveMobActions(m.NextMove)
	if hp := playerCurHP(p.Char); hp >= hpBefore {
		t.Fatalf("ataque ativo nao reduziu HP: %d -> %d", hpBefore, hp)
	}
	if m.NextAttack.IsZero() || p.LastAttackerID != m.ID {
		t.Fatal("ataque nao registrou cooldown/agressor")
	}
	if session.QueuedPacketsForTest() == 0 {
		t.Fatal("ataque nao publicou animacao/vitais")
	}
}

func TestMobCannotAttackAcrossIncompatibleTerrain(t *testing.T) {
	p, session := networkedTestPlayer(1, "Target", 101, 100)
	p.Char.Score.Defense = 0
	p.Char.Score.Dex = 0
	applyExtendedScore(p.Char)
	m := combatMob(1000, 100, 100)
	m.TargetID = p.ID
	w := worldWithNetworkedPlayers(p)
	w.rng = fixedRNG{value: 0}
	w.terrain = loadedFlatTerrain()
	w.terrain.Height[100*model.TerrainWidth+101] = 20 // walkable, but over the native MH=8 edge
	w.registerMobSpatial(m)
	w.mobs = append(w.mobs, m)
	w.activeMobs[m.ID] = m
	p.show(m.ID)

	hpBefore := playerCurHP(p.Char)
	w.tickActiveMobActions(time.Unix(1, 0))
	if hp := playerCurHP(p.Char); hp != hpBefore {
		t.Fatalf("mob hit through incompatible terrain: %d -> %d", hpBefore, hp)
	}
	if !m.NextAttack.IsZero() {
		t.Fatal("blocked hit consumed the mob attack deadline")
	}
	if session.QueuedPacketsForTest() != 0 {
		t.Fatal("blocked hit published a combat packet")
	}
}

func TestMobCombatSleepsWithoutPlayersAndReturnsHomeOutsideLeash(t *testing.T) {
	m := combatMob(1000, 100, 100)
	w := testSpatialWorld([]*Mob{m})
	w.activeMobs[m.ID] = m
	w.tickMobCombat(time.Now(), 0, 1, true)
	if m.Awake || m.TargetID != 0 {
		t.Fatal("mob sem jogadores nao voltou a dormir")
	}
	if _, active := w.activeMobs[m.ID]; active {
		t.Fatal("mob dormindo permaneceu no conjunto ativo")
	}

	p, _ := networkedTestPlayer(1, "Nearby", 120, 100)
	m = combatMob(1001, 120, 100)
	m.Segments[0] = model.GenerSegment{X: 100, Y: 100}
	w = worldWithNetworkedPlayers(p)
	w.registerMobSpatial(m)
	w.activeMobs[m.ID] = m
	w.tickMobCombat(time.Now(), 0, 1, true)
	if m.TargetID != 0 || m.X >= 120 {
		t.Fatalf("mob fora do leash nao iniciou retorno: target=%d pos=(%d,%d)", m.TargetID, m.X, m.Y)
	}
}

func TestMobCombatRespectsShardMovementFlagAndHiddenTargets(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Hidden", 101, 100)
	p.Char.Affects[0] = model.Affect{Type: 28, ExpiresAt: time.Now().Add(time.Minute)}
	m := combatMob(1001, 100, 100)
	w := worldWithNetworkedPlayers(p)
	w.registerMobSpatial(m)
	w.activeMobs[m.ID] = m

	w.tickMobCombat(time.Now(), 0, 2, true) // ID impar pertence ao shard 1.
	if m.TargetID != 0 {
		t.Fatal("shard incorreto processou o mob")
	}
	w.tickMobCombat(time.Now(), 1, 2, true)
	if m.TargetID != 0 {
		t.Fatal("mob adquiriu jogador oculto/morto")
	}

	p.Char.Affects[0] = model.Affect{}
	p.X, p.Y = 103, 100
	w.updatePlayerSpatial(p)
	oldX, oldY := m.X, m.Y
	w.tickMobCombat(time.Now(), 1, 2, false)
	if m.TargetID != p.ID {
		t.Fatal("percepcao deveria funcionar mesmo com movimento desativado")
	}
	if m.X != oldX || m.Y != oldY {
		t.Fatal("shard sem permissao de movimento moveu o mob")
	}
}

func TestMobActivityAndHomeLeashHelpers(t *testing.T) {
	w := newZoneTestWorld()
	m := combatMob(1000, 100, 100)
	if !m.insideHomeLeash() {
		t.Fatal("mob na origem deveria estar dentro do leash")
	}
	m.X = 117
	if m.insideHomeLeash() {
		t.Fatal("mob a 17 tiles deveria estar fora do leash")
	}
	w.activeMobs[m.ID] = m
	if _, active := w.activeMobs[m.ID]; !active {
		t.Fatal("indice ativo nao foi reconhecido")
	}
	var nilMob *Mob
	if nilMob.insideHomeLeash() {
		t.Fatal("mob nil nao pode estar dentro do leash")
	}
}
