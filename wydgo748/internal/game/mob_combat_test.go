package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	netpkg "wydgo/internal/net"
)

func livingPlayer(id, x, y uint16) (*netpkg.Session, *Player) {
	s := &netpkg.Session{ID: int64(id)}
	return s, &Player{ID: id, InWorld: true, X: x, Y: y,
		Char: &model.Char{Score: testScore(model.Score{MaxHP: 100, CurHP: 100})}}
}

func TestNearestLivingPlayerIgnoresDeadAndOutOfRange(t *testing.T) {
	s1, dead := livingPlayer(1, 101, 100)
	setPlayerCurHP(dead.Char, 0)
	s2, far := livingPlayer(2, 120, 120)
	s3, near := livingPlayer(3, 104, 103)
	w := testSpatialWorld(nil, dead, far, near)
	// Preserve as sessoes nomeadas desta fixture; o indice espacial e a fonte
	// usada pela busca, mas o mapa de sessoes continua representando o World.
	w.players = map[*netpkg.Session]*Player{s1: dead, s2: far, s3: near}

	if got := w.nearestLivingPlayer(100, 100, 8); got != near {
		t.Fatalf("alvo=%v, esperado jogador vivo id=%d", got, near.ID)
	}
	if got := w.nearestLivingPlayer(100, 100, 2); got != nil {
		t.Fatalf("jogador fora do alcance foi adquirido: id=%d", got.ID)
	}
}

// Mystic Land generator 2309 patrols 24 tiles, beyond the 16-tile leash. The
// AI leash follows the route, and a player can attack the mob from anywhere.
func TestMobLeashFollowsPatrolRoute(t *testing.T) {
	w := newZoneTestWorld()
	m := combatMob(1000, 2404, 1560)
	m.Segments = [model.MaxGenerSegments]model.GenerSegment{{X: 2380, Y: 1560}, {X: 2404, Y: 1560}}
	_, farPlayer := livingPlayer(1, 2440, 1600)
	if !m.insideHomeLeash() {
		t.Fatal("patrol destination is outside the leash")
	}
	if !w.playerCanInteractWithMob(farPlayer, m) {
		t.Fatal("a player far from the mob route cannot attack it")
	}
	m.X, m.Y = 2400, 1580
	if x, y, distance, ok := m.leashAnchor(m.X, m.Y); m.insideHomeLeash() || !ok ||
		y != 1560 || x < 2380 || x > 2404 || distance != 20 {
		t.Fatalf("off-route mob anchor=(%d,%d) distance=%d ok=%v", x, y, distance, ok)
	}
	m.X, m.Y = 2410, 1566
	if x, y, distance, _ := m.leashAnchor(m.X, m.Y); x != 2404 || y != 1560 || distance != 6 {
		t.Fatalf("mob beyond the destination anchor=(%d,%d) distance=%d", x, y, distance)
	}
}

func TestCraftMerchantCannotExecuteActiveAttack(t *testing.T) {
	_, target := livingPlayer(1, 100, 100)
	merchant := &Mob{
		ID: 1500, X: 100, Y: 100, TargetID: target.ID,
		Def: &model.NPCDef{
			Name: "Compositor", Tipo: model.TipoMonstro,
			Score: &model.Score{Merchant: craftingMerchant, Attack: 9999},
		},
	}
	w := &World{
		playersByID: map[uint16]*Player{target.ID: target},
		activeMobs:  map[uint16]*Mob{merchant.ID: merchant},
	}
	w.tickActiveMobActions(time.Now())
	if got := playerCurHP(target.Char); got != 100 {
		t.Fatalf("mercador causou dano: HP=%d", got)
	}
	if _, active := w.activeMobs[merchant.ID]; active || merchant.TargetID != 0 || merchant.Awake {
		t.Fatal("mercador permaneceu ativo na IA de combate")
	}
}
