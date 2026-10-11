package wire

import (
	"encoding/binary"
	"testing"
)

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
	req := PremiumBuyRequest{ID: 9, NPC: 1200, ShopSlot: 26, CarryHint: 5, CatalogVersion: 42, Price: 1000}
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
		"version":       mutate(12, 2),
		"slot":          mutate(16, PremiumShopSlots),
		"price":         func() []byte { c := append([]byte{}, b...); binary.LittleEndian.PutUint32(c[24:28], 0); return c }(),
	} {
		if _, err := ParsePremiumBuy(bad); err == nil {
			t.Fatalf("%s accepted", name)
		}
	}
}
