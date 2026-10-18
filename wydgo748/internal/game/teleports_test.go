package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestTeleportAtAcceptsOneTileAroundAttributeBlock(t *testing.T) {
	portals := []model.Teleport{{SourceX: 2118, SourceY: 2102, DestX: 1044, DestY: 1724}}
	for x := 2114; x <= 2121; x++ {
		for y := 2098; y <= 2105; y++ {
			got, ok := teleportAt(portals, uint16(x), uint16(y))
			want := x >= 2115 && x <= 2120 && y >= 2099 && y <= 2104
			if ok != want || (ok && (got.DestX != 1044 || got.DestY != 1724)) {
				t.Fatalf("tile (%d,%d): want route=%v, got ok=%v portal=%+v",
					x, y, want, ok, got)
			}
		}
	}
	if samePortalAttributeBlock(2120, 2102, 2118, 2102) {
		t.Fatal("activation margin must not change exact portal identity checks")
	}
}

func TestTeleportAtPrefersExactBlockOverNeighborMargin(t *testing.T) {
	portals := []model.Teleport{
		{SourceX: 2118, SourceY: 2102, DestX: 1044},
		{SourceX: 2122, SourceY: 2102, DestX: 2000},
	}
	got, ok := teleportAt(portals, 2120, 2102)
	if !ok || got.DestX != 2000 {
		t.Fatalf("exact route lost to a neighbor margin: ok=%v portal=%+v", ok, got)
	}
}

func TestTeleportAtMarginDoesNotWrapCoordinates(t *testing.T) {
	for _, source := range []uint16{0, 65535} {
		portals := []model.Teleport{{SourceX: source, SourceY: source}}
		if _, ok := teleportAt(portals, source, source); !ok {
			t.Fatalf("valid edge source %d rejected", source)
		}
		if _, ok := teleportAt(portals, 65535-source, source); ok {
			t.Fatalf("X margin wrapped at source %d", source)
		}
		if _, ok := teleportAt(portals, source, 65535-source); ok {
			t.Fatalf("Y margin wrapped at source %d", source)
		}
	}
}

func TestPortalActivationMarginKeepsFeesAndRejectsOutside(t *testing.T) {
	for _, tc := range []struct {
		name string
		x, y uint16
		gold uint32
		want bool
	}{
		{"left", 2115, 2102, 1000, true},
		{"right", 2120, 2102, 1000, true},
		{"below", 2118, 2099, 1000, true},
		{"above", 2118, 2104, 1000, true},
		{"diagonal", 2120, 2104, 1000, true},
		{"outside X", 2121, 2102, 1000, false},
		{"outside Y", 2118, 2105, 1000, false},
		{"insufficient gold", 2120, 2102, 699, false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			p, session := networkedTestPlayer(1, "Traveler", tc.x, tc.y)
			st := &craftStore{}
			w := worldWithNetworkedPlayers(p)
			w.store = st
			w.teleports = []model.Teleport{{SourceX: 2118, SourceY: 2102,
				DestX: 2200, DestY: 2201, Price: 700}}
			p.Char.Gold = tc.gold
			w.onReqTeleport(session, make([]byte, 16))
			if tc.want {
				if p.X != 2200 || p.Y != 2201 || p.Char.Gold != tc.gold-700 || st.saves != 1 {
					t.Fatalf("portal did not relocate and charge once: pos=(%d,%d) gold=%d saves=%d",
						p.X, p.Y, p.Char.Gold, st.saves)
				}
			} else if p.X != tc.x || p.Y != tc.y || p.Char.Gold != tc.gold || st.saves != 0 {
				t.Fatalf("rejected portal changed state: pos=(%d,%d) gold=%d saves=%d",
					p.X, p.Y, p.Char.Gold, st.saves)
			}
		})
	}
}

