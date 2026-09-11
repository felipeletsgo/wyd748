package graphics

import (
	"encoding/binary"
	"errors"
	"fmt"
	"math"

	"wydclient748/internal/assets"
)

// ErrInvalidMeshGeometry marks an MSH that cannot safely reach a renderer.
var ErrInvalidMeshGeometry = errors.New("clientgo748: invalid mesh geometry")

const meshWeightEpsilon = 2e-6

// Position3 is the native-confirmed XYZ prefix of one MSH vertex record.
type Position3 struct {
	X float32
	Y float32
	Z float32
}

// TexCoord2 representa a coordenada UV gravada nos layouts MSH texturizados.
type TexCoord2 struct {
	U float32
	V float32
}

// MeshVertex é a forma tipada que o renderer recebe depois da interpretação do
// registro MSH. Os layouts skinned seguem a semântica D3DFVF usada pelo
// TMProject e validada com os assets 7.48: N influências armazenam N-1 pesos
// explícitos, o último peso é implícito e o último beta contém quatro índices
// UBYTE da paleta. Apenas InfluenceCount índices/pesos são significativos.
//
// HasTexCoord distingue a única forma estática atual (FVF 18), que possui
// posição+normal sem UV, das formas skinned texturizadas.
type MeshVertex struct {
	Position       Position3
	Normal         Position3
	TexCoord       TexCoord2
	BlendWeights   [4]float32
	PaletteIndices [4]uint8
	InfluenceCount uint8
	HasTexCoord    bool
}

// MeshGeometry contém os vértices MSH já interpretados e os índices de
// triangle-list. A animação/esqueleto ainda é aplicada em uma etapa posterior;
// este tipo somente torna explícitos os atributos que já existem no asset.
type MeshGeometry struct {
	Vertices []MeshVertex
	Indices  []uint16
}

// ExtractMeshGeometry interpreta somente os cinco layouts materializados no
// catálogo 7.48 atual. Os quatro layouts skinned reutilizam a interpretação FVF
// do TMProject validada pelo usuário; combinações desconhecidas falham em vez de
// serem inferidas silenciosamente.
func ExtractMeshGeometry(mesh assets.Mesh) (MeshGeometry, error) {
	textured, influenceCount, err := meshVertexLayout(mesh)
	if err != nil {
		return MeshGeometry{}, err
	}
	expectedVertexBytes := uint64(mesh.VertexStride) * uint64(mesh.VertexCount)
	if expectedVertexBytes != uint64(len(mesh.Vertices)) {
		return MeshGeometry{}, fmt.Errorf("%w: vertex buffer has %d bytes, expected %d", ErrInvalidMeshGeometry, len(mesh.Vertices), expectedVertexBytes)
	}
	if mesh.FaceIndexCount%3 != 0 || uint64(mesh.FaceIndexCount) != uint64(len(mesh.Indices)) {
		return MeshGeometry{}, fmt.Errorf("%w: index count %d does not match triangle buffer length %d", ErrInvalidMeshGeometry, mesh.FaceIndexCount, len(mesh.Indices))
	}

	vertices := make([]MeshVertex, mesh.VertexCount)
	stride := int(mesh.VertexStride)
	for i := range vertices {
		off := i * stride
		vertex := MeshVertex{
			Position: Position3{
				X: meshFloat32(mesh.Vertices, off),
				Y: meshFloat32(mesh.Vertices, off+4),
				Z: meshFloat32(mesh.Vertices, off+8),
			},
			InfluenceCount: uint8(influenceCount),
			HasTexCoord:    textured,
		}

		attributeOff := off + 12
		if influenceCount > 0 {
			explicitWeightCount := influenceCount - 1
			weightSum := float32(0)
			for j := 0; j < explicitWeightCount; j++ {
				weight := meshFloat32(mesh.Vertices, attributeOff+j*4)
				if !finiteFloat32(weight) || weight < -meshWeightEpsilon || weight > 1+meshWeightEpsilon {
					return MeshGeometry{}, fmt.Errorf("%w: vertex %d has invalid blend weight %g", ErrInvalidMeshGeometry, i, weight)
				}
				if weight < 0 {
					weight = 0
				} else if weight > 1 {
					weight = 1
				}
				vertex.BlendWeights[j] = weight
				weightSum += weight
			}
			finalWeight := 1 - weightSum
			if finalWeight < 0 && finalWeight >= -meshWeightEpsilon {
				finalWeight = 0
			}
			if !finiteFloat32(finalWeight) || finalWeight < 0 || finalWeight > 1+meshWeightEpsilon {
				return MeshGeometry{}, fmt.Errorf("%w: vertex %d has invalid implicit blend weight %g", ErrInvalidMeshGeometry, i, finalWeight)
			}
			vertex.BlendWeights[influenceCount-1] = finalWeight

			paletteOff := attributeOff + explicitWeightCount*4
			for j := 0; j < 4; j++ {
				vertex.PaletteIndices[j] = mesh.Vertices[paletteOff+j]
			}
			for j := 0; j < influenceCount; j++ {
				if int(vertex.PaletteIndices[j]) >= len(mesh.Palette) {
					return MeshGeometry{}, fmt.Errorf("%w: vertex %d palette index %d outside palette size %d", ErrInvalidMeshGeometry, i, vertex.PaletteIndices[j], len(mesh.Palette))
				}
			}
			attributeOff = paletteOff + 4
		}

		vertex.Normal = Position3{
			X: meshFloat32(mesh.Vertices, attributeOff),
			Y: meshFloat32(mesh.Vertices, attributeOff+4),
			Z: meshFloat32(mesh.Vertices, attributeOff+8),
		}
		attributeOff += 12
		if textured {
			vertex.TexCoord = TexCoord2{
				U: meshFloat32(mesh.Vertices, attributeOff),
				V: meshFloat32(mesh.Vertices, attributeOff+4),
			}
		}
		if !finitePosition(vertex.Position) || !finitePosition(vertex.Normal) || (textured && (!finiteFloat32(vertex.TexCoord.U) || !finiteFloat32(vertex.TexCoord.V))) {
			return MeshGeometry{}, fmt.Errorf("%w: vertex %d contains non-finite attributes", ErrInvalidMeshGeometry, i)
		}
		vertices[i] = vertex
	}

	indices := append([]uint16(nil), mesh.Indices...)
	for _, index := range indices {
		if uint32(index) >= mesh.VertexCount {
			return MeshGeometry{}, fmt.Errorf("%w: vertex index %d outside vertex count %d", ErrInvalidMeshGeometry, index, mesh.VertexCount)
		}
	}
	return MeshGeometry{Vertices: vertices, Indices: indices}, nil
}

