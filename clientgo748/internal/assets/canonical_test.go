package assets

import (
	"bytes"
	"testing"
)

func TestCanonicalTextureRoundTrip(t *testing.T) {
	want := Texture{Width: 2, Height: 1, SourceBits: 24, Pixels: []byte{1, 2, 3, 4, 5, 6, 7, 8}}
	encoded, err := EncodeTexture(want)
	if err != nil {
		t.Fatalf("EncodeTexture() error = %v", err)
	}
	got, err := DecodeTexture(encoded)
	if err != nil {
		t.Fatalf("DecodeTexture() error = %v", err)
	}
	if got.Width != want.Width || got.Height != want.Height || got.SourceBits != want.SourceBits || !bytes.Equal(got.Pixels, want.Pixels) {
		t.Fatalf("round trip = %#v, want %#v", got, want)
	}
	got.Pixels[0] = 99
	if encoded[CanonicalHeaderSize] == 99 {
		t.Fatal("DecodeTexture returned aliased payload")
	}
}

func TestDecodeTextureRejectsTampering(t *testing.T) {
	encoded, err := EncodeTexture(Texture{Width: 1, Height: 1, SourceBits: 32, Pixels: []byte{1, 2, 3, 4}})
	if err != nil {
		t.Fatal(err)
	}
	encoded[CanonicalHeaderSize] ^= 0xff
	if _, err := DecodeTexture(encoded); !errorsIs(err, ErrCanonicalIntegrity) {
		t.Fatalf("DecodeTexture() error = %v, want integrity error", err)
	}
}

func TestDecodeTextureRejectsTrailingBytes(t *testing.T) {
	encoded, err := EncodeTexture(Texture{Width: 1, Height: 1, SourceBits: 32, Pixels: []byte{1, 2, 3, 4}})
	if err != nil {
		t.Fatal(err)
	}
	encoded = append(encoded, 0)
	if _, err := DecodeTexture(encoded); !errorsIs(err, ErrInvalidCanonical) {
		t.Fatalf("DecodeTexture() error = %v, want invalid asset", err)
	}
}

func TestDecodeTextureRejectsUnsupportedSourceBits(t *testing.T) {
	encoded, err := EncodeTexture(Texture{Width: 1, Height: 1, SourceBits: 32, Pixels: []byte{1, 2, 3, 4}})
	if err != nil {
		t.Fatal(err)
	}
	encoded[20] = 8
	if _, err := DecodeTexture(encoded); !errorsIs(err, ErrInvalidCanonical) {
		t.Fatalf("DecodeTexture() error = %v, want invalid asset", err)
	}
}
