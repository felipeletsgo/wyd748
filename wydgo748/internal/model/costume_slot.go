package model

// Costume slot contract (EXTENSAO_COORDENADA, client NativeCostumeSlot.h).
//
// Native 7.48 equips costumes and familiars in slot 13 and the guild medal in
// slot 12. Medals are server-side state in this project, so costumes move to
// slot 12 and slot 13 keeps familiars, letting both be worn at once. A costume
// is an item whose itemlist entry uses the costume body mesh 36 and the native
// slot-13 position; the client applies the same rule to its own item list.
const (
	CostumeSlot           = 12
	FamiliarSlot          = 13
	CostumeMesh           = 36
	CostumeSlotMask       = 1 << CostumeSlot
	NativeCostumeSlotMask = 1 << FamiliarSlot
)

// IsCostumeDef reports whether def is a costume under either position.
func IsCostumeDef(def ItemDef) bool {
	return def.Mesh == CostumeMesh && (def.Pos == NativeCostumeSlotMask || def.Pos == CostumeSlotMask)
}

// RemapCostumeSlot moves a native slot-13 costume to the costume slot. It is
// applied once when the item catalog is loaded.
func RemapCostumeSlot(def *ItemDef) {
	if def != nil && def.Mesh == CostumeMesh && def.Pos == NativeCostumeSlotMask {
		def.Pos = CostumeSlotMask
	}
}

// MigrateCostumeSlot moves a costume persisted in slot 13 by an older build to
// the empty costume slot. It reports whether the character changed; an
// occupied slot 12 leaves the item in place for the operator to resolve.
func MigrateCostumeSlot(ch *Char, items map[uint16]ItemDef) bool {
	if ch == nil {
		return false
	}
	legacy := ch.Equip[FamiliarSlot]
	def, ok := items[legacy.Index]
	if legacy.Index == 0 || !ok || !IsCostumeDef(def) || ch.Equip[CostumeSlot].Index != 0 {
		return false
	}
	ch.Equip[CostumeSlot], ch.Equip[FamiliarSlot] = legacy, Item{}
	return true
}
