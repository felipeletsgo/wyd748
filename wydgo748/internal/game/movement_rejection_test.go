package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func movementRequestForTest(x, y, targetX, targetY uint16, route string) []byte {
	pkt := make([]byte, 52)
	binary.LittleEndian.PutUint16(pkt[12:14], x)
	binary.LittleEndian.PutUint16(pkt[14:16], y)
	binary.LittleEndian.PutUint16(pkt[24:26], targetX)
	binary.LittleEndian.PutUint16(pkt[26:28], targetY)
	copy(pkt[28:], route)
	return pkt
}

func TestRejectedMovementStopsPendingRouteAndCorrectsOwner(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	initialX, initialY := p.X, p.Y
	w.onMove(p.Session, movementRequestForTest(p.X, p.Y, p.X+2, p.Y, "66"))
	if !p.MovePublished || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("valid movement should remain pending without an owner correction")
	}

	blockedX := p.X + 3
	w.terrain.Height[int(p.Y)*model.TerrainWidth+int(blockedX)] = model.TerrainBlockedByte
	w.onMove(p.Session, movementRequestForTest(p.X, p.Y, blockedX, p.Y, "666"))
	if p.X != initialX || p.Y != initialY || p.MovePublished || len(p.MoveAuthorityRoute) != 0 {
		t.Fatalf("rejected movement changed authority or kept the old route: pos=(%d,%d) pending=%v steps=%d",
			p.X, p.Y, p.MovePublished, len(p.MoveAuthorityRoute))
	}
	if got := p.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("owner received %d corrections, want one", got)
	}
	response, ok := p.Session.DequeuePacketForTest()
	if !ok || !wire.Decrypt(response) || len(response) != 52 {
		t.Fatal("owner correction is missing or malformed")
	}
	header := wire.ParseHeader(response)
	if header.Type != wire.OpAction || header.ID != p.ID ||
		binary.LittleEndian.Uint16(response[12:14]) != initialX ||
		binary.LittleEndian.Uint16(response[14:16]) != initialY ||
		binary.LittleEndian.Uint32(response[20:24]) != 8 ||
		binary.LittleEndian.Uint16(response[24:26]) != initialX ||
		binary.LittleEndian.Uint16(response[26:28]) != initialY {
		t.Fatalf("owner correction does not stop at the authoritative position: header=%+v payload=%x", header, response[12:28])
	}
	if binary.LittleEndian.Uint32(response[16:20]) != 0 {
		t.Fatal("route correction unexpectedly carries movement speed")
	}
	for _, step := range response[28:52] {
		if step != 0 {
			t.Fatal("route correction unexpectedly carries movement steps")
		}
	}
	if state := w.security[p.Session]; state == nil || state.violations != 1 {
		t.Fatalf("rejected route was not recorded as a security violation: %+v", state)
	}
}

func TestDeadPlayerMovementDoesNotSendCorrection(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	setPlayerCurHP(p.Char, 0)
	w.onMove(p.Session, movementRequestForTest(p.X, p.Y, p.X+1, p.Y, "6"))
	if got := p.Session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("dead player received %d movement corrections", got)
	}
}
