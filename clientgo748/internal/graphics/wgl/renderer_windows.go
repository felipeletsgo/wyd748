//go:build windows

// Package wgl implementa o primeiro renderer OpenGL do client em Go puro.
// Ele possui o contexto e o DC, mas ainda não interpreta assets proprietários.
package wgl

import (
	"errors"
	"fmt"
	"structs"
	"sync"
	"unsafe"

	"github.com/ebitengine/purego"
	"golang.org/x/sys/windows"
)

const (
	pfdDrawToWindow  = uint32(0x00000004)
	pfdSupportOpenGL = uint32(0x00000020)
	pfdDoubleBuffer  = uint32(0x00000001)
	pfdTypeRGBA      = uint8(0)
	pfdMainPlane     = int8(0)
	glColorBufferBit = uint32(0x00004000)
	glDepthBufferBit = uint32(0x00000100)
)

type rect struct {
	_                        structs.HostLayout
	Left, Top, Right, Bottom int32
}

type pixelFormatDescriptor struct {
	_              structs.HostLayout
	Size           uint16
	Version        uint16
	Flags          uint32
	PixelType      uint8
	ColorBits      uint8
	RedBits        uint8
	RedShift       uint8
	GreenBits      uint8
	GreenShift     uint8
	BlueBits       uint8
	BlueShift      uint8
	AlphaBits      uint8
	AlphaShift     uint8
	AccumBits      uint8
	AccumRedBits   uint8
	AccumGreenBits uint8
	AccumBlueBits  uint8
	AccumAlphaBits uint8
	DepthBits      uint8
	StencilBits    uint8
	AuxBuffers     uint8
	LayerType      int8
	Reserved       uint8
	LayerMask      uint32
	VisibleMask    uint32
	DamageMask     uint32
}

type api struct {
	getDC             func(uintptr) uintptr
	releaseDC         func(uintptr, uintptr) int32
	getClientRect     func(uintptr, *rect) int32
	choosePixelFormat func(uintptr, *pixelFormatDescriptor) int32
	setPixelFormat    func(uintptr, int32, *pixelFormatDescriptor) int32
	swapBuffers       func(uintptr) int32
	createContext     func(uintptr) uintptr
	makeCurrent       func(uintptr, uintptr) int32
	deleteContext     func(uintptr) int32
	clearColor        func(float32, float32, float32, float32)
	clear             func(uint32)
	viewport          func(int32, int32, int32, int32)
}

var (
	apiOnce   sync.Once
	sharedAPI *api
	apiErr    error
)

// Renderer possui um HDC e um HGLRC enquanto initialized for verdadeiro.
type Renderer struct {
	windowHandle uintptr
	dc           uintptr
	context      uintptr
	initialized  bool
}

// New cria um renderer ainda sem recursos externos.
func New() *Renderer { return &Renderer{} }

// Initialize escolhe o pixel format uma única vez, cria o contexto e o torna
// corrente na thread do chamador. Qualquer falha desfaz as aquisições locais.
func (r *Renderer) Initialize(windowHandle uintptr) error {
	if windowHandle == 0 {
		return fmt.Errorf("clientgo748: a valid window is required for OpenGL")
	}
	if r.initialized {
		return fmt.Errorf("clientgo748: renderer is already initialized")
	}
	a, err := loadAPI()
	if err != nil {
		return err
	}
	dc := a.getDC(windowHandle)
	if dc == 0 {
		return lastError("acquire the window device context")
	}
	context := uintptr(0)
	success := false
	defer func() {
		if success {
			return
		}
		if context != 0 {
			a.deleteContext(context)
		}
		a.releaseDC(windowHandle, dc)
	}()

	pfd := pixelFormatDescriptor{
		Size:        uint16(unsafe.Sizeof(pixelFormatDescriptor{})),
		Version:     1,
		Flags:       pfdDrawToWindow | pfdSupportOpenGL | pfdDoubleBuffer,
		PixelType:   pfdTypeRGBA,
		ColorBits:   32,
		DepthBits:   24,
		StencilBits: 8,
		LayerType:   pfdMainPlane,
	}
	format := a.choosePixelFormat(dc, &pfd)
	if format == 0 {
		return lastError("choose an OpenGL pixel format")
	}
	if a.setPixelFormat(dc, format, &pfd) == 0 {
		return lastError("set the OpenGL pixel format")
	}
	context = a.createContext(dc)
	if context == 0 {
		return lastError("create the OpenGL rendering context")
	}
	if a.makeCurrent(dc, context) == 0 {
		return lastError("activate the OpenGL rendering context")
	}

	r.windowHandle = windowHandle
	r.dc = dc
	r.context = context
	r.initialized = true
	success = true
	return nil
}

