package assets

import (
	"encoding/binary"
	"errors"
	"fmt"
	"os"
)

const wysDDSHeaderSize = 128

var (
	ErrInvalidWYS     = errors.New("clientgo748: invalid WYS texture")
	ErrUnsupportedWYS = errors.New("clientgo748: unsupported WYS texture")
	ErrTruncatedWYS   = errors.New("clientgo748: truncated WYS texture")
)

// LoadWYSFile loads one native 7.48 environment texture without modifying the
// official source asset. The returned texture is expanded to RGBA8.
func LoadWYSFile(path string) (Texture, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return Texture{}, fmt.Errorf("clientgo748: read WYS %q: %w", path, err)
	}
	texture, err := ParseWYS(data)
	if err != nil {
		return Texture{}, fmt.Errorf("clientgo748: parse WYS %q: %w", path, err)
	}
	return texture, nil
}

// ParseWYS decodes the native 7.48 WS10 wrapper used by Env textures.
// FUN_004b9d99 skips the first file byte, restores the first three bytes of the
// resulting buffer to "DDS", then restores the FourCC at offset 0x54. The
// official Env corpus currently materialized by clientgo748 uses discriminator
// '2', which the native client restores as DXT1. DXT3 remains intentionally
// unsupported until a real 7.48 asset requiring it enters the active path.
func ParseWYS(data []byte) (Texture, error) {
	if len(data) < 1+wysDDSHeaderSize {
		return Texture{}, ErrTruncatedWYS
	}
	if string(data[:4]) != "WS10" || data[4] != ' ' {
		return Texture{}, fmt.Errorf("%w: missing WS10/DDS wrapper signature", ErrInvalidWYS)
	}

	dds := data[1:]
	if binary.LittleEndian.Uint32(dds[4:8]) != 124 || binary.LittleEndian.Uint32(dds[76:80]) != 32 {
		return Texture{}, fmt.Errorf("%w: invalid DDS header sizes", ErrInvalidWYS)
	}
	if binary.LittleEndian.Uint32(dds[80:84])&0x4 == 0 {
		return Texture{}, fmt.Errorf("%w: DDS pixel format is not FourCC-compressed", ErrInvalidWYS)
	}
	if dds[0x54] != '2' {
		return Texture{}, fmt.Errorf("%w: WYS discriminator %#02x", ErrUnsupportedWYS, dds[0x54])
	}

	height32 := binary.LittleEndian.Uint32(dds[12:16])
	width32 := binary.LittleEndian.Uint32(dds[16:20])
	if width32 == 0 || height32 == 0 || width32 > uint32(MaxTextureDimension) || height32 > uint32(MaxTextureDimension) {
		return Texture{}, fmt.Errorf("%w: invalid dimensions %dx%d", ErrInvalidWYS, width32, height32)
	}
	if uint64(width32)*uint64(height32) > MaxTexturePixels {
		return Texture{}, ErrTextureTooLarge
	}

	blocksWide := (width32 + 3) / 4
	blocksHigh := (height32 + 3) / 4
	payloadSize := uint64(blocksWide) * uint64(blocksHigh) * 8
	payloadStart := uint64(1 + wysDDSHeaderSize)
	if payloadStart+payloadSize > uint64(len(data)) {
		return Texture{}, ErrTruncatedWYS
	}

	pixels := make([]byte, int(uint64(width32)*uint64(height32)*4))
	payload := data[payloadStart : payloadStart+payloadSize]
	for blockY := uint32(0); blockY < blocksHigh; blockY++ {
		for blockX := uint32(0); blockX < blocksWide; blockX++ {
			off := (blockY*blocksWide + blockX) * 8
			decodeDXT1Block(pixels, width32, height32, blockX, blockY, payload[off:off+8])
		}
	}

	return Texture{Width: uint16(width32), Height: uint16(height32), SourceBits: 32, Pixels: pixels}, nil
}

func decodeDXT1Block(dst []byte, width, height, blockX, blockY uint32, block []byte) {
	c0 := binary.LittleEndian.Uint16(block[0:2])
	c1 := binary.LittleEndian.Uint16(block[2:4])
	colors := [4][4]uint8{dxtColor565(c0), dxtColor565(c1)}
	if c0 > c1 {
		colors[2] = dxtMix(colors[0], colors[1], 2, 1, 3)
		colors[3] = dxtMix(colors[0], colors[1], 1, 2, 3)
	} else {
		colors[2] = dxtMix(colors[0], colors[1], 1, 1, 2)
		colors[3] = [4]uint8{0, 0, 0, 0}
	}

	indices := binary.LittleEndian.Uint32(block[4:8])
	for py := uint32(0); py < 4; py++ {
		y := blockY*4 + py
		if y >= height {
			continue
		}
		for px := uint32(0); px < 4; px++ {
			x := blockX*4 + px
			if x >= width {
				continue
			}
			selector := (indices >> (2 * (py*4 + px))) & 0x3
			out := (y*width + x) * 4
			copy(dst[out:out+4], colors[selector][:])
		}
	}
}

func dxtColor565(value uint16) [4]uint8 {
	return [4]uint8{
		expand5(uint8((value >> 11) & 0x1f)),
		expand6(uint8((value >> 5) & 0x3f)),
		expand5(uint8(value & 0x1f)),
		0xff,
	}
}

func dxtMix(a, b [4]uint8, wa, wb, divisor uint16) [4]uint8 {
	return [4]uint8{
		uint8((uint16(a[0])*wa + uint16(b[0])*wb) / divisor),
		uint8((uint16(a[1])*wa + uint16(b[1])*wb) / divisor),
		uint8((uint16(a[2])*wa + uint16(b[2])*wb) / divisor),
		0xff,
	}
}

func expand6(value uint8) uint8 { return (value << 2) | (value >> 4) }
