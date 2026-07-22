package game

import (
	"testing"

	"wydgo/internal/model"
	netpkg "wydgo/internal/net"
)

func livingPlayer(id, x, y uint16) (*netpkg.Session, *Player) {
	s := &netpkg.Session{ID: int64(id)}
	return s, &Player{ID: id, InWorld: true, X: x, Y: y,
		Char: &model.Char{Extended: testExtended(model.ExtendedScore{MaxHP: 100, CurHP: 100})}}
}

func TestNearestLivingPlayerIgnoresDeadAndOutOfRange(t *testing.T) {
	s1, dead := livingPlayer(1, 101, 100)
	setPlayerCurHP(dead.Char, 0)
	s2, far := livingPlayer(2, 120, 120)
	s3, near := livingPlayer(3, 104, 103)
	w := &World{players: map[*netpkg.Session]*Player{s1: dead, s2: far, s3: near}}

	if got := w.nearestLivingPlayer(100, 100, 8); got != near {
		t.Fatalf("alvo=%v, esperado jogador vivo id=%d", got, near.ID)
	}
	if got := w.nearestLivingPlayer(100, 100, 2); got != nil {
		t.Fatalf("jogador fora do alcance foi adquirido: id=%d", got.ID)
	}
}

func TestMobLeashUsesGeneratorOrigin(t *testing.T) {
	m := &Mob{Segments: [model.MaxGenerSegments]model.GenerSegment{{X: 100, Y: 100}}}
	_, near := livingPlayer(1, 130, 100)
	_, far := livingPlayer(2, 131, 100)
	if !m.insideLeash(near) || m.insideLeash(far) {
		t.Fatal("limite de perseguicao nao respeitou os 30 tiles")
	}
}
