// Command wydclient compõe o primeiro processo gráfico real do client Go 7.48.
// A janela, o contexto e a primeira cena de textura já são concretos; rede e
// cenas de gameplay serão adicionadas somente após seus contratos nativos.
package main

import (
	"context"
	"errors"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"
	"strings"

	"wydclient748/internal/app"
	"wydclient748/internal/assets"
	"wydclient748/internal/config"
	"wydclient748/internal/graphics"
	"wydclient748/internal/graphics/wgl"
	"wydclient748/internal/login"
	"wydclient748/internal/loginflow"
	"wydclient748/internal/platform/win32"
	"wydclient748/internal/protocol"
	"wydclient748/internal/world"
)

func main() {
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, "WYD Client initialization failed:", err)
		os.Exit(1)
	}
}

func run() error {
	cfg := config.Default()
	if err := cfg.Validate(); err != nil {
		return err
	}
	serverAddress, err := config.ServerAddressFromEnv(cfg.ServerAddress)
	if err != nil {
		return err
	}
	serverEntries, err := config.ServerEntriesFromEnv(displayServerAddress(cfg.ServerAddress, serverAddress))
	if err != nil {
		return err
	}
	var protectedSource assets.TextureSource
	protectedSettings, protectedEnabled, err := config.LoadProtectedAssetSettingsFromEnv()
	if err != nil {
		return fmt.Errorf("load protected asset settings: %w", err)
	}
	if protectedEnabled {
		packageData, err := os.ReadFile(protectedSettings.PackagePath)
		if err != nil {
			return fmt.Errorf("read protected asset package: %w", err)
		}
		protectedSource, err = assets.NewProtectedAssetCache(packageData, protectedSettings.ContentKey, protectedSettings.PublicKey)
		if err != nil {
			return fmt.Errorf("open protected asset package: %w", err)
		}
	}

	state := login.NewSessionState()
	var coordinator *loginflow.Coordinator
	renderer := wgl.New()
	loginTexture, err := assets.LoadWYTFile(loginTexturePath())
	if err != nil {
		return fmt.Errorf("load official login UI: %w", err)
	}
	// UITextureSetList entry 162 (login_box2) selects the 215x153 region at
	// the origin of loginbox2.wyt; the remaining atlas area is not part of the
	// native panel and must never be presented.
	loginTexture, err = loginTexture.Crop(0, 0, 215, 153)
	if err != nil {
		return fmt.Errorf("select official login UI region: %w", err)
	}
	loginLogoLeft, err := assets.LoadWYTFile(loginLogoLeftPath())
	if err != nil {
		return fmt.Errorf("load official left login logo: %w", err)
	}
	loginLogoRight, err := assets.LoadWYTFile(loginLogoRightPath())
	if err != nil {
		return fmt.Errorf("load official right login logo: %w", err)
	}
	serverTexture, err := assets.LoadWYTFile(serverSelectionTexturePath())
	if err != nil {
		return fmt.Errorf("load official server selection UI: %w", err)
	}
	// Materialize the same resource selected by native FUN_004A8F14 before
	// creating scenes. This validates the active indexed RC contract and keeps
	// login bindings tied to resource IDs rather than guessed coordinates.
	serverSceneData, err := os.ReadFile(loginAssetPath("SelServerScene2.bin"))
	if err != nil {
		return fmt.Errorf("load official server/login scene: %w", err)
	}
	serverControls, err := assets.ParseScene(serverSceneData)
	if err != nil {
		return fmt.Errorf("parse official server/login scene: %w", err)
	}
	if len(serverControls) != 22 {
		return fmt.Errorf("official server/login scene: got %d controls, want 22", len(serverControls))
	}
	stringData, err := os.ReadFile(loginAssetPath("UIString.txt"))
	if err != nil {
		return fmt.Errorf("load UI captions: %w", err)
	}
	uiStrings, err := assets.ParseUIStrings(stringData)
	if err != nil {
		return err
	}
	characterScenePath := loginAssetPath("SelCharScene2.bin")
	characterSceneData, err := os.ReadFile(characterScenePath)
	if err != nil {
		return fmt.Errorf("load official character selection scene: %w", err)
	}
	characterControls, err := assets.ParseScene(characterSceneData)
	if err != nil {
		return fmt.Errorf("parse official character selection scene: %w", err)
	}
	characterAssetRoot := filepath.Dir(filepath.Dir(characterScenePath))
	itemList, err := assets.LoadItemListFile(filepath.Join(characterAssetRoot, "ItemList.bin"))
	if err != nil {
		return fmt.Errorf("load character ItemList: %w", err)
	}
	boneAnimations, err := assets.LoadBoneAnimationSet(characterAssetRoot)
	if err != nil {
		return fmt.Errorf("load character BoneAnimation set: %w", err)
	}
	characterVisuals := &loginflow.CharacterSelectVisualAssets{
		AssetRoot:      characterAssetRoot,
		ItemList:       &itemList,
		BoneAnimations: &boneAnimations,
		Controls:       characterControls,
	}
	loginBackdrop, err := loadLoginBackdrop(loginBackdropTerrainPath(), loginAssetPath("DemoCamAction4.bin"))
	if err != nil {
		return err
	}
	characterTerrain, err := loadOptionalTerrain(characterTerrainPath())
	if err != nil {
		return err
	}
	var client *app.Application
	selectedAddress := displayServerAddress(cfg.ServerAddress, serverAddress)
	session := protocol.NewSession(selectedAddress, protocol.SessionOptions{})
	options := app.Options{
		Title:               "WYD 7.48",
		Width:               cfg.WindowWidth,
		Height:              cfg.WindowHeight,
		InitialSceneID:      loginflow.ServerSelectionSceneID,
		DeferSessionConnect: true,
		LogoPath:            "",
		SceneFactories: loginflow.SceneFactoriesWithVisuals(state, loginflow.VisualOptions{
			ShapeRenderer: renderer,
			LoginBackdrop: loginBackdrop,
			ServerTexture: &serverTexture,
			Servers:       toLoginServerEntries(serverEntries),
			RequestClose: func() error {
				if client == nil {
					return fmt.Errorf("client application is not initialized")
				}
				return client.RequestClose()
			},
			SelectServer: func(entry loginflow.ServerEntry) error {
				if client == nil {
					return fmt.Errorf("client application is not initialized")
				}
				if err := session.SetAddress(entry.Address); err != nil {
					return err
				}
				if coordinator == nil {
					return fmt.Errorf("login coordinator is not initialized")
				}
				return coordinator.ServerSelected()
			},
			LoginTexture:     &loginTexture,
			LoginControls:    serverControls,
			UIStrings:        uiStrings,
			LoginLogoLeft:    &loginLogoLeft,
			LoginLogoRight:   &loginLogoRight,
			CharacterVisuals: characterVisuals,
			Authenticate: func(account string, password []byte) error {
				if coordinator == nil {
					return fmt.Errorf("login coordinator is not initialized")
				}
				// TMSelectServerScene connects only from B_LOGIN_OK, after the
				// endpoint has been selected and the account form is validated.
				if err := client.ConnectSession(); err != nil {
					return err
				}
				return coordinator.Authenticate(account, password, [4]uint32{}, 0)
			},
			SelectCharacter: func(slot int32) error {
				if coordinator == nil {
					return fmt.Errorf("login coordinator is not initialized")
				}
				return coordinator.SelectCharacter(slot, 0, 0)
			},
			WorldEntities: func() []world.Entity {
				if coordinator == nil {
					return nil
				}
				return coordinator.WorldEntities()
			},
			RequestMove: func(targetX, targetY uint16) error {
				if coordinator == nil {
					return fmt.Errorf("login coordinator is not initialized")
				}
				return coordinator.Move(targetX, targetY)
			},
			Terrain: characterTerrain,
		}),
		SessionEventsPerFrame: 64,
		SceneSynchronizer: func() error {
			if coordinator == nil {
				return nil
			}
			return coordinator.Synchronize()
		},
		Session: session,
		SessionConnected: func() error {
			if coordinator == nil {
				return fmt.Errorf("login coordinator is not initialized")
			}
			return coordinator.SessionConnected()
		},
		SessionDisconnected: func() {
			if coordinator != nil {
				coordinator.SessionDisconnected()
			}
		},
		SessionEventHandler: func(event protocol.SessionEvent) error {
			if coordinator == nil {
				return fmt.Errorf("login coordinator is not initialized")
			}
			_, err := coordinator.HandleSessionEvent(event)
			return err
		},
	}
	if protectedEnabled {
		options.LogoPath = ""
		options.LogoSource = protectedSource
		options.LogoAssetPath = protectedSettings.LogoPath
	}
	client, err = app.New(
		options,
		win32.New(),
		renderer,
	)
	if err != nil {
		if protectedSource != nil {
			_ = protectedSource.Close()
		}
		return err
	}
	coordinator, err = loginflow.New(state, session, client, loginflow.Options{})
	if err != nil {
		_ = client.Close()
		return err
	}
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt)
	defer stop()
	return client.Run(ctx)
}

