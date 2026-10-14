package game

import "wydgo/internal/model"

// ensureCapeEmblem runs on a transaction's detached proposal (or the legacy
// synchronous snapshot), never on a persistence worker. A kingdom cape's first
// grant draws once; later promotions, re-equips and relogs retain that draw.
// Legacy equipped kingdom capes receive the same one-time migration at entry.
func (w *World) ensureCapeEmblem(ch *model.Char) bool {
	if ch == nil || model.KingdomFromCape(ch.Equip[model.CapeSlot].Index) == model.KingdomNeutral ||
		(ch.CapeEmblem >= 1 && ch.CapeEmblem <= model.CapeEmblemCount) {
		return false
	}
	ch.CapeEmblem = byte(w.intn(model.CapeEmblemCount) + 1)
	return true
}
