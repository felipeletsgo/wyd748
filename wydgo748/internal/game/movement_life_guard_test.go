package game

import (
	"encoding/binary"
	"testing"
	"time"
)

func TestDeadPlayerActionStopCannotStartRoute(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	startX, startY := p.X, p.Y
	stop := make([]byte, 52)
	binary.LittleEndian.PutUint16(stop[12:14], startX)
	binary.LittleEndian.PutUint16(stop[14:16], startY)
	binary.LittleEndian.PutUint16(stop[24:26], startX+1)
	binary.LittleEndian.PutUint16(stop[26:28], startY)

	setPlayerCurHP(p.Char, 0)
	w.onActionStop(p.Session, stop)
	w.advancePlayerMovement(p, w.now().Add(time.Second))
	if p.X != startX || p.Y != startY || p.MovePublished || len(p.MoveAuthorityRoute) != 0 {
		t.Fatalf("dead player acquired an ActionStop route: position=(%d,%d) published=%v route=%q",
			p.X, p.Y, p.MovePublished, p.MoveAuthorityRoute)
	}

	setPlayerCurHP(p.Char, 1)
	w.onActionStop(p.Session, stop)
	if !p.MovePublished || len(p.MoveAuthorityRoute) != 1 {
		t.Fatal("revived player could not start a valid ActionStop route")
	}
}

func TestPendingMovementCannotAdvanceAfterDeath(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	startX, startY := p.X, p.Y
	now := w.now()
	w.beginPlayerMovement(p, startX, startY, startX+1, startY,
		[]byte("6"), []byte("6"), 0, now)
	if !p.MovePublished {
		t.Fatal("live player's route was not published")
	}

	setPlayerCurHP(p.Char, 0)
	w.advancePlayerMovement(p, now.Add(time.Second))
	if p.X != startX || p.Y != startY || p.MovePublished || len(p.MoveAuthorityRoute) != 0 {
		t.Fatalf("dead player retained or advanced a pending route: position=(%d,%d) published=%v route=%q",
			p.X, p.Y, p.MovePublished, p.MoveAuthorityRoute)
	}
}
