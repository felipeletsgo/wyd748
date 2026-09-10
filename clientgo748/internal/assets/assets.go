// Package assets define os tipos canônicos que serão usados pelo client Go.
//
// Os loaders preservam a fonte original e devolvem representações próprias do
// client Go. Nenhum conversor sobrescreve os assets oficiais.
package assets

import (
	"encoding/binary"
	"errors"
	"fmt"
	"os"
)

// FormatVersion identifica a versão do formato normalizado do client Go.
// Alterações incompatíveis devem criar uma nova versão e invalidar o cache
// derivado correspondente.
const FormatVersion uint16 = 1

// AssetKind identifica a família lógica de um asset convertido.
type AssetKind uint8

const (
	KindUnknown AssetKind = iota
	KindTexture
	KindMesh
	KindAnimation
	KindMap
	KindUI
	KindData
)

// AssetRef descreve um asset no manifesto sem assumir como ele será carregado
// pela API gráfica. Hash e tamanho são metadados de integridade; não são uma
// implementação de autenticação por si só.
type AssetRef struct {
	ID       string
	Kind     AssetKind
	Path     string
	Size     uint64
	SHA256   [32]byte
	Revision uint32
}

// Texture is the backend-independent representation used by the renderer.
// Pixels are tightly packed RGBA8 pixels in top-left origin order.
type Texture struct {
	Width      uint16
	Height     uint16
	SourceBits uint8
	Pixels     []byte
}

// Crop returns an independent texture containing the requested pixel region.
// UI WYT files can be atlases; callers must select the region described by the
// official UITextureSetList before presenting it as a standalone panel.
func (t Texture) Crop(x, y, width, height uint16) (Texture, error) {
	if width == 0 || height == 0 || x >= t.Width || y >= t.Height ||
		x+width > t.Width || y+height > t.Height {
		return Texture{}, fmt.Errorf("clientgo748: texture crop %dx%d+%d+%d outside %dx%d", width, height, x, y, t.Width, t.Height)
	}
	pixels := make([]byte, int(width)*int(height)*4)
	for row := uint16(0); row < height; row++ {
		src := (int(y)+int(row))*int(t.Width)*4 + int(x)*4
		dst := int(row) * int(width) * 4
		copy(pixels[dst:dst+int(width)*4], t.Pixels[src:src+int(width)*4])
	}
	return Texture{Width: width, Height: height, SourceBits: t.SourceBits, Pixels: pixels}, nil
}

const (
	wytPrefixSize  = 4
	tgaHeaderSize  = 18
	wytTrailerSize = 8

	// These bounds protect the parser from allocating attacker-controlled
	// amounts of memory while remaining above every official 7.48 texture.
	MaxTextureDimension uint16 = 4096
	MaxTexturePixels    uint64 = 16 * 1024 * 1024
)

var (
	ErrInvalidWYT      = errors.New("clientgo748: invalid WYT texture")
	ErrUnsupportedWYT  = errors.New("clientgo748: unsupported WYT texture")
	ErrTruncatedWYT    = errors.New("clientgo748: truncated WYT texture")
	ErrTextureTooLarge = errors.New("clientgo748: WYT texture exceeds safety limits")
)

// LoadWYTFile loads one official WT10 texture without modifying the source
// file. The returned pixel buffer is owned by the caller.
func LoadWYTFile(path string) (Texture, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return Texture{}, fmt.Errorf("clientgo748: read WYT %q: %w", path, err)
	}
	texture, err := ParseWYT(data)
	if err != nil {
		return Texture{}, fmt.Errorf("clientgo748: parse WYT %q: %w", path, err)
	}
	return texture, nil
}

