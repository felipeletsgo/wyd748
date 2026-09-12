package assets

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"
	"os"
)

const (
	fieldObjectBaseRecordSize = 28
	fieldObjectScaleSize      = 8
	maxFieldObjectRecords     = 0x1000
)

var (
	ErrInvalidFieldObjectDAT   = errors.New("clientgo748: invalid field object DAT")
	ErrTruncatedFieldObjectDAT = errors.New("clientgo748: truncated field object DAT")
)

// FieldObjectRecord is one serialized ObjectFileItem from the native 7.48
// field-object stream. ScaleH/ScaleV are physically present only for the
// special effect/object ranges consumed with the extra 8-byte suffix.
type FieldObjectRecord struct {
	ObjectType      uint32
	PositionX       float32
	PositionY       float32
	Height          float32
	Angle           float32
	TextureSetIndex int32
	MaskIndex       int32
	ScaleH          float32
	ScaleV          float32
	HasScale        bool
}

func LoadFieldObjectDATFile(path string) ([]FieldObjectRecord, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return nil, fmt.Errorf("clientgo748: read field object DAT %q: %w", path, err)
	}
	records, err := ParseFieldObjectDAT(data)
	if err != nil {
		return nil, fmt.Errorf("clientgo748: parse field object DAT %q: %w", path, err)
	}
	return records, nil
}

func fieldObjectHasScaleSuffix(objectType uint32) bool {
	// FUN_00541065 / TMObjectContainer::Load advance the stream by eight
	// additional bytes for 501..506, 511..518, and 520..599. Type 519 is a
	// deliberate hole in the native dispatch and therefore has no suffix.
	return objectType >= 501 && objectType <= 506 ||
		objectType >= 511 && objectType <= 518 ||
		objectType >= 520 && objectType <= 599
}

func ParseFieldObjectDAT(data []byte) ([]FieldObjectRecord, error) {
	if len(data) == 0 {
		return nil, fmt.Errorf("%w: empty stream", ErrInvalidFieldObjectDAT)
	}

	records := make([]FieldObjectRecord, 0, 256)
	for off := 0; off < len(data); {
		if len(records) >= maxFieldObjectRecords {
			return nil, fmt.Errorf("%w: more than %d records", ErrInvalidFieldObjectDAT, maxFieldObjectRecords)
		}
		if len(data)-off < fieldObjectBaseRecordSize {
			return nil, fmt.Errorf("%w: record %d has %d base bytes", ErrTruncatedFieldObjectDAT, len(records), len(data)-off)
		}

		base := data[off : off+fieldObjectBaseRecordSize]
		record := FieldObjectRecord{
			ObjectType:      binary.LittleEndian.Uint32(base[0:4]),
			PositionX:       math.Float32frombits(binary.LittleEndian.Uint32(base[4:8])),
			PositionY:       math.Float32frombits(binary.LittleEndian.Uint32(base[8:12])),
			Height:          math.Float32frombits(binary.LittleEndian.Uint32(base[12:16])),
			Angle:           math.Float32frombits(binary.LittleEndian.Uint32(base[16:20])),
			TextureSetIndex: int32(binary.LittleEndian.Uint32(base[20:24])),
			MaskIndex:       int32(binary.LittleEndian.Uint32(base[24:28])),
		}
		off += fieldObjectBaseRecordSize

		if fieldObjectHasScaleSuffix(record.ObjectType) {
			if len(data)-off < fieldObjectScaleSize {
				return nil, fmt.Errorf("%w: record %d type %d is missing scale suffix", ErrTruncatedFieldObjectDAT, len(records), record.ObjectType)
			}
			record.ScaleH = math.Float32frombits(binary.LittleEndian.Uint32(data[off : off+4]))
			record.ScaleV = math.Float32frombits(binary.LittleEndian.Uint32(data[off+4 : off+8]))
			record.HasScale = true
			off += fieldObjectScaleSize
		}

		records = append(records, record)
	}

	return records, nil
}
