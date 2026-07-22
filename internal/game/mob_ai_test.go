package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

func testSpatialWorld(mobs []*Mob, players ...*Player) *World {
	w := &World{
		mobs:           mobs,
		players:        make(map[*gameNet.Session]*Player),
		playersByID:    make(map[uint16]*Player),
		mobsByID:       make(map[uint16]*Mob),
		mobCells:       make(map[uint32]map[uint16]*Mob),
		playerCells:    make(map[uint32]map[uint16]*Player),
		mobCell:        make(map[uint16]uint32),
		playerCell:     make(map[uint16]uint32),
		activeMobs:     make(map[uint16]*Mob),
		summons:        make(map[uint16]*Mob),
		sephiraObjects: make(map[uint16]*Mob),
	}
	for _, m := range mobs {
		w.registerMobSpatial(m)
	}
	for _, p := range players {
		w.playersByID[p.ID] = p
		w.players[&gameNet.Session{}] = p
		w.updatePlayerSpatial(p)
	}
	return w
}

func TestRouteType2MovesAndReturns(t *testing.T) {
	m := &Mob{ID: 1000, X: 10, Y: 10, RouteType: 2, SegmentProgress: 0,
		Def: testNPCDef(model.ExtendedScore{AttackRun: 1})}
	m.Segments[0] = model.GenerSegment{X: 10, Y: 10}
	m.Segments[4] = model.GenerSegment{X: 12, Y: 12}
	w := testSpatialWorld([]*Mob{m})
	w.activeMobs[m.ID] = m

	now := time.Now()
	w.tickMobRoutes(now, 0, 1)
	if m.X != 12 || m.Y != 12 || m.SegmentProgress != 4 {
		t.Fatalf("primeiro trecho = (%d,%d) segment=%d", m.X, m.Y, m.SegmentProgress)
	}
	w.activeMobs[m.ID] = m
	w.tickMobRoutes(now.Add(time.Second), 0, 1)
	if m.X != 12 || m.Y != 12 {
		t.Fatalf("iniciou outro trecho antes de terminar o atual: (%d,%d)", m.X, m.Y)
	}
	w.activeMobs[m.ID] = m
	w.tickMobRoutes(now.Add(2*time.Second), 0, 1)
	if m.X != 10 || m.Y != 10 || m.SegmentDirection != 1 {
		t.Fatalf("nao iniciou retorno: (%d,%d) direction=%d", m.X, m.Y, m.SegmentDirection)
	}
}

func TestMobMoveDelayFollowsNativeSpeedRange(t *testing.T) {
	if got := mobMoveDelay(1); got != time.Second {
		t.Fatalf("speed 1 delay = %s", got)
	}
	if got := mobMoveDelay(3); got != time.Second/3 {
		t.Fatalf("speed 3 delay = %s", got)
	}
	if got := mobMoveDelay(0); got != time.Second {
		t.Fatalf("speed 0 deveria ser limitado a 1: %s", got)
	}
	if got := mobMoveDelay(15); got != time.Second/4 {
		t.Fatalf("speed 15 deveria ser limitado a 4: %s", got)
	}
}

func TestMobMovementUsesOneEightTileSegmentAtSpeedFour(t *testing.T) {
	m := &Mob{ID: 1000, X: 10, Y: 10,
		Def: testNPCDef(model.ExtendedScore{AttackRun: 6})}
	w := testSpatialWorld([]*Mob{m})
	now := time.Now()

	steps := w.moveMobToward(m, 30, 10, 0, now)
	if steps != 8 || m.X != 18 || m.Y != 10 {
		t.Fatalf("trecho incorreto: steps=%d pos=(%d,%d)", steps, m.X, m.Y)
	}
	if want := now.Add(2 * time.Second); !m.NextMove.Equal(want) {
		t.Fatalf("fim do trecho=%s, quer %s", m.NextMove, want)
	}
}

func TestMobCollisionDoesNotShareCoordinate(t *testing.T) {
	blocker := &Mob{ID: 1000, X: 11, Y: 11}
	mover := &Mob{ID: 1001, X: 10, Y: 10}
	w := testSpatialWorld([]*Mob{blocker, mover})
	x, y := w.freeStepAround(mover, 12, 12)
	if x == blocker.X && y == blocker.Y {
		t.Fatal("movimento escolheu coordenada ocupada")
	}
	if x == mover.X && y == mover.Y {
		t.Fatal("havia desvio livre, mas o mob nao se moveu")
	}
}

func TestFindFreeSpawnPosition(t *testing.T) {
	w := testSpatialWorld([]*Mob{{ID: 1000, X: 20, Y: 20}})
	x, y := w.findFreePosition(20, 20, 2)
	if x == 20 && y == 20 {
		t.Fatal("spawn permaneceu sobre coordenada ocupada")
	}
}

func TestFindFreeRecallPositionPushesPlayerSideways(t *testing.T) {
	blocker := &Player{ID: 1, InWorld: true, X: 2112, Y: 2088,
		Char: &model.Char{Extended: testExtended(model.ExtendedScore{MaxHP: 100, CurHP: 100})}}
	recalling := &Player{ID: 2, InWorld: true, X: 2200, Y: 2100,
		Char: &model.Char{Extended: testExtended(model.ExtendedScore{MaxHP: 200, CurHP: 200})}}
	w := testSpatialWorld(nil, blocker)

	x, y := w.findFreePlayerPosition(2112, 2088, 8, recalling)
	if x != 2113 || y != 2088 {
		t.Fatalf("recall livre esperado ao lado direito (2113,2088), recebeu (%d,%d)", x, y)
	}
}

func TestFindFreeRecallPositionIgnoresTheRecallingPlayer(t *testing.T) {
	recalling := &Player{ID: 2, InWorld: true, X: 2112, Y: 2088,
		Char: &model.Char{Extended: testExtended(model.ExtendedScore{MaxHP: 200, CurHP: 200})}}
	w := testSpatialWorld(nil, recalling)

	x, y := w.findFreePlayerPosition(2112, 2088, 8, recalling)
	if x != 2112 || y != 2088 {
		t.Fatalf("o proprio jogador deslocou o recall para (%d,%d)", x, y)
	}
}
