package assets

import (
	"encoding/binary"
	"errors"
	"math"
	"os"
	"path/filepath"
	"testing"
)

func TestLoadOfficialField0813DAT(t *testing.T) {
	path := filepath.Join("..", "..", "assets", "current", "env", "Field0813.dat")
	data, err := os.ReadFile(path)
	if err != nil {
		t.Fatalf("ReadFile(%q): %v", path, err)
	}
	if got, want := len(data), 23600; got != want {
		t.Fatalf("Field0813.dat size = %d, want %d", got, want)
	}

	records, err := ParseFieldObjectDAT(data)
	if err != nil {
		t.Fatalf("ParseFieldObjectDAT(): %v", err)
	}
	if got, want := len(records), 830; got != want {
		t.Fatalf("record count = %d, want %d", got, want)
	}
}

func TestParseFieldObjectDATScaleSuffixRangesAnd519Hole(t *testing.T) {
	data := appendFieldObjectTestRecord(nil, 501, true)
	data = appendFieldObjectTestRecord(data, 506, true)
	data = appendFieldObjectTestRecord(data, 511, true)
	data = appendFieldObjectTestRecord(data, 518, true)
	data = appendFieldObjectTestRecord(data, 519, false)
	data = appendFieldObjectTestRecord(data, 520, true)
	data = appendFieldObjectTestRecord(data, 599, true)
	data = appendFieldObjectTestRecord(data, 600, false)

	records, err := ParseFieldObjectDAT(data)
	if err != nil {
		t.Fatalf("ParseFieldObjectDAT(): %v", err)
	}
	if len(records) != 8 {
		t.Fatalf("record count = %d, want 8", len(records))
	}
	for i, record := range records {
		wantScale := record.ObjectType != 519 && record.ObjectType != 600
		if record.HasScale != wantScale {
			t.Fatalf("record %d type %d HasScale = %v, want %v", i, record.ObjectType, record.HasScale, wantScale)
		}
		if wantScale && (record.ScaleH != 1.25 || record.ScaleV != 2.5) {
			t.Fatalf("record %d scale = (%v,%v)", i, record.ScaleH, record.ScaleV)
		}
	}
}

func TestParseFieldObjectDATRejectsTruncatedBaseAndScale(t *testing.T) {
	for name, data := range map[string][]byte{
		"base":  make([]byte, fieldObjectBaseRecordSize-1),
		"scale": appendFieldObjectTestRecord(nil, 501, false),
	} {
		t.Run(name, func(t *testing.T) {
			_, err := ParseFieldObjectDAT(data)
			if !errors.Is(err, ErrTruncatedFieldObjectDAT) {
				t.Fatalf("ParseFieldObjectDAT() error = %v, want %v", err, ErrTruncatedFieldObjectDAT)
			}
		})
	}
}

func appendFieldObjectTestRecord(dst []byte, objectType uint32, withScale bool) []byte {
	base := make([]byte, fieldObjectBaseRecordSize)
	binary.LittleEndian.PutUint32(base[0:4], objectType)
	binary.LittleEndian.PutUint32(base[4:8], math.Float32bits(10.5))
	binary.LittleEndian.PutUint32(base[8:12], math.Float32bits(20.5))
	binary.LittleEndian.PutUint32(base[12:16], math.Float32bits(3.5))
	binary.LittleEndian.PutUint32(base[16:20], math.Float32bits(0.75))
	binary.LittleEndian.PutUint32(base[20:24], 7)
	binary.LittleEndian.PutUint32(base[24:28], 9)
	dst = append(dst, base...)
	if withScale {
		scale := make([]byte, fieldObjectScaleSize)
		binary.LittleEndian.PutUint32(scale[0:4], math.Float32bits(1.25))
		binary.LittleEndian.PutUint32(scale[4:8], math.Float32bits(2.5))
		dst = append(dst, scale...)
	}
	return dst
}
