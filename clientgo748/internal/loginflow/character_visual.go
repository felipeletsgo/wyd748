package loginflow

import (
	"fmt"

	"wydclient748/internal/assets"
	"wydclient748/internal/login"
)

const (
	characterClassEffectIndex = int16(18) // EF_CLASS
	characterBodyPartCount    = 6
)

// characterVisualPart conserva o item que originou a aparência e a projeção
// efetivamente consumida pelo renderer. Mesh/Texture nunca são derivados do
// número do Equip diretamente: passam pelo ItemList.bin 7.48.
type characterVisualPart struct {
	ItemIndex uint16
	Mesh      int16
	Texture   int16
	MeshName  string
}

// characterVisual descreve somente o estado necessário para montar o avatar
// da tela de seleção. Regras de animação e ownership de GPU ficam fora deste
// tipo para que a projeção 7.48 possa ser testada sem um contexto OpenGL.
type characterVisual struct {
	Class        int
	SkinMeshType int
	Expanded     bool
	Body         [characterBodyPartCount]characterVisualPart
	Left         characterVisualPart
	Right        characterVisualPart
	Costume      characterVisualPart
	Extra14      characterVisualPart
	Mantle       characterVisualPart
}

// characterItemAbility replica o trecho de BASE_GetItemAbility necessário
// aqui: soma todos os efeitos estáticos do ItemList com o tipo solicitado.
func characterItemAbility(item assets.ItemDefinition, effectIndex int16) int {
	value := 0
	for _, effect := range item.Effects {
		if effect.Index == effectIndex {
			value += int(effect.Value)
		}
	}
	return value
}

func characterClass(itemList *assets.ItemList, raceItemIndex uint16) int {
	return characterItemAbility(itemList.Lookup(raceItemIndex), characterClassEffectIndex)
}

// characterSkinMeshType porta BASE_DefineSkinMeshType. O valor de retorno é o
// índice de skin usado pelo TMHuman; classes desconhecidas mantêm fallback 0.
func characterSkinMeshType(class int) int {
	switch class {
	case 1:
		return 0
	case 2:
		return 1
	case 4:
		return 0
	case 8:
		return 1
	case 16:
		return 20
	case 17:
		return 21
	case 18:
		return 22
	case 19:
		return 23
	case 20:
		return 24
	case 21:
		return 2
	case 22:
		return 25
	case 23:
		return 26
	case 24:
		return 27
	case 25:
		return 2
	case 26:
		return 3
	case 27:
		return 28
	case 28:
		return 29
	case 29:
		return 6
	case 30:
		return 4
	case 31:
		return 32
	case 32:
		return 7
	case 33:
		return 8
	case 34:
		return 0
	case 35:
		return 29
	case 36:
		return 0
	case 37, 38:
		return 1
	case 39, 40:
		return 0
	case 41:
		return 69
	case 42:
		return 30
	case 43:
		return 31
	case 44:
		return 33
	case 45:
		return 23
	case 46:
		return 11
	case 47:
		return 35
	case 48:
		return 34
	case 49:
		return 36
	case 50:
		return 37
	case 51:
		return 38
	case 52:
		return 39
	case 53:
		return 40
	case 54:
		return 9
	case 55:
		return 10
	case 56:
		return 41
	case 57:
		return 12
	case 58:
		return 42
	case 59:
		return 43
	case 60:
		return 0
	case 61:
		return 1
	case 62:
		return 5
	case 63:
		return 0
	case 64:
		return 44
	case 66:
		return 45
	case 67:
		return 46
	case 68:
		return 47
	case 69:
		return 48
	case 70:
		return 53
	case 71:
		return 54
	case 72:
		return 55
	case 73:
		return 56
	case 74:
		return 57
	default:
		return 0
	}
}

