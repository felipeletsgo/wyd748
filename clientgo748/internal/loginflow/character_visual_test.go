package loginflow

import (
	"testing"

	"wydclient748/internal/assets"
	"wydclient748/internal/login"
)

func TestCharacterItemAbilitySumsEFClass(t *testing.T) {
	item := assets.ItemDefinition{}
	item.Effects[0] = assets.ItemStaticEffect{Index: characterClassEffectIndex, Value: 16}
	item.Effects[1] = assets.ItemStaticEffect{Index: 7, Value: 99}
	item.Effects[2] = assets.ItemStaticEffect{Index: characterClassEffectIndex, Value: 23}

	if got := characterItemAbility(item, characterClassEffectIndex); got != 39 {
		t.Fatalf("EF_CLASS=%d want=39", got)
	}
}

func TestCharacterSkinMeshTypeMatches748Table(t *testing.T) {
	want := map[int]int{
		1: 0, 2: 1, 4: 0, 8: 1, 16: 20, 17: 21, 18: 22, 19: 23,
		20: 24, 21: 2, 22: 25, 23: 26, 24: 27, 25: 2, 26: 3, 27: 28,
		28: 29, 29: 6, 30: 4, 31: 32, 32: 7, 33: 8, 34: 0, 35: 29,
		36: 0, 37: 1, 38: 1, 39: 0, 40: 0, 41: 69, 42: 30, 43: 31,
		44: 33, 45: 23, 46: 11, 47: 35, 48: 34, 49: 36, 50: 37, 51: 38,
		52: 39, 53: 40, 54: 9, 55: 10, 56: 41, 57: 12, 58: 42, 59: 43,
		60: 0, 61: 1, 62: 5, 63: 0, 64: 44, 66: 45, 67: 46, 68: 47,
		69: 48, 70: 53, 71: 54, 72: 55, 73: 56, 74: 57,
	}
	for class, skin := range want {
		if got := characterSkinMeshType(class); got != skin {
			t.Fatalf("class %d skin=%d want=%d", class, got, skin)
		}
	}
	if got := characterSkinMeshType(65); got != 0 {
		t.Fatalf("unknown class fallback=%d want=0", got)
	}
}

func TestAssembleCharacterVisualProjects748Equipment(t *testing.T) {
	var list assets.ItemList
	setVisualItem := func(index uint16, mesh, texture int16) {
		list.Records[index] = assets.ItemDefinition{IndexMesh: mesh, IndexTexture: texture}
	}

	list.Records[22].Effects[0] = assets.ItemStaticEffect{Index: characterClassEffectIndex, Value: 39}
	setVisualItem(42, 7, 107)
	setVisualItem(100, 2, 102)
	setVisualItem(101, 3, 103)
	setVisualItem(102, 4, 104)
	setVisualItem(103, 5, 105)
	setVisualItem(105, 17, 117)
	setVisualItem(106, 18, 118)
	setVisualItem(113, 23, 123)
	setVisualItem(114, 24, 124)
	setVisualItem(115, 25, 125)

	var equip [18]uint16
	equip[0] = 22
	equip[1] = 3501
	equip[2] = 100
	equip[3] = 101
	equip[4] = 102
	equip[5] = 103
	equip[6] = 105
	equip[7] = 106
	equip[13] = 113
	equip[14] = 114
	equip[15] = 115

	visual := assembleCharacterVisual(equip, 42, &list, `mesh\ch01`, 0)
	if visual.Class != 39 || visual.SkinMeshType != 0 || !visual.Expanded {
		t.Fatalf("class projection=%+v", visual)
	}
	if visual.Body[0].ItemIndex != 42 || visual.Body[0].Mesh != 7 || visual.Body[0].Texture != 107 {
		t.Fatalf("battle master face=%+v", visual.Body[0])
	}
	if visual.Body[1] != (characterVisualPart{}) {
		t.Fatalf("hidden helmet=%+v want zero", visual.Body[1])
	}
	if visual.Body[2].Mesh != 2 || visual.Body[5].Mesh != 5 {
		t.Fatalf("body projection=%+v", visual.Body)
	}
	if visual.Left.Mesh != 17 || visual.Left.Texture != 117 || visual.Right.Mesh != 18 || visual.Right.Texture != 118 {
		t.Fatalf("hands left=%+v right=%+v", visual.Left, visual.Right)
	}
	if visual.Costume.Mesh != 23 || visual.Extra14.Mesh != 24 || visual.Mantle.Mesh != 25 {
		t.Fatalf("extended visuals costume=%+v extra14=%+v mantle=%+v", visual.Costume, visual.Extra14, visual.Mantle)
	}
	if got, want := visual.Body[0].MeshName, `mesh\ch010128.msh`; got != want {
		t.Fatalf("face mesh name=%q want=%q", got, want)
	}
	if got, want := visual.Body[2].MeshName, `mesh\ch010323.msh`; got != want {
		t.Fatalf("body mesh name=%q want=%q", got, want)
	}
}

func TestCharacterSkinMeshNameSpecialBoneAnimationsForceVariantOne(t *testing.T) {
	for _, index := range []int{45, 46, 53, 54} {
		if got, want := characterSkinMeshName(`mesh\giant`, index, 2, 19, true), `mesh\giant0301.msh`; got != want {
			t.Fatalf("bone animation %d mesh name=%q want=%q", index, got, want)
		}
	}
}

func TestCharacterHelmetHiddenUsesModuloItemList(t *testing.T) {
	for _, index := range []uint16{3500, 3501, 3502, 3507, 10000, 10001, 10002, 10007} {
		if !characterHelmetHidden(index) {
			t.Fatalf("helmet %d should be hidden", index)
		}
	}
	for _, index := range []uint16{3499, 3503, 3506, 3508} {
		if characterHelmetHidden(index) {
			t.Fatalf("helmet %d should remain visible", index)
		}
	}
}

func TestCharacterBattleMasterFromListMatchesSelectionLoop(t *testing.T) {
	var list login.CharacterList
	list.Characters[0].Name = "normal"
	list.Characters[0].Equip[0].Index = 22
	list.Characters[1].Name = "first-bm"
	list.Characters[1].Equip[0].Index = 41
	list.Characters[2].Name = "ignored"
	list.Characters[2].Equip[0].Index = 32
	list.Characters[3].Name = "last-bm"
	list.Characters[3].Equip[0].Index = 47

	if got, want := characterBattleMasterFromList(list), uint16(47); got != want {
		t.Fatalf("battle master=%d want=%d", got, want)
	}
}

func TestCharacterBattleMasterFromListIgnoresEmptySlots(t *testing.T) {
	var list login.CharacterList
	list.Characters[0].Name = "normal"
	list.Characters[0].Equip[0].Index = 22
	list.Characters[1].Equip[0].Index = 41

	if got := characterBattleMasterFromList(list); got != 0 {
		t.Fatalf("battle master=%d want=0", got)
	}
}
