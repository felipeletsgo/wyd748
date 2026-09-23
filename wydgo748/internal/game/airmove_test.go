package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func airMovePacket(id uint16, route, mode uint32) []byte {
	pkt := inboundPacket(wire.OpAirMove, airMovePacketSize)
	binary.LittleEndian.PutUint16(pkt[6:8], id)
	binary.LittleEndian.PutUint32(pkt[12:16], route)
	binary.LittleEndian.PutUint32(pkt[16:20], mode)
	return pkt
}

func airMoveTestWorld(t *testing.T) (*World, *Player, *fakeClock) {
	t.Helper()
	w, p, _ := handlerTestWorld(t)
	clock := newFakeClock(time.Unix(100, 0))
	w.clock = clock
	m := &Mob{ID: 1100, X: 2101, Y: 2100, Def: &model.NPCDef{
		Name: "AirMove", Tipo: model.TipoNPC,
		Equip: model.Equip{Rosto: model.Item{Index: 63}},
		Score: &model.Score{Merchant: 7},
	}}
	w.registerMobSpatial(m)
	p.show(m.ID)
	return w, p, clock
}

func TestAirMoveCompletesOnlyMatchingNativeRouteAfterFlight(t *testing.T) {
	for route, destination := range airMoveDestinations {
		t.Run(string(rune('0'+route)), func(t *testing.T) {
			w, p, clock := airMoveTestWorld(t)
			w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, uint32(route), 1)})
			if !p.AirMoveActive || p.AirMoveRoute != route || p.X != 2100 || p.Y != 2100 {
				t.Fatalf("inicio perdeu estado ou moveu cedo: active=%v route=%d pos=(%d,%d)",
					p.AirMoveActive, p.AirMoveRoute, p.X, p.Y)
			}
			clock.Advance(airMoveMinimumDuration)
			w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, uint32(route), 2)})
			if p.AirMoveActive || p.X != destination.x || p.Y != destination.y {
				t.Fatalf("fim nao publicou destino %v: active=%v pos=(%d,%d)",
					destination, p.AirMoveActive, p.X, p.Y)
			}
		})
	}
}

func TestAirMoveRejectsForgedStartsAndPrematureEnd(t *testing.T) {
	tests := []struct {
		name  string
		edit  func(*World, *Player)
		id    uint16
		route uint32
		mode  uint32
	}{
		{name: "invalid-route", route: 5, mode: 1},
		{name: "invalid-mode", route: 0, mode: 3},
		{name: "wrong-id", id: 999, route: 0, mode: 1},
		{name: "no-visible-npc", route: 0, mode: 1, edit: func(_ *World, p *Player) { p.Visible = nil }},
		{name: "wrong-chunk", route: 0, mode: 1, edit: func(_ *World, p *Player) { p.X, p.Y = 1900, 1900 }},
		{name: "end-without-start", route: 0, mode: 2},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, p, _ := airMoveTestWorld(t)
			if tc.edit != nil {
				tc.edit(w, p)
			}
			id := p.ID
			if tc.id != 0 {
				id = tc.id
			}
			beforeX, beforeY := p.X, p.Y
			w.handle(command{s: p.Session, pkt: airMovePacket(id, tc.route, tc.mode)})
			if p.AirMoveActive || p.X != beforeX || p.Y != beforeY {
				t.Fatalf("pedido falso alterou estado: active=%v pos=(%d,%d)", p.AirMoveActive, p.X, p.Y)
			}
			if state := w.security[p.Session]; state == nil || state.violations != 1 {
				t.Fatalf("pedido falso sem violacao: %#v", state)
			}
		})
	}

	w, p, clock := airMoveTestWorld(t)
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 2, 1)})
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 1, 2)})
	if !p.AirMoveActive {
		t.Fatal("fim de rota diferente encerrou voo valido")
	}
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 2, 2)})
	if p.AirMoveActive || p.X != 2100 || p.Y != 2100 {
		t.Fatal("fim antecipado concedeu teleporte")
	}
	clock.Advance(airMoveMinimumDuration)
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 2, 2)})
	if p.X != 2100 || p.Y != 2100 {
		t.Fatal("replay de fim concedeu teleporte")
	}
}

func TestAirMoveBlocksGameplayUntilEndAndClearsOnTeleport(t *testing.T) {
	w, p, _ := airMoveTestWorld(t)
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 0, 1)})
	pkt := inboundPacket(wire.OpMessageChat, 108)
	w.handle(command{s: p.Session, pkt: pkt})
	if !p.AirMoveActive {
		t.Fatal("outro pacote cancelou estado de voo")
	}
	if !w.teleportPlayer(p, 2112, 2051) || p.AirMoveActive {
		t.Fatal("teleporte externo nao limpou o voo")
	}
}

func TestAirMoveDeathReleasesGameplayGateWithoutGrantingDestination(t *testing.T) {
	w, p, clock := airMoveTestWorld(t)
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 4, 1)})
	if !p.AirMoveActive {
		t.Fatal("inicio nao entrou em voo")
	}
	effectiveScore(p.Char).CurHP = 0
	w.handle(command{s: p.Session, pkt: inboundPacket(wire.OpPing, 12)})
	if p.AirMoveActive {
		t.Fatal("morte manteve gate de voo")
	}
	clock.Advance(airMoveMinimumDuration)
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 4, 2)})
	if p.X != 2100 || p.Y != 2100 {
		t.Fatal("fim tardio apos morte concedeu destino")
	}
}

func TestAirMoveRejectsTruncatedAndOversizedFrames(t *testing.T) {
	for _, size := range []int{airMovePacketSize - 1, airMovePacketSize + 1} {
		w, p, _ := airMoveTestWorld(t)
		pkt := inboundPacket(wire.OpAirMove, size)
		binary.LittleEndian.PutUint16(pkt[6:8], p.ID)
		binary.LittleEndian.PutUint32(pkt[12:16], 0)
		w.handle(command{s: p.Session, pkt: pkt})
		if p.AirMoveActive || p.X != 2100 || p.Y != 2100 {
			t.Fatalf("pacote %d bytes alterou estado do voo", size)
		}
		if state := w.security[p.Session]; state == nil || state.violations != 1 {
			t.Fatalf("pacote %d bytes nao recebeu violacao: %#v", size, state)
		}
	}
}

func TestAirMoveLogoutClearsPendingRoute(t *testing.T) {
	w, p, clock := airMoveTestWorld(t)
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 3, 1)})
	if !p.AirMoveActive {
		t.Fatal("inicio nao criou viagem pendente")
	}
	oldID := p.ID
	resetCharacterRuntime(p)
	if p.AirMoveActive || p.AirMoveRoute != 0 || !p.AirMoveStartedAt.IsZero() ||
		p.AirMoveSourceX != 0 || p.AirMoveSourceY != 0 {
		t.Fatal("logout reteve a rota ou origem da viagem")
	}
	clock.Advance(airMoveMinimumDuration)
	w.handle(command{s: p.Session, pkt: airMovePacket(oldID, 3, 2)})
	if p.X != 0 || p.Y != 0 {
		t.Fatal("fim tardio apos logout concedeu destino")
	}
}
