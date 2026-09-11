package assets

import (
	"encoding/binary"
	"path/filepath"
	"testing"
)

func TestParseItemListOfficialAsset(t *testing.T) {
	list, err := LoadItemListFile(filepath.Join("..", "..", "assets", "current", "ItemList.bin"))
	if err != nil {
		t.Fatal(err)
	}
	if len(list.Records) != ItemListRecordCount {
		t.Fatalf("records=%d, want %d", len(list.Records), ItemListRecordCount)
	}
}

func TestParseItemListUses748DiskOffsetsAndLeavesStampUndecoded(t *testing.T) {
	const payloadSize = ItemListRecordCount * ItemListRecordSize
	data := make([]byte, payloadSize+ItemListStampSize)
	decoded := make([]byte, payloadSize)
	copy(decoded[:64], []byte("Visual Test\x00"))
	binary.LittleEndian.PutUint16(decoded[0x40:], uint16(321))
	binary.LittleEndian.PutUint16(decoded[0x42:], uint16(654))
	binary.LittleEndian.PutUint16(decoded[0x44:], uint16(7))
	binary.LittleEndian.PutUint16(decoded[0x50:], uint16(11))
	binary.LittleEndian.PutUint16(decoded[0x52:], uint16(22))
	binary.LittleEndian.PutUint32(decoded[0x80:], uint32(123456))
	binary.LittleEndian.PutUint16(decoded[0x86:], uint16(0x3456))
	binary.LittleEndian.PutUint16(decoded[0x8A:], uint16(9))
	for i := range decoded {
		data[i] = decoded[i] ^ itemListXORKey
	}
	binary.LittleEndian.PutUint32(data[payloadSize:], 0x78563412)

	list, err := ParseItemList(data)
	if err != nil {
		t.Fatal(err)
	}
	got := list.Records[0]
	if got.Name != "Visual Test" || got.IndexMesh != 321 || got.IndexTexture != 654 || got.IndexVisualEffect != 7 {
		t.Fatalf("unexpected visual fields: %+v", got)
	}
	if got.Effects[0] != (ItemStaticEffect{Index: 11, Value: 22}) || got.Price != 123456 || got.Position != 0x3456 || got.Grade != 9 {
		t.Fatalf("unexpected ABI fields: %+v", got)
	}
	if list.Stamp != 0x78563412 {
		t.Fatalf("stamp=%#x, want %#x", list.Stamp, uint32(0x78563412))
	}
}

func TestParseItemListRejectsMalformedSize(t *testing.T) {
	if _, err := ParseItemList(make([]byte, ItemListRecordCount*ItemListRecordSize)); err == nil {
		t.Fatal("expected malformed ItemList size to fail")
	}
}
