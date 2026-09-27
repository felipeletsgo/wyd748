package game

import (
	"wydgo/internal/model"
)

// Each bit is a nonzero cell in the client's 6x6 g_pGroundMask[mask][rotation]
// (Basedef.h). The fixture positions and rotations come from init_items.csv;
// this table describes only the client-side collision contract.
var groundMaskBits = [10][4]uint64{
	{},
	{0x00000e000, 0x000104100, 0x00000e000, 0x000104100},
	{0x00000f000, 0x004104100, 0x00001e000, 0x004104100},
	{0x00000c000, 0x000104000, 0x00000c000, 0x000104000},
	{0x00000f000, 0x004104100, 0x00001e000, 0x004104100},
	{0xfc0000fc0, 0x492492492, 0x03f00003f, 0x492492492},
	{0x000004000, 0x000008000, 0x000200000, 0x000100000},
	{0x000004000, 0x000004000, 0x000004000, 0x000004000},
	{0x01e79e780, 0x01e79e780, 0x01e79e780, 0x01e79e780},
	{0x00001f000, 0x004104104, 0x00001f000, 0x004104104},
}

// applyGroundItemHeight mirrors BASE_UpdateItem2: every selected height cell
// becomes 16 for a closed object or 0 for an opened one. The world owns a
// private Height slice so opening a gate cannot mutate the loaded base map.
func (w *World) applyGroundItemHeight(item *GroundItem) {
	if item == nil || !w.terrain.Loaded() {
		return
	}
	def, ok := w.items[item.Item.Index]
	if !ok {
		return
	}
	mask := staticEffect(def, "EF_GROUND")
	if mask <= 0 || mask >= len(groundMaskBits) || item.Rotate >= 4 {
		return
	}
	bits := groundMaskBits[mask][item.Rotate]
	height := byte(0)
	if item.State == gateClosed {
		height = clientGateHeight
	}
	for cell := 0; cell < 36; cell++ {
		if bits&(uint64(1)<<cell) == 0 {
			continue
		}
		x := int(item.X) + cell%6 - 2
		y := int(item.Y) + cell/6 - 2
		if x >= 0 && x < model.TerrainWidth && y >= 0 && y < model.TerrainHeight {
			w.terrain.Height[y*model.TerrainWidth+x] = height
		}
	}
}
