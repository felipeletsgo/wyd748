package assets

import (
	"bytes"
	"crypto/sha256"
	"encoding/binary"
	"errors"
	"fmt"
)

// O formato canônico mantém o renderer independente do cabeçalho histórico.
// O manifesto e o pacote protegido serão adicionados numa unidade posterior.
const (
	CanonicalMagic      = "WYDASSET"
	CanonicalHeaderSize = 64
)

var (
	ErrInvalidCanonical     = errors.New("clientgo748: invalid canonical asset")
	ErrUnsupportedCanonical = errors.New("clientgo748: unsupported canonical asset")
	ErrCanonicalIntegrity   = errors.New("clientgo748: canonical asset integrity failure")
)

// EncodeTexture serializa uma textura validada em formato determinístico.
// O resultado é uma cópia independente e não contém ponteiros ou handles
// gráficos. O hash cobre somente os pixels RGBA8.
func EncodeTexture(texture Texture) ([]byte, error) {
	if err := validateTexture(texture); err != nil {
		return nil, err
	}
	payloadSize := uint64(len(texture.Pixels))
	if payloadSize > uint64(^uint64(0)>>1) {
		return nil, ErrTextureTooLarge
	}
	encoded := make([]byte, CanonicalHeaderSize+len(texture.Pixels))
	copy(encoded[:8], CanonicalMagic)
	binary.LittleEndian.PutUint16(encoded[8:10], FormatVersion)
	encoded[10] = byte(KindTexture)
	binary.LittleEndian.PutUint32(encoded[12:16], uint32(texture.Width))
	binary.LittleEndian.PutUint32(encoded[16:20], uint32(texture.Height))
	encoded[20] = texture.SourceBits
	binary.LittleEndian.PutUint64(encoded[24:32], payloadSize)
	digest := sha256.Sum256(texture.Pixels)
	copy(encoded[32:64], digest[:])
	copy(encoded[CanonicalHeaderSize:], texture.Pixels)
	return encoded, nil
}

// DecodeTexture valida e decodifica uma textura canônica. O retorno possui um
// buffer novo, portanto o chamador pode liberar o pacote de entrada depois.
func DecodeTexture(data []byte) (Texture, error) {
	if len(data) < CanonicalHeaderSize {
		return Texture{}, ErrTruncatedWYT
	}
	if string(data[:8]) != CanonicalMagic {
		return Texture{}, fmt.Errorf("%w: magic", ErrInvalidCanonical)
	}
	if binary.LittleEndian.Uint16(data[8:10]) != FormatVersion {
		return Texture{}, fmt.Errorf("%w: version %d", ErrUnsupportedCanonical, binary.LittleEndian.Uint16(data[8:10]))
	}
	if AssetKind(data[10]) != KindTexture {
		return Texture{}, fmt.Errorf("%w: kind %d", ErrUnsupportedCanonical, data[10])
	}
	if !bytes.Equal(data[11:12], []byte{0}) || !bytes.Equal(data[21:24], []byte{0, 0, 0}) {
		return Texture{}, fmt.Errorf("%w: reserved bytes", ErrInvalidCanonical)
	}
	width := binary.LittleEndian.Uint32(data[12:16])
	height := binary.LittleEndian.Uint32(data[16:20])
	if width > uint32(MaxTextureDimension) || height > uint32(MaxTextureDimension) || width == 0 || height == 0 {
		return Texture{}, ErrTextureTooLarge
	}
	sourceBits := data[20]
	if sourceBits != 16 && sourceBits != 24 && sourceBits != 32 {
		return Texture{}, fmt.Errorf("%w: source bits %d", ErrInvalidCanonical, sourceBits)
	}
	pixels := uint64(width) * uint64(height)
	if pixels > MaxTexturePixels || pixels*4 > uint64(len(data)-CanonicalHeaderSize) {
		return Texture{}, ErrTruncatedWYT
	}
	payloadSize := binary.LittleEndian.Uint64(data[24:32])
	if payloadSize != pixels*4 || payloadSize != uint64(len(data)-CanonicalHeaderSize) {
		return Texture{}, fmt.Errorf("%w: payload size", ErrInvalidCanonical)
	}
	payload := data[CanonicalHeaderSize:]
	digest := sha256.Sum256(payload)
	if !bytes.Equal(digest[:], data[32:64]) {
		return Texture{}, ErrCanonicalIntegrity
	}
	out := append([]byte(nil), payload...)
	return Texture{
		Width:      uint16(width),
		Height:     uint16(height),
		SourceBits: sourceBits,
		Pixels:     out,
	}, nil
}

func validateTexture(texture Texture) error {
	if texture.Width == 0 || texture.Height == 0 ||
		texture.Width > MaxTextureDimension || texture.Height > MaxTextureDimension {
		return ErrTextureTooLarge
	}
	pixels := uint64(texture.Width) * uint64(texture.Height)
	if pixels > MaxTexturePixels || uint64(len(texture.Pixels)) != pixels*4 {
		return fmt.Errorf("%w: expected %d RGBA8 bytes, got %d", ErrInvalidCanonical, pixels*4, len(texture.Pixels))
	}
	if texture.SourceBits != 16 && texture.SourceBits != 24 && texture.SourceBits != 32 {
		return fmt.Errorf("%w: source bits %d", ErrInvalidCanonical, texture.SourceBits)
	}
	return nil
}
