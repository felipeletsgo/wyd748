package graphics

import (
	"errors"
	"fmt"

	"wydclient748/internal/assets"
)

var ErrInvalidSkeleton = errors.New("clientgo748: invalid skeleton")
var ErrInvalidSkinning = errors.New("clientgo748: invalid mesh skinning")

// Skeleton representa somente a hierarquia necessária para animação e skinning.
// O nó 0 é a raiz sintética criada por TMSkinMesh; os nós BON começam em 1.
// Isso preserva a diferença entre a raiz sintética ID 0 e o osso BON ID 0.
type Skeleton struct {
	nodes         []skeletonNode
	animatedByID  map[uint32]int
	firstNodeByID map[uint32]int
}

type skeletonNode struct {
	ID       uint32
	Parent   int
	Local    assets.MeshMatrix
	Combined assets.MeshMatrix
}

// BuildSkeleton porta a semântica estrutural de TMSkinMesh::RestoreDeviceObjects:
// cria uma raiz sintética ID 0 e adiciona cada BoneLink ao parent já existente.
func BuildSkeleton(bone assets.BoneFile) (*Skeleton, error) {
	s := &Skeleton{
		nodes:         []skeletonNode{{ID: 0, Parent: -1, Local: identityMeshMatrix(), Combined: identityMeshMatrix()}},
		animatedByID:  make(map[uint32]int, len(bone.Links)),
		firstNodeByID: map[uint32]int{0: 0},
	}

	for i, link := range bone.Links {
		parent := 0
		if link.ParentID != ^uint32(0) {
			var ok bool
			// A hierarquia BON referencia somente ossos BON. Em particular,
			// ParentID 0 deve apontar para o osso BON 0, não para a raiz
			// sintética que também usa ID 0 no TMSkinMesh.
			parent, ok = s.animatedByID[link.ParentID]
			if !ok {
				return nil, fmt.Errorf("%w: BON link %d references missing parent %d", ErrInvalidSkeleton, i, link.ParentID)
			}
		}
		if _, exists := s.animatedByID[link.BoneID]; exists {
			return nil, fmt.Errorf("%w: duplicate BON bone id %d", ErrInvalidSkeleton, link.BoneID)
		}

		nodeIndex := len(s.nodes)
		s.nodes = append(s.nodes, skeletonNode{
			ID:       link.BoneID,
			Parent:   parent,
			Local:    identityMeshMatrix(),
			Combined: identityMeshMatrix(),
		})
		s.animatedByID[link.BoneID] = nodeIndex
		if _, exists := s.firstNodeByID[link.BoneID]; !exists {
			s.firstNodeByID[link.BoneID] = nodeIndex
		}
	}

	return s, nil
}

// ApplyFrameMatrices aplica as matrizes ANI pelo índice de frame, exatamente
// como m_pframeToAnimate[Frame] no TMProject. IDs sem matriz permanecem em
// identidade; depois a matriz combinada é Local * ParentCombined.
func (s *Skeleton) ApplyFrameMatrices(matrices []assets.MeshMatrix) {
	for i := 1; i < len(s.nodes); i++ {
		s.nodes[i].Local = identityMeshMatrix()
	}
	for frameID, matrix := range matrices {
		if nodeIndex, ok := s.animatedByID[uint32(frameID)]; ok {
			s.nodes[nodeIndex].Local = matrix
		}
	}
	s.nodes[0].Combined = s.nodes[0].Local
	for i := 1; i < len(s.nodes); i++ {
		parent := s.nodes[i].Parent
		s.nodes[i].Combined = multiplyMeshMatrix(s.nodes[i].Local, s.nodes[parent].Combined)
	}
}

// BuildSkinPalette reproduz CFrame::LinkBones + CMesh::RenderMesh: cada nome
// resolve para o primeiro frame encontrado a partir da raiz e a matriz final é
// BindPalette * FrameCombined. A raiz sintética vence o lookup do ID 0, como em
// CFrame::FindFrame.
func (s *Skeleton) BuildSkinPalette(mesh assets.Mesh) ([]assets.MeshMatrix, error) {
	if len(mesh.Palette) != len(mesh.BoneNames) {
		return nil, fmt.Errorf("%w: mesh palette=%d bone names=%d", ErrInvalidSkeleton, len(mesh.Palette), len(mesh.BoneNames))
	}
	result := make([]assets.MeshMatrix, len(mesh.Palette))
	for i, boneID := range mesh.BoneNames {
		nodeIndex, ok := s.firstNodeByID[boneID]
		if !ok {
			return nil, fmt.Errorf("%w: mesh palette entry %d references missing bone %d", ErrInvalidSkeleton, i, boneID)
		}
		result[i] = multiplyMeshMatrix(mesh.Palette[i], s.nodes[nodeIndex].Combined)
	}
	return result, nil
}

