package assets

import (
	"errors"
	"fmt"
	"os"
)

const (
	envTextureRecordCount = 512
	envTextureRecordSize  = 0x108
	envTextureNameLimit   = 0xff
)

var ErrInvalidEnvTextureList = errors.New("clientgo748: invalid EnvTextureList3")

// EnvTextureList mirrors the 512 native texture slots loaded by FUN_004b9ce5.
// Only the zero-terminated filename at the start of each 0x108-byte record is
// promoted; unknown record bytes keep no invented semantics in the Go client.
type EnvTextureList struct {
	paths [envTextureRecordCount]string
}

func LoadEnvTextureListFile(path string) (EnvTextureList, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return EnvTextureList{}, fmt.Errorf("clientgo748: read EnvTextureList3 %q: %w", path, err)
	}
	list, err := ParseEnvTextureList(data)
	if err != nil {
		return EnvTextureList{}, fmt.Errorf("clientgo748: parse EnvTextureList3 %q: %w", path, err)
	}
	return list, nil
}

func ParseEnvTextureList(data []byte) (EnvTextureList, error) {
	if len(data) != envTextureRecordCount*envTextureRecordSize {
		return EnvTextureList{}, fmt.Errorf("%w: size %d, want %d", ErrInvalidEnvTextureList, len(data), envTextureRecordCount*envTextureRecordSize)
	}
	var list EnvTextureList
	for slot := 0; slot < envTextureRecordCount; slot++ {
		record := data[slot*envTextureRecordSize : (slot+1)*envTextureRecordSize]
		end := 0
		for end < envTextureNameLimit && record[end] != 0 {
			end++
		}
		if end == envTextureNameLimit {
			return EnvTextureList{}, fmt.Errorf("%w: slot %d has no terminated filename", ErrInvalidEnvTextureList, slot)
		}
		list.paths[slot] = string(record[:end])
	}
	return list, nil
}

func (l EnvTextureList) Path(slot uint16) (string, bool) {
	if int(slot) >= len(l.paths) || l.paths[slot] == "" {
		return "", false
	}
	return l.paths[slot], true
}
