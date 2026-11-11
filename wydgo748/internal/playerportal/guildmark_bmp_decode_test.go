package playerportal

import (
	"encoding/binary"
	"image"
	"image/color"
	"testing"
)

func uncompressedBMPFixture(width, height int, bits uint16, topDown bool) []byte {
	bytesPerPixel := int(bits) / 8
	stride := (width*bytesPerPixel + 3) &^ 3
	raw := make([]byte, 54+stride*height)
	raw[0], raw[1] = 'B', 'M'
	binary.LittleEndian.PutUint32(raw[2:], uint32(len(raw)))
	binary.LittleEndian.PutUint32(raw[10:], 54)
	binary.LittleEndian.PutUint32(raw[14:], 40)
	binary.LittleEndian.PutUint32(raw[18:], uint32(int32(width)))
	encodedHeight := int32(height)
	if topDown {
		encodedHeight = -encodedHeight
	}
	binary.LittleEndian.PutUint32(raw[22:], uint32(encodedHeight))
	binary.LittleEndian.PutUint16(raw[26:], 1)
	binary.LittleEndian.PutUint16(raw[28:], bits)
	for y := 0; y < height; y++ {
		row := y
		if !topDown {
			row = height - 1 - y
		}
		for x := 0; x < width; x++ {
			at := 54 + row*stride + x*bytesPerPixel
			raw[at], raw[at+1], raw[at+2] = byte(30+x+y), byte(20+y), byte(10+x)
			if bytesPerPixel == 4 {
				raw[at+3] = 0
			}
		}
	}
	return raw
}

func TestDecodeUncompressedBMPHandles24And32BitRows(t *testing.T) {
	for _, tc := range []struct {
		name    string
		bits    uint16
		topDown bool
	}{
		{name: "24-bit bottom-up with row padding", bits: 24},
		{name: "32-bit top-down", bits: 32, topDown: true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			img, err := decodeUncompressedBMP(uncompressedBMPFixture(2, 2, tc.bits, tc.topDown))
			if err != nil || img.Bounds() != image.Rect(0, 0, 2, 2) {
				t.Fatalf("decoded image bounds=%v err=%v", img, err)
			}
			first := color.NRGBAModel.Convert(img.At(0, 0)).(color.NRGBA)
			last := color.NRGBAModel.Convert(img.At(1, 1)).(color.NRGBA)
			if first != (color.NRGBA{R: 10, G: 20, B: 30, A: 255}) || last != (color.NRGBA{R: 11, G: 21, B: 32, A: 255}) {
				t.Fatalf("decoded colors first=%+v last=%+v", first, last)
			}
		})
	}
}

func TestDecodeUncompressedBMPRejectsInvalidHeadersAndTruncation(t *testing.T) {
	valid := uncompressedBMPFixture(2, 2, 24, false)
	mutate := func(at int, value uint32) []byte {
		bad := append([]byte(nil), valid...)
		binary.LittleEndian.PutUint32(bad[at:], value)
		return bad
	}
	wrongBits := append([]byte(nil), valid...)
	binary.LittleEndian.PutUint16(wrongBits[28:], 16)
	badOffset := mutate(10, 53)
	compressed := mutate(30, 1)
	tooWide := mutate(18, 1025)
	tooTall := mutate(22, 1025)
	zeroWidth := mutate(18, 0)
	zeroHeight := mutate(22, 0)
	negativeWidth := mutate(18, ^uint32(1))
	truncated := valid[:len(valid)-1]
	for name, raw := range map[string][]byte{
		"short header": valid[:53], "wrong depth": wrongBits, "bad pixel offset": badOffset,
		"compressed": compressed, "width over limit": tooWide, "height over limit": tooTall,
		"zero width": zeroWidth, "zero height": zeroHeight, "negative width": negativeWidth,
		"truncated pixels": truncated,
	} {
		t.Run(name, func(t *testing.T) {
			if _, err := decodeUncompressedBMP(raw); err == nil {
				t.Fatal("invalid bitmap was accepted")
			}
		})
	}
}
