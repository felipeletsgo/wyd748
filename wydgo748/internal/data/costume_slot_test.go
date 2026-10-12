package data

import (
	"testing"

	"wydgo/internal/model"
)

// The costume-slot extension moves exactly the 135 mesh-36 costumes to slot 12;
// familiars, emblems and guild medals keep their native positions.
func TestCatalogMovesCostumesToCostumeSlot(t *testing.T) {
	catalog, err := LoadCatalog("../../data/itemlist.csv", "../../data/Itemname.csv", "../../data/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	costumes := 0
	for _, def := range catalog.Items {
		if def.Mesh == model.CostumeMesh && def.Pos == model.NativeCostumeSlotMask {
			t.Fatalf("item %d %q is still a slot-13 costume", def.Index, def.Name)
		}
		if model.IsCostumeDef(def) {
			costumes++
			if def.Pos != model.CostumeSlotMask {
				t.Fatalf("costume %d has position %d", def.Index, def.Pos)
			}
		}
	}
	if costumes != 135 {
		t.Fatalf("costumes in slot 12 = %d, want 135", costumes)
	}
	for index, pos := range map[uint16]int{
		4150: model.CostumeSlotMask, 4490: model.CostumeSlotMask,
		753: model.NativeCostumeSlotMask, 769: model.NativeCostumeSlotMask, 1726: model.NativeCostumeSlotMask,
		3900: model.NativeCostumeSlotMask, 3915: model.NativeCostumeSlotMask, 4081: model.NativeCostumeSlotMask,
		508: model.CostumeSlotMask,
	} {
		if got := catalog.Items[index].Pos; got != pos {
			t.Fatalf("item %d position = %d, want %d", index, got, pos)
		}
	}
}
