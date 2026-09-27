package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestGroundItemCreatePacketProjectsClientGateState(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		458: {Index: 458, StaticEffects: []model.StaticEffect{
			{Name: "EF_GROUND", Value: 1}, {Name: "EF_KEYID", Value: 2},
		}},
		471: {Index: 471, StaticEffects: []model.StaticEffect{
			{Name: "EF_GROUND", Value: 2},
		}},
	}}
	tests := []struct {
		name       string
		index      uint16
		server     byte
		wantState  byte
		wantHeight byte
	}{
		{"locked keyed gate", 458, gateClosed, clientGateLocked, clientGateHeight},
		{"open keyed gate", 458, gateOpen, clientGateOpen, 0},
		{"closed unkeyed obstacle", 471, gateClosed, clientGateClosed, clientGateHeight},
		{"ordinary item", 900, gateClosed, gateClosed, 0},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			g := &GroundItem{ID: 10000, Item: model.Item{Index: tt.index},
				X: 2100, Y: 2101, Rotate: 2, Permanent: true, State: tt.server}
			pkt := w.groundItemCreatePacket(g)
			if len(pkt) != 32 || pkt[26] != g.Rotate || pkt[27] != tt.wantState || pkt[28] != tt.wantHeight {
				t.Fatalf("CreateItem length/rotation/state/height = %d/%d/%d/%d; want 32/%d/%d/%d",
					len(pkt), pkt[26], pkt[27], pkt[28], g.Rotate, tt.wantState, tt.wantHeight)
			}
			if g.State != tt.server {
				t.Fatalf("packet projection changed authoritative state to %d", g.State)
			}
		})
	}
}

func TestGroundItemUpdatePacketRestoresClosedCollision(t *testing.T) {
	g := &GroundItem{ID: 10000, State: gateClosed}
	closed := groundItemUpdatePacket(g)
	if len(closed) != 20 || closed[16] != gateClosed || closed[18] != clientGateHeight {
		t.Fatalf("closed gate update length/state/height = %d/%d/%d; want 20/0/16",
			len(closed), closed[16], closed[18])
	}
	g.State = gateOpen
	open := groundItemUpdatePacket(g)
	if len(open) != 20 || open[16] != gateOpen || open[18] != 0 {
		t.Fatalf("open gate update length/state/height = %d/%d/%d; want 20/1/0",
			len(open), open[16], open[18])
	}
}
