package assets

import (
	"encoding/binary"
	"errors"
	"fmt"
	"os"
)

const (
	msaHeaderSize           = 12
	msaAttributeRangeSize   = 20
	msaTextureNameSize      = 11
	maxMSAAttributeCount    = 32
	msaNativePassThroughFVF = 0x142
	msaDuplicatedUVFVF      = 0x212
)

var (
	ErrInvalidMSA   = errors.New("clientgo748: invalid MSA mesh")
	ErrTruncatedMSA = errors.New("clientgo748: truncated MSA mesh")
	ErrMSATooLarge  = errors.New("clientgo748: MSA mesh exceeds safety limits")
)

type MSAAttributeRange struct {
	AttribID    uint32
	FaceStart   uint32
	FaceCount   uint32
	VertexStart uint32
	VertexCount uint32
}

// MSAMesh represents the common/static mesh framing consumed by
// FUN_004bc7c7. Vertices contain the runtime layout after the native +8-byte
// expansion applied to every FVF except 0x142.
type MSAMesh struct {
	FVF          uint32
	SourceFVF    uint32
	VertexStride uint32
	SourceStride uint32
	VertexCount  uint32
	Attributes   []MSAAttributeRange
	TextureNames []string
	Vertices     []byte
	Indices      []uint16
}

func LoadMSAFile(path string) (MSAMesh, error) {
	data, err := os.ReadFile(path)
	if err != nil {
		return MSAMesh{}, fmt.Errorf("clientgo748: read MSA %q: %w", path, err)
	}
	mesh, err := ParseMSA(data)
	if err != nil {
		return MSAMesh{}, fmt.Errorf("clientgo748: parse MSA %q: %w", path, err)
	}
	return mesh, nil
}

