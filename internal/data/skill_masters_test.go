package data

import "testing"

func TestSkillMastersReserveClientBranchSeparators(t *testing.T) {
	npcs, err := LoadNPCs("../../data/npcs")
	if err != nil {
		t.Fatal(err)
	}
	wantFirst := map[string][3]uint16{
		"Knight_Leader": {5000, 5008, 5016},
		"Foema_Ancian":  {5024, 5032, 5040},
		"Mestre_Archi":  {5048, 5056, 5064},
		"ForeLearner":   {5072, 5080, 5088},
	}
	found := 0
	for _, npc := range npcs {
		want, ok := wantFirst[npc.Name]
		if !ok {
			continue
		}
		found++
		if len(npc.Vende) != 27 {
			t.Fatalf("%s: %d slots, quer 27", npc.Name, len(npc.Vende))
		}
		for _, slot := range []int{8, 17, 26} {
			if npc.Vende[slot].Index != 0 {
				t.Fatalf("%s: separador %d contém item %d", npc.Name, slot, npc.Vende[slot].Index)
			}
		}
		usable := 0
		for slot, item := range npc.Vende {
			if slot != 8 && slot != 17 && slot != 26 && item.Index != 0 {
				usable++
			}
		}
		if usable != 24 {
			t.Fatalf("%s: %d skills utilizaveis, quer 24", npc.Name, usable)
		}
		for i, slot := range []int{0, 9, 18} {
			if npc.Vende[slot].Index != want[i] {
				t.Fatalf("%s: ramo %d começa com %d, quer %d", npc.Name, i, npc.Vende[slot].Index, want[i])
			}
		}
	}
	if found != len(wantFirst) {
		t.Fatalf("encontrou %d mestres, quer %d", found, len(wantFirst))
	}
}
