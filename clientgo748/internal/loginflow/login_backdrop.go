package loginflow

import (
	"errors"
	"fmt"
	"math"
	"time"

	"wydclient748/internal/assets"
	"wydclient748/internal/graphics"
)

var ErrInvalidLoginBackdrop = errors.New("loginflow: invalid login backdrop")

const (
	loginTerrainStep        float32 = 2
	loginTerrainHeightScale float32 = .1
	loginTerrainSectorSize  float32 = 128
)

// LoginBackdrop owns the visual terrain and camera timeline shared by the
// Server and Login scenes. Scene transitions must not reset elapsed time.
type LoginBackdrop struct {
	materials       []loginTerrainMaterial
	textures        map[uint16]assets.Texture
	uploaded        map[uint16]bool
	staticObjects   []LoginStaticObject
	modelTextures   map[uint16]assets.Texture
	modelUploaded   map[uint16]bool
	transform       graphics.SceneTransform
	action          assets.CameraAction
	elapsed         time.Duration
}

type loginTerrainMaterial struct {
	primarySlot   uint16
	secondarySlot uint16
	geometry      graphics.MeshGeometry
}

type loginTerrainMaterialKey struct {
	primarySlot   uint16
	secondarySlot uint16
}

type LoginStaticMaterial struct {
	TextureSlot uint16
	Geometry    graphics.MeshGeometry
}

type LoginStaticObject struct {
	Materials []LoginStaticMaterial
	Transform graphics.SceneTransform
}

func NewLoginBackdrop(terrain assets.Terrain, action assets.CameraAction) (*LoginBackdrop, error) {
	materials, transform, err := loginTerrainGeometry(terrain)
	if err != nil {
		return nil, err
	}
	if len(action.Ticks) == 0 {
		return nil, fmt.Errorf("%w: camera action has no keyframes", ErrInvalidLoginBackdrop)
	}
	ticks := append([]assets.CameraActionTick(nil), action.Ticks...)
	return &LoginBackdrop{
		materials: materials,
		transform: transform,
		action:    assets.CameraAction{Ticks: ticks},
	}, nil
}

// MaterialSlots returns every native EnvTextureList3 slot needed by this
// backdrop, in deterministic first-use order. Login terrain is category 0 in
// the native constructor, so both stage-0 and stage-1 slots are required.
func (b *LoginBackdrop) MaterialSlots() []uint16 {
	if b == nil {
		return nil
	}
	slots := make([]uint16, 0, len(b.materials)*2)
	seen := make(map[uint16]bool, len(b.materials)*2)
	for _, material := range b.materials {
		for _, slot := range [...]uint16{material.primarySlot, material.secondarySlot} {
			if seen[slot] {
				continue
			}
			seen[slot] = true
			slots = append(slots, slot)
		}
	}
	return slots
}

// SetTerrainTextures installs decoded stage-0/stage-1 textures. GPU ownership
// remains with the renderer; this object only keeps the decoded source until
// the first successful upload for each native slot.
func (b *LoginBackdrop) SetTerrainTextures(textures map[uint16]assets.Texture) error {
	if b == nil {
		return ErrInvalidLoginBackdrop
	}
	for _, slot := range b.MaterialSlots() {
		texture, ok := textures[slot]
		if !ok {
			return fmt.Errorf("%w: missing terrain texture slot %d", ErrInvalidLoginBackdrop, slot)
		}
		if texture.Width == 0 || texture.Height == 0 || len(texture.Pixels) == 0 {
			return fmt.Errorf("%w: empty terrain texture slot %d", ErrInvalidLoginBackdrop, slot)
		}
	}
	b.textures = make(map[uint16]assets.Texture, len(textures))
	for slot, texture := range textures {
		b.textures[slot] = texture
	}
	b.uploaded = make(map[uint16]bool, len(textures))
	return nil
}

// NewLoginStaticObject applies the native field-object placement contract to a
// materialized MSA object. Field0813 uses the same 128-unit sector origin as
// its paired TRN. The DAT angle is radians; SceneTransform consumes degrees.
func NewLoginStaticObject(record assets.FieldObjectRecord, terrain assets.Terrain, materials []LoginStaticMaterial) (LoginStaticObject, error) {
	if len(materials) == 0 {
		return LoginStaticObject{}, fmt.Errorf("%w: static object type %d has no materials", ErrInvalidLoginBackdrop, record.ObjectType)
	}
	for i, material := range materials {
		if len(material.Geometry.Vertices) == 0 || len(material.Geometry.Indices) == 0 {
			return LoginStaticObject{}, fmt.Errorf("%w: static object type %d material %d has empty geometry", ErrInvalidLoginBackdrop, record.ObjectType, i)
		}
	}
	return LoginStaticObject{
		Materials: append([]LoginStaticMaterial(nil), materials...),
		Transform: graphics.SceneTransform{
			Position: graphics.Position3{
				X: float32(terrain.HeaderWidth)*loginTerrainSectorSize + record.PositionX,
				Y: record.Height,
				Z: float32(terrain.HeaderHeight)*loginTerrainSectorSize + record.PositionY,
			},
			Yaw:   record.Angle * float32(180/math.Pi),
			Pitch: -90,
			Scale: 1,
		},
	}, nil
}

