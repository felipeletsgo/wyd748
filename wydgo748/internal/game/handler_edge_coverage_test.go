package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestOnSwapItemRejectsExpiredTimedEquipmentWithoutMutation(t *testing.T) {
	w, p, store, session := newTimedSwapWorld(t, nil)
	now := w.now()
	expired := p.Char.Inv[0]
	expired.ActivatedUnix = now.Add(-31 * 24 * time.Hour).Unix()
	expired.ExpiresUnix = now.Add(-24 * time.Hour).Unix()
	p.Char.Inv[0] = expired

	w.onSwapItem(session, timedSwapPacket(placeInv, 0, placeEquip, fairySlot))
	if p.Char.Inv[0] != expired || p.Char.Equip[fairySlot] != (model.Item{}) || store.saves != 0 {
		t.Fatalf("expired item changed inventory/equipment or persisted: inv=%+v equip=%+v saves=%d",
			p.Char.Inv[0], p.Char.Equip[fairySlot], store.saves)
	}
	for i, want := range []uint16{wire.OpSendItem, wire.OpSendItem, wire.OpMessagePanel} {
		packet, ok := session.DequeuePacketForTest()
		if !ok || !wire.Decrypt(packet) || wire.ParseHeader(packet).Type != want {
			t.Fatalf("rejection packet %d missing or has wrong type: ok=%t packet=%v", i, ok, packet)
		}
	}
	if session.QueuedPacketsForTest() != 0 {
		t.Fatal("expired item rejection queued extra packets")
	}
}

func TestClientEquipProjectionReturnsEmptyEquipmentForNilCharacter(t *testing.T) {
	if got := clientEquipProjection(nil); got != ([model.MaxEquipSlots]model.Item{}) {
		t.Fatalf("nil character projection=%+v", got)
	}
}

func TestFinalizeIronSpearSecondaryIgnoresMissingPlayerTarget(t *testing.T) {
	(&World{}).finalizeIronSpearSecondary(nil, ironSpearSecondaryResult{ok: true})
}
