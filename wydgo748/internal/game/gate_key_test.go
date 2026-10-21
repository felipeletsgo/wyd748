package game

import (
	"encoding/binary"
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// A key and a gate are matched by EF_KEYID, never by item index: the item list
// gives First_Gate_Key (451) and the gate (458) the same EF_KEYID 2.

func worldWithGate(gateKeyID int, gateX, gateY uint16) (*World, *GroundItem) {
	w := &World{
		groundItems: map[uint16]*GroundItem{},
		items: map[uint16]model.ItemDef{
			451: {Index: 451, Name: "First_Gate_Key",
				StaticEffects: []model.StaticEffect{{Name: "EF_KEYID", Value: 2}}},
			452: {Index: 452, Name: "Second_Gate_Key",
				StaticEffects: []model.StaticEffect{{Name: "EF_KEYID", Value: 3}}},
			458: {Index: 458, Name: "First_Gate",
				StaticEffects: []model.StaticEffect{{Name: "EF_KEYID", Value: gateKeyID}}},
		},
		volatiles: model.VolatileCatalog{},
	}
	gate := &GroundItem{ID: 10000, Item: model.Item{Index: 458},
		X: gateX, Y: gateY, Permanent: true, State: gateClosed}
	w.groundItems[gate.ID] = gate
	return w, gate
}

func playerWithKey(w *World, keyIndex uint16, x, y uint16) (*Player, *net.Session) {
	session := net.NewTestSession(1, 64)
	acc := &model.Account{Name: "account", Chars: []model.Char{{Name: "A",
		Score: &model.Score{Version: model.ScoreVersion, MaxHP: 100, CurHP: 100}}}}
	p := &Player{ID: 1, Session: session, Account: acc, Char: &acc.Chars[0],
		InWorld: true, X: x, Y: y, Visible: map[uint16]struct{}{}}
	p.Char.Inv[0] = model.Item{Index: keyIndex}
	w.players = map[*net.Session]*Player{session: p}
	w.store = &craftStore{}
	return p, session
}

func TestGateKeyOpensGateAndDisappears(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	p, s := playerWithKey(w, 451, 2100, 2100)

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if gate.State != gateOpen {
		t.Error("the gate remained closed")
	}
	if p.Char.Inv[0].Index != 0 {
		t.Errorf("the key remained in the inventory: %d", p.Char.Inv[0].Index)
	}
}

// EF_KEYID is authoritative. A key with another ID cannot open or be consumed.
func TestWrongKeyDoesNotOpenGate(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	p, s := playerWithKey(w, 452, 2100, 2100) // EF_KEYID 3, gate is 2

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if gate.State != gateClosed {
		t.Error("the gate opened with the wrong key")
	}
	if p.Char.Inv[0].Index != 452 {
		t.Error("the wrong key was consumed")
	}
}

// A key used from beyond the gate reach cannot open or consume anything.
func TestDistantKeyDoesNotOpenGate(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	p, s := playerWithKey(w, 451, 2100+uint16(gateReach)+5, 2100)

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if gate.State != gateClosed {
		t.Error("the gate opened from a distance")
	}
	if p.Char.Inv[0].Index != 451 {
		t.Error("the key was consumed without opening a gate")
	}
}

// Persist-before-confirm: a failed save cannot leave the player without the
// key while the gate remains closed.
func TestKeyIsRestoredWhenSaveFails(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	p, s := playerWithKey(w, 451, 2100, 2100)
	w.store = &craftStore{err: errors.New("disk full")}

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if p.Char.Inv[0].Index != 451 {
		t.Error("the key disappeared despite the persistence failure")
	}
	if gate.State != gateClosed {
		t.Error("the gate opened despite the persistence failure")
	}
}

func TestOpenGateDoesNotConsumeKey(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	gate.State = gateOpen
	p, s := playerWithKey(w, 451, 2100, 2100)

	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if p.Char.Inv[0].Index != 451 {
		t.Error("the key was consumed by an already open gate")
	}
}

// The state must travel in CreateItem, or a late viewer would see a closed gate
// and waste another key. State is byte 27 in the packet.
func TestOpenGateStateReachesLateViewer(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	p, s := playerWithKey(w, 451, 2100, 2100)
	w.useGateKey(s, p, &p.Char.Inv[0], 0, model.VolatileRule{Consume: true}, 3)

	if gate.State != gateOpen {
		t.Fatal("the gate did not open")
	}
	pkt := wire.CreateItem(gate.X, gate.Y, gate.ID, gate.Item, gate.Rotate,
		gate.State, 0, 0, 0)
	if len(pkt) != 32 {
		t.Fatalf("CreateItem has %d bytes, want 32", len(pkt))
	}
	if pkt[27] != gateOpen {
		t.Errorf("the gate was sent with State=%d, want %d (open)", pkt[27], gateOpen)
	}
}

func updateGroundItemPacket(id uint16, state uint32) []byte {
	pkt := make([]byte, 20)
	binary.LittleEndian.PutUint32(pkt[12:16], uint32(id))
	binary.LittleEndian.PutUint32(pkt[16:20], state)
	return pkt
}

func TestUpdateGroundItemUsesMatchingInventoryKey(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	p, s := playerWithKey(w, 451, 2100, 2100)
	p.show(gate.ID)

	w.onUpdateGroundItem(s, updateGroundItemPacket(gate.ID, uint32(gateOpen)))

	if gate.State != gateOpen || p.Char.Inv[0].Index != 0 {
		t.Fatalf("0x374 did not open/consume: state=%d key=%+v", gate.State, p.Char.Inv[0])
	}
}

func TestUpdateGroundItemRejectsForgedStateAndMissingKey(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	p, s := playerWithKey(w, 452, 2100, 2100)
	p.show(gate.ID)

	w.onUpdateGroundItem(s, updateGroundItemPacket(gate.ID, 6))
	if gate.State != gateClosed || p.Char.Inv[0].Index != 452 {
		t.Fatal("a forged state changed the gate")
	}

	w.onUpdateGroundItem(s, updateGroundItemPacket(gate.ID, uint32(gateOpen)))
	if gate.State != gateClosed || p.Char.Inv[0].Index != 452 {
		t.Fatal("a key with a different EF_KEYID opened the gate")
	}
}

func TestUpdateGroundItemRollsBackKeyWhenSaveFails(t *testing.T) {
	w, gate := worldWithGate(2, 2100, 2100)
	p, s := playerWithKey(w, 451, 2100, 2100)
	p.show(gate.ID)
	w.store = &craftStore{err: errors.New("postgres unavailable")}

	w.onUpdateGroundItem(s, updateGroundItemPacket(gate.ID, uint32(gateOpen)))

	if gate.State != gateClosed || p.Char.Inv[0].Index != 451 {
		t.Fatalf("save failure split the key/gate state: state=%d key=%+v",
			gate.State, p.Char.Inv[0])
	}
}
