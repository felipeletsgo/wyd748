package loginflow

import (
	"errors"
	"fmt"
	"path/filepath"
	"strings"
	"time"

	"wydclient748/internal/assets"
	"wydclient748/internal/graphics"
	"wydclient748/internal/login"
	"wydclient748/internal/ui"
)

const (
	characterSelectRootID     = int32(0x0502)
	characterSelectEnterID    = int32(0x1204)
	characterSelectRootWidth  = int32(249)
	characterSelectRootHeight = int32(370)
	characterStand02Slot      = 1
)

// Procedência desta unidade: PARIDADE_NATIVA para recurso SelCharScene2,
// STAND02 e transforms já rastreados no 7.48; MODERNIZACAO_COMPATIVEL apenas
// para ownership/caches Go que não alteram o contrato observável.
// CharacterSelectVisualAssets agrupa somente os assets imutáveis necessários
// à apresentação da seleção. O cache de meshes e os Skeletons permanecem
// pertencendo à cena, evitando estado gráfico compartilhado entre entradas.
type CharacterSelectVisualAssets struct {
	AssetRoot      string
	ItemList       *assets.ItemList
	BoneAnimations *assets.BoneAnimationSet
	Controls       []assets.SceneControl
}

type characterSelectModel struct {
	visual    characterVisual
	animation *assets.BoneAnimation
	clipIndex int
	skeleton  *graphics.Skeleton
	meshes    []assets.Mesh
	transform graphics.SceneTransform
}

func copyCharacterSelectControls(controls []assets.SceneControl) ([]assets.SceneControl, error) {
	if len(controls) == 0 {
		return nil, nil
	}
	root, ok := assets.FindControl(controls, characterSelectRootID)
	if !ok || root.Kind != 1 || len(root.Words) != 10 || root.Words[1] != 0 ||
		root.Words[5] != characterSelectRootWidth || root.Words[6] != characterSelectRootHeight {
		return nil, errors.New("loginflow: invalid SelCharScene2 root control")
	}
	owned := make([]assets.SceneControl, len(controls))
	seen := make(map[int32]struct{}, len(controls))
	for i, control := range controls {
		id, ok := control.ID()
		if !ok {
			return nil, errors.New("loginflow: character-select control without ID")
		}
		if _, duplicate := seen[id]; duplicate {
			return nil, fmt.Errorf("loginflow: duplicate character-select control %d", id)
		}
		seen[id] = struct{}{}
		owned[i] = assets.SceneControl{Kind: control.Kind, Words: append([]int32(nil), control.Words...)}
	}
	return owned, nil
}

func copyCharacterSelectStrings(table map[int32]string) map[int32]string {
	if len(table) == 0 {
		return nil
	}
	owned := make(map[int32]string, len(table))
	for id, value := range table {
		owned[id] = value
	}
	return owned
}

func characterSelectRootFor(renderer graphics.ShapeRenderer) ui.Rect {
	width, height := int32(800), int32(600)
	if viewport, ok := renderer.(graphics.ViewportProvider); ok {
		if w, h := viewport.ClientViewport(); w > 0 && h > 0 {
			width, height = w, h
		}
	}
	return ui.Rect{
		X:      width*3/4 - characterSelectRootWidth/2,
		Y:      height/2 - characterSelectRootHeight/2,
		Width:  characterSelectRootWidth,
		Height: characterSelectRootHeight,
	}
}

func characterSelectControlRect(controls []assets.SceneControl, id int32, root ui.Rect) (ui.Rect, bool) {
	control, ok := assets.FindControl(controls, id)
	if !ok || len(control.Words) < 7 || control.Words[1] != characterSelectRootID || control.Words[5] <= 0 || control.Words[6] <= 0 {
		return ui.Rect{}, false
	}
	return ui.Rect{
		X:      root.X + control.Words[3],
		Y:      root.Y + control.Words[4],
		Width:  control.Words[5],
		Height: control.Words[6],
	}, true
}

func characterSelectTransform(slot int) graphics.SceneTransform {
	const step = float32(0.333)
	start := graphics.Position3{X: 2053.0, Y: 0, Z: 2048.2}
	end := graphics.Position3{X: 2048.2, Y: 0, Z: 2053.0}
	t := float32(slot) * step
	return graphics.SceneTransform{
		Position: graphics.Position3{
			X: start.X + (end.X-start.X)*t,
			Y: start.Y + (end.Y-start.Y)*t,
			Z: start.Z + (end.Z-start.Z)*t,
		},
		Yaw:   -45,
		Scale: 1,
	}
}

