package assets

import (
	"encoding/binary"
	"path/filepath"
	"runtime"
	"testing"
)

func TestParseWYT24BitBottomLeft(t *testing.T) {
	// File order is bottom row first for a descriptor with bottom-left origin.
	data := testWYT(2, 2, 24, 0x00, []byte{
		0xff, 0x00, 0x00, // blue
		0xff, 0xff, 0xff, // white
		0x00, 0x00, 0xff, // red
		0x00, 0xff, 0x00, // green
	})
	texture, err := ParseWYT(data)
	if err != nil {
		t.Fatalf("ParseWYT() error = %v", err)
	}
	want := []byte{
		0xff, 0x00, 0x00, 0xff, 0x00, 0xff, 0x00, 0xff,
		0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
	}
	assertTexture(t, texture, 2, 2, 24, want)
}

func TestParseWYT32BitTopRight(t *testing.T) {
	// The first pixel in each row is the rightmost pixel when bit 4 is set.
	data := testWYT(2, 1, 32, 0x38, []byte{
		0x00, 0x00, 0xff, 0x40, // red, alpha 0x40 (rightmost)
		0x00, 0xff, 0x00, 0x80, // green, alpha 0x80 (leftmost)
	})
	texture, err := ParseWYT(data)
	if err != nil {
		t.Fatalf("ParseWYT() error = %v", err)
	}
	want := []byte{
		0x00, 0xff, 0x00, 0x80,
		0xff, 0x00, 0x00, 0x40,
	}
	assertTexture(t, texture, 2, 1, 32, want)
}

func TestParseWYT16Bit(t *testing.T) {
	data := testWYT(1, 1, 16, 0x01, []byte{0x00, 0xfc}) // opaque 5-bit red
	texture, err := ParseWYT(data)
	if err != nil {
		t.Fatalf("ParseWYT() error = %v", err)
	}
	assertTexture(t, texture, 1, 1, 16, []byte{0xff, 0x00, 0x00, 0xff})
}

func TestLoadOfficialLogo1(t *testing.T) {
	_, filename, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("runtime.Caller failed")
	}
	path := filepath.Join(filepath.Dir(filename), "..", "..", "CLIENT OFICIAL 7.48", "UI", "logo1.wyt")
	texture, err := LoadWYTFile(path)
	if err != nil {
		t.Fatalf("LoadWYTFile() error = %v", err)
	}
	if texture.Width != 256 || texture.Height != 256 || texture.SourceBits != 32 {
		t.Fatalf("official logo metadata = %dx%d %dbpp", texture.Width, texture.Height, texture.SourceBits)
	}
	if len(texture.Pixels) != 256*256*4 {
		t.Fatalf("official logo pixel length = %d", len(texture.Pixels))
	}
}

func TestParseWYTRejectsMalformedInput(t *testing.T) {
	valid := testWYT(1, 1, 24, 0, []byte{0, 0, 0})
	tests := []struct {
		name string
		data []byte
		want error
	}{
		{name: "short", data: []byte("WT10"), want: ErrTruncatedWYT},
		{name: "signature", data: append([]byte("NOPE"), valid[4:]...), want: ErrInvalidWYT},
		{name: "truncated payload", data: valid[:len(valid)-9], want: ErrTruncatedWYT},
		{name: "invalid footer", data: append(valid[:len(valid)-8], []byte{1, 0, 0, 0, 0, 0, 0, 0}...), want: ErrInvalidWYT},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			_, err := ParseWYT(tc.data)
			if !errorsIs(err, tc.want) {
				t.Fatalf("ParseWYT() error = %v, want %v", err, tc.want)
			}
		})
	}
}

func TestTextureCropCopiesRegion(t *testing.T) {
	texture := Texture{Width: 3, Height: 2, SourceBits: 32, Pixels: make([]byte, 3*2*4)}
	for i := range texture.Pixels {
		texture.Pixels[i] = byte(i)
	}
	cropped, err := texture.Crop(1, 0, 2, 2)
	if err != nil {
		t.Fatalf("Crop() error = %v", err)
	}
	if cropped.Width != 2 || cropped.Height != 2 {
		t.Fatalf("Crop() dimensions = %dx%d, want 2x2", cropped.Width, cropped.Height)
	}
	want := append([]byte{}, texture.Pixels[4:12]...)
	want = append(want, texture.Pixels[16:24]...)
	if string(cropped.Pixels) != string(want) {
		t.Fatalf("Crop() pixels = %v, want %v", cropped.Pixels, want)
	}
}

func testWYT(width, height uint16, bits, descriptor uint8, payload []byte) []byte {
	data := make([]byte, wytPrefixSize+tgaHeaderSize+len(payload)+wytTrailerSize)
	copy(data, []byte("WT10"))
	header := data[wytPrefixSize:]
	header[2] = 2
	binary.LittleEndian.PutUint16(header[12:14], width)
	binary.LittleEndian.PutUint16(header[14:16], height)
	header[16] = bits
	header[17] = descriptor
	copy(data[wytPrefixSize+tgaHeaderSize:], payload)
	return data
}

func assertTexture(t *testing.T, texture Texture, width, height uint16, bits uint8, want []byte) {
	t.Helper()
	if texture.Width != width || texture.Height != height || texture.SourceBits != bits {
		t.Fatalf("metadata = %dx%d %dbpp", texture.Width, texture.Height, texture.SourceBits)
	}
	if string(texture.Pixels) != string(want) {
		t.Fatalf("pixels = %x, want %x", texture.Pixels, want)
	}
}

// errorsIs keeps this package testable on the minimum supported Go toolchain
// without exposing implementation details of wrapped parser errors.
func errorsIs(got, want error) bool {
	for got != nil {
		if got == want {
			return true
		}
		type unwrapper interface{ Unwrap() error }
		u, ok := got.(unwrapper)
		if !ok {
			return false
		}
		got = u.Unwrap()
	}
	return false
}
