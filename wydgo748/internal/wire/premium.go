package wire

import (
	"encoding/binary"
	"errors"
)

// Premium shop, EXTENSAO_COORDENADA v1 (DOCS/protocol/premium-shop.md).
// The shop window itself is the native 0x17C with ShopPremium; these packets
// carry the donate prices, the balance and the purchase request.
const (
	OpPremiumShopState = 0x7F20
	OpPremiumBuy       = 0x7F21

	PremiumShopStateSize = 132
	PremiumBuySize       = 28
	PremiumVersion       = 1
	PremiumShopSlots     = 27

	// ShopPremium is the 0x17C ShopType the source client renders as the
	// premium shop; stock 7.48 ignores it.
	ShopPremium = 4

	PremiumStateBalance = 0
	PremiumStateOpen    = 1
)

// PremiumShopState builds 0x7F20. Kind PremiumStateBalance only reports the
// balance; PremiumStateOpen also carries the catalog version and prices.
func PremiumShopState(id uint16, kind uint16, catalogVersion, balance uint32, prices [PremiumShopSlots]uint32) []byte {
	b := Build(OpPremiumShopState, id, PremiumShopStateSize)
	putU16(b, 12, PremiumVersion)
	putU16(b, 14, kind)
	if kind == PremiumStateOpen {
		putU32(b, 16, catalogVersion)
		for i, price := range prices {
			putU32(b, 24+i*4, price)
		}
	}
	putU32(b, 20, balance)
	return b
}

// PremiumBalance is the kind-0 0x7F20 sent after any balance change.
func PremiumBalance(id uint16, balance uint32) []byte {
	return PremiumShopState(id, PremiumStateBalance, 0, balance, [PremiumShopSlots]uint32{})
}

type PremiumBuyRequest struct {
	ID             uint16
	NPC            uint16
	ShopSlot       uint16
	CarryHint      uint16
	CatalogVersion uint32
	Price          uint32
}

var errPremiumBuy = errors.New("premium buy: malformed packet")

// ParsePremiumBuy accepts only the exact v1 0x7F21 envelope.
func ParsePremiumBuy(b []byte) (PremiumBuyRequest, error) {
	var r PremiumBuyRequest
	if len(b) != PremiumBuySize {
		return r, errPremiumBuy
	}
	h := ParseHeader(b)
	if int(h.Size) != PremiumBuySize || h.Type != OpPremiumBuy ||
		binary.LittleEndian.Uint16(b[12:14]) != PremiumVersion {
		return r, errPremiumBuy
	}
	r.ID = h.ID
	r.NPC = binary.LittleEndian.Uint16(b[14:16])
	r.ShopSlot = binary.LittleEndian.Uint16(b[16:18])
	r.CarryHint = binary.LittleEndian.Uint16(b[18:20])
	r.CatalogVersion = binary.LittleEndian.Uint32(b[20:24])
	r.Price = binary.LittleEndian.Uint32(b[24:28])
	if r.ShopSlot >= PremiumShopSlots || r.Price == 0 {
		return r, errPremiumBuy
	}
	return r, nil
}

// PremiumBuyPacket builds a 0x7F21 request; used by tests and tools.
func PremiumBuyPacket(r PremiumBuyRequest) []byte {
	b := Build(OpPremiumBuy, r.ID, PremiumBuySize)
	putU16(b, 0, PremiumBuySize)
	putU16(b, 12, PremiumVersion)
	putU16(b, 14, r.NPC)
	putU16(b, 16, r.ShopSlot)
	putU16(b, 18, r.CarryHint)
	putU32(b, 20, r.CatalogVersion)
	putU32(b, 24, r.Price)
	return b
}