// characterSelectPreviewCamera é uma câmera interna do renderer Go, não um
// claim de paridade nativa. Ela enquadra as posições 7.48 já confirmadas até o
// lifecycle de câmera histórico ser integrado como unidade própria.
func characterSelectPreviewCamera() graphics.Camera {
	return graphics.Camera{
		Position:   graphics.Position3{X: 2056.8, Y: 5.6, Z: 2043.2},
		Target:     graphics.Position3{X: 2050.6, Y: 1.05, Z: 2050.6},
		Up:         graphics.Position3{Y: 1},
		FOVDegrees: 38,
		Near:       0.05,
		Far:        64,
	}
}

// characterStand02ClipIndex resolve o slot lógico STAND02 pelo ValidIndex.
// ANI ausente pode compactar Clips, portanto o slot lógico nunca é usado como
// índice direto do slice carregado.
func characterStand02ClipIndex(set *assets.BoneAnimationSet, skin int) (int, error) {
	if set == nil || skin < 0 || skin >= assets.MaxBoneAnimationList {
		return 0, errors.New("loginflow: invalid character animation set")
	}
	animation := set.Entries[skin]
	if animation == nil {
		return 0, fmt.Errorf("loginflow: missing bone animation %d", skin)
	}
	validIndex := set.ValidIndices[skin][characterStand02Slot]
	for clipIndex, clip := range animation.Clips {
		if clip.ValidIndex == validIndex {
			return clipIndex, nil
		}
	}
	return 0, fmt.Errorf("loginflow: STAND02 valid index %d is not loaded for skin %d", validIndex, skin)
}

func characterMeshPath(assetRoot, meshName string) string {
	normalized := filepath.FromSlash(strings.ReplaceAll(meshName, "\\", "/"))
	return filepath.Join(assetRoot, normalized)
}

func buildCharacterSelectModels(list login.CharacterList, visualAssets *CharacterSelectVisualAssets) ([login.SelectionCharacterCount]*characterSelectModel, error) {
	var models [login.SelectionCharacterCount]*characterSelectModel
	if visualAssets == nil || visualAssets.ItemList == nil || visualAssets.BoneAnimations == nil || visualAssets.AssetRoot == "" {
		return models, nil
	}
	battleMaster := characterBattleMasterFromList(list)
	meshCache := make(map[string]assets.Mesh)
	for slot, character := range list.Characters {
		if !character.Occupied() {
			continue
		}
		class := characterClass(visualAssets.ItemList, character.Equip[0].Index)
		skin := characterSkinMeshType(class)
		if skin < 0 || skin >= assets.MaxBoneAnimationList || visualAssets.BoneAnimations.Entries[skin] == nil {
			return models, fmt.Errorf("loginflow: character slot %d has unavailable skin %d", slot, skin)
		}
		animation := visualAssets.BoneAnimations.Entries[skin]
		visual := assembleCharacterVisual(characterEquipIndices(character), battleMaster, visualAssets.ItemList, animation.Name, skin)
		clipIndex, err := characterStand02ClipIndex(visualAssets.BoneAnimations, skin)
		if err != nil {
			return models, fmt.Errorf("loginflow: character slot %d: %w", slot, err)
		}
		skeleton, err := graphics.BuildSkeleton(animation.Bone)
		if err != nil {
			return models, fmt.Errorf("loginflow: character slot %d skeleton: %w", slot, err)
		}
		meshes := make([]assets.Mesh, 0, characterBodyPartCount)
		for _, part := range visual.Body {
			if part.MeshName == "" {
				continue
			}
			mesh, ok := meshCache[part.MeshName]
			if !ok {
				mesh, err = assets.LoadMSHFile(characterMeshPath(visualAssets.AssetRoot, part.MeshName))
				if err != nil {
					return models, fmt.Errorf("loginflow: character slot %d mesh %q: %w", slot, part.MeshName, err)
				}
				meshCache[part.MeshName] = mesh
			}
			meshes = append(meshes, mesh)
		}
		models[slot] = &characterSelectModel{
			visual:    visual,
			animation: animation,
			clipIndex: clipIndex,
			skeleton:  skeleton,
			meshes:    meshes,
			transform: characterSelectTransform(slot),
		}
	}
	return models, nil
}

func characterEquipIndices(character login.CharacterSummary) [18]uint16 {
	var equip [18]uint16
	for i := range character.Equip {
		equip[i] = character.Equip[i].Index
	}
	return equip
}

