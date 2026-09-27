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
		Name:  "p",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 1000, CurHP: hp},
	}}}
	p := &Player{ID: id, Session: session, Account: acc, Char: &acc.Chars[0],
		InWorld: true, X: x, Y: y, Visible: map[uint16]struct{}{}}
	w.players[session] = p
	w.playersByID[id] = p
	w.updatePlayerSpatial(p)
	return p
}

// nearRecall checks the beginner field, allowing adjacent tiles when multiple
// low-level mortals recall at the same time.
func nearRecall(p *Player) bool {
	return chebyshev(p.X, p.Y, mortalBeginnerSpawnX, mortalBeginnerSpawnY) <= 8
}

func TestQuestZoneResetRecallsInsideRevivesDeadLeavesOutside(t *testing.T) {
	w := newZoneTestWorld()
	alive := addZonePlayer(w, 1, 2400, 2100, 500) // Inside, alive.
	dead := addZonePlayer(w, 2, 2385, 2080, 0)    // Inside, dead.
	outside := addZonePlayer(w, 3, 100, 100, 500) // Outside.

	w.tickQuestZoneReset(time.Now())

	if !nearRecall(alive) {
		t.Errorf("living player inside zone was not recalled: (%d,%d)", alive.X, alive.Y)
	}
	if !nearRecall(dead) {
		t.Errorf("dead player inside zone was not recalled: (%d,%d)", dead.X, dead.Y)
	}
	if playerCurHP(dead.Char) == 0 {
		t.Error("dead player inside zone was not revived before recall")
	}
	if outside.X != 100 || outside.Y != 100 {
		t.Errorf("player outside zone was affected: (%d,%d)", outside.X, outside.Y)
	}
}

func TestQuestZoneResetNoZonesIsNoop(t *testing.T) {
	w := newZoneTestWorld()
	w.questZones = nil
	p := addZonePlayer(w, 1, 2400, 2100, 500)
	w.tickQuestZoneReset(time.Now())
	if p.X != 2400 || p.Y != 2100 {
		t.Errorf("no configured zones should leave the player in place: (%d,%d)", p.X, p.Y)
	}
}