func TestArmiaTeleportWorksAfterContinuous748Movement(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	w.teleports = []model.Teleport{{
		SourceX: 2118, SourceY: 2102,
		DestX: 1044, DestY: 1724,
		Price: 700, Name: "Armia to Noatum",
	}}
	p.Char.Gold = 1_000

	first := make([]byte, 52)
	binary.LittleEndian.PutUint16(first[12:14], p.X)
	binary.LittleEndian.PutUint16(first[14:16], p.Y)
	binary.LittleEndian.PutUint16(first[24:26], 2107)
	binary.LittleEndian.PutUint16(first[26:28], 2100)
	copy(first[28:], directMovementRoute(p.X, p.Y, 2107, 2100))
	w.onMove(p.Session, first)

	clock.Advance(50 * time.Millisecond)
	w.advancePlayerMovement(p, clock.Now())
	second := make([]byte, 52)
	binary.LittleEndian.PutUint16(second[12:14], 2110)
	binary.LittleEndian.PutUint16(second[14:16], 2100)
	binary.LittleEndian.PutUint16(second[24:26], 2118)
	binary.LittleEndian.PutUint16(second[26:28], 2102)
	copy(second[28:], directMovementRoute(2110, 2100, 2118, 2102))
	w.onMove(p.Session, second)

	clock.Advance(5 * time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2118 || p.Y != 2102 {
		t.Fatalf("movement did not reach the portal area: (%d,%d)", p.X, p.Y)
	}

	w.onReqTeleport(p.Session, make([]byte, 16))
	if chebyshev(p.X, p.Y, 1044, 1724) > 3 {
		t.Fatalf("Armia->Noatum did not complete: (%d,%d)", p.X, p.Y)
	}
	if p.Char.Gold != 300 {
		t.Fatalf("portal price was not charged exactly once: gold=%d", p.Char.Gold)
	}
}

func TestArmiaTeleportWorksAfterNative748ActionStop(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	w.teleports = []model.Teleport{{
		SourceX: 2118, SourceY: 2102,
		DestX: 1044, DestY: 1724,
		Price: 700, Name: "Armia to Noatum",
	}}
	p.Char.Gold = 1_000

	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint16(move[24:26], 2120)
	binary.LittleEndian.PutUint16(move[26:28], 2102)
	copy(move[28:], directMovementRoute(p.X, p.Y, 2120, 2102))
	w.onMove(p.Session, move)
	clock.Advance(2500 * time.Millisecond) // Server movement still trails the visual position.
	w.advancePlayerMovement(p, clock.Now())

	// The client stopped inside the portal block. The native wire carries a
	// visual Pos and final Target without Route[24].
	stop := make([]byte, 52)
	binary.LittleEndian.PutUint16(stop[12:14], 2113)
	binary.LittleEndian.PutUint16(stop[14:16], 2101)
	binary.LittleEndian.PutUint32(stop[16:20], 4)
	binary.LittleEndian.PutUint16(stop[24:26], 2118)
	binary.LittleEndian.PutUint16(stop[26:28], 2102)
	w.onActionStop(p.Session, stop)
	if p.X == 2118 && p.Y == 2102 {
		t.Fatal("ActionStop promoted the portal coordinate before arrival")
	}

	clock.Advance(2 * time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2118 || p.Y != 2102 {
		t.Fatalf("stop did not reconcile the portal area: (%d,%d)", p.X, p.Y)
	}
	w.onReqTeleport(p.Session, make([]byte, 16))
	if chebyshev(p.X, p.Y, 1044, 1724) > 3 {
		t.Fatalf("portal rejected after a valid ActionStop: (%d,%d)", p.X, p.Y)
	}
}

func TestArmiaTeleportRouteCanCrossTestShopRow(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = loadedFlatTerrain()
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	w.teleports = []model.Teleport{{
		SourceX: 2118, SourceY: 2102,
		DestX: 1044, DestY: 1724,
		Price: 700, Name: "Armia to Noatum",
	}}
	p.Char.Gold = 1_000

	// Reproduce the test shop row between spawn and the portal. Destination
	// (2116,2101) is free, but the route crosses the NPC at (2109,2100).
	shop := &Mob{ID: 1000, X: 2109, Y: 2100, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100, Merchant: 1})}
	w.appendMobInstance(shop)
	w.registerMobSpatial(shop)
	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint16(move[24:26], 2116)
	binary.LittleEndian.PutUint16(move[26:28], 2101)
	copy(move[28:], []byte("6666666666666669"))
	w.onMove(p.Session, move)
	clock.Advance(4 * time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2116 || p.Y != 2101 {
		t.Fatalf("intermediate shop blocked the player at (%d,%d)", p.X, p.Y)
	}

	w.onReqTeleport(p.Session, make([]byte, 16))
	if chebyshev(p.X, p.Y, 1044, 1724) > 3 {
		t.Fatalf("portal rejected after crossing shops: (%d,%d)", p.X, p.Y)
	}
}
