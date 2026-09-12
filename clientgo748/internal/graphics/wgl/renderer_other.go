//go:build !windows

// Package wgl mantém um stub explícito para builds de teste não-Windows.
package wgl

import (
	"errors"
	"wydclient748/internal/assets"
	"wydclient748/internal/graphics"
)

// ErrUnsupported informa que o backend concreto exige WGL no Windows.
var ErrUnsupported = errors.New("clientgo748: the OpenGL WGL renderer requires Windows")

// Renderer não possui recursos fora do Windows.
type Renderer struct{}

// New cria o stub não-Windows.
func New() *Renderer { return &Renderer{} }

// Initialize nunca finge que um contexto WGL foi criado.
func (*Renderer) Initialize(uintptr) error { return ErrUnsupported }

// BeginFrame não possui contexto no stub.
func (*Renderer) BeginFrame() {}

// EndFrame não possui backbuffer no stub.
func (*Renderer) EndFrame() {}

// ClientViewport keeps the optional renderer contract available to portable
// tests without pretending that a non-Windows backend owns a real viewport.
func (*Renderer) ClientViewport() (int32, int32) { return 0, 0 }

func (*Renderer) DrawRect(int32, int32, int32, int32, graphics.Color)    {}
func (*Renderer) DrawTextureAt(int32, int32, int32, int32)               {}
func (*Renderer) UploadTextureLayer(string, assets.Texture) error        { return ErrUnsupported }
func (*Renderer) DrawTextureLayer(string, int32, int32, int32, int32)    {}
func (*Renderer) DrawText(int32, int32, string, int32, graphics.Color)   {}
func (*Renderer) DrawMesh(assets.Mesh) error                             { return ErrUnsupported }
func (*Renderer) DrawSkinnedMesh(assets.Mesh, []assets.MeshMatrix) error { return ErrUnsupported }
func (*Renderer) DrawMeshScene(assets.Mesh, graphics.SceneTransform, graphics.Camera) error {
	return ErrUnsupported
}
func (*Renderer) DrawSkinnedMeshScene(assets.Mesh, []assets.MeshMatrix, graphics.SceneTransform, graphics.Camera) error {
	return ErrUnsupported
}
func (*Renderer) DrawSceneGeometry(graphics.MeshGeometry, graphics.SceneTransform, graphics.Camera) error {
	return ErrUnsupported
}
func (*Renderer) UploadTerrainTexture(uint16, assets.Texture) error { return ErrUnsupported }
func (*Renderer) DrawTerrainGeometry(uint16, uint16, graphics.MeshGeometry, graphics.SceneTransform, graphics.Camera) error {
	return ErrUnsupported
}
func (*Renderer) UploadModelTexture(uint16, assets.Texture) error { return ErrUnsupported }
func (*Renderer) DrawModelGeometry(uint16, graphics.MeshGeometry, graphics.SceneTransform, graphics.Camera) error {
	return ErrUnsupported
}

// Close é idempotente sem recursos.
func (*Renderer) Close() error { return nil }
