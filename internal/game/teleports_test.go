package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestTeleportAtUsesMicronicsFourByFourGate(t *testing.T) {
	portals := []model.Teleport{{SourceX: 2118, SourceY: 2102, DestX: 1044, DestY: 1724}}
	got, ok := teleportAt(portals, 2121, 2105)
	if !ok || got.DestX != 1044 || got.DestY != 1724 {
		t.Fatalf("portal nao encontrado: ok=%v portal=%+v", ok, got)
	}
	if _, ok := teleportAt(portals, 2122, 2105); ok {
		t.Fatal("posicao fora do quadrado 4x4 ativou o portal")
	}
}
