package assets

import (
	"encoding/binary"
	"errors"
	"path/filepath"
	"testing"
)

func TestParseOfficialKSWA08MSA(t *testing.T) {
	path := filepath.Join("..", "..", "assets", "current", "mesh", "kswa08.msa")
	mesh, err := LoadMSAFile(path)
	if err != nil {
		t.Fatal(err)
	}
	if mesh.SourceFVF != 0x112 || mesh.FVF != 0x212 || mesh.SourceStride != 32 || mesh.VertexStride != 40 {
		t.Fatalf("unexpected FVF/stride: source=%#x/%d runtime=%#x/%d", mesh.SourceFVF, mesh.SourceStride, mesh.FVF, mesh.VertexStride)
	}
	if mesh.VertexCount != 1536 || len(mesh.Indices) != 1938 || len(mesh.Attributes) != 1 || len(mesh.TextureNames) != 1 {
		t.Fatalf("unexpected MSA sections: vertices=%d indices=%d attrs=%d textures=%d", mesh.VertexCount, len(mesh.Indices), len(mesh.Attributes), len(mesh.TextureNames))
	}
	if mesh.TextureNames[0] != "kswa01.tga" {
		t.Fatalf("texture = %q", mesh.TextureNames[0])
	}
	want := MSAAttributeRange{AttribID: 0, FaceStart: 0, FaceCount: 646, VertexStart: 0, VertexCount: 1536}
	if mesh.Attributes[0] != want {
		t.Fatalf("attribute = %+v, want %+v", mesh.Attributes[0], want)
	}
	if len(mesh.Vertices) != int(mesh.VertexCount*mesh.VertexStride) {
		t.Fatalf("expanded vertex bytes = %d", len(mesh.Vertices))
	}
	if string(mesh.Vertices[24:32]) != string(mesh.Vertices[32:40]) {
		t.Fatal("native 0x112 -> 0x212 UV duplication was not applied")
	}
}

func TestParseMSARejectsMalformedInput(t *testing.T) {
	if _, err := ParseMSA([]byte("bad")); !errors.Is(err, ErrTruncatedMSA) {
		t.Fatalf("unexpected short error: %v", err)
	}

	badAttributes := make([]byte, msaHeaderSize)
	binary.LittleEndian.PutUint32(badAttributes[4:], 32)
	binary.LittleEndian.PutUint32(badAttributes[8:], maxMSAAttributeCount+1)
	if _, err := ParseMSA(badAttributes); !errors.Is(err, ErrMSATooLarge) {
		t.Fatalf("unexpected attribute-count error: %v", err)
	}

	oddIndex := make([]byte, msaHeaderSize+4+3+4)
	binary.LittleEndian.PutUint32(oddIndex[0:], msaNativePassThroughFVF)
	binary.LittleEndian.PutUint32(oddIndex[4:], 12)
	binary.LittleEndian.PutUint32(oddIndex[msaHeaderSize:], 3)
	if _, err := ParseMSA(oddIndex); !errors.Is(err, ErrInvalidMSA) {
		t.Fatalf("unexpected odd-index error: %v", err)
	}
}