// characterEquipmentLook aplica somente as substituições por índice de item
// que antecedem a consulta ao ItemList. Slots 13..15 permanecem porque o fluxo
// 7.48 adaptado ainda os usa para costume/extra/mantle.
func characterEquipmentLook(equip [18]uint16, battleMaster uint16) [16]uint16 {
	var look [16]uint16
	look[0] = equip[0]
	if characterUsesBattleMasterFace(look[0]) {
		look[0] = battleMaster
	}

	if !characterHelmetHidden(equip[1]) {
		look[1] = equip[1]
	}
	copy(look[2:8], equip[2:8])
	copy(look[13:16], equip[13:16])
	return look
}

func characterUsesBattleMasterFace(index uint16) bool {
	switch index {
	case 22, 23, 24, 25, 32:
		return true
	default:
		return false
	}
}

// characterBattleMasterFromList reproduz o estado global atualizado pela UI
// de seleção 7.48 antes da criação dos TMHuman. O valor vem do item racial
// recebido no próprio snapshot de personagens; não existe campo BattleMaster
// no Score. A última entrada ocupada cuja classe decimal termina em 1 ou 6..9
// vence, preservando a ordem observada do loop original.
func characterBattleMasterFromList(list login.CharacterList) uint16 {
	var battleMaster uint16
	for _, character := range list.Characters {
		if !character.Occupied() {
			continue
		}
		index := character.Equip[0].Index
		classDigit := index % 10
		if classDigit == 1 || classDigit >= 6 {
			battleMaster = index
		}
	}
	return battleMaster
}

func characterHelmetHidden(index uint16) bool {
	index %= assets.ItemListRecordCount
	return (index >= 3500 && index <= 3502) || index == 3507
}

func projectCharacterVisualPart(itemList *assets.ItemList, index uint16) characterVisualPart {
	item := itemList.Lookup(index)
	return characterVisualPart{ItemIndex: index, Mesh: item.IndexMesh, Texture: item.IndexTexture}
}

// characterSkinMeshName aplica a regra base de TMSkinMesh para as seis partes
// do corpo. Os quatro BoneAnimation especiais sempre usam variante 01.
func characterSkinMeshName(baseName string, boneAnimationIndex, partIndex int, mesh int16, expanded bool) string {
	variant := int(mesh) + 1
	if expanded {
		variant += 20
	}
	switch boneAnimationIndex {
	case 45, 46, 53, 54:
		variant = 1
	}
	return fmt.Sprintf("%s%02d%02d.msh", baseName, partIndex+1, variant)
}

// assembleCharacterVisual fecha a projeção determinística usada antes de
// carregar meshes: EF_CLASS vem do item racial original, enquanto BattleMaster
// altera somente a face visual, como no fluxo TMHuman observado.
func assembleCharacterVisual(equip [18]uint16, battleMaster uint16, itemList *assets.ItemList, boneAnimationBase string, boneAnimationIndex int) characterVisual {
	class := characterClass(itemList, equip[0])
	visual := characterVisual{
		Class:        class,
		SkinMeshType: characterSkinMeshType(class),
		Expanded:     characterSkinExpanded(class),
	}

	look := characterEquipmentLook(equip, battleMaster)
	for part := range visual.Body {
		if part == 1 && characterHelmetHidden(equip[1]) {
			continue
		}
		visual.Body[part] = projectCharacterVisualPart(itemList, look[part])
		visual.Body[part].MeshName = characterSkinMeshName(boneAnimationBase, boneAnimationIndex, part, visual.Body[part].Mesh, visual.Expanded)
	}

	visual.Left = projectCharacterVisualPart(itemList, look[6])
	visual.Right = projectCharacterVisualPart(itemList, look[7])
	visual.Costume = projectCharacterVisualPart(itemList, look[13])
	visual.Extra14 = projectCharacterVisualPart(itemList, look[14])
	visual.Mantle = projectCharacterVisualPart(itemList, look[15])
	return visual
}

func characterSkinExpanded(class int) bool {
	switch class {
	case 4, 8, 36, 39, 40, 60, 63:
		return true
	default:
		return false
	}
}