func displayServerAddress(defaultAddress, configuredAddress string) string {
	if configuredAddress != "" {
		return configuredAddress
	}
	return defaultAddress
}

func toLoginServerEntries(entries []config.ServerEntry) []loginflow.ServerEntry {
	converted := make([]loginflow.ServerEntry, len(entries))
	for i, entry := range entries {
		converted[i] = loginflow.ServerEntry{Name: entry.Name, Channel: entry.Channel, Address: entry.Address}
	}
	return converted
}

func initialLogoPath() string {
	return runtimeAssetPath("UI", "logo1.wyt")
}

func loginTexturePath() string {
	return runtimeAssetPath("UI", "loginbox2.wyt")
}

func serverSelectionTexturePath() string {
	return runtimeAssetPath("UI", "ServerList2.wyt")
}

func characterTerrainPath() string {
	return runtimeAssetPath("Env", "Character.trn")
}

func loginBackdropTerrainPath() string {
	return runtimeAssetPath("Env", "Field0813.trn")
}

func loadLoginBackdrop(terrainPath, cameraPath string) (*loginflow.LoginBackdrop, error) {
	terrainBytes, err := os.ReadFile(terrainPath)
	if err != nil {
		return nil, fmt.Errorf("load official login backdrop terrain: %w", err)
	}
	terrain, err := assets.ParseTerrain(terrainBytes)
	if err != nil {
		return nil, fmt.Errorf("parse official login backdrop terrain: %w", err)
	}
	action, err := assets.LoadCameraActionFile(cameraPath)
	if err != nil {
		return nil, fmt.Errorf("load official login camera action: %w", err)
	}
	backdrop, err := loginflow.NewLoginBackdrop(terrain, action)
	if err != nil {
		return nil, fmt.Errorf("build official login backdrop: %w", err)
	}
	textures, err := loadLoginBackdropTextures(backdrop, terrainPath)
	if err != nil {
		return nil, err
	}
	if err := backdrop.SetTerrainTextures(textures); err != nil {
		return nil, fmt.Errorf("bind official login backdrop textures: %w", err)
	}
	if err := loadLoginBackdropStaticObjects(backdrop, terrain, terrainPath); err != nil {
		return nil, err
	}
	return backdrop, nil
}

