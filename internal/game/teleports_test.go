package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestTeleportAtUsesNativeAttributeBlock(t *testing.T) {
	portals := []model.Teleport{{SourceX: 2118, SourceY: 2102, DestX: 1044, DestY: 1724}}
	for x := 2116; x <= 2119; x++ {
		for y := 2100; y <= 2103; y++ {
			got, ok := teleportAt(portals, uint16(x), uint16(y))
			if !ok || got.DestX != 1044 || got.DestY != 1724 {
				t.Fatalf("tile (%d,%d) do bloco 4x4 nao encontrou portal: ok=%v portal=%+v",
					x, y, ok, got)
			}
		}
	}
	if _, ok := teleportAt(portals, 2120, 2102); ok {
		t.Fatal("posicao fora do bloco 4x4 ativou o portal")
	}
	if _, ok := teleportAt(portals, 2118, 2099); ok {
		t.Fatal("linha anterior ao bloco 4x4 ativou o portal")
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
		Price: 700, Name: "Armia para Noatum",
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
		t.Fatalf("movimento nao chegou a area do portal: (%d,%d)", p.X, p.Y)
	}

	w.onReqTeleport(p.Session, make([]byte, 16))
	if chebyshev(p.X, p.Y, 1044, 1724) > 3 {
		t.Fatalf("Armia->Noatum nao concluiu: (%d,%d)", p.X, p.Y)
	}
	if p.Char.Gold != 300 {
		t.Fatalf("preco do portal nao foi aplicado uma vez: gold=%d", p.Char.Gold)
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
		Price: 700, Name: "Armia para Noatum",
	}}
	p.Char.Gold = 1_000

	move := make([]byte, 52)
	binary.LittleEndian.PutUint16(move[12:14], p.X)
	binary.LittleEndian.PutUint16(move[14:16], p.Y)
	binary.LittleEndian.PutUint16(move[24:26], 2120)
	binary.LittleEndian.PutUint16(move[26:28], 2102)
	copy(move[28:], directMovementRoute(p.X, p.Y, 2120, 2102))
	w.onMove(p.Session, move)
	clock.Advance(2500 * time.Millisecond) // autoridade ainda esta atras da tela.
	w.advancePlayerMovement(p, clock.Now())

	// O client escolheu parar dentro do bloco do portal. Como no wire nativo,
	// ele informa um Pos visual e um Target final, sem Route[24].
	stop := make([]byte, 52)
	binary.LittleEndian.PutUint16(stop[12:14], 2113)
	binary.LittleEndian.PutUint16(stop[14:16], 2101)
	binary.LittleEndian.PutUint32(stop[16:20], 4)
	binary.LittleEndian.PutUint16(stop[24:26], 2118)
	binary.LittleEndian.PutUint16(stop[26:28], 2102)
	w.onActionStop(p.Session, stop)
	if p.X == 2118 && p.Y == 2102 {
		t.Fatal("ActionStop promoveu a coordenada do portal antes do tempo")
	}

	clock.Advance(2 * time.Second)
	w.advancePlayerMovement(p, clock.Now())
	if p.X != 2118 || p.Y != 2102 {
		t.Fatalf("parada nao reconciliou a area do portal: (%d,%d)", p.X, p.Y)
	}
	w.onReqTeleport(p.Session, make([]byte, 16))
	if chebyshev(p.X, p.Y, 1044, 1724) > 3 {
		t.Fatalf("portal recusado apos ActionStop valido: (%d,%d)", p.X, p.Y)
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
		Price: 700, Name: "Armia para Noatum",
	}}
	p.Char.Gold = 1_000

	// Reproduz a fileira de lojas de teste entre o nascimento e o portal. O
	// destino (2116,2101) está livre, mas o trajeto atravessa o NPC em 2109,2100.
	shop := &Mob{ID: 1000, X: 2109, Y: 2100, HP: 100,
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100, Merchant: 1})}
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
		t.Fatalf("loja intermediaria travou o jogador em (%d,%d)", p.X, p.Y)
	}

	w.onReqTeleport(p.Session, make([]byte, 16))
	if chebyshev(p.X, p.Y, 1044, 1724) > 3 {
		t.Fatalf("portal recusado apos cruzar lojas: (%d,%d)", p.X, p.Y)
	}
}