// SkinMeshGeometry aplica a palette já resolvida aos atributos tipados do MSH.
// A transformação segue a convenção D3DX usada pelo TMProject: vértices são
// row-vectors, portanto a translação ocupa os índices 12..14. O parser e a
// hierarquia permanecem independentes do backend gráfico; o WGL recebe apenas
// a geometria final pronta para desenhar.
func SkinMeshGeometry(geometry MeshGeometry, palette []assets.MeshMatrix) (MeshGeometry, error) {
	vertices := append([]MeshVertex(nil), geometry.Vertices...)
	for i := range vertices {
		vertex := &vertices[i]
		if vertex.InfluenceCount == 0 {
			continue
		}
		if vertex.InfluenceCount > 4 {
			return MeshGeometry{}, fmt.Errorf("%w: vertex %d has %d influences", ErrInvalidSkinning, i, vertex.InfluenceCount)
		}

		var position Position3
		var normal Position3
		for influence := 0; influence < int(vertex.InfluenceCount); influence++ {
			paletteIndex := int(vertex.PaletteIndices[influence])
			if paletteIndex >= len(palette) {
				return MeshGeometry{}, fmt.Errorf("%w: vertex %d palette index %d outside palette size %d", ErrInvalidSkinning, i, paletteIndex, len(palette))
			}
			weight := vertex.BlendWeights[influence]
			if !finiteFloat32(weight) || weight < 0 || weight > 1 {
				return MeshGeometry{}, fmt.Errorf("%w: vertex %d has invalid weight %g", ErrInvalidSkinning, i, weight)
			}
			if weight == 0 {
				continue
			}

			matrix := palette[paletteIndex]
			transformedPosition := transformMeshPosition(vertex.Position, matrix)
			transformedNormal := transformMeshDirection(vertex.Normal, matrix)
			position.X += transformedPosition.X * weight
			position.Y += transformedPosition.Y * weight
			position.Z += transformedPosition.Z * weight
			normal.X += transformedNormal.X * weight
			normal.Y += transformedNormal.Y * weight
			normal.Z += transformedNormal.Z * weight
		}
		if !finitePosition(position) || !finitePosition(normal) {
			return MeshGeometry{}, fmt.Errorf("%w: vertex %d produced non-finite attributes", ErrInvalidSkinning, i)
		}
		vertex.Position = position
		vertex.Normal = normal
	}
	return MeshGeometry{Vertices: vertices, Indices: append([]uint16(nil), geometry.Indices...)}, nil
}

func identityMeshMatrix() assets.MeshMatrix {
	return assets.MeshMatrix{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	}
}

func multiplyMeshMatrix(a, b assets.MeshMatrix) assets.MeshMatrix {
	var out assets.MeshMatrix
	for row := 0; row < 4; row++ {
		for col := 0; col < 4; col++ {
			var value float32
			for k := 0; k < 4; k++ {
				value += a[row*4+k] * b[k*4+col]
			}
			out[row*4+col] = value
		}
	}
	return out
}

func transformMeshPosition(position Position3, matrix assets.MeshMatrix) Position3 {
	return Position3{
		X: position.X*matrix[0] + position.Y*matrix[4] + position.Z*matrix[8] + matrix[12],
		Y: position.X*matrix[1] + position.Y*matrix[5] + position.Z*matrix[9] + matrix[13],
		Z: position.X*matrix[2] + position.Y*matrix[6] + position.Z*matrix[10] + matrix[14],
	}
}

func transformMeshDirection(direction Position3, matrix assets.MeshMatrix) Position3 {
	return Position3{
		X: direction.X*matrix[0] + direction.Y*matrix[4] + direction.Z*matrix[8],
		Y: direction.X*matrix[1] + direction.Y*matrix[5] + direction.Z*matrix[9],
		Z: direction.X*matrix[2] + direction.Y*matrix[6] + direction.Z*matrix[10],
	}
}
