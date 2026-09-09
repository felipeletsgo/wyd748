//go:build !windows

// Package win32 fornece um stub explícito para que testes e análise estática
// continuem compilando fora do Windows. Ele não simula uma janela real.
package win32

import "errors"

// ErrUnsupported informa que a implementação concreta exige Windows.
var ErrUnsupported = errors.New("clientgo748: the Win32 window requires Windows")

// Window é o owner vazio usado somente em builds não-Windows.
type Window struct{}

// New cria o stub não-Windows.
func New() *Window { return &Window{} }

// Open nunca finge sucesso fora do Windows.
func (*Window) Open(string, int, int) error { return ErrUnsupported }

// Handle não possui HWND fora do Windows.
func (*Window) Handle() uintptr { return 0 }

// PollEvents não possui fila Win32 fora do Windows.
func (*Window) PollEvents() {}

// ShouldClose mantém o stub terminal.
func (*Window) ShouldClose() bool { return true }

// Close é idempotente mesmo sem recurso adquirido.
func (*Window) Close() error { return nil }
