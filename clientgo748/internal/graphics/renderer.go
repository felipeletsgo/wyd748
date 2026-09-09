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
