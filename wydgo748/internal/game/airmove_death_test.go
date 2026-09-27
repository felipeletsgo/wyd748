package game

import (
	"testing"

	"wydgo/internal/wire"
)

func TestAirMoveDeathPublicationCancelsFlightImmediately(t *testing.T) {
	w, p, clock := airMoveTestWorld(t)
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 4, 1)})
	if !p.AirMoveActive {
		t.Fatal("flight did not start")
	}

	setPlayerCurHP(p.Char, 0)
	w.publishPlayerDeath(p, 1100)
	if p.AirMoveActive || p.AirMoveRoute != 0 || !p.AirMoveStartedAt.IsZero() ||
		p.AirMoveSourceX != 0 || p.AirMoveSourceY != 0 {
		t.Fatal("death publication retained the pending flight")
	}

	clock.Advance(airMoveMinimumDuration)
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 4, 2)})
	if p.X != 2100 || p.Y != 2100 {
		t.Fatal("late flight completion granted a destination after death")
	}
	w.handle(command{s: p.Session, pkt: inboundPacket(wire.OpPing, 12)})
	if p.AirMoveActive {
		t.Fatal("later gameplay packet restored the canceled flight")
	}
}
