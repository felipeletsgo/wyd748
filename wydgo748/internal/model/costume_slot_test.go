package model

import "testing"

func TestMigrateCostumeSlot(t *testing.T) {
	items := map[uint16]ItemDef{
		4150: {Index: 4150, Mesh: CostumeMesh, Pos: CostumeSlotMask},
		3900: {Index: 3900, Mesh: 2773, Pos: NativeCostumeSlotMask},
		508:  {Index: 508, Mesh: 10, Pos: CostumeSlotMask},
	}
	costume := Item{Index: 4150, UID: "c", ActivatedUnix: 10, ExpiresUnix: 20}

	ch := &Char{}
	ch.Equip[FamiliarSlot] = costume
	if !MigrateCostumeSlot(ch, items) || ch.Equip[CostumeSlot] != costume || ch.Equip[FamiliarSlot].Index != 0 {
		t.Fatalf("legacy costume was not moved with its UID and lifetime: %+v", ch.Equip[CostumeSlot])
	}
	if MigrateCostumeSlot(ch, items) {
		t.Fatal("a migrated character changed again")
	}

	familiar := &Char{}
	familiar.Equip[FamiliarSlot] = Item{Index: 3900}
	if MigrateCostumeSlot(familiar, items) || familiar.Equip[FamiliarSlot].Index != 3900 {
		t.Fatal("a familiar left slot 13")
	}

	occupied := &Char{}
	occupied.Equip[FamiliarSlot] = costume
	occupied.Equip[CostumeSlot] = Item{Index: 508}
	if MigrateCostumeSlot(occupied, items) || occupied.Equip[CostumeSlot].Index != 508 || occupied.Equip[FamiliarSlot] != costume {
		t.Fatal("migration overwrote an occupied costume slot")
	}
	if MigrateCostumeSlot(nil, items) {
		t.Fatal("nil character reported a change")
	}
}

func TestRemapCostumeSlotOnlyMovesNativeCostumes(t *testing.T) {
	costume := ItemDef{Mesh: CostumeMesh, Pos: NativeCostumeSlotMask}
	RemapCostumeSlot(&costume)
	familiar := ItemDef{Mesh: 46, Pos: NativeCostumeSlotMask}
	RemapCostumeSlot(&familiar)
	armor := ItemDef{Mesh: CostumeMesh, Pos: 4}
	RemapCostumeSlot(&armor)
	if costume.Pos != CostumeSlotMask || familiar.Pos != NativeCostumeSlotMask || armor.Pos != 4 {
		t.Fatalf("costume=%d familiar=%d armor=%d", costume.Pos, familiar.Pos, armor.Pos)
	}
	RemapCostumeSlot(nil)
}
