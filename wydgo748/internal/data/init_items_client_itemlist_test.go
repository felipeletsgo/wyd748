package data

import (
	"encoding/binary"
	"os"
	"path/filepath"
	"testing"
)

func TestInitItemDefinitionsMatchClient748ItemList(t *testing.T) {
	dataRoot := filepath.Join("..", "..", "data")
	catalog, err := LoadCatalog(filepath.Join(dataRoot, "itemlist.csv"),
		filepath.Join(dataRoot, "Itemname.csv"), filepath.Join(dataRoot, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	if catalog.ItemEffects[34] != "EF_GROUND" || catalog.ItemEffects[39] != "EF_KEYID" {
		t.Fatal("ground/key effect IDs differ from the client 7.48 item list")
	}
	objects, err := LoadInitItems(filepath.Join(dataRoot, "init_items.csv"), catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	assetPath := filepath.Join("..", "..", "..", "tmproject", "client748", "ItemList.bin")
	encoded, err := os.ReadFile(assetPath)
	if err != nil {
		t.Fatal(err)
	}
	const itemCount = 6500
	const recordSize = 140
	if len(encoded) != itemCount*recordSize+4 {
		t.Fatalf("client item list has %d bytes; want %d", len(encoded), itemCount*recordSize+4)
	}

	seen := make(map[uint16]bool)
	for _, object := range objects {
		if seen[object.Index] {
			continue
		}
		seen[object.Index] = true
		definition := catalog.Items[object.Index]
		if object.Index >= itemCount {
			t.Fatalf("initial object %d is outside the client item list", object.Index)
		}
		var row [recordSize]byte
		for i := range row {
			row[i] = encoded[int(object.Index)*recordSize+i] ^ 0x5a
		}
		mesh := int(int16(binary.LittleEndian.Uint16(row[64:66])))
		if mesh != definition.Mesh {
			t.Errorf("item %d mesh: client %d, server %d", object.Index, mesh, definition.Mesh)
		}
		for effectID, effectName := range map[uint16]string{34: "EF_GROUND", 39: "EF_KEYID"} {
			clientValue := int16(0)
			for slot := 0; slot < 12; slot++ {
				offset := 80 + slot*4
				if binary.LittleEndian.Uint16(row[offset:offset+2]) == effectID {
					clientValue = int16(binary.LittleEndian.Uint16(row[offset+2 : offset+4]))
					break
				}
			}
			serverValue := 0
			for _, effect := range definition.StaticEffects {
				if effect.Name == effectName {
					serverValue = effect.Value
					break
				}
			}
			if int(clientValue) != serverValue {
				t.Errorf("item %d %s: client %d, server %d",
					object.Index, effectName, clientValue, serverValue)
			}
		}
	}
	if len(seen) != 26 {
		t.Fatalf("checked %d initial-object definitions; want 26", len(seen))
	}
}
