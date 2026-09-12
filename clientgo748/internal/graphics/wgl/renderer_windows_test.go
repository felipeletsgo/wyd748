//go:build windows

package wgl

import (
	"testing"
	"unsafe"

	"wydclient748/internal/graphics"
)

func TestIndexedGeometryBatchesAndReleasesBorrowedArrays(t *testing.T) {
	previous := sharedAPI
	defer func() { sharedAPI = previous }()
	for _, stages := range []int{0, 1, 2} {
		draws, enabled, disabled := 0, 0, 0
		var vertex, uv unsafe.Pointer
		var active uint32
		sharedAPI = &api{
			enableClientState:  func(uint32) { enabled++ },
			disableClientState: func(uint32) { disabled++ },
			vertexPointer:      func(_ int32, _ uint32, _ int32, p unsafe.Pointer) { vertex = p },
			texCoordPointer:    func(_ int32, _ uint32, _ int32, p unsafe.Pointer) { uv = p },
			drawElements: func(mode uint32, count int32, kind uint32, p unsafe.Pointer) {
				draws++
				if mode != glTriangles || count != 3000 || kind != 0x1403 || p == nil || vertex == nil || (stages > 0 && uv == nil) {
					t.Fatal("invalid indexed draw")
				}
			},
		}
		r := &Renderer{clientActiveTexture: func(unit uint32) { active = unit }}
		geometry := graphics.MeshGeometry{Vertices: []graphics.MeshVertex{{HasTexCoord: true, HasSecondaryTexCoord: true}}, Indices: make([]uint16, 3000)}
		if err := r.drawIndexedGeometry(geometry, stages); err != nil {
			t.Fatal(err)
		}
		if draws != 1 || enabled != stages+1 || disabled != enabled || vertex != nil || uv != nil || active != glTexture0 {
			t.Fatalf("draws=%d enables=%d disables=%d active=%x", draws, enabled, disabled, active)
		}
		geometry.Indices[0] = 1
		if err := r.drawIndexedGeometry(geometry, stages); err == nil {
			t.Fatal("invalid index accepted")
		}
		if draws != 1 {
			t.Fatal("invalid geometry reached GL")
		}
	}
}
