// Package graphics define a fronteira gráfica do client Go.
//
// A implementação inicial é deliberadamente agnóstica de backend. O backend
// OpenGL/purego será adicionado somente depois que os loaders de assets e o
// ciclo de vida da janela estiverem documentados e testados separadamente.
package graphics

import "errors"

// ErrNotImplemented marca operações que ainda não têm implementação ativa.
// Retornar um erro explícito é preferível a simular uma cena ou descartar
// recursos silenciosamente.
var ErrNotImplemented = errors.New("clientgo748: graphics backend not implemented")

// Renderer representa o ciclo mínimo esperado pelo loop do client.
// Ownership dos recursos gráficos pertencerá ao renderer concreto; chamadores
// não devem manter ponteiros para recursos depois de Close.
type Renderer interface {
	Initialize(windowHandle uintptr) error
	BeginFrame()
	EndFrame()
	Close() error
}