// SetStaticObjects binds the decoded model textures required by the ordinary
// Field0813 MSA objects. GPU texture ownership remains with the renderer.
func (b *LoginBackdrop) SetStaticObjects(objects []LoginStaticObject, textures map[uint16]assets.Texture) error {
	if b == nil {
		return ErrInvalidLoginBackdrop
	}
	needed := make(map[uint16]bool)
	for objectIndex, object := range objects {
		if len(object.Materials) == 0 {
			return fmt.Errorf("%w: static object %d has no materials", ErrInvalidLoginBackdrop, objectIndex)
		}
		for materialIndex, material := range object.Materials {
			if len(material.Geometry.Vertices) == 0 || len(material.Geometry.Indices) == 0 {
				return fmt.Errorf("%w: static object %d material %d has empty geometry", ErrInvalidLoginBackdrop, objectIndex, materialIndex)
			}
			needed[material.TextureSlot] = true
		}
	}
	for slot := range needed {
		texture, ok := textures[slot]
		if !ok {
			return fmt.Errorf("%w: missing model texture slot %d", ErrInvalidLoginBackdrop, slot)
		}
		if texture.Width == 0 || texture.Height == 0 || len(texture.Pixels) == 0 {
			return fmt.Errorf("%w: empty model texture slot %d", ErrInvalidLoginBackdrop, slot)
		}
	}
	b.staticObjects = append([]LoginStaticObject(nil), objects...)
	b.modelTextures = make(map[uint16]assets.Texture, len(textures))
	for slot, texture := range textures {
		b.modelTextures[slot] = texture
	}
	b.modelUploaded = make(map[uint16]bool, len(textures))
	return nil
}

func loginTerrainGeometry(terrain assets.Terrain) ([]loginTerrainMaterial, graphics.SceneTransform, error) {
	if terrain.Columns < 2 || terrain.Rows < 2 || len(terrain.Cells) != terrain.Columns*terrain.Rows {
		return nil, graphics.SceneTransform{}, fmt.Errorf("%w: invalid terrain dimensions", ErrInvalidLoginBackdrop)
	}

	byMaterial := make(map[loginTerrainMaterialKey]*graphics.MeshGeometry)
	order := make([]loginTerrainMaterialKey, 0, 64)
	for y := 0; y < terrain.Rows-1; y++ {
		for x := 0; x < terrain.Columns-1; x++ {
			topLeft := terrain.Cells[y*terrain.Columns+x]
			topRight := terrain.Cells[y*terrain.Columns+x+1]
			bottomLeft := terrain.Cells[(y+1)*terrain.Columns+x]
			bottomRight := terrain.Cells[(y+1)*terrain.Columns+x+1]
			primaryUV, ok := assets.PrimaryTerrainUV(topLeft.PrimaryUVSelector)
			if !ok {
				return nil, graphics.SceneTransform{}, fmt.Errorf("%w: primary UV selector %d", ErrInvalidLoginBackdrop, topLeft.PrimaryUVSelector)
			}
			secondaryUV, ok := assets.SecondaryTerrainUV(topLeft.SecondaryUVSelector)
			if !ok {
				return nil, graphics.SceneTransform{}, fmt.Errorf("%w: secondary UV selector %d", ErrInvalidLoginBackdrop, topLeft.SecondaryUVSelector)
			}

			key := loginTerrainMaterialKey{
				primarySlot:   topLeft.PrimaryTextureSlot(),
				secondarySlot: topLeft.SecondaryTextureSlot(),
			}
			geometry := byMaterial[key]
			if geometry == nil {
				geometry = &graphics.MeshGeometry{}
				byMaterial[key] = geometry
				order = append(order, key)
			}
			if len(geometry.Vertices) > int(^uint16(0))-3 {
				return nil, graphics.SceneTransform{}, fmt.Errorf("%w: material slots %d/%d exceed 16-bit vertex indices", ErrInvalidLoginBackdrop, key.primarySlot, key.secondarySlot)
			}
			base := uint16(len(geometry.Vertices))
			geometry.Vertices = append(geometry.Vertices,
				loginTerrainVertex(x, y, topLeft.Height, primaryUV[0], secondaryUV[0]),
				loginTerrainVertex(x, y+1, bottomLeft.Height, primaryUV[1], secondaryUV[1]),
				loginTerrainVertex(x+1, y, topRight.Height, primaryUV[2], secondaryUV[2]),
				loginTerrainVertex(x+1, y+1, bottomRight.Height, primaryUV[3], secondaryUV[3]),
			)
			geometry.Indices = append(geometry.Indices,
				base, base+1, base+2,
				base+2, base+1, base+3,
			)
		}
	}

	transform := graphics.SceneTransform{
		Position: graphics.Position3{
			X: float32(terrain.HeaderWidth) * loginTerrainSectorSize,
			Z: float32(terrain.HeaderHeight) * loginTerrainSectorSize,
		},
		Scale: 1,
	}
	materials := make([]loginTerrainMaterial, 0, len(order))
	for _, key := range order {
		materials = append(materials, loginTerrainMaterial{
			primarySlot:   key.primarySlot,
			secondarySlot: key.secondarySlot,
			geometry:      *byMaterial[key],
		})
	}
	return materials, transform, nil
}

