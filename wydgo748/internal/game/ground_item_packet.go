package game

import "wydgo/internal/wire"

const (
	clientGateOpen   byte = 1
	clientGateClosed byte = 2
	clientGateLocked byte = 3
	clientGateHeight byte = 16
)

// The server stores a door as closed/open (0/1). CreateItem carries the
// client's initial visual state; UpdateItem uses a different state encoding.
func (w *World) groundItemCreatePacket(g *GroundItem) []byte {
	state := g.State
	height := byte(0)
	if def, ok := w.items[g.Item.Index]; ok && staticEffect(def, "EF_GROUND") > 0 {
		if state == gateClosed {
			height = clientGateHeight
			state = clientGateClosed
			if staticEffect(def, "EF_KEYID") > 0 {
				state = clientGateLocked
			}
		} else if state == gateOpen {
			state = clientGateOpen
		}
	}
	return wire.CreateItem(g.X, g.Y, g.ID, g.Item, g.Rotate, state, height, 0, 0)
}

// UpdateItem state 0 makes the 7.48 client lock the gate again. It must also
// restore the collision height after a rejected interaction.
func groundItemUpdatePacket(g *GroundItem) []byte {
	pkt := wire.UpdateItem(g.ID, uint16(g.State))
	if g.State == gateClosed {
		pkt[18] = clientGateHeight
	}
	return pkt
}
