package data

import (
	"encoding/binary"
	"os"
	"path/filepath"
	"reflect"
	"testing"

	"wydgo/internal/model"
)

func TestInitItemsMatchClient748Asset(t *testing.T) {
	assetPath := filepath.Join("..", "..", "..", "tmproject", "client748", "InitItem.bin")
	encoded, err := os.ReadFile(assetPath)
	if err != nil {
		t.Fatal(err)
	}
	const recordSize = 8
	const recordCount = 96
	if len(encoded) != recordSize*recordCount {
		t.Fatalf("client asset has %d bytes; want %d records of %d bytes", len(encoded), recordCount, recordSize)
	}

	want := make([]model.InitItem, 0, recordCount)
	items := make(map[uint16]model.ItemDef)
	for offset := 0; offset < len(encoded); offset += recordSize {
		var decoded [recordSize]byte
		for i := range decoded {
			decoded[i] = encoded[offset+i] ^ 0xff
		}
		rotation := binary.LittleEndian.Uint16(decoded[6:8])
		if rotation > 255 {
			t.Fatalf("client record %d has rotation %d outside the server byte range", offset/recordSize, rotation)
		}
		obj := model.InitItem{
			X:      binary.LittleEndian.Uint16(decoded[0:2]),
			Y:      binary.LittleEndian.Uint16(decoded[2:4]),
			Index:  binary.LittleEndian.Uint16(decoded[4:6]),
			Rotate: byte(rotation),
		}
		want = append(want, obj)
		items[obj.Index] = model.ItemDef{Index: obj.Index}
	}

	got, err := LoadInitItems(filepath.Join("..", "..", "data", "init_items.csv"), items)
	if err != nil {
		t.Fatal(err)
	}
	if !reflect.DeepEqual(got, want) {
		for i := range want {
			if i >= len(got) || got[i] != want[i] {
				t.Fatalf("CSV record %d: got %v, want %v", i, itemAt(got, i), want[i])
			}
		}
		t.Fatalf("CSV has %d records; want %d", len(got), len(want))
	}
}

func itemAt(items []model.InitItem, index int) model.InitItem {
	if index < len(items) {
		return items[index]
	}
	return model.InitItem{}
}
