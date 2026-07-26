package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func newZoneTestWorld() *World {
	w := &World{
		store:          &craftStore{},
		players:        make(map[*net.Session]*Player),
		playersByID:    make(map[uint16]*Player),
		mobsByID:       make(map[uint16]*Mob),
		mobCells:       make(map[uint32]map[uint16]*Mob),
		playerCells:    make(map[uint32]map[uint16]*Player),
		mobCell:        make(map[uint16]uint32),
		playerCell:     make(map[uint16]uint32),
		activeMobs:     make(map[uint16]*Mob),
		summons:        make(map[uint16]*Mob),
		sephiraObjects: make(map[uint16]*Mob),
		groundItems:    make(map[uint16]*GroundItem),
		ghostShops:     make(map[uint16]*GhostShop),
	}
	w.questZones = []model.QuestZone{{Name: "Gravedigger", X1: 2379, Y1: 2076, X2: 2426, Y2: 2133}}
	return w
}

func addZonePlayer(w *World, id uint16, x, y uint16, hp uint32) *Player {
	session := net.NewTestSession(int64(id), 64)
	acc := &model.Account{Name: "p", Chars: []model.Char{{
		Name:     "p",
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 1000, CurHP: hp},
	}}}
	p := &Player{ID: id, Session: session, Account: acc, Char: &acc.Chars[0],
		InWorld: true, X: x, Y: y, Visible: map[uint16]struct{}{}}
	w.players[session] = p
	w.playersByID[id] = p
	w.updatePlayerSpatial(p)
	return p
}

// nearRecall usa Chebyshev pois o recall pode empurrar 1 tile quando dois
// jogadores caem no mesmo ponto de reentrada.
func nearRecall(p *Player) bool {
	return chebyshev(p.X, p.Y, recallX, recallY) <= 8
}

func TestQuestZoneResetRecallsInsideRevivesDeadLeavesOutside(t *testing.T) {
	w := newZoneTestWorld()
	alive := addZonePlayer(w, 1, 2400, 2100, 500) // dentro, vivo
	dead := addZonePlayer(w, 2, 2385, 2080, 0)    // dentro, morto
	outside := addZonePlayer(w, 3, 100, 100, 500) // fora

	w.tickQuestZoneReset(time.Now())

	if !nearRecall(alive) {
		t.Errorf("vivo dentro nao foi recolhido: (%d,%d)", alive.X, alive.Y)
	}
	if !nearRecall(dead) {
		t.Errorf("morto dentro nao foi recolhido: (%d,%d)", dead.X, dead.Y)
	}
	if playerCurHP(dead.Char) == 0 {
		t.Error("morto dentro nao foi revivido antes do recall")
	}
	if outside.X != 100 || outside.Y != 100 {
		t.Errorf("jogador fora da zona foi afetado: (%d,%d)", outside.X, outside.Y)
	}
}

func TestQuestZoneResetNoZonesIsNoop(t *testing.T) {
	w := newZoneTestWorld()
	w.questZones = nil
	p := addZonePlayer(w, 1, 2400, 2100, 500)
	w.tickQuestZoneReset(time.Now())
	if p.X != 2400 || p.Y != 2100 {
		t.Errorf("sem zonas configuradas nao deveria mover ninguem: (%d,%d)", p.X, p.Y)
	}
}
