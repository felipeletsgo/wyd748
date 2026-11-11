package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestOnUseItemDispatchesLegacyCelestialRoutes(t *testing.T) {
	for _, tc := range []struct {
		name, action string
		item         uint16
	}{
		{name: "fury stone", action: "celestial_fury", item: furyStoneItem},
		{name: "mysterious stone", action: "celestial_switch", item: mysteriousStoneItem},
	} {
		t.Run(tc.name, func(t *testing.T) {
			wantPackets := 1
			if tc.action == "celestial_switch" {
				wantPackets = 2
			}
			w, p, _ := useItemWorld(model.VolatileRule{Action: tc.action, Consume: true})
			p.Char.Inv[0] = model.Item{Index: tc.item}
			w.items[tc.item] = model.ItemDef{Index: tc.item}
			w.volatiles.Items = map[uint16]model.VolatileRule{tc.item: {Action: tc.action, Consume: true}}
			w.volatiles.ItemCodes = map[uint16]int{tc.item: 1}

			w.onUseItem(p.Session, useItemPacket(0, 0))

			if p.Char.Inv[0].Index != tc.item || p.Char.Evolution != "" || p.Session.QueuedPacketsForTest() != wantPackets {
				t.Fatalf("ineligible %s use changed state or returned the wrong response: item=%d evolution=%q packets=%d want=%d",
					tc.name, p.Char.Inv[0].Index, p.Char.Evolution, p.Session.QueuedPacketsForTest(), wantPackets)
			}
		})
	}
}

func TestOnUseItemDispatchesGateKeyRoute(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	p, session := playerWithKey(w, 451, 2100, 2100)
	w.volatiles = model.VolatileCatalog{
		Items:     map[uint16]model.VolatileRule{451: {Action: "gate_key", Consume: true}},
		ItemCodes: map[uint16]int{451: 3},
	}

	w.onUseItem(session, useItemPacket(0, 0))

	if gate.State != gateOpen || p.Char.Inv[0].Index != 0 {
		t.Fatalf("use-item dispatch did not open the matching gate: state=%d key=%d", gate.State, p.Char.Inv[0].Index)
	}
}
