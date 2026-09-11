package assets

import (
	"encoding/binary"
	"errors"
	"fmt"
	"os"
	"strings"
)

const (
	ItemListRecordCount = 6500
	ItemListRecordSize  = 140
	ItemListStampSize   = 4
	itemListXORKey      = byte(0x5A)
)

var ErrInvalidItemList = errors.New("clientgo748: invalid ItemList.bin")

// ItemStaticEffect preserves one 7.48 STRUCT_STATICEFFECT pair from disk.
type ItemStaticEffect struct {
	Index int16
	Value int16
}

// ItemDefinition is the compact 7.48 ItemList.bin record used by visual
// assembly. Its fields follow the proven 140-byte disk ABI, not the larger
// later TMProject runtime struct.
type ItemDefinition struct {
	Name                 string
	IndexMesh            int16
	IndexTexture         int16
	IndexVisualEffect    int16
	RequiredLevel        int16
	RequiredStrength     int16
	RequiredIntelligence int16
	RequiredDexterity    int16
	RequiredConstitution int16
	Effects              [12]ItemStaticEffect
	Price                int32
	Unique               int16
	Position             uint16
	Extra                int16
	Grade                int16
}

// ItemList owns all 6500 decoded records. Index performs the same modulo
// lookup used by TMProject when translating equipped STRUCT_ITEM indices.
type ItemList struct {
	Records [ItemListRecordCount]ItemDefinition
	Stamp   uint32
}

func (l *ItemList) Lookup(index uint16) ItemDefinition {
	if l == nil {
		return ItemDefinition{}
	}
	return l.Records[int(index)%ItemListRecordCount]
}

func LoadItemListFile(path string) (ItemList, error) {
	b, err := os.ReadFile(path)
	if err != nil {
		return ItemList{}, fmt.Errorf("clientgo748: read ItemList %q: %w", path, err)
	}
	list, err := ParseItemList(b)
	if err != nil {
		return ItemList{}, fmt.Errorf("clientgo748: parse ItemList %q: %w", path, err)
	}
	return list, nil
}

func ParseItemList(data []byte) (ItemList, error) {
	const payloadSize = ItemListRecordCount * ItemListRecordSize
	if len(data) != payloadSize+ItemListStampSize {
		return ItemList{}, fmt.Errorf("%w: size %d, want %d", ErrInvalidItemList, len(data), payloadSize+ItemListStampSize)
	}

	decoded := append([]byte(nil), data[:payloadSize]...)
	for i := range decoded {
		decoded[i] ^= itemListXORKey
	}

	var result ItemList
	result.Stamp = binary.LittleEndian.Uint32(data[payloadSize:])
	for i := range result.Records {
		row := decoded[i*ItemListRecordSize : (i+1)*ItemListRecordSize]
		name := row[:64]
		if end := indexByte(name, 0); end >= 0 {
			name = name[:end]
		}
		item := ItemDefinition{
			Name:                 strings.TrimRight(string(name), " \x00"),
			IndexMesh:            int16(binary.LittleEndian.Uint16(row[0x40:])),
			IndexTexture:         int16(binary.LittleEndian.Uint16(row[0x42:])),
			IndexVisualEffect:    int16(binary.LittleEndian.Uint16(row[0x44:])),
			RequiredLevel:        int16(binary.LittleEndian.Uint16(row[0x46:])),
			RequiredStrength:     int16(binary.LittleEndian.Uint16(row[0x48:])),
			RequiredIntelligence: int16(binary.LittleEndian.Uint16(row[0x4A:])),
			RequiredDexterity:    int16(binary.LittleEndian.Uint16(row[0x4C:])),
			RequiredConstitution: int16(binary.LittleEndian.Uint16(row[0x4E:])),
			Price:                int32(binary.LittleEndian.Uint32(row[0x80:])),
			Unique:               int16(binary.LittleEndian.Uint16(row[0x84:])),
			Position:             binary.LittleEndian.Uint16(row[0x86:]),
			Extra:                int16(binary.LittleEndian.Uint16(row[0x88:])),
			Grade:                int16(binary.LittleEndian.Uint16(row[0x8A:])),
		}
		for effect := range item.Effects {
			off := 0x50 + effect*4
			item.Effects[effect] = ItemStaticEffect{
				Index: int16(binary.LittleEndian.Uint16(row[off:])),
				Value: int16(binary.LittleEndian.Uint16(row[off+2:])),
			}
		}
		result.Records[i] = item
	}
	return result, nil
}

func indexByte(data []byte, value byte) int {
	for i, b := range data {
		if b == value {
			return i
		}
	}
	return -1
}