func ParseMSA(data []byte) (MSAMesh, error) {
	if len(data) < msaHeaderSize {
		return MSAMesh{}, ErrTruncatedMSA
	}

	sourceFVF := binary.LittleEndian.Uint32(data[0:4])
	sourceStride := binary.LittleEndian.Uint32(data[4:8])
	attributeCount := binary.LittleEndian.Uint32(data[8:12])
	if sourceStride < 12 {
		return MSAMesh{}, fmt.Errorf("%w: vertex stride %d is smaller than XYZ", ErrInvalidMSA, sourceStride)
	}
	if sourceStride > maxMeshVertexStride || attributeCount > maxMSAAttributeCount {
		return MSAMesh{}, ErrMSATooLarge
	}

	off := uint64(msaHeaderSize)
	need := func(n uint64) (int, error) {
		if n > uint64(len(data))-off {
			return 0, ErrTruncatedMSA
		}
		start := int(off)
		off += n
		return start, nil
	}

	attributeBytes := uint64(attributeCount) * msaAttributeRangeSize
	attributeStart, err := need(attributeBytes)
	if err != nil {
		return MSAMesh{}, err
	}
	attributes := make([]MSAAttributeRange, attributeCount)
	for i := range attributes {
		base := attributeStart + i*msaAttributeRangeSize
		attributes[i] = MSAAttributeRange{
			AttribID:    binary.LittleEndian.Uint32(data[base:]),
			FaceStart:   binary.LittleEndian.Uint32(data[base+4:]),
			FaceCount:   binary.LittleEndian.Uint32(data[base+8:]),
			VertexStart: binary.LittleEndian.Uint32(data[base+12:]),
			VertexCount: binary.LittleEndian.Uint32(data[base+16:]),
		}
	}

	textureStart, err := need(uint64(attributeCount) * msaTextureNameSize)
	if err != nil {
		return MSAMesh{}, err
	}
	textureNames := make([]string, attributeCount)
	for i := range textureNames {
		record := data[textureStart+i*msaTextureNameSize : textureStart+(i+1)*msaTextureNameSize]
		end := 0
		for end < len(record) && record[end] != 0 {
			end++
		}
		textureNames[i] = string(record[:end])
	}

	indexSizePos, err := need(4)
	if err != nil {
		return MSAMesh{}, err
	}
	indexBytes := binary.LittleEndian.Uint32(data[indexSizePos:])
	if indexBytes%2 != 0 {
		return MSAMesh{}, fmt.Errorf("%w: index buffer has odd byte size %d", ErrInvalidMSA, indexBytes)
	}
	indexCount := indexBytes / 2
	if indexCount > maxMeshIndices {
		return MSAMesh{}, ErrMSATooLarge
	}
	indexStart, err := need(uint64(indexBytes))
	if err != nil {
		return MSAMesh{}, err
	}
	indices := make([]uint16, indexCount)
	for i := range indices {
		indices[i] = binary.LittleEndian.Uint16(data[indexStart+i*2:])
	}

	vertexSizePos, err := need(4)
	if err != nil {
		return MSAMesh{}, err
	}
	vertexBytes := binary.LittleEndian.Uint32(data[vertexSizePos:])
	if vertexBytes%sourceStride != 0 {
		return MSAMesh{}, fmt.Errorf("%w: vertex buffer size %d is not divisible by stride %d", ErrInvalidMSA, vertexBytes, sourceStride)
	}
	vertexCount := vertexBytes / sourceStride
	if vertexCount > maxMeshVertices {
		return MSAMesh{}, ErrMSATooLarge
	}
	vertexStart, err := need(uint64(vertexBytes))
	if err != nil {
		return MSAMesh{}, err
	}
	if off != uint64(len(data)) {
		return MSAMesh{}, fmt.Errorf("%w: trailing %d bytes", ErrInvalidMSA, uint64(len(data))-off)
	}

	runtimeFVF := sourceFVF
	runtimeStride := sourceStride
	vertices := append([]byte(nil), data[vertexStart:vertexStart+int(vertexBytes)]...)
	if sourceFVF != msaNativePassThroughFVF {
		if sourceStride > ^uint32(0)-8 {
			return MSAMesh{}, ErrMSATooLarge
		}
		runtimeFVF += 0x100
		runtimeStride += 8
		expandedBytes := uint64(runtimeStride) * uint64(vertexCount)
		if expandedBytes > uint64(^uint(0)>>1) {
			return MSAMesh{}, ErrMSATooLarge
		}
		expanded := make([]byte, int(expandedBytes))
		for i := uint32(0); i < vertexCount; i++ {
			src := vertices[i*sourceStride : (i+1)*sourceStride]
			dst := expanded[i*runtimeStride : (i+1)*runtimeStride]
			copy(dst, src)
			if runtimeFVF == msaDuplicatedUVFVF {
				if sourceStride < 32 {
					return MSAMesh{}, fmt.Errorf("%w: FVF %#x requires UVs at offsets 24..31", ErrInvalidMSA, sourceFVF)
				}
				copy(dst[32:40], src[24:32])
			}
		}
		vertices = expanded
	}

	for i, attribute := range attributes {
		if uint64(attribute.VertexStart)+uint64(attribute.VertexCount) > uint64(vertexCount) {
			return MSAMesh{}, fmt.Errorf("%w: attribute %d vertex range exceeds %d vertices", ErrInvalidMSA, i, vertexCount)
		}
		if (uint64(attribute.FaceStart)+uint64(attribute.FaceCount))*3 > uint64(indexCount) {
			return MSAMesh{}, fmt.Errorf("%w: attribute %d face range exceeds %d indices", ErrInvalidMSA, i, indexCount)
		}
	}

	return MSAMesh{
		FVF:          runtimeFVF,
		SourceFVF:    sourceFVF,
		VertexStride: runtimeStride,
		SourceStride: sourceStride,
		VertexCount:  vertexCount,
		Attributes:   attributes,
		TextureNames: textureNames,
		Vertices:     vertices,
		Indices:      indices,
	}, nil
}
