package game

import (
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func TestWorldOptionsCoverUxmalAndLoadtestSpawnCopies(t *testing.T) {
	w := &World{}
	uxmal := model.VolatileInstance{ID: "uxmal", Name: "original"}
	WithUxmal(uxmal)(w)
	uxmal.Name = "mutated"
	if w.uxmal == nil || w.uxmal.ID != "uxmal" || w.uxmal.Name != "original" {
		t.Fatalf("WithUxmal nao preservou copia: %+v", w.uxmal)
	}

	spawn := model.CharacterSpawn{X: 1162, Y: 1700}
	WithLoadtestSpawn(spawn, " BOT ")(w)
	if w.loadtestSpawn != spawn || w.loadtestAccountPrefix != "bot" {
		t.Fatalf("WithLoadtestSpawn divergente: spawn=%+v prefix=%q", w.loadtestSpawn, w.loadtestAccountPrefix)
	}
}

func TestPositionOccupiedWrapperHonorsExceptMob(t *testing.T) {
	def := &model.NPCDef{Name: "Monster", Tipo: model.TipoMonstro,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 10, CurHP: 10}}
	mob := &Mob{ID: 1000, Def: def, X: 100, Y: 101, HP: 10}
	key := spatialKey(mob.X, mob.Y)
	w := &World{mobCells: map[uint32]map[uint16]*Mob{key: {mob.ID: mob}}}
	if !w.positionOccupied(mob.X, mob.Y, nil) {
		t.Fatal("tile com mob nao foi marcado ocupado")
	}
	if w.positionOccupied(mob.X, mob.Y, mob) {
		t.Fatal("exceptMob continuou bloqueando o proprio tile")
	}
	if w.positionOccupied(200, 200, nil) {
		t.Fatal("tile vazio foi marcado ocupado")
	}
}

func TestFindWalkablePositionCoversBlockedOriginAndSearchRing(t *testing.T) {
	terrain := model.TerrainMap{
		Height: make([]byte, model.TerrainCells),
		Attribute: make([]byte, model.AttributeCells),
	}
	x, y := uint16(100), uint16(100)
	terrain.Height[int(y)*model.TerrainWidth+int(x)] = model.TerrainBlockedByte
	w := &World{terrain: terrain}
	gotX, gotY := w.findWalkablePosition(x, y, 0)
	if gotX == x && gotY == y {
		t.Fatalf("origem bloqueada nao foi corrigida: (%d,%d)", gotX, gotY)
	}
	if !terrain.Walkable(gotX, gotY) {
		t.Fatalf("substituto nao e caminhavel: (%d,%d)", gotX, gotY)
	}

	openX, openY := w.findWalkablePosition(200, 200, 1)
	if openX != 200 || openY != 200 {
		t.Fatalf("origem caminhavel foi deslocada: (%d,%d)", openX, openY)
	}
}

func TestScatterCoversZeroRadiusDeterministicOffsetAndLowerClamp(t *testing.T) {
	w := &World{rng: contractRNG{value: 0}}
	if x, y := w.scatter(50, 60, 0); x != 50 || y != 60 {
		t.Fatalf("scatter zero=(%d,%d)", x, y)
	}
	if x, y := w.scatter(10, 10, 2); x != 8 || y != 8 {
		t.Fatalf("scatter deterministico=(%d,%d), esperado 8,8", x, y)
	}
	if x, y := w.scatter(1, 1, 4); x != 1 || y != 1 {
		t.Fatalf("scatter nao aplicou clamp inferior: (%d,%d)", x, y)
	}
}

func TestRequeueCommandQueuesPreservesSessionOrderAheadOfExistingPending(t *testing.T) {
	s1 := gameNet.NewTestSession(1, 4)
	s2 := gameNet.NewTestSession(2, 4)
	w := &World{pendingCommands: []command{{s: s2, pkt: []byte{9}}}}
	queues := map[*gameNet.Session][]command{
		s1: {{s: s1, pkt: []byte{1}}, {s: s1, pkt: []byte{2}}},
		s2: {{s: s2, pkt: []byte{3}}},
	}
	w.requeueCommandQueues([]*gameNet.Session{s1, s2}, queues)
	if len(w.pendingCommands) != 4 || w.pendingCommands[0].pkt[0] != 1 ||
		w.pendingCommands[1].pkt[0] != 2 || w.pendingCommands[2].pkt[0] != 3 ||
		w.pendingCommands[3].pkt[0] != 9 {
		t.Fatalf("requeue perdeu ordem: %+v", w.pendingCommands)
	}

	before := len(w.pendingCommands)
	w.requeueCommandQueues([]*gameNet.Session{s1}, map[*gameNet.Session][]command{s1: nil})
	if len(w.pendingCommands) != before {
		t.Fatal("requeue vazio alterou pending")
	}
}

func TestPingIsExplicitNoOp(t *testing.T) {
	w := &World{}
	s := gameNet.NewTestSession(7, 4)
	w.onPing(s, []byte{1, 2, 3})
	if s.QueuedPacketsForTest() != 0 {
		t.Fatal("ping nativo nao deveria responder")
	}
}
