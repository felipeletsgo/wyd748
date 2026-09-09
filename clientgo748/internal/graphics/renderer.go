// Package graphics define a fronteira gráfica do client Go.
//
// A interface permanece deliberadamente agnóstica de backend. O primeiro
// backend concreto usa OpenGL/WGL em Go puro; loaders de assets continuam
// separados para que nenhum formato histórico vaze para esta fronteira.
package graphics

import (
	"errors"

	"wydclient748/internal/assets"
)

// ErrNotImplemented marca operações de render ainda não oferecidas por um
// backend. Retornar um erro explícito é preferível a simular uma cena ou
// descartar recursos silenciosamente.
var ErrNotImplemented = errors.New("clientgo748: graphics backend not implemented")

// Renderer representa o ciclo mínimo esperado pelo loop do client. O renderer
// concreto possui os recursos gráficos; chamadores não devem manter referências
// para eles depois de Close.
type Renderer interface {
	Initialize(windowHandle uintptr) error
	BeginFrame()
	EndFrame()
	Close() error
}

// TextureRenderer is the optional first-scene extension of Renderer. Keeping
// it separate preserves the small lifecycle interface for headless tests and
// for future backends that do not use OpenGL.
type TextureRenderer interface {
	Renderer
	UploadTexture(assets.Texture) error
	DrawTexture()
}

// TexturePlacementRenderer is an optional extension for backends that can
// place the currently uploaded texture in client-area pixels. Keeping this
// capability separate preserves compatibility with headless/fake
// TextureRenderer implementations used by tests and future backends.
type TexturePlacementRenderer interface {
	TextureRenderer
	DrawTextureAt(x, y, width, height int32)
}

// LayeredTextureRenderer mantém texturas auxiliares residentes no backend.
// Isso permite compor a tela nativa (fundo, logo e painel) sem substituir a
// textura ativa a cada desenho. A cena continua dona apenas dos dados de
// entrada; o renderer assume a cópia gráfica até Close.
type LayeredTextureRenderer interface {
	TexturePlacementRenderer
	UploadTextureLayer(name string, texture assets.Texture) error
	DrawTextureLayer(name string, x, y, width, height int32)
}

// ViewportProvider exposes the current client-area size in logical pixels.
// Scenes use it to keep native 7.48 compositions and their hitboxes anchored
// to the same root when the window is resized or displayed with DPI scaling.
type ViewportProvider interface {
	ClientViewport() (width, height int32)
}

// ShapeRenderer is an optional primitive surface used by the first UI scenes.
// Coordinates are client-area pixels with a top-left origin.
type ShapeRenderer interface {
	Renderer
	DrawRect(x, y, width, height int32, color Color)
}

// TextRenderer is an optional text surface for interactive scenes. Coordinates
// use the same top-left client-area pixels as ShapeRenderer.
type TextRenderer interface {
	ShapeRenderer
	DrawText(x, y int32, text string, size int32, color Color)
}

// Color is an RGBA color in the [0,1] range.
type Color struct{ R, G, B, A float32 }
