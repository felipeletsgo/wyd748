package wire

import (
	"encoding/binary"
	"errors"
)

// Premium shop, EXTENSAO_COORDENADA v1/v3/v4 (DOCS/protocol/premium-shop.md).
// Categorized opens use v4; balances and legacy purchases retain v1, web buys v3.
const (
	OpPremiumShopState = 0x7F20
	OpPremiumBuy       = 0x7F21
	OpPremiumBuyResult = 0x7F22

	PremiumShopStateSize     = 132
	PremiumBuySize           = 28
	PremiumVersion           = 1
	PremiumWebVersion        = 3
	PremiumWebBuySize        = 32
	PremiumBuyResultSize     = 24
	PremiumShopSlots         = 27
	PremiumCategoryVersion   = 4
	PremiumCategoryStateSize = 160

	// ShopPremium is the 0x17C ShopType the source client renders as the
	// premium shop; stock 7.48 ignores it.
	ShopPremium = 4

	PremiumStateBalance = 0
	PremiumStateOpen    = 1
)

// Web purchases receive an explicit completion only after durable settlement.
const (
	PremiumResultSuccess uint16 = iota
	PremiumResultUnavailable
	PremiumResultBusy
	PremiumResultExpired
	PremiumResultCatalogChanged
	PremiumResultInsufficient
	PremiumResultInventoryFull
	PremiumResultItemUnavailable
	PremiumResultFailed
)

func PremiumBuyResult(id, code uint16, balance, requestID uint32) []byte {
	b := Build(OpPremiumBuyResult, id, PremiumBuyResultSize)
	putU16(b, 12, PremiumWebVersion)
	putU16(b, 14, code)
	putU32(b, 16, balance)
	putU32(b, 20, requestID)
	return b
}

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

// PremiumCategorizedShopState appends one category byte per slot and one
// reserved zero byte to the v1 open-state layout. Purchases remain v1/v3.
func PremiumCategorizedShopState(id uint16, catalogVersion, balance uint32, prices [PremiumShopSlots]uint32, categories [PremiumShopSlots]uint8) []byte {
	b := make([]byte, PremiumCategoryStateSize)
	copy(b, PremiumShopState(id, PremiumStateOpen, catalogVersion, balance, prices))
	putU16(b, 0, PremiumCategoryStateSize)
	putU16(b, 12, PremiumCategoryVersion)
	copy(b[PremiumShopStateSize:], categories[:])
	return b
}

type PremiumBuyRequest struct {
	Version        uint16
	ID             uint16
	NPC            uint16
	ShopSlot       uint16
	CarryHint      uint16
	CatalogVersion uint32
	Price          uint32
	RequestID      uint32
}

var errPremiumBuy = errors.New("premium buy: malformed packet")

// ParsePremiumBuy accepts the legacy grid and web completion envelopes.
func ParsePremiumBuy(b []byte) (PremiumBuyRequest, error) {
	var r PremiumBuyRequest
	if len(b) != PremiumBuySize && len(b) != PremiumWebBuySize {
		return r, errPremiumBuy
	}
	h := ParseHeader(b)
	r.Version = binary.LittleEndian.Uint16(b[12:14])
	if int(h.Size) != len(b) || h.Type != OpPremiumBuy ||
		!((r.Version == PremiumVersion && len(b) == PremiumBuySize) ||
			(r.Version == PremiumWebVersion && len(b) == PremiumWebBuySize)) {
		return r, errPremiumBuy
	}
	r.ID = h.ID
	r.NPC = binary.LittleEndian.Uint16(b[14:16])
	r.ShopSlot = binary.LittleEndian.Uint16(b[16:18])
	r.CarryHint = binary.LittleEndian.Uint16(b[18:20])
	r.CatalogVersion = binary.LittleEndian.Uint32(b[20:24])
	r.Price = binary.LittleEndian.Uint32(b[24:28])
	if r.Version == PremiumWebVersion {
		r.RequestID = binary.LittleEndian.Uint32(b[28:32])
		if r.RequestID == 0 {
			return r, errPremiumBuy
		}
	}
	if r.ShopSlot >= PremiumShopSlots || r.Price == 0 {
		return r, errPremiumBuy
	}
	return r, nil
}

// PremiumBuyPacket builds a 0x7F21 request; used by tests and tools.
func PremiumBuyPacket(r PremiumBuyRequest) []byte {
	size := PremiumBuySize
	if r.Version == PremiumWebVersion {
		size = PremiumWebBuySize
	}
	b := Build(OpPremiumBuy, r.ID, size)
	putU16(b, 0, uint16(size))
	version := r.Version
	if version == 0 {
		version = PremiumVersion
	}
	putU16(b, 12, version)
	putU16(b, 14, r.NPC)
	putU16(b, 16, r.ShopSlot)
	putU16(b, 18, r.CarryHint)
	putU32(b, 20, r.CatalogVersion)
	putU32(b, 24, r.Price)
	if r.Version == PremiumWebVersion {
		putU32(b, 28, r.RequestID)
	}
	return b
}
