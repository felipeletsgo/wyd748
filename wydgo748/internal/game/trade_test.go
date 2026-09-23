package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func tradePacket(opponent uint16, ch *model.Char, slots ...int) []byte {
	var items [maxTradeItems]model.Item
	positions := emptyTradePositions()
	for i, slot := range slots {
		items[i] = ch.Inv[slot]
		positions[i] = int8(slot)
	}
	return wire.Trade(1, items, positions, 0, false, opponent)
}

func TestParseTradeRequestUsesAuthoritativeInventory(t *testing.T) {
	ch := &model.Char{Gold: 500}
	ch.Inv[7] = model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	pkt := tradePacket(2, ch, 7)
	binary.LittleEndian.PutUint32(pkt[148:152], 300)

	req, err := parseTradeRequest(pkt, ch)
	if err != nil {
		t.Fatal(err)
	}
	if req.OpponentID != 2 || req.Gold != 300 || req.CarryPos[0] != 7 || req.Items[0] != ch.Inv[7] {
		t.Fatalf("incorrect offer: %+v", req)
	}
}

func TestParseTradeRequestIgnoresResidualPositionsForEmptyItems(t *testing.T) {
	ch := &model.Char{}
	ch.Inv[0] = model.Item{Index: 4011, Eff: [6]byte{43, 9}}
	pkt := tradePacket(2, ch)
	for i := 132; i < 147; i++ {
		pkt[i] = 0
	}

	req, err := parseTradeRequest(pkt, ch)
	if err != nil {
		t.Fatal(err)
	}
	if req.Items != [maxTradeItems]model.Item{} {
		t.Fatalf("empty invitation produced items: %+v", req.Items)
	}
	if req.CarryPos != emptyTradePositions() {
		t.Fatalf("residual positions were not canonicalized: %+v", req.CarryPos)
	}
}

func TestParseTradeRequestRejectsEffectsWithoutItem(t *testing.T) {
	ch := &model.Char{}
	pkt := tradePacket(2, ch)
	pkt[14] = 1

	if _, err := parseTradeRequest(pkt, ch); err == nil {
		t.Fatal("effect without an item was accepted")
	}
}

func TestParseTradeRequestRejectsTamperingAndDuplicatePosition(t *testing.T) {
	ch := &model.Char{Gold: 500}
	ch.Inv[7] = model.Item{Index: 4011}

	tampered := tradePacket(2, ch, 7)
	binary.LittleEndian.PutUint16(tampered[12:14], 4012)
	if _, err := parseTradeRequest(tampered, ch); err == nil {
		t.Fatal("tampered item was accepted")
	}

	duplicate := tradePacket(2, ch, 7, 7)
	if _, err := parseTradeRequest(duplicate, ch); err == nil {
		t.Fatal("duplicate position was accepted")
	}

	tooMuchGold := tradePacket(2, ch)
	binary.LittleEndian.PutUint32(tooMuchGold[148:152], 501)
	if _, err := parseTradeRequest(tooMuchGold, ch); err == nil {
		t.Fatal("gold offer above the balance was accepted")
	}
}

func TestBuildTradeInventoryRemovesOfferThenReceivesItems(t *testing.T) {
	ch := &model.Char{}
	ch.Inv[0] = model.Item{Index: 100}
	ch.Inv[1] = model.Item{Index: 101}
	outgoing := &TradeState{CarryPos: emptyTradePositions()}
	outgoing.CarryPos[0], outgoing.Items[0] = 0, ch.Inv[0]
	incoming := &TradeState{CarryPos: emptyTradePositions()}
	incoming.Items[0] = model.Item{Index: 200}
	incoming.Items[1] = model.Item{Index: 201}

	got, ok := buildTradeInventory(ch, outgoing, incoming)
	if !ok || got[0].Index != 200 || got[1].Index != 101 || got[2].Index != 201 {
		t.Fatalf("incorrect final inventory: ok=%v slots=%d,%d,%d", ok, got[0].Index, got[1].Index, got[2].Index)
	}
}

func TestBuildTradeInventoryRejectsFullDestinationAndChangedOffer(t *testing.T) {
	ch := &model.Char{}
	for i := 0; i < model.PlayerCarrySlots; i++ {
		ch.Inv[i] = model.Item{Index: uint16(100 + i)}
	}
	emptyOffer := &TradeState{CarryPos: emptyTradePositions()}
	incoming := &TradeState{CarryPos: emptyTradePositions(), Items: [maxTradeItems]model.Item{{Index: 999}}}
	if _, ok := buildTradeInventory(ch, emptyOffer, incoming); ok {
		t.Fatal("trade without space was accepted")
	}

	outgoing := &TradeState{CarryPos: emptyTradePositions()}
	outgoing.CarryPos[0] = 0
	outgoing.Items[0] = model.Item{Index: 777}
	if _, ok := buildTradeInventory(ch, outgoing, emptyOffer); ok {
		t.Fatal("changed item snapshot was accepted")
	}
}

func TestTradeGoldCapsAtTwoBillion(t *testing.T) {
	if got, ok := tradeGold(1000, 300, 500); !ok || got != 1200 {
		t.Fatalf("final gold=%d ok=%v", got, ok)
	}
	if _, ok := tradeGold(100, 101, 0); ok {
		t.Fatal("offer above the balance was accepted")
	}
	if _, ok := tradeGold(maxCharacterGold, 0, 1); ok {
		t.Fatal("gold overflow was accepted")
	}
}

func TestUpdateTradeOfferUncheckInvalidatesBothConfirmations(t *testing.T) {
	state := &TradeState{CarryPos: emptyTradePositions(), Checked: true}
	opponent := &TradeState{CarryPos: emptyTradePositions(), Checked: true}
	req := tradeRequest{CarryPos: emptyTradePositions()}
	if err := updateTradeOffer(state, opponent, req); err != nil {
		t.Fatal(err)
	}
	if state.Checked || opponent.Checked {
		t.Fatalf("confirmations were not invalidated: mine=%v opponent=%v", state.Checked, opponent.Checked)
	}
}

func TestUpdateTradeOfferCannotChangeAndConfirmTogether(t *testing.T) {
	state := &TradeState{CarryPos: emptyTradePositions()}
	opponent := &TradeState{CarryPos: emptyTradePositions()}
	req := tradeRequest{CarryPos: emptyTradePositions(), Checked: true, Gold: 1}
	if err := updateTradeOffer(state, opponent, req); err == nil {
		t.Fatal("change and confirmation in the same packet were accepted")
	}
}

func TestValidateTradableItemsUsesServerCatalog(t *testing.T) {
	w := &World{items: map[uint16]model.ItemDef{
		100: {Index: 100},
		101: {Index: 101, StaticEffects: []model.StaticEffect{{Name: "EF_NOTRADE", Value: 1}}},
	}}
	var items [maxTradeItems]model.Item
	items[0] = model.Item{Index: 100}
	if err := w.validateTradableItems(items); err != nil {
		t.Fatalf("tradable item was rejected: %v", err)
	}
	items[0] = model.Item{Index: 101}
	if err := w.validateTradableItems(items); err == nil || err.Error() != "This item cannot be traded." {
		t.Fatalf("expected English non-tradable item message, got %v", err)
	}
	items[0] = model.Item{Index: 102}
	if err := w.validateTradableItems(items); err == nil || err.Error() != "An unknown item cannot be traded." {
		t.Fatalf("expected English unknown item message, got %v", err)
	}
}
