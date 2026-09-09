//go:build !windows

// Package wgl mantém um stub explícito para builds de teste não-Windows.
package wgl

import (
	"errors"
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

func (*Renderer) DrawRect(int32, int32, int32, int32, graphics.Color) {}

// Close é idempotente sem recursos.
func (*Renderer) Close() error { return nil }
