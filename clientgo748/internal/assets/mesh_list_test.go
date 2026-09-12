package assets

import (
	"errors"
	"path/filepath"
	"testing"
)

func TestLoadOfficialMeshListResolvesStaticObjectMeshes(t *testing.T) {
	path := filepath.Join("..", "..", "assets", "current", "mesh", "MeshList.txt")
	list, err := LoadMeshListFile(path)
	if err != nil {
		t.Fatalf("LoadMeshListFile(%q): %v", path, err)
	}

	tests := map[uint32]string{
		1570: `mesh\kswa08.msa`,
		1571: `mesh\kswa09.msa`,
		1572: `mesh\kswa10.msa`,
		1601: `mesh\ksob12.msa`,
		1697: `mesh\ksoc09.msa`,
		1698: `mesh\ksoc10.msa`,
	}
	for index, want := range tests {
		got, ok := list.Resolve(index)
		if !ok {
			t.Fatalf("Resolve(%d): missing", index)
		}
		if got != want {
			t.Fatalf("Resolve(%d) = %q, want %q", index, got, want)
		}
	}
}

func TestParseMeshListRejectsMalformedRows(t *testing.T) {
	for name, data := range map[string][]byte{
		"malformed": []byte("1570\n"),
		"bad-index": []byte("x mesh\\kswa08.msa\n"),
		"empty":     nil,
	} {
		t.Run(name, func(t *testing.T) {
			_, err := ParseMeshList(data)
			if !errors.Is(err, ErrInvalidMeshList) {
				t.Fatalf("ParseMeshList() error = %v, want %v", err, ErrInvalidMeshList)
			}
		})
	}
}

func TestParseMeshListDuplicateIndexUsesFinalRow(t *testing.T) {
	list, err := ParseMeshList([]byte("1710 mesh\\first.msa\n1710 mesh\\second.msa\n"))
	if err != nil {
		t.Fatalf("ParseMeshList(): %v", err)
	}
	got, ok := list.Resolve(1710)
	if !ok || got != `mesh\second.msa` {
		t.Fatalf("Resolve(1710) = %q, %v, want final row", got, ok)
	}
}