func loginTerrainVertex(x, y int, height int8, primaryUV, secondaryUV assets.TerrainUV) graphics.MeshVertex {
	return graphics.MeshVertex{
		Position: graphics.Position3{
			X: float32(x) * loginTerrainStep,
			Y: float32(height) * loginTerrainHeightScale,
			Z: float32(y) * loginTerrainStep,
		},
		Normal:               graphics.Position3{Y: 1},
		TexCoord:             graphics.TexCoord2{U: primaryUV.U, V: primaryUV.V},
		SecondaryTexCoord:    graphics.TexCoord2{U: secondaryUV.U, V: secondaryUV.V},
		HasTexCoord:          true,
		HasSecondaryTexCoord: true,
	}
}

func loginBackdropCamera(pose assets.CameraActionPose) graphics.Camera {
	horizon := float64(pose.HorizonAngle)
	vertical := float64(pose.VerticalAngle)
	cosVertical := math.Cos(vertical)
	direction := graphics.Position3{
		X: float32(math.Cos(horizon) * cosVertical),
		Y: float32(math.Sin(vertical)),
		Z: float32(math.Sin(horizon) * cosVertical),
	}
	position := graphics.Position3{X: pose.Position[0], Y: pose.Position[1], Z: pose.Position[2]}
	return graphics.Camera{
		Position: position,
		Target: graphics.Position3{
			X: position.X + direction.X,
			Y: position.Y + direction.Y,
			Z: position.Z + direction.Z,
		},
		Up: graphics.Position3{Y: 1},
	}
}

func (b *LoginBackdrop) Advance(dt time.Duration) error {
	if b == nil {
		return ErrInvalidLoginBackdrop
	}
	if dt < 0 {
		return fmt.Errorf("%w: negative elapsed delta", ErrInvalidLoginBackdrop)
	}
	b.elapsed += dt
	return nil
}

func (b *LoginBackdrop) Render(renderer graphics.Renderer) error {
	if b == nil {
		return ErrInvalidLoginBackdrop
	}
	pose, ok := b.action.Pose(b.elapsed, [3]float32{})
	if !ok {
		return fmt.Errorf("%w: camera action is empty", ErrInvalidLoginBackdrop)
	}
	camera := loginBackdropCamera(pose)
	if terrainRenderer, ok := renderer.(graphics.TerrainMaterialRenderer); ok && len(b.textures) != 0 {
		for _, material := range b.materials {
			for _, slot := range [...]uint16{material.primarySlot, material.secondarySlot} {
				if b.uploaded[slot] {
					continue
				}
				if err := terrainRenderer.UploadTerrainTexture(slot, b.textures[slot]); err != nil {
					return err
				}
				b.uploaded[slot] = true
			}
			if err := terrainRenderer.DrawTerrainGeometry(material.primarySlot, material.secondarySlot, material.geometry, b.transform, camera); err != nil {
				return err
			}
		}
	} else {
		sceneRenderer, ok := renderer.(graphics.SceneGeometryRenderer)
		if !ok {
			return graphics.ErrNotImplemented
		}
		for _, material := range b.materials {
			if err := sceneRenderer.DrawSceneGeometry(material.geometry, b.transform, camera); err != nil {
				return err
			}
		}
	}

	if len(b.staticObjects) == 0 {
		return nil
	}
	modelRenderer, ok := renderer.(graphics.ModelMaterialRenderer)
	if !ok {
		return graphics.ErrNotImplemented
	}
	for _, object := range b.staticObjects {
		for _, material := range object.Materials {
			if !b.modelUploaded[material.TextureSlot] {
				if err := modelRenderer.UploadModelTexture(material.TextureSlot, b.modelTextures[material.TextureSlot]); err != nil {
					return err
				}
				b.modelUploaded[material.TextureSlot] = true
			}
			if err := modelRenderer.DrawModelGeometry(material.TextureSlot, material.Geometry, object.Transform, camera); err != nil {
				return err
			}
		}
	}
	return nil
}
