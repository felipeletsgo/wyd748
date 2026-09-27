package game

import (
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"regexp"
	"strconv"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func TestGroundMaskBitsMatchClientSource(t *testing.T) {
	path := filepath.Join("..", "..", "..", "tmproject", "TMProject748", "internal", "core", "Basedef.h")
	source, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	table := regexp.MustCompile(`(?s)g_pGroundMask\[10\]\[4\]\[6\]\[6\]\s*=\s*\{(.*?)\};`).FindSubmatch(source)
	if table == nil {
		t.Fatal("client ground-mask table not found")
	}
	values := regexp.MustCompile(`\b\d+\b`).FindAllString(string(table[1]), -1)
	if len(values) != 10*4*36 {
		t.Fatalf("client ground-mask cells = %d; want 1440", len(values))
	}
	nativeTable := make([]byte, 4*len(values))
	for i, value := range values {
		cell, err := strconv.Atoi(value)
		if err != nil {
			t.Fatal(err)
		}
		binary.LittleEndian.PutUint32(nativeTable[i*4:], uint32(cell))
		mask, rotation, bit := i/144, (i/36)%4, i%36
		if got := groundMaskBits[mask][rotation]&(uint64(1)<<bit) != 0; got != (cell != 0) {
			t.Fatalf("mask %d rotation %d cell %d: server=%t client=%t", mask, rotation, bit, got, cell != 0)
		}
	}
	// The 5,760-byte table is identical in both native 7.48 executables at
	// file offset 0x1BED50 (VA 0x005BED50); FUN_005554CC references that VA.
	const nativeSHA256 = "b53392beb7de3b2e74a026d4256df822f7a5f445a858d5d348f36b196d03ef3d"
	if got := fmt.Sprintf("%x", sha256.Sum256(nativeTable)); got != nativeSHA256 {
		t.Fatalf("client ground-mask table hash = %s; want native 7.48 %s", got, nativeSHA256)
	}
}

func TestClosedGroundMaskRejectsRouteUntilOpened(t *testing.T) {
	w := &World{
		items: map[uint16]model.ItemDef{458: {Index: 458,
			StaticEffects: []model.StaticEffect{{Name: "EF_GROUND", Value: 1}}}},
		terrain: model.TerrainMap{Height: make([]byte, model.TerrainCells),
			Attribute: make([]byte, model.AttributeCells)},
	}
	gate := &GroundItem{Item: model.Item{Index: 458}, X: 100, Y: 100, Rotate: 0}
	w.applyGroundItemHeight(gate)
	if w.terrain.RouteHeightCompatible(100, 99, 100, 100) {
		t.Fatal("closed gate permitted a route through its collision mask")
	}
	gate.State = gateOpen
	w.applyGroundItemHeight(gate)
	if !w.terrain.RouteHeightCompatible(100, 99, 100, 100) {
		t.Fatal("opened gate still blocks its former collision mask")
	}
}

func TestGatePersistenceControlsTerrainTransition(t *testing.T) {
	w, gate := mundoComPorta(2, 100, 100)
	def := w.items[gate.Item.Index]
	def.StaticEffects = append(def.StaticEffects, model.StaticEffect{Name: "EF_GROUND", Value: 1})
	w.items[gate.Item.Index] = def
	w.terrain = model.TerrainMap{Height: make([]byte, model.TerrainCells),
		Attribute: make([]byte, model.AttributeCells)}
	w.applyGroundItemHeight(gate)
	player, session := jogadorComChave(w, 451, 100, 100)
	w.store = &craftStore{err: errors.New("storage unavailable")}

	if w.openGateWithKey(session, player, gate, 0, true, 3) {
		t.Fatal("gate opened despite failed persistence")
	}
	if gate.State != gateClosed || w.terrain.RouteHeightCompatible(100, 99, 100, 100) {
		t.Fatal("failed persistence changed the gate or its collision")
	}
	w.store = &craftStore{}
	if !w.openGateWithKey(session, player, gate, 0, true, 3) {
		t.Fatal("gate did not open after successful persistence")
	}
	if gate.State != gateOpen || !w.terrain.RouteHeightCompatible(100, 99, 100, 100) {
		t.Fatal("successful persistence did not clear the gate collision")
	}
}

func TestCSVFixturesUpdateServerTerrainLikeClient(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	objects, err := data.LoadInitItems(filepath.Join(root, "init_items.csv"), catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	if len(objects) != 96 {
		t.Fatalf("loaded %d permanent objects; want 96", len(objects))
	}
	occupied := make(map[int]model.InitItem)
	for _, object := range objects {
		mask := staticEffect(catalog.Items[object.Index], "EF_GROUND")
		if mask <= 0 || mask >= len(groundMaskBits) {
			continue
		}
		for cell := 0; cell < 36; cell++ {
			if groundMaskBits[mask][object.Rotate]&(uint64(1)<<cell) == 0 {
				continue
			}
			x := int(object.X) + cell%6 - 2
			y := int(object.Y) + cell/6 - 2
			if x < 0 || y < 0 || x >= model.TerrainWidth || y >= model.TerrainHeight {
				continue
			}
			key := y*model.TerrainWidth + x
			if previous, overlap := occupied[key]; overlap {
				t.Fatalf("ground objects %d and %d overlap at (%d,%d)", previous.Index, object.Index, x, y)
			}
			occupied[key] = object
		}
	}
	base := model.TerrainMap{Height: make([]byte, model.TerrainCells), Attribute: make([]byte, model.AttributeCells)}
	w := &World{items: catalog.Items, terrain: base, initItems: objects,
		groundItems: make(map[uint16]*GroundItem), nextItemID: 10000}
	if err := w.spawnInitItems(); err != nil {
		t.Fatal(err)
	}
	if len(w.groundItems) != 96 {
		t.Fatalf("spawned %d permanent objects; want 96", len(w.groundItems))
	}
	machines := 0
	for _, object := range objects {
		if object.Index != 4102 && object.Index != 4103 {
			continue
		}
		machines++
		if mask := staticEffect(catalog.Items[object.Index], "EF_GROUND"); mask != 10 {
			t.Fatalf("slot machine %d has ground effect %d; want unmasked effect 10", object.Index, mask)
		}
		for dy := -2; dy <= 3; dy++ {
			for dx := -2; dx <= 3; dx++ {
				x, y := int(object.X)+dx, int(object.Y)+dy
				if w.terrain.Height[y*model.TerrainWidth+x] != 0 {
					t.Fatalf("slot machine %d unexpectedly changed collision height at (%d,%d)", object.Index, x, y)
				}
			}
		}
	}
	if machines != 2 {
		t.Fatalf("spawned %d slot machines; want 2", machines)
	}
	var gate *GroundItem
	for _, item := range w.groundItems {
		mask := staticEffect(catalog.Items[item.Item.Index], "EF_GROUND")
		if mask > 0 && mask < len(groundMaskBits) && groundMaskBits[mask][item.Rotate] != 0 {
			gate = item
			break
		}
	}
	if gate == nil {
		t.Fatal("CSV contains no masked ground object")
	}
	mask := staticEffect(catalog.Items[gate.Item.Index], "EF_GROUND")
	for cell := 0; cell < 36; cell++ {
		if groundMaskBits[mask][gate.Rotate]&(uint64(1)<<cell) == 0 {
			continue
		}
		x := int(gate.X) + cell%6 - 2
		y := int(gate.Y) + cell/6 - 2
		if x < 0 || y < 0 || x >= model.TerrainWidth || y >= model.TerrainHeight {
			continue
		}
		offset := y*model.TerrainWidth + x
		if w.terrain.Height[offset] != clientGateHeight || base.Height[offset] != 0 {
			t.Fatalf("closed object at (%d,%d): server=%d base=%d", x, y, w.terrain.Height[offset], base.Height[offset])
		}
		gate.State = gateOpen
		w.applyGroundItemHeight(gate)
		if w.terrain.Height[offset] != 0 {
			t.Fatalf("opened object at (%d,%d) retained height %d", x, y, w.terrain.Height[offset])
		}
		break
	}
}
