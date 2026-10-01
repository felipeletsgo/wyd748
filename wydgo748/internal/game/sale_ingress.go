package game

import (
	"encoding/binary"

	"wydgo/internal/model"
)

// validSaleInventorySource validates the native signed words before the legacy
// sale handler projects them into byte-sized inventory coordinates. Otherwise
// a high-byte alias can sell a different item or cancel a live trade.
func validSaleInventorySource(pkt []byte) bool {
	if len(pkt) != 20 {
		return false
	}
	sourceType := int16(binary.LittleEndian.Uint16(pkt[14:16]))
	position := int16(binary.LittleEndian.Uint16(pkt[16:18]))
	return sourceType == placeInv && position >= 0 && position < model.PlayerCarrySlots
}
