package game

import (
	"testing"

	"wydgo/internal/wire"
)

// The 7.48 UpdateItem request carries a 16-bit State followed by a separate
// Height byte. Height is client input, never the authoritative gate height.
func TestUpdateGroundItemDecodesStateWithoutClientHeight(t *testing.T) {
	w, gate := mundoComPorta(2, 2100, 2100)
	p, session := jogadorComChave(w, 451, 2100, 2100)
	p.show(gate.ID)

	pkt := wire.UpdateItem(gate.ID, uint16(gateOpen))
	pkt[18] = 16
	w.onUpdateGroundItem(session, pkt)

	if gate.State != gateOpen || p.Char.Inv[0].Index != 0 {
		t.Fatalf("valid 16-bit gate state was rejected with a separate height byte: state=%d key=%d",
			gate.State, p.Char.Inv[0].Index)
	}
}

func TestUpdateGroundItemRejectsNegativeSignedState(t *testing.T) {
	w, gate := mundoComPorta(2, 2100, 2100)
	p, session := jogadorComChave(w, 451, 2100, 2100)
	p.show(gate.ID)

	pkt := wire.UpdateItem(gate.ID, 0xffff)
	w.onUpdateGroundItem(session, pkt)

	if gate.State != gateClosed || p.Char.Inv[0].Index != 451 {
		t.Fatalf("negative signed gate state changed authoritative state: state=%d key=%d",
			gate.State, p.Char.Inv[0].Index)
	}
}
