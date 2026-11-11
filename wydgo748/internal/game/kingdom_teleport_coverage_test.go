package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func TestKingdomTeleportRejectsDeadPlayersAndUnalignedKingRequests(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	setPlayerCurHP(p.Char, 0)
	beforeX, beforeY := p.X, p.Y
	w.kingdomCommandTeleport(p.Session, p, false)
	if p.X != beforeX || p.Y != beforeY || p.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("dead player teleport changed position or skipped its rejection")
	}

	w, p, _ = handlerTestWorld(t)
	w.kingdomCommandTeleport(p.Session, p, true)
	if p.X != 2100 || p.Y != 2100 || p.Session.QueuedPacketsForTest() != 1 {
		t.Fatal("king teleport without a kingdom changed position or skipped its rejection")
	}
}

func TestNeutralKingdomTeleportUsesPublicLandingPoint(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.kingdomCommandTeleport(p.Session, p, false)
	if p.X != 1702 || p.Y != 1726 || p.NextKingdomTeleport.IsZero() {
		t.Fatalf("neutral public teleport=(%d,%d) cooldown=%v", p.X, p.Y, p.NextKingdomTeleport)
	}
}

func TestHekalotiaKingdomTeleportUsesItsLandingPoints(t *testing.T) {
	for _, tc := range []struct {
		name  string
		king  bool
		wantX uint16
		wantY uint16
	}{
		{"king", true, 1748, 1574},
		{"public", false, 1690, 1618},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, p, _ := handlerTestWorld(t)
			p.Char.Equip[model.CapeSlot] = model.Item{Index: 545}
			w.kingdomCommandTeleport(p.Session, p, tc.king)
			if p.X != tc.wantX || p.Y != tc.wantY {
				t.Fatalf("teleport=(%d,%d), want (%d,%d)", p.X, p.Y, tc.wantX, tc.wantY)
			}
		})
	}
}

func TestKingdomTeleportDoesNotChangeStateForRetainedDisconnect(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.criticalDisconnects = map[*net.Session]*criticalDisconnect{
		p.Session: {player: p},
	}
	beforeX, beforeY := p.X, p.Y
	w.kingdomCommandTeleport(p.Session, p, false)
	if p.X != beforeX || p.Y != beforeY || p.Session.QueuedPacketsForTest() != 0 {
		t.Fatal("reserved player's teleport changed state or sent a response")
	}
}

func TestKingdomTeleportRestoresPositionWhenPersistenceFails(t *testing.T) {
	w, p, st := handlerTestWorld(t)
	p.Char.Equip[model.CapeSlot] = model.Item{Index: 546}
	w.clock = newFakeClock(time.Date(2026, time.July, 27, 12, 0, 0, 0, time.Local))
	st.err = errors.New("disk full")
	oldX, oldY := p.X, p.Y
	w.kingdomCommandTeleport(p.Session, p, true)
	if p.X != oldX || p.Y != oldY || p.Char.X != oldX || p.Char.Y != oldY ||
		!p.NextKingdomTeleport.IsZero() || p.Session.QueuedPacketsForTest() == 0 {
		t.Fatalf("failed teleport was not rolled back: pos=(%d,%d) char=(%d,%d) cooldown=%v packets=%d",
			p.X, p.Y, p.Char.X, p.Char.Y, p.NextKingdomTeleport, p.Session.QueuedPacketsForTest())
	}
}