func loadLoginBackdropStaticObjects(backdrop *loginflow.LoginBackdrop, terrain assets.Terrain, terrainPath string) error {
	ext := filepath.Ext(terrainPath)
	datPath := strings.TrimSuffix(terrainPath, ext) + ".dat"
	records, err := assets.LoadFieldObjectDATFile(datPath)
	if err != nil {
		return fmt.Errorf("load official login field objects: %w", err)
	}

	hasOrdinary := false
	for _, record := range records {
		if isOrdinaryLoginStaticObjectType(record.ObjectType) {
			hasOrdinary = true
			break
		}
	}
	if !hasOrdinary {
		if err := backdrop.SetStaticObjects(nil, nil); err != nil {
			return fmt.Errorf("bind official login static objects: %w", err)
		}
		return nil
	}

	assetRoot := filepath.Dir(filepath.Dir(terrainPath))
	meshDir := filepath.Join(assetRoot, "mesh")
	meshList, err := assets.LoadMeshListFile(filepath.Join(meshDir, "MeshList.txt"))
	if err != nil {
		return fmt.Errorf("load official static mesh list: %w", err)
	}
	textureList, err := assets.LoadModelTextureListFile(filepath.Join(meshDir, "MeshTextureList.bin"))
	if err != nil {
		return fmt.Errorf("load official model texture list: %w", err)
	}

	type cachedMSA struct {
		mesh     assets.MSAMesh
		geometry graphics.MeshGeometry
	}
	meshCache := make(map[string]cachedMSA)
	textures := make(map[uint16]assets.Texture)
	objects := make([]loginflow.LoginStaticObject, 0, len(records))
	for _, record := range records {
		if !isOrdinaryLoginStaticObjectType(record.ObjectType) {
			continue
		}
		nativeMeshPath, ok := meshList.Resolve(record.ObjectType)
		if !ok {
			return fmt.Errorf("official MeshList has no static object type %d", record.ObjectType)
		}
		meshPath, err := resolveNativeAssetPath(assetRoot, nativeMeshPath)
		if err != nil {
			return fmt.Errorf("resolve static object type %d mesh: %w", record.ObjectType, err)
		}
		cached, ok := meshCache[meshPath]
		if !ok {
			mesh, err := assets.LoadMSAFile(meshPath)
			if err != nil {
				return fmt.Errorf("load static object type %d mesh: %w", record.ObjectType, err)
			}
			geometry, err := graphics.ExtractMSAGeometry(mesh)
			if err != nil {
				return fmt.Errorf("decode static object type %d geometry: %w", record.ObjectType, err)
			}
			cached = cachedMSA{mesh: mesh, geometry: geometry}
			meshCache[meshPath] = cached
		}
		if len(cached.mesh.Attributes) != len(cached.mesh.TextureNames) {
			return fmt.Errorf("static object type %d has %d material ranges but %d texture names", record.ObjectType, len(cached.mesh.Attributes), len(cached.mesh.TextureNames))
		}

		materials := make([]loginflow.LoginStaticMaterial, 0, len(cached.mesh.Attributes))
		for attributeIndex, attribute := range cached.mesh.Attributes {
			if attribute.FaceCount == 0 {
				continue
			}
			slotIndex, textureRecord, ok := textureList.ResolveMSATexture(cached.mesh.TextureNames[attributeIndex])
			if !ok {
				return fmt.Errorf("static object type %d material %d texture %q is absent from MeshTextureList", record.ObjectType, attributeIndex, cached.mesh.TextureNames[attributeIndex])
			}
			if slotIndex < 0 || slotIndex > int(^uint16(0)) {
				return fmt.Errorf("static object type %d material %d texture slot %d exceeds renderer range", record.ObjectType, attributeIndex, slotIndex)
			}
			slot := uint16(slotIndex)
			materialGeometry, err := graphics.ExtractMSAMaterialGeometry(cached.mesh, cached.geometry, attributeIndex)
			if err != nil {
				return fmt.Errorf("slice static object type %d material %d: %w", record.ObjectType, attributeIndex, err)
			}
			materials = append(materials, loginflow.LoginStaticMaterial{TextureSlot: slot, Geometry: materialGeometry})

			if _, loaded := textures[slot]; !loaded {
				texturePath, err := resolveNativeAssetPath(assetRoot, textureRecord.Path)
				if err != nil {
					return fmt.Errorf("resolve model texture slot %d: %w", slot, err)
				}
				texture, err := assets.LoadWYSFile(texturePath)
				if err != nil {
					return fmt.Errorf("load model texture slot %d: %w", slot, err)
				}
				textures[slot] = texture
			}
		}
		if len(materials) == 0 {
			continue
		}
		object, err := loginflow.NewLoginStaticObject(record, terrain, materials)
		if err != nil {
			return fmt.Errorf("build static object type %d: %w", record.ObjectType, err)
		}
		objects = append(objects, object)
	}
	if err := backdrop.SetStaticObjects(objects, textures); err != nil {
		return fmt.Errorf("bind official login static objects: %w", err)
	}
	return nil
}

