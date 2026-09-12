package model

import "testing"

func TestCharacterTemplateCreatesIndependentCleanCharacter(t *testing.T) {
	tpl := CharacterTemplate{
		Class:     2,
		Score:     Score{MaxHP: 70, MaxMP: 55, CurHP: 70, CurMP: 55},
		Equip:     CharacterEquipment{Rosto: Item{Index: 21}, Arma: Item{Index: 861}},
		Inventory: []Item{{Index: 400}},
	}
	ch := tpl.NewCharacter("TesteBM", CharacterSpawn{X: 2112, Y: 2088})
	if ch.Name != "TesteBM" || ch.Class != 2 || ch.X != 2112 || ch.Y != 2088 {
		t.Fatalf("identidade/nascimento incorretos: %+v", ch)
	}
	if ch.Equip[0].Index != 21 || ch.Equip[6].Index != 861 || ch.Inv[0].Index != 400 {
		t.Fatalf("itens iniciais incorretos: equip=%v inv0=%+v", ch.Equip, ch.Inv[0])
	}
	for i, skill := range ch.ShortSkill {
		if skill != 0xFF {
			t.Fatalf("shortSkill[%d]=%d, quer 255", i, skill)
		}
	}
	ch.Inv[0] = Item{}
	if tpl.Inventory[0].Index != 400 {
		t.Fatal("personagem compartilhou inventario mutavel com o template")
	}
}
