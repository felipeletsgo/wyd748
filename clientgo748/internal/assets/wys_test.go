package assets

import (
	"encoding/binary"
	"errors"
	"path/filepath"
	"runtime"
	"testing"
)

func TestLoadOfficialTile01010WYS(t *testing.T) {
	_, filename, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("runtime.Caller failed")
	}
	path := filepath.Join(filepath.Dir(filename), "..", "..", "assets", "current", "Env", "Tile01010.wys")
	texture, err := LoadWYSFile(path)
	if err != nil {
		t.Fatalf("LoadWYSFile() error = %v", err)
	}
	if texture.Width != 128 || texture.Height != 128 || texture.SourceBits != 32 {
		t.Fatalf("official WYS metadata = %dx%d %dbpp", texture.Width, texture.Height, texture.SourceBits)
	}
	if len(texture.Pixels) != 128*128*4 {
		t.Fatalf("official WYS pixel length = %d", len(texture.Pixels))
	}
	allSame := true
	for i := 4; i < len(texture.Pixels); i += 4 {
		if string(texture.Pixels[i:i+4]) != string(texture.Pixels[:4]) {
			allSame = false
			break
		}
	}
	if allSame {
		t.Fatal("official WYS unexpectedly decoded to a single flat color")
	}
}

func TestParseWYSDXT1Block(t *testing.T) {
	// DXT1 endpoints are red and green. Every selector in the first row is
	// exercised so endpoint and interpolation ordering remain regression-safe.
	block := make([]byte, 8)
	binary.LittleEndian.PutUint16(block[0:2], 0xf800)
	binary.LittleEndian.PutUint16(block[2:4], 0x07e0)
	binary.LittleEndian.PutUint32(block[4:8], 0xe4)
	texture, err := ParseWYS(testWYS(4, 4, block))
	if err != nil {
		t.Fatalf("ParseWYS() error = %v", err)
	}
	want := []byte{
		0xff, 0x00, 0x00, 0xff,
		0x00, 0xff, 0x00, 0xff,
		0xaa, 0x55, 0x00, 0xff,
		0x55, 0xaa, 0x00, 0xff,
	}
	if string(texture.Pixels[:16]) != string(want) {
		t.Fatalf("first decoded row = %x, want %x", texture.Pixels[:16], want)
	}
}

func TestParseWYSRejectsMalformedInput(t *testing.T) {
	valid := testWYS(4, 4, make([]byte, 8))
	tests := []struct {
		name string
		data []byte
		want error
	}{
		{name: "short", data: []byte("WS10"), want: ErrTruncatedWYS},
		{name: "signature", data: append([]byte("NOPE"), valid[4:]...), want: ErrInvalidWYS},
		{name: "truncated payload", data: valid[:len(valid)-1], want: ErrTruncatedWYS},
		{name: "unsupported discriminator", data: func() []byte { d := append([]byte(nil), valid...); d[85] = '3'; return d }(), want: ErrUnsupportedWYS},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			_, err := ParseWYS(tc.data)
			if !errors.Is(err, tc.want) {
				t.Fatalf("ParseWYS() error = %v, want %v", err, tc.want)
			}
		})
	}
}

func testWYS(width, height uint32, payload []byte) []byte {
	data := make([]byte, 1+wysDDSHeaderSize+len(payload))
	copy(data[:5], []byte("WS10 "))
	dds := data[1:]
	binary.LittleEndian.PutUint32(dds[4:8], 124)
	binary.LittleEndian.PutUint32(dds[12:16], height)
	binary.LittleEndian.PutUint32(dds[16:20], width)
	binary.LittleEndian.PutUint32(dds[76:80], 32)
	binary.LittleEndian.PutUint32(dds[80:84], 4)
	dds[0x54] = '2'
	copy(data[1+wysDDSHeaderSize:], payload)
	return data
}