// ParseWYT decodes the native 7.48 WT10 wrapper around an uncompressed TGA
// true-color image. Native files carry the eight-byte TGA footer offsets but
// omit the final TRUEVISION-XFILE signature; the native loader supplies that
// signature before handing the image to its graphics API. The parser does not
// need to synthesize the footer because it reads the image payload directly.
func ParseWYT(data []byte) (Texture, error) {
	if len(data) < wytPrefixSize+tgaHeaderSize {
		return Texture{}, ErrTruncatedWYT
	}
	if string(data[:wytPrefixSize]) != "WT10" {
		return Texture{}, fmt.Errorf("%w: missing WT10 signature", ErrInvalidWYT)
	}
	header := data[wytPrefixSize : wytPrefixSize+tgaHeaderSize]
	if header[0] != 0 || header[1] != 0 {
		return Texture{}, fmt.Errorf("%w: color-mapped or identified images are not supported", ErrUnsupportedWYT)
	}
	if header[2] != 2 {
		return Texture{}, fmt.Errorf("%w: TGA image type %d", ErrUnsupportedWYT, header[2])
	}

	width := binary.LittleEndian.Uint16(header[12:14])
	height := binary.LittleEndian.Uint16(header[14:16])
	bits := header[16]
	descriptor := header[17]
	if width == 0 || height == 0 {
		return Texture{}, fmt.Errorf("%w: zero texture dimension", ErrInvalidWYT)
	}
	if width > MaxTextureDimension || height > MaxTextureDimension {
		return Texture{}, ErrTextureTooLarge
	}
	pixels := uint64(width) * uint64(height)
	if pixels > MaxTexturePixels {
		return Texture{}, ErrTextureTooLarge
	}
	bytesPerPixel, ok := wytBytesPerPixel(bits)
	if !ok {
		return Texture{}, fmt.Errorf("%w: %d bits per pixel", ErrUnsupportedWYT, bits)
	}
	payloadSize := pixels * uint64(bytesPerPixel)
	payloadStart := uint64(wytPrefixSize + tgaHeaderSize)
	payloadEnd := payloadStart + payloadSize
	if payloadEnd > uint64(len(data)) {
		return Texture{}, ErrTruncatedWYT
	}
	trailer := data[payloadEnd:]
	if len(trailer) != 0 && len(trailer) != wytTrailerSize {
		return Texture{}, fmt.Errorf("%w: unexpected trailing data", ErrInvalidWYT)
	}
	for _, b := range trailer {
		if b != 0 {
			return Texture{}, fmt.Errorf("%w: non-zero WYT footer offsets", ErrInvalidWYT)
		}
	}

	outSize := pixels * 4
	if outSize > uint64(^uint(0)>>1) {
		return Texture{}, ErrTextureTooLarge
	}
	out := make([]byte, int(outSize))
	originTop := descriptor&0x20 != 0
	originRight := descriptor&0x10 != 0
	for y := uint32(0); y < uint32(height); y++ {
		srcY := y
		if !originTop {
			srcY = uint32(height) - 1 - y
		}
		for x := uint32(0); x < uint32(width); x++ {
			srcX := x
			if originRight {
				srcX = uint32(width) - 1 - x
			}
			src := (uint64(srcY)*uint64(width) + uint64(srcX)) * uint64(bytesPerPixel)
			dst := (uint64(y)*uint64(width) + uint64(x)) * 4
			writeWYTRGBA(out[dst:dst+4], data[payloadStart+src:payloadStart+src+uint64(bytesPerPixel)], bits, descriptor)
		}
	}
	return Texture{Width: width, Height: height, SourceBits: bits, Pixels: out}, nil
}

func wytBytesPerPixel(bits uint8) (uint64, bool) {
	switch bits {
	case 16:
		return 2, true
	case 24:
		return 3, true
	case 32:
		return 4, true
	default:
		return 0, false
	}
}

func writeWYTRGBA(dst []byte, src []byte, bits uint8, descriptor uint8) {
	switch bits {
	case 16:
		value := binary.LittleEndian.Uint16(src)
		dst[0] = expand5(uint8((value >> 10) & 0x1f))
		dst[1] = expand5(uint8((value >> 5) & 0x1f))
		dst[2] = expand5(uint8(value & 0x1f))
		if descriptor&0x0f == 1 {
			dst[3] = uint8((value >> 15) * 0xff)
		} else {
			dst[3] = 0xff
		}
	case 24:
		dst[0], dst[1], dst[2], dst[3] = src[2], src[1], src[0], 0xff
	case 32:
		dst[0], dst[1], dst[2], dst[3] = src[2], src[1], src[0], src[3]
	}
}

func expand5(value uint8) uint8 { return (value << 3) | (value >> 2) }
