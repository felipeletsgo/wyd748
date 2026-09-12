package assets

import (
	"errors"
	"fmt"
	"os"
	"strings"
)

const (
	modelTextureRecordCount = 0x800
	modelTextureRecordSize  = 0x108
	modelTextureNameLimit   = 0xff
)

var ErrInvalidModelTextureList = errors.New("clientgo748: invalid MeshTextureList")

type ModelTextureRecord struct {
	Path  string
	Alpha byte
}

// ModelTextureList mirrors the compact table consumed by FUN_004b93c4 and
// searched by FUN_004b9bb9 in the native 7.48 client.
type ModelTextureList struct {
	records [modelTextureRecordCount]ModelTextureRecord
}

func LoadModelTextureListFile(path string) (ModelTextureList, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return ModelTextureList{}, fmt.Errorf("clientgo748: read MeshTextureList %q: %w", path, err)
	}
	list, err := ParseModelTextureList(data)
	if err != nil {
		return ModelTextureList{}, fmt.Errorf("clientgo748: parse MeshTextureList %q: %w", path, err)
	}
	return list, nil
}

func ParseModelTextureList(data []byte) (ModelTextureList, error) {
	want := modelTextureRecordCount * modelTextureRecordSize
	if len(data) != want {
		return ModelTextureList{}, fmt.Errorf("%w: size %d, want %d", ErrInvalidModelTextureList, len(data), want)
	}

	var list ModelTextureList
	for slot := 0; slot < modelTextureRecordCount; slot++ {
		record := data[slot*modelTextureRecordSize : (slot+1)*modelTextureRecordSize]
		end := 0
		for end < modelTextureNameLimit && record[end] != 0 {
			end++
		}
		if end == modelTextureNameLimit {
			return ModelTextureList{}, fmt.Errorf("%w: slot %d has no terminated filename", ErrInvalidModelTextureList, slot)
		}
		alpha := record[0xff]
		if alpha != 0 && alpha != 'N' && alpha != 'A' && alpha != 'a' && alpha != 'C' {
			return ModelTextureList{}, fmt.Errorf("%w: slot %d has alpha mode %#02x", ErrInvalidModelTextureList, slot, alpha)
		}
		list.records[slot] = ModelTextureRecord{Path: string(record[:end]), Alpha: alpha}
	}
	return list, nil
}

func (l ModelTextureList) Record(slot int) (ModelTextureRecord, bool) {
	if slot < 0 || slot >= len(l.records) || l.records[slot].Path == "" {
		return ModelTextureRecord{}, false
	}
	return l.records[slot], true
}

// ResolveMSATexture reproduces the native LoadMsa + FUN_004b9bb9 lookup.
// A serialized name such as "kswa01.tga" first becomes
// "mesh\\kswa01.wyt"; FUN_004b9bb9 then replaces only the final three
// extension characters with those of each table row before comparing.
func (l ModelTextureList) ResolveMSATexture(serializedName string) (int, ModelTextureRecord, bool) {
	name := strings.TrimRight(serializedName, "\x00")
	name = strings.ReplaceAll(name, "/", "\\")
	if slash := strings.LastIndexByte(name, '\\'); slash >= 0 {
		name = name[slash+1:]
	}
	if dot := strings.LastIndexByte(name, '.'); dot >= 0 {
		name = name[:dot]
	}
	if name == "" {
		return -1, ModelTextureRecord{}, false
	}

	candidate := "mesh\\" + name + ".wyt"
	if len(candidate) < 3 {
		return -1, ModelTextureRecord{}, false
	}
	for index, record := range l.records {
		if record.Path == "" || len(record.Path) < 3 {
			continue
		}
		probe := candidate[:len(candidate)-3] + record.Path[len(record.Path)-3:]
		if probe == record.Path {
			return index, record, true
		}
	}
	return -1, ModelTextureRecord{}, false
}