// BeginFrame acompanha o tamanho atual da área cliente e limpa o backbuffer.
func (r *Renderer) BeginFrame() {
	if !r.initialized {
		return
	}
	a := sharedAPI
	width, height := int32(1), int32(1)
	var bounds rect
	if a.getClientRect(r.windowHandle, &bounds) != 0 {
		if candidate := bounds.Right - bounds.Left; candidate > 0 {
			width = candidate
		}
		if candidate := bounds.Bottom - bounds.Top; candidate > 0 {
			height = candidate
		}
	}
	a.viewport(0, 0, width, height)
	a.clearColor(0.025, 0.045, 0.085, 1.0)
	a.clear(glColorBufferBit | glDepthBufferBit)
}

// EndFrame apresenta o backbuffer. A interface será promovida para retornar
// erro quando o loop possuir política explícita de recuperação do device.
func (r *Renderer) EndFrame() {
	if r.initialized {
		sharedAPI.swapBuffers(r.dc)
	}
}

// Close desfaz o contexto corrente, destrói o HGLRC e libera o DC, sempre
// tentando todos os passos. Chamadas posteriores não repetem o teardown.
func (r *Renderer) Close() error {
	if !r.initialized {
		return nil
	}
	a := sharedAPI
	windowHandle, dc, context := r.windowHandle, r.dc, r.context
	r.windowHandle, r.dc, r.context = 0, 0, 0
	r.initialized = false

	var errs []error
	if a.makeCurrent(0, 0) == 0 {
		errs = append(errs, lastError("release the current OpenGL context"))
	}
	if a.deleteContext(context) == 0 {
		errs = append(errs, lastError("delete the OpenGL rendering context"))
	}
	if a.releaseDC(windowHandle, dc) == 0 {
		errs = append(errs, lastError("release the window device context"))
	}
	return errors.Join(errs...)
}

func loadAPI() (*api, error) {
	apiOnce.Do(func() {
		user32 := windows.NewLazySystemDLL("user32.dll")
		gdi32 := windows.NewLazySystemDLL("gdi32.dll")
		opengl32 := windows.NewLazySystemDLL("opengl32.dll")
		for name, dll := range map[string]*windows.LazyDLL{
			"user32.dll":   user32,
			"gdi32.dll":    gdi32,
			"opengl32.dll": opengl32,
		} {
			if err := dll.Load(); err != nil {
				apiErr = fmt.Errorf("clientgo748: load %s: %w", name, err)
				return
			}
		}

		a := &api{}
		purego.RegisterLibFunc(&a.getDC, user32.Handle(), "GetDC")
		purego.RegisterLibFunc(&a.releaseDC, user32.Handle(), "ReleaseDC")
		purego.RegisterLibFunc(&a.getClientRect, user32.Handle(), "GetClientRect")
		purego.RegisterLibFunc(&a.choosePixelFormat, gdi32.Handle(), "ChoosePixelFormat")
		purego.RegisterLibFunc(&a.setPixelFormat, gdi32.Handle(), "SetPixelFormat")
		purego.RegisterLibFunc(&a.swapBuffers, gdi32.Handle(), "SwapBuffers")
		purego.RegisterLibFunc(&a.createContext, opengl32.Handle(), "wglCreateContext")
		purego.RegisterLibFunc(&a.makeCurrent, opengl32.Handle(), "wglMakeCurrent")
		purego.RegisterLibFunc(&a.deleteContext, opengl32.Handle(), "wglDeleteContext")
		purego.RegisterLibFunc(&a.clearColor, opengl32.Handle(), "glClearColor")
		purego.RegisterLibFunc(&a.clear, opengl32.Handle(), "glClear")
		purego.RegisterLibFunc(&a.viewport, opengl32.Handle(), "glViewport")
		sharedAPI = a
	})
	return sharedAPI, apiErr
}

func lastError(action string) error {
	err := windows.GetLastError()
	if err == windows.ERROR_SUCCESS {
		return fmt.Errorf("clientgo748: could not %s", action)
	}
	return fmt.Errorf("clientgo748: could not %s: %w", action, err)
}