func isOrdinaryLoginStaticObjectType(objectType uint32) bool {
	if objectType >= 311 && objectType <= 322 ||
		objectType >= 331 && objectType <= 342 ||
		objectType >= 351 && objectType <= 378 ||
		objectType >= 487 && objectType <= 489 ||
		objectType >= 501 && objectType <= 599 ||
		objectType >= 251 && objectType <= 254 {
		return false
	}
	switch objectType {
	case 2, 3, 4, 5, 6, 7, 8, 9, 10, 12, 13, 121, 343, 344,
		195, 273, 274, 292, 474, 490, 607, 610, 614, 657, 658, 697, 699,
		1520, 1526, 1535, 1665, 1695, 1696, 1711, 1739, 1750,
		1855, 1993, 2005:
		return false
	default:
		return true
	}
}

func loadLoginBackdropTextures(backdrop *loginflow.LoginBackdrop, terrainPath string) (map[uint16]assets.Texture, error) {
	envDir := filepath.Dir(terrainPath)
	list, err := assets.LoadEnvTextureListFile(filepath.Join(envDir, "EnvTextureList3.bin"))
	if err != nil {
		return nil, fmt.Errorf("load official environment texture list: %w", err)
	}
	assetRoot := filepath.Dir(envDir)
	textures := make(map[uint16]assets.Texture, len(backdrop.MaterialSlots()))
	for _, slot := range backdrop.MaterialSlots() {
		nativePath, ok := list.Path(slot)
		if !ok {
			return nil, fmt.Errorf("official environment texture list has no path for slot %d", slot)
		}
		texturePath, err := resolveNativeAssetPath(assetRoot, nativePath)
		if err != nil {
			return nil, fmt.Errorf("resolve environment texture slot %d: %w", slot, err)
		}
		texture, err := assets.LoadWYSFile(texturePath)
		if err != nil {
			return nil, fmt.Errorf("load environment texture slot %d: %w", slot, err)
		}
		textures[slot] = texture
	}
	return textures, nil
}

