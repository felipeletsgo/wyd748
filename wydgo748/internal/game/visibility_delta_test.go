package game

import (
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

func visibilityBoundsContainAny(regions [4]visibilityBounds, count int, x, y uint16) bool {
	for i := 0; i < count; i++ {
		if regions[i].contains(x, y) {
			return true
		}
	}
	return false
}

func TestVisibilityBoundsDifferenceMatchesExactViewDelta(t *testing.T) {
	tests := []struct {
		name       string
		oldX, oldY uint16
		newX, newY uint16
	}{
		{name: "east same cell", oldX: 100, oldY: 100, newX: 101, newY: 100},
		{name: "west", oldX: 100, oldY: 100, newX: 99, newY: 100},
		{name: "diagonal", oldX: 100, oldY: 100, newX: 101, newY: 101},
		{name: "catchup segment", oldX: 100, oldY: 100, newX: 112, newY: 108},
		{name: "map edge", oldX: 0, oldY: 0, newX: 1, newY: 1},
		{name: "max map edge", oldX: 65535, oldY: 65535, newX: 65534, newY: 65534},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			oldBounds := playerVisibilityBounds(tt.oldX, tt.oldY)
			newBounds := playerVisibilityBounds(tt.newX, tt.newY)
			entered, enteredCount := visibilityBoundsDifference(newBounds, oldBounds)
			left, leftCount := visibilityBoundsDifference(oldBounds, newBounds)

			minX := oldBounds.minX
			if newBounds.minX < minX {
				minX = newBounds.minX
			}
			maxX := oldBounds.maxX
			if newBounds.maxX > maxX {
				maxX = newBounds.maxX
			}
			minY := oldBounds.minY
			if newBounds.minY < minY {
				minY = newBounds.minY
			}
			maxY := oldBounds.maxY
			if newBounds.maxY > maxY {
				maxY = newBounds.maxY
			}

			for y := minY; y <= maxY; y++ {
				for x := minX; x <= maxX; x++ {
					ux, uy := uint16(x), uint16(y)
					wasVisible := inView(tt.oldX, tt.oldY, ux, uy)
					nowVisible := inView(tt.newX, tt.newY, ux, uy)
					if got := visibilityBoundsContainAny(entered, enteredCount, ux, uy); got != (nowVisible && !wasVisible) {
						t.Fatalf("entered mismatch at %d,%d: got=%v was=%v now=%v", x, y, got, wasVisible, nowVisible)
					}
					if got := visibilityBoundsContainAny(left, leftCount, ux, uy); got != (wasVisible && !nowVisible) {
						t.Fatalf("left mismatch at %d,%d: got=%v was=%v now=%v", x, y, got, wasVisible, nowVisible)
					}
				}
			}
		})
	}
}

func TestIncrementalPlayerVisibilityHandlesSameCellPvPBoundary(t *testing.T) {
	newPlayer := func(id, x, y uint16) *Player {
		s := net.NewTestSession(int64(id), 16)
		ch := &model.Char{
			Name:  "Player",
			X:     x,
			Y:     y,
			Score: testScore(model.Score{MaxHP: 100, CurHP: 100}),
		}
		return &Player{
			ID:      id,
			Session: s,
			Char:    ch,
			InWorld: true,
			X:       x,
			Y:       y,
			Visible: make(map[uint16]struct{}),
		}
	}

	mover := newPlayer(1, 100, 100)
	leaving := newPlayer(2, 68, 100)   // distance 32 -> 33
	entering := newPlayer(3, 133, 100) // distance 33 -> 32
	stable := newPlayer(4, 101, 101)

	w := &World{
		players: map[*net.Session]*Player{
			mover.Session:    mover,
			leaving.Session:  leaving,
			entering.Session: entering,
			stable.Session:   stable,
		},
		playersByID: map[uint16]*Player{
			mover.ID:    mover,
			leaving.ID:  leaving,
			entering.ID: entering,
			stable.ID:   stable,
		},
	}
	w.updatePlayerSpatial(mover)
	w.updatePlayerSpatial(leaving)
	w.updatePlayerSpatial(entering)
	w.updatePlayerSpatial(stable)

	// Establish the previous visibility state without setup packets.
	mover.show(leaving.ID)
	leaving.show(mover.ID)
	mover.show(stable.ID)
	stable.show(mover.ID)

	mover.X, mover.Y = 101, 100 // remains in the same 16-tile spatial cell.
	mover.Char.X, mover.Char.Y = mover.X, mover.Y
	w.refreshPlayerVisibilityAfterMove(mover, 100, 100)

	if mover.hasVisible(leaving.ID) || leaving.hasVisible(mover.ID) {
		t.Fatal("player crossing the 32-tile boundary remained visible")
	}
	if !mover.hasVisible(entering.ID) || !entering.hasVisible(mover.ID) {
		t.Fatal("player entering the 32-tile boundary was not materialized immediately")
	}
	if !mover.hasVisible(stable.ID) || !stable.hasVisible(mover.ID) {
		t.Fatal("player remaining in view lost visibility")
	}
	if got := leaving.Session.QueuedPacketsForTest(); got != 1 {
		t.Fatalf("leaving player received %d packets; expected only RemoveMob", got)
	}
	if got := entering.Session.QueuedPacketsForTest(); got != 3 {
		t.Fatalf("entering player received %d packets; expected CreateMob+HP+Stop", got)
	}
	if got := stable.Session.QueuedPacketsForTest(); got != 0 {
		t.Fatalf("stable player received %d packets; delta rematerialized an unchanged entity", got)
	}
}

func TestIncrementalGroundItemVisibilityProjectsClientGateState(t *testing.T) {
	s := net.NewTestSession(1, 8)
	p := &Player{
		ID: 1, Session: s, InWorld: true, X: 101, Y: 100,
		Visible: make(map[uint16]struct{}),
	}
	w := &World{
		items: map[uint16]model.ItemDef{
			458: {Index: 458, StaticEffects: []model.StaticEffect{
				{Name: "EF_GROUND", Value: 1}, {Name: "EF_KEYID", Value: 2},
			}},
		},
	}
	w.registerGroundItem(&GroundItem{
		ID: 10000, Item: model.Item{Index: 458}, X: 133, Y: 100,
		Rotate: 2, Permanent: true, State: gateClosed,
	})

	oldBounds := playerVisibilityBounds(100, 100)
	newBounds := playerVisibilityBounds(p.X, p.Y)
	entered, count := visibilityBoundsDifference(newBounds, oldBounds)
	w.applyGroundItemVisibilityRegions(p, entered, count, true)

	pkt, ok := s.DequeuePacketForTest()
	if !ok || !wire.Decrypt(pkt) {
		t.Fatal("incremental visibility did not send a decodable CreateItem packet")
	}
	if len(pkt) != 32 {
		t.Fatalf("gate CreateItem size = %d; want 32", len(pkt))
	}
	if wire.ParseHeader(pkt).Type != wire.OpCreateItem ||
		pkt[26] != 2 || pkt[27] != clientGateLocked || pkt[28] != clientGateHeight {
		t.Fatalf("unexpected gate CreateItem projection: type=%#x rotation=%d state=%d height=%d",
			wire.ParseHeader(pkt).Type, pkt[26], pkt[27], pkt[28])
	}
	if !p.hasVisible(10000) || s.QueuedPacketsForTest() != 0 {
		t.Fatal("incremental visibility did not register exactly one gate")
	}
}
