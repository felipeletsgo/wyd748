package assets

import (
	"encoding/binary"
	"os"
	"path/filepath"
	"testing"
)

func TestParseMSHOfficialAsset(t *testing.T) {
	p := filepath.Join("..", "..", "assets", "current", "mesh", "ag010101.msh")
	b, err := os.ReadFile(p)
	if err != nil {
		t.Fatal(err)
	}
	m, err := ParseMSH(b)
	if err != nil {
		t.Fatal(err)
	}
	if m.ID != 1 || m.VertexStride != 44 || m.VertexCount != 722 || m.FaceIndexCount != 2328 {
		t.Fatalf("unexpected header: %+v", m)
	}
	if len(m.Palette) != 10 || len(m.BoneNames) != 10 || len(m.Vertices) != int(44*722) || len(m.Indices) != 2328 {
		t.Fatalf("unexpected sections: palette=%d names=%d vertices=%d indices=%d", len(m.Palette), len(m.BoneNames), len(m.Vertices), len(m.Indices))
	}
}

func TestParseMSHRejectsMalformedInput(t *testing.T) {
	truncated := make([]byte, 32)
	binary.LittleEndian.PutUint32(truncated[12:], 12)
	binary.LittleEndian.PutUint32(truncated[24:], 1)
	if _, err := ParseMSH(truncated); err == nil {
		t.Fatal("expected truncated mesh")
	}
	badStride := make([]byte, 32)
	binary.LittleEndian.PutUint32(badStride[12:], 8)
	if _, err := ParseMSH(badStride); err == nil {
		t.Fatal("expected invalid vertex stride")
	}
	if _, err := ParseMSH([]byte("bad")); err != ErrTruncatedMSH {
		t.Fatalf("unexpected short error: %v", err)
	}
}