func drawCharacterSelectModels(renderer graphics.Scene3DRenderer, models [login.SelectionCharacterCount]*characterSelectModel, elapsed time.Duration) error {
	if renderer == nil {
		return nil
	}
	camera := characterSelectPreviewCamera()
	for slot, model := range models {
		if model == nil {
			continue
		}
		if err := model.skeleton.ApplyAnimationClip(model.animation, model.clipIndex, elapsed, true); err != nil {
			return fmt.Errorf("loginflow: animate character slot %d: %w", slot, err)
		}
		for _, mesh := range model.meshes {
			palette, err := model.skeleton.BuildSkinPalette(mesh)
			if err != nil {
				return fmt.Errorf("loginflow: skin character slot %d: %w", slot, err)
			}
			if err := renderer.DrawSkinnedMeshScene(mesh, palette, model.transform, camera); err != nil {
				return fmt.Errorf("loginflow: draw character slot %d: %w", slot, err)
			}
		}
	}
	return nil
}

func drawCharacterSelectUI(renderer graphics.ShapeRenderer, controls []assets.SceneControl, stringsTable map[int32]string, itemList *assets.ItemList, list login.CharacterList, selected int, status string) {
	if renderer == nil || len(controls) == 0 {
		return
	}
	root := characterSelectRootFor(renderer)
	renderer.DrawRect(root.X, root.Y, root.Width, root.Height, graphics.Color{R: .055, G: .07, B: .09, A: .92})
	text, hasText := renderer.(graphics.TextRenderer)
	if !hasText {
		return
	}

	for _, control := range controls {
		id, ok := control.ID()
		if !ok || len(control.Words) < 7 || control.Words[1] != characterSelectRootID {
			continue
		}
		rect := ui.Rect{X: root.X + control.Words[3], Y: root.Y + control.Words[4], Width: control.Words[5], Height: control.Words[6]}
		switch control.Kind {
		case 2:
			renderer.DrawRect(rect.X, rect.Y, rect.Width, rect.Height, graphics.Color{R: .12, G: .22, B: .34, A: 1})
			if label, ok := characterControlCaption(control, stringsTable); ok {
				text.DrawText(rect.X+6, rect.Y+5, label, 9, graphics.Color{R: 1, G: 1, B: 1, A: 1})
			}
		case 12:
			if id >= 0x1600 {
				if label, ok := characterControlCaption(control, stringsTable); ok {
					text.DrawText(rect.X, rect.Y+3, label, 9, graphics.Color{R: .76, G: .82, B: .88, A: 1})
				}
			}
		}
	}

	if selected >= 0 && selected < len(list.Characters) && list.Characters[selected].Occupied() {
		character := list.Characters[selected]
		class := 0
		if itemList != nil {
			class = characterClass(itemList, character.Equip[0].Index)
		}
		values := map[int32]string{
			0x0506: character.Name,
			0x0508: fmt.Sprintf("%d", class),
			0x0520: fmt.Sprintf("%d", character.Score.Level),
			0x0509: fmt.Sprintf("%d", character.Guild),
			0x0521: fmt.Sprintf("%d", character.Coin),
			0x0522: fmt.Sprintf("%d", character.Exp),
			0x0510: fmt.Sprintf("%d, %d", character.HomeTownX, character.HomeTownY),
			0x0524: fmt.Sprintf("%d", character.Score.Str),
			0x0525: fmt.Sprintf("%d", character.Score.Int),
			0x0526: fmt.Sprintf("%d", character.Score.Dex),
			0x0527: fmt.Sprintf("%d", character.Score.Con),
			0x0529: fmt.Sprintf("%d", character.Score.Mastery[0]),
			0x052A: fmt.Sprintf("%d", character.Score.Mastery[1]),
			0x052B: fmt.Sprintf("%d", character.Score.Mastery[2]),
			0x052C: fmt.Sprintf("%d", character.Score.Mastery[3]),
		}
		for id, value := range values {
			if rect, ok := characterSelectControlRect(controls, id, root); ok {
				text.DrawText(rect.X, rect.Y+3, value, 9, graphics.Color{R: 1, G: 1, B: 1, A: 1})
			}
		}
	}
	if status != "" {
		text.DrawText(root.X, root.Y+root.Height+8, status, 9, graphics.Color{R: .9, G: .82, B: .68, A: 1})
	}
}

func characterControlCaption(control assets.SceneControl, stringsTable map[int32]string) (string, bool) {
	if stringsTable == nil || len(control.Words) == 0 {
		return "", false
	}
	index := control.Words[len(control.Words)-1]
	value, ok := stringsTable[index]
	return value, ok && value != ""
}