func resolveNativeAssetPath(assetRoot, nativePath string) (string, error) {
	if assetRoot == "" || nativePath == "" {
		return "", fmt.Errorf("empty asset root or native path")
	}
	relative := filepath.Clean(filepath.FromSlash(strings.ReplaceAll(nativePath, `\`, "/")))
	if filepath.IsAbs(relative) || relative == ".." || strings.HasPrefix(relative, ".."+string(filepath.Separator)) {
		return "", fmt.Errorf("path %q escapes the runtime asset root", nativePath)
	}
	root := filepath.Clean(assetRoot)
	candidate := filepath.Join(root, relative)
	rel, err := filepath.Rel(root, candidate)
	if err != nil || rel == ".." || strings.HasPrefix(rel, ".."+string(filepath.Separator)) {
		return "", fmt.Errorf("path %q escapes the runtime asset root", nativePath)
	}
	return candidate, nil
}

// loadOptionalTerrain keeps the diagnostic TRN independent from bootstrap.
// A missing file disables only the visual aid; a present but malformed file is
// reported so corrupted assets cannot be silently accepted.
func loadOptionalTerrain(path string) (*assets.Terrain, error) {
	terrainBytes, err := os.ReadFile(path)
	if errors.Is(err, os.ErrNotExist) {
		return nil, nil
	}
	if err != nil {
		return nil, fmt.Errorf("load official character terrain: %w", err)
	}
	parsedTerrain, err := assets.ParseTerrain(terrainBytes)
	if err != nil {
		return nil, fmt.Errorf("parse official character terrain: %w", err)
	}
	return &parsedTerrain, nil
}

func loginLogoLeftPath() string {
	return loginAssetPath("logo1.wyt")
}

func loginLogoRightPath() string {
	return loginAssetPath("logo2.wyt")
}

func loginAssetPath(name string) string {
	return runtimeAssetPath("UI", name)
}

func runtimeAssetPath(parts ...string) string {
	relativeParts := append([]string{"assets", "current"}, parts...)
	relativePath := filepath.Join(relativeParts...)
	if executable, err := os.Executable(); err == nil {
		candidate := filepath.Clean(filepath.Join(filepath.Dir(executable), relativePath))
		if _, err := os.Stat(candidate); err == nil {
			return candidate
		}
	}
	return relativePath
}
