package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/wire"
)

func assertInstanceHUDCleared(t *testing.T, p *Player, want bool) {
	t.Helper()
	cleared := map[uint16]bool{}
	for {
		packet, ok := p.Session.DequeuePacketForTest()
		if !ok {
			break
		}
		if !wire.Decrypt(packet) {
			t.Fatal("invalid outbound checksum")
		}
		op := wire.ParseHeader(packet).Type
		if op == wire.OpInstanceTime || op == wire.OpInstanceMobs {
			if len(packet) != 16 || binary.LittleEndian.Uint32(packet[12:]) != 0 {
				t.Fatalf("instance HUD opcode %#x was not cleared", op)
			}
			cleared[op] = true
		}
	}
	if (cleared[wire.OpInstanceTime] && cleared[wire.OpInstanceMobs]) != want ||
		(!want && len(cleared) != 0) {
		t.Fatalf("cleared HUD packets=%v want both=%v", cleared, want)
	}
}

func TestRecallClearsInstanceHUDAfterDeath(t *testing.T) {
	w, p, member, _, clock := instanceTestWorld()
	p.Party, member.Party = nil, nil
	w.onUseItem(p.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil || !itemInstanceHasMember(inst, p.ID) {
		t.Fatal("instance entry failed")
	}
	for p.Session.QueuedPacketsForTest() > 0 {
		p.Session.DequeuePacketForTest()
	}
	setPlayerCurHP(p.Char, 0)
	if !w.recallPlayer(p, "restart") {
		t.Fatal("recall failed")
	}
	assertInstanceHUDCleared(t, p, true)
	if itemInstanceHasMember(inst, p.ID) || w.playerRuntimeInstanceID(p.ID) != "" {
		t.Fatal("recall retained instance membership")
	}
	w.tickItemInstances(clock.Now())
	if len(w.itemInstances) != 0 {
		t.Fatal("empty instance did not close")
	}
}
