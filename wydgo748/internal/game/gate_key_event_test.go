package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestEventGateKeyIDCannotBeOpenedByPlayer(t *testing.T) {
	w, gate := worldWithGate(eventGateKeyID, 2100, 2100)
	w.items[459] = model.ItemDef{Index: 459, Name: "Event_Gate_Key",
		StaticEffects: []model.StaticEffect{{Name: "EF_KEYID", Value: eventGateKeyID}}}
	p, s := playerWithKey(w, 459, 2100, 2100)

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)
	if gate.State != gateClosed || p.Char.Inv[0].Index != 459 {
		t.Fatalf("event gate key changed state or inventory: state=%d item=%d", gate.State, p.Char.Inv[0].Index)
	}

	p.show(gate.ID)
	w.onUpdateGroundItem(s, updateGroundItemPacket(gate.ID, uint32(gateOpen)))
	if gate.State != gateClosed || p.Char.Inv[0].Index != 459 {
		t.Fatalf("forged event gate key opened or consumed: state=%d item=%d", gate.State, p.Char.Inv[0].Index)
	}
	if err := w.queueGateKey(p, gate, 0, true, 3); err == nil {
		t.Fatal("critical gate admission accepted the event-only key ID")
	}
}
