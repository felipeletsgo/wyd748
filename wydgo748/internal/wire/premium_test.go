package wire

import (
	"encoding/binary"
	"testing"
)

func TestPremiumCategorizedStateLayout(t *testing.T) {
	var prices [PremiumShopSlots]uint32
	var categories [PremiumShopSlots]uint8
	prices[2], prices[26] = 1234, 5678
	categories[2], categories[26] = 1, 3
	b := PremiumCategorizedShopState(9, 42, 7000, prices, categories)
	h := ParseHeader(b)
	if len(b) != 160 || int(h.Size) != 160 || h.Type != OpPremiumShopState || h.ID != 9 ||
		binary.LittleEndian.Uint16(b[12:14]) != 4 || binary.LittleEndian.Uint16(b[14:16]) != PremiumStateOpen ||
		binary.LittleEndian.Uint32(b[16:20]) != 42 || binary.LittleEndian.Uint32(b[20:24]) != 7000 ||
		binary.LittleEndian.Uint32(b[32:36]) != 1234 || binary.LittleEndian.Uint32(b[128:132]) != 5678 ||
		b[134] != 1 || b[158] != 3 || b[159] != 0 {
		t.Fatalf("categorized state layout: % x", b)
	}
}

func TestPremiumShopStateLayout(t *testing.T) {
	var prices [PremiumShopSlots]uint32
	prices[0], prices[26] = 1000, 25
	b := PremiumShopState(9, PremiumStateOpen, 42, 7000, prices)
	h := ParseHeader(b)
	if len(b) != PremiumShopStateSize || h.Type != OpPremiumShopState || h.ID != 9 ||
		binary.LittleEndian.Uint16(b[12:14]) != PremiumVersion ||
		binary.LittleEndian.Uint16(b[14:16]) != PremiumStateOpen ||
		binary.LittleEndian.Uint32(b[16:20]) != 42 || binary.LittleEndian.Uint32(b[20:24]) != 7000 ||
		binary.LittleEndian.Uint32(b[24:28]) != 1000 || binary.LittleEndian.Uint32(b[128:132]) != 25 {
		t.Fatalf("open state layout: % x", b)
	}
	balance := PremiumBalance(9, 300)
	if binary.LittleEndian.Uint16(balance[14:16]) != PremiumStateBalance ||
		binary.LittleEndian.Uint32(balance[16:20]) != 0 || binary.LittleEndian.Uint32(balance[20:24]) != 300 {
		t.Fatal("balance state layout")
	}
	for i := 24; i < PremiumShopStateSize; i++ {
		if balance[i] != 0 {
			t.Fatal("balance state must not carry prices")
		}
	}
}

func TestPremiumBuyParse(t *testing.T) {
	req := PremiumBuyRequest{Version: PremiumVersion, ID: 9, NPC: 1200, ShopSlot: 26, CarryHint: 5, CatalogVersion: 42, Price: 1000}
	b := PremiumBuyPacket(req)
	got, err := ParsePremiumBuy(b)
	if err != nil || got != req {
		t.Fatalf("parse: %+v %v", got, err)
	}
	for n := 0; n < PremiumBuySize; n++ {
		if _, err := ParsePremiumBuy(b[:n]); err == nil {
			t.Fatalf("truncated %d accepted", n)
		}
	}
	if _, err := ParsePremiumBuy(append(append([]byte{}, b...), 0)); err == nil {
		t.Fatal("oversized packet accepted")
	}
	mutate := func(off int, v uint16) []byte {
		c := append([]byte{}, b...)
		binary.LittleEndian.PutUint16(c[off:off+2], v)
		return c
	}
	for name, bad := range map[string][]byte{
		"declared size": mutate(0, PremiumBuySize+1),
		"opcode":        mutate(4, OpPremiumShopState),
		"version":       mutate(12, 3),
		"slot":          mutate(16, PremiumShopSlots),
		"price":         func() []byte { c := append([]byte{}, b...); binary.LittleEndian.PutUint32(c[24:28], 0); return c }(),
	} {
		if _, err := ParsePremiumBuy(bad); err == nil {
			t.Fatalf("%s accepted", name)
		}
	}
}

func TestPremiumWebCompletionContract(t *testing.T) {
	req := PremiumBuyRequest{Version: PremiumWebVersion, ID: 9, NPC: 1200, ShopSlot: 2, CatalogVersion: 42, Price: 1000, RequestID: 7}
	got, err := ParsePremiumBuy(PremiumBuyPacket(req))
	if err != nil || got != req {
		t.Fatalf("web request: %+v %v", got, err)
	}
	b := PremiumBuyResult(9, PremiumResultInventoryFull, 7000, 7)
	h := ParseHeader(b)
	if len(b) != 24 || h.Type != OpPremiumBuyResult || h.ID != 9 ||
		binary.LittleEndian.Uint16(b[12:]) != 3 || binary.LittleEndian.Uint16(b[14:]) != PremiumResultInventoryFull ||
		binary.LittleEndian.Uint32(b[16:]) != 7000 || binary.LittleEndian.Uint32(b[20:]) != 7 {
		t.Fatalf("completion layout: %x", b)
	}
	request := PremiumBuyPacket(req)
	for n := 0; n < PremiumWebBuySize; n++ {
		if _, err := ParsePremiumBuy(request[:n]); err == nil {
			t.Fatalf("truncated web request accepted at %d", n)
		}
	}
	for _, version := range []uint16{1, 2, 4} {
		binary.LittleEndian.PutUint16(request[12:], version)
		if _, err := ParsePremiumBuy(request); err == nil {
			t.Fatalf("invalid web version accepted: %d", version)
		}
	}
	binary.LittleEndian.PutUint16(request[12:], PremiumWebVersion)
	binary.LittleEndian.PutUint32(request[28:], 0)
	if _, err := ParsePremiumBuy(request); err == nil {
		t.Fatal("zero request ID accepted")
	}
}
