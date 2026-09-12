package assets

import (
	"bufio"
	"errors"
	"fmt"
	"os"
	"strconv"
	"strings"
)

var ErrInvalidMeshList = errors.New("clientgo748: invalid MeshList")

// MeshList is the sparse numeric lookup populated from MeshList.txt. Static
// object records use dwObjType directly as this index in the native client.
type MeshList struct {
	entries map[uint32]string
}

func LoadMeshListFile(path string) (MeshList, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return MeshList{}, fmt.Errorf("clientgo748: read MeshList %q: %w", path, err)
	}
	list, err := ParseMeshList(data)
	if err != nil {
		return MeshList{}, fmt.Errorf("clientgo748: parse MeshList %q: %w", path, err)
	}
	return list, nil
}

func ParseMeshList(data []byte) (MeshList, error) {
	entries := make(map[uint32]string)
	scanner := bufio.NewScanner(strings.NewReader(string(data)))
	for lineNo := 1; scanner.Scan(); lineNo++ {
		line := strings.TrimSpace(scanner.Text())
		if line == "" {
			continue
		}

		fields := strings.Fields(line)
		if len(fields) != 2 {
			return MeshList{}, fmt.Errorf("%w: line %d has %d fields", ErrInvalidMeshList, lineNo, len(fields))
		}
		index64, err := strconv.ParseUint(fields[0], 10, 32)
		if err != nil {
			return MeshList{}, fmt.Errorf("%w: line %d index %q: %v", ErrInvalidMeshList, lineNo, fields[0], err)
		}
		index := uint32(index64)
		// The checked-in 7.48 list preserves doubled Windows separators from
		// its source representation. The native fopen path treats them as the
		// same relative separator, so expose a canonical single-separator path.
		name := fields[1]
		for strings.Contains(name, `\\`) {
			name = strings.ReplaceAll(name, `\\`, `\`)
		}
		// MeshManager::InitMeshManager writes directly to m_stCommonMesh[index]
		// for every scanned row. Preserve that contract: if an index appears
		// more than once, the final row replaces the previous filename.
		entries[index] = name
	}
	if err := scanner.Err(); err != nil {
		return MeshList{}, fmt.Errorf("%w: scan: %v", ErrInvalidMeshList, err)
	}
	if len(entries) == 0 {
		return MeshList{}, fmt.Errorf("%w: no entries", ErrInvalidMeshList)
	}
	return MeshList{entries: entries}, nil
}

func (l MeshList) Resolve(index uint32) (string, bool) {
	if l.entries == nil {
		return "", false
	}
	name, ok := l.entries[index]
	return name, ok
}

func (l MeshList) Len() int {
	return len(l.entries)
}
