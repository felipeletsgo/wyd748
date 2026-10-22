package staffcontent

import (
	"bytes"
	"image"
	"image/color"
	"image/png"
	"sort"
	"sync"

	"wydgo/internal/model"
)

// Zones answers "what applies at this cell" from static data only: terrain,
// teleports, quest zones and instance areas. It never reads the World.
type Zones struct {
	terrain   model.TerrainMap
	teleports []model.Teleport
	quests    []model.QuestZone
	clears    []model.QuestZone
	instances []model.VolatileInstance

	pngOnce sync.Once
	png     []byte
	pngErr  error
}

func NewZones(terrain model.TerrainMap, teleports []model.Teleport, quests model.QuestZoneFile, volatiles model.VolatileCatalog) *Zones {
	z := &Zones{terrain: terrain, teleports: teleports, quests: quests.Zones, clears: quests.ClearAreas}
	for _, inst := range volatiles.Instances {
		z.instances = append(z.instances, inst)
	}
	sort.Slice(z.instances, func(i, j int) bool { return z.instances[i].ID < z.instances[j].ID })
	return z
}

type ZoneTeleport struct {
	Name  string `json:"name"`
	DestX uint16 `json:"destX"`
	DestY uint16 `json:"destY"`
	Price uint32 `json:"price"`
}

type ZoneInstance struct {
	ID         string `json:"id"`
	Name       string `json:"name"`
	X          uint16 `json:"x"`
	Y          uint16 `json:"y"`
	AreaRadius int    `json:"areaRadius"`
}

// ZoneInfo names only attribute bits whose meaning the server relies on:
// 0x02 blocks movement, 0x10 activates a client teleport, 0x80 is the newbie
// training field. Attribute keeps the raw byte for every other bit.
type ZoneInfo struct {
	Version            int            `json:"version"`
	X                  uint16         `json:"x"`
	Y                  uint16         `json:"y"`
	TerrainLoaded      bool           `json:"terrainLoaded"`
	Height             int            `json:"height"`
	Attribute          uint8          `json:"attribute"`
	Walkable           bool           `json:"walkable"`
	Blocked            bool           `json:"blocked"`
	TeleportActivation bool           `json:"teleportActivation"`
	NewbieZone         bool           `json:"newbieZone"`
	Teleports          []ZoneTeleport `json:"teleports"`
	QuestZones         []string       `json:"questZones"`
	ClearAreas         []string       `json:"clearAreas"`
	Instances          []ZoneInstance `json:"instances"`
}

func chebyshev(ax, ay, bx, by uint16) int {
	return max(int(max(ax, bx)-min(ax, bx)), int(max(ay, by)-min(ay, by)))
}

func (z *Zones) At(x, y uint16) ZoneInfo {
	info := ZoneInfo{Version: 1, X: x, Y: y, Teleports: []ZoneTeleport{}, QuestZones: []string{}, ClearAreas: []string{}, Instances: []ZoneInstance{}}
	if z.terrain.Loaded() && int(x) < model.TerrainWidth && int(y) < model.TerrainHeight {
		info.TerrainLoaded = true
		height := z.terrain.Height[int(y)*model.TerrainWidth+int(x)]
		info.Height = int(int8(height))
		info.Attribute = z.terrain.Attribute[(int(y)/4)*model.AttributeWidth+int(x)/4]
		info.Blocked = height == model.TerrainBlockedByte || info.Attribute&0x02 != 0
		info.TeleportActivation = info.Attribute&0x10 != 0
		info.NewbieZone = info.Attribute&0x80 != 0
	}
	info.Walkable = z.terrain.Walkable(x, y)
	for _, t := range z.teleports {
		// A portal fires in the 4x4 square whose origin is SourceX/SourceY.
		if x >= t.SourceX && x < t.SourceX+4 && y >= t.SourceY && y < t.SourceY+4 {
			info.Teleports = append(info.Teleports, ZoneTeleport{Name: t.Name, DestX: t.DestX, DestY: t.DestY, Price: t.Price})
		}
	}
	for _, zone := range z.quests {
		if zone.Contains(x, y) {
			info.QuestZones = append(info.QuestZones, zone.Name)
		}
	}
	for _, zone := range z.clears {
		if zone.Contains(x, y) {
			info.ClearAreas = append(info.ClearAreas, zone.Name)
		}
	}
	for _, inst := range z.instances {
		if inst.AreaRadius > 0 && chebyshev(x, y, inst.X, inst.Y) <= inst.AreaRadius {
			info.Instances = append(info.Instances, ZoneInstance{ID: inst.ID, Name: inst.Name, X: inst.X, Y: inst.Y, AreaRadius: inst.AreaRadius})
		}
	}
	return info
}

// TerrainPNG renders the attribute grid (one pixel per 4x4 block, 1024x1024)
// once and caches it. Blocked blocks are dark, walkable blocks are shaded by
// height, teleport and newbie bits are tinted.
func (z *Zones) TerrainPNG() ([]byte, error) {
	z.pngOnce.Do(func() {
		img := image.NewRGBA(image.Rect(0, 0, model.AttributeWidth, model.AttributeHeight))
		loaded := z.terrain.Loaded()
		for by := 0; by < model.AttributeHeight; by++ {
			for bx := 0; bx < model.AttributeWidth; bx++ {
				c := color.RGBA{R: 40, G: 44, B: 40, A: 255}
				if loaded {
					attribute := z.terrain.Attribute[by*model.AttributeWidth+bx]
					height := z.terrain.Height[(by*4)*model.TerrainWidth+bx*4]
					switch {
					case height == model.TerrainBlockedByte || attribute&0x02 != 0:
					case attribute&0x10 != 0:
						c = color.RGBA{R: 214, G: 160, B: 60, A: 255}
					case attribute&0x80 != 0:
						c = color.RGBA{R: 120, G: 170, B: 210, A: 255}
					default:
						shade := uint8(120 + (int(int8(height))+128)/4)
						c = color.RGBA{R: shade - 30, G: shade, B: shade - 40, A: 255}
					}
				}
				img.SetRGBA(bx, by, c)
			}
		}
		var out bytes.Buffer
		z.pngErr = png.Encode(&out, img)
		z.png = out.Bytes()
	})
	return z.png, z.pngErr
}
