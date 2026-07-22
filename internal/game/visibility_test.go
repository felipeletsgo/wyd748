package game

import (
	"encoding/binary"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestInViewUsesExpanded65By65Window(t *testing.T) {
	if !inView(100, 100, 132, 132) {
		t.Fatal("a borda inclusiva de 32 tiles deveria estar visivel")
	}
	if inView(100, 100, 133, 100) || inView(100, 100, 100, 133) {
		t.Fatal("entidade fora da janela 65x65 nao deveria estar visivel")
	}
}

func TestPlayerVisibilitySet(t *testing.T) {
	p := &Player{}
	if p.hasVisible(1000) {
		t.Fatal("mapa vazio marcou entidade visivel")
	}
	p.show(1000)
	if !p.hasVisible(1000) {
		t.Fatal("show nao registrou entidade")
	}
	p.hide(1000)
	if p.hasVisible(1000) {
		t.Fatal("hide nao removeu entidade")
	}
}

func TestPlayerEnterViewRestoresAliveStateBeforeMovement(t *testing.T) {
	p := &Player{
		ID: 77,
		X:  2200,
		Y:  2100,
		Char: &model.Char{
			Name: "Revived",
			Extended: testExtended(model.ExtendedScore{
				MaxHP: 123,
				CurHP: 123,
				MaxMP: 45,
				CurMP: 45,
			}),
		},
	}
	p.Char.Affects[0] = model.Affect{Type: 13, ExpiresAt: time.Now().Add(80 * time.Second)}

	packets := playerEnterViewPackets(p)
	if len(packets) != 3 {
		t.Fatalf("entrada na visibilidade gerou %d pacotes, esperado 3", len(packets))
	}
	wantType := []uint16{wire.OpCreateMob, wire.OpSetHpMp, wire.OpAction}
	wantID := []uint16{wire.SceneField, p.ID, p.ID}
	for i, pkt := range packets {
		h := wire.ParseHeader(pkt)
		if h.Type != wantType[i] || h.ID != wantID[i] {
			t.Fatalf("pacote %d: Type=0x%X ID=%d, esperado Type=0x%X ID=%d",
				i, h.Type, h.ID, wantType[i], wantID[i])
		}
	}
	if got := binary.LittleEndian.Uint16(packets[0][16:18]); got != p.ID {
		t.Fatalf("CreateMob materializou ID=%d, esperado %d", got, p.ID)
	}
	if packets[0][66] != 10 || packets[0][67] != 13 {
		t.Fatalf("CreateMob nao restaurou affect visual: % X", packets[0][66:70])
	}
}
