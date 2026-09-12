package assets

import (
	"errors"
	"os"
	"path/filepath"
	"testing"
)

func TestLoadOfficialModelTextureList(t *testing.T) {
	path := filepath.Join("..", "..", "assets", "current", "mesh", "MeshTextureList.bin")
	list, err := LoadModelTextureListFile(path)
	if err != nil {
		t.Fatal(err)
	}

	record, ok := list.Record(194)
	if !ok || record.Path != "mesh\\kswa01.wys" || record.Alpha != 'N' {
		t.Fatalf("slot 194 = %+v, %v", record, ok)
	}
	index, resolved, ok := list.ResolveMSATexture("kswa01.tga")
	if !ok || index != 194 || resolved != record {
		t.Fatalf("ResolveMSATexture(kswa01.tga) = %d, %+v, %v", index, resolved, ok)
	}
	index, resolved, ok = list.ResolveMSATexture("legacy\\ksob05.tga")
	if !ok || index != 174 || resolved.Path != "mesh\\ksob05.wys" {
		t.Fatalf("ResolveMSATexture(ksob05.tga) = %d, %+v, %v", index, resolved, ok)
	}
}

func TestParseModelTextureListRejectsMalformedInput(t *testing.T) {
	if _, err := ParseModelTextureList(make([]byte, 10)); !errors.Is(err, ErrInvalidModelTextureList) {
		t.Fatalf("unexpected short-table error: %v", err)
	}

	data := make([]byte, modelTextureRecordCount*modelTextureRecordSize)
	for i := 0; i < modelTextureNameLimit; i++ {
		data[i] = 'x'
	}
	if _, err := ParseModelTextureList(data); !errors.Is(err, ErrInvalidModelTextureList) {
		t.Fatalf("unexpected unterminated-name error: %v", err)
	}

	data = make([]byte, modelTextureRecordCount*modelTextureRecordSize)
	copy(data, "mesh\\bad.wys")
	data[0xff] = 'X'
	if _, err := ParseModelTextureList(data); !errors.Is(err, ErrInvalidModelTextureList) {
		t.Fatalf("unexpected alpha-mode error: %v", err)
	}
}

func TestOfficialModelTextureListHasNativeSize(t *testing.T) {
	path := filepath.Join("..", "..", "assets", "current", "mesh", "MeshTextureList.bin")
	data, err := os.ReadFile(path)
	if err != nil {
		t.Fatal(err)
	}
	if len(data) != modelTextureRecordCount*modelTextureRecordSize {
		t.Fatalf("MeshTextureList.bin size = %d", len(data))
	}
}