// meshVertexLayout mantém o contrato de layout em um único ponto. FVF 18 é o
// único layout estático atual; 4374..4380 são XYZBn + LASTBETA_UBYTE4 + NORMAL
// + TEX1, onde FaceInfluence informa 1..4 influências.
func meshVertexLayout(mesh assets.Mesh) (textured bool, influenceCount int, err error) {
	switch {
	case mesh.FVF == 18 && mesh.VertexStride == 24 && mesh.FaceInfluence == 0:
		return false, 0, nil
	case mesh.FVF == 4374 && mesh.VertexStride == 36 && mesh.FaceInfluence == 1:
		return true, 1, validateSkinnedPalette(mesh)
	case mesh.FVF == 4376 && mesh.VertexStride == 40 && mesh.FaceInfluence == 2:
		return true, 2, validateSkinnedPalette(mesh)
	case mesh.FVF == 4378 && mesh.VertexStride == 44 && mesh.FaceInfluence == 3:
		return true, 3, validateSkinnedPalette(mesh)
	case mesh.FVF == 4380 && mesh.VertexStride == 48 && mesh.FaceInfluence == 4:
		return true, 4, validateSkinnedPalette(mesh)
	default:
		return false, 0, fmt.Errorf("%w: unsupported FVF/stride/influence %d/%d/%d", ErrInvalidMeshGeometry, mesh.FVF, mesh.VertexStride, mesh.FaceInfluence)
	}
}

func validateSkinnedPalette(mesh assets.Mesh) error {
	if len(mesh.Palette) == 0 || len(mesh.BoneNames) != len(mesh.Palette) {
		return fmt.Errorf("%w: skinned mesh palette=%d bone names=%d", ErrInvalidMeshGeometry, len(mesh.Palette), len(mesh.BoneNames))
	}
	return nil
}

func meshFloat32(data []byte, off int) float32 {
	return math.Float32frombits(binary.LittleEndian.Uint32(data[off:]))
}

func finiteFloat32(value float32) bool {
	return !math.IsNaN(float64(value)) && !math.IsInf(float64(value), 0)
}

func finitePosition(position Position3) bool {
	return finiteFloat32(position.X) && finiteFloat32(position.Y) && finiteFloat32(position.Z)
}
