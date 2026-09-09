// Package app orquestra recursos do processo sem conhecer Win32 ou OpenGL.
// Dependências injetáveis permitem provar aquisição, falha e teardown sem
// criar handles reais nos testes.
package app

import (
	"context"
	"errors"
	"fmt"
	"runtime"
	"sync"

	"wydclient748/internal/graphics"
	"wydclient748/internal/platform"
)

// Options contém somente a fronteira observável do primeiro bootstrap.
type Options struct {
	Title  string
	Width  int
	Height int
}

// Application possui janela e renderer depois que cada estágio conclui.
type Application struct {
	options  Options
	window   platform.Window
	renderer graphics.Renderer

	windowOwned   bool
	rendererOwned bool
	closeOnce     sync.Once
	closeErr      error
}

// New valida dependências antes que qualquer recurso externo seja criado.
func New(options Options, window platform.Window, renderer graphics.Renderer) (*Application, error) {
	if options.Title == "" {
		return nil, fmt.Errorf("clientgo748: application title is required")
	}
	if options.Width <= 0 || options.Height <= 0 {
		return nil, fmt.Errorf("clientgo748: invalid application size %dx%d", options.Width, options.Height)
	}
	if window == nil {
		return nil, fmt.Errorf("clientgo748: window dependency is required")
	}
	if renderer == nil {
		return nil, fmt.Errorf("clientgo748: renderer dependency is required")
	}
	return &Application{options: options, window: window, renderer: renderer}, nil
}

// Run mantém criação, frames e destruição na mesma thread do sistema, requisito
// de Win32/WGL. O retorno sempre inclui eventual erro de teardown.
func (a *Application) Run(ctx context.Context) (err error) {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()
	defer func() { err = errors.Join(err, a.Close()) }()

	if err := a.window.Open(a.options.Title, a.options.Width, a.options.Height); err != nil {
		return fmt.Errorf("clientgo748: initialize window: %w", err)
	}
	a.windowOwned = true
	if err := a.renderer.Initialize(a.window.Handle()); err != nil {
		return fmt.Errorf("clientgo748: initialize renderer: %w", err)
	}
	a.rendererOwned = true

	for !a.window.ShouldClose() {
		select {
		case <-ctx.Done():
			return nil
		default:
		}
		a.window.PollEvents()
		if a.window.ShouldClose() {
			break
		}
		a.renderer.BeginFrame()
		a.renderer.EndFrame()
	}
	return nil
}

// Close libera somente recursos cuja inicialização transferiu ownership. A
// ordem é renderer antes de janela, e chamadas repetidas retornam o mesmo erro.
func (a *Application) Close() error {
	a.closeOnce.Do(func() {
		var errs []error
		if a.rendererOwned {
			a.rendererOwned = false
			if err := a.renderer.Close(); err != nil {
				errs = append(errs, fmt.Errorf("clientgo748: close renderer: %w", err))
			}
		}
		if a.windowOwned {
			a.windowOwned = false
			if err := a.window.Close(); err != nil {
				errs = append(errs, fmt.Errorf("clientgo748: close window: %w", err))
			}
		}
		a.closeErr = errors.Join(errs...)
	})
	return a.closeErr
}
