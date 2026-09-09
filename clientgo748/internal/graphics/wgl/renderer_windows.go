//go:build windows

// Package wgl implementa o primeiro renderer OpenGL do client em Go puro.
// Ele possui o contexto e o DC, mas ainda não interpreta assets proprietários.
package wgl

import (
	"errors"
	"fmt"
	"runtime"
	"structs"
	"sync"
	"unsafe"

	"github.com/ebitengine/purego"
	"golang.org/x/sys/windows"

	"wydclient748/internal/assets"
)

const (
	pfdDrawToWindow  = uint32(0x00000004)
	pfdSupportOpenGL = uint32(0x00000020)
	pfdDoubleBuffer  = uint32(0x00000001)
	pfdTypeRGBA      = uint8(0)
	pfdMainPlane     = int8(0)
	glColorBufferBit = uint32(0x00004000)
	glDepthBufferBit = uint32(0x00000100)
	glTexture2D      = uint32(0x0DE1)
	glRGBA           = uint32(0x1908)
	glUnsignedByte   = uint32(0x1401)
	glTextureMin     = uint32(0x2801)
	glTextureMag     = uint32(0x2800)
	glLinear         = int32(0x2601)
	glBlend          = uint32(0x0BE2)
	glSrcAlpha       = uint32(0x0302)
	glOneMinusSrcA   = uint32(0x0303)
	glQuads          = uint32(0x0007)
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
	genTextures       func(int32, *uint32)
	deleteTextures    func(int32, *uint32)
	bindTexture       func(uint32, uint32)
	texParameteri     func(uint32, uint32, int32)
	texImage2D        func(uint32, int32, int32, int32, int32, int32, uint32, uint32, unsafe.Pointer)
	enable            func(uint32)
	disable           func(uint32)
	blendFunc         func(uint32, uint32)
	begin             func(uint32)
	end               func()
	texCoord2f        func(float32, float32)
	vertex2f          func(float32, float32)
}

var (
	apiOnce   sync.Once
	sharedAPI *api
	apiErr    error
)

// Renderer possui um HDC e um HGLRC enquanto initialized for verdadeiro.
type Renderer struct {
	windowHandle  uintptr
	dc            uintptr
	context       uintptr
	initialized   bool
	texture       uint32
	textureWidth  int32
	textureHeight int32
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

// UploadTexture owns a copy of the decoded pixel data inside the OpenGL
// texture. The caller may release the Texture immediately after this call.
func (r *Renderer) UploadTexture(texture assets.Texture) error {
	if !r.initialized {
		return errors.New("clientgo748: renderer is not initialized")
	}
	if texture.Width == 0 || texture.Height == 0 {
		return errors.New("clientgo748: texture dimensions must be non-zero")
	}
	want := uint64(texture.Width) * uint64(texture.Height) * 4
	if uint64(len(texture.Pixels)) != want {
		return fmt.Errorf("clientgo748: texture pixel length is %d, want %d", len(texture.Pixels), want)
	}
	a := sharedAPI
	var id uint32
	a.genTextures(1, &id)
	if id == 0 {
		return errors.New("clientgo748: OpenGL did not create a texture")
	}
	a.bindTexture(glTexture2D, id)
	a.texParameteri(glTexture2D, glTextureMin, glLinear)
	a.texParameteri(glTexture2D, glTextureMag, glLinear)
	a.texImage2D(
		glTexture2D,
		0,
		int32(glRGBA),
		int32(texture.Width),
		int32(texture.Height),
		0,
		glRGBA,
		glUnsignedByte,
		unsafe.Pointer(&texture.Pixels[0]),
	)
	runtime.KeepAlive(texture.Pixels)
	if r.texture != 0 {
		a.deleteTextures(1, &r.texture)
	}
	r.texture = id
	r.textureWidth = int32(texture.Width)
	r.textureHeight = int32(texture.Height)
	return nil
}

// DrawTexture draws the initial texture as a full-window quad. Later scenes
// will replace this presentation step with their own scene graph.
func (r *Renderer) DrawTexture() {
	if !r.initialized || r.texture == 0 {
		return
	}
	a := sharedAPI
	a.enable(glTexture2D)
	a.enable(glBlend)
	a.blendFunc(glSrcAlpha, glOneMinusSrcA)
	a.bindTexture(glTexture2D, r.texture)
	a.begin(glQuads)
	// The decoded buffer is top-left origin; invert T so the logo is upright.
	a.texCoord2f(0, 1)
	a.vertex2f(-1, -1)
	a.texCoord2f(1, 1)
	a.vertex2f(1, -1)
	a.texCoord2f(1, 0)
	a.vertex2f(1, 1)
	a.texCoord2f(0, 0)
	a.vertex2f(-1, 1)
	a.end()
	a.disable(glBlend)
	a.disable(glTexture2D)
}

// Close desfaz o contexto corrente, destrói o HGLRC e libera o DC, sempre
// tentando todos os passos. Chamadas posteriores não repetem o teardown.
func (r *Renderer) Close() error {
	if !r.initialized {
		return nil
	}
	a := sharedAPI
	windowHandle, dc, context := r.windowHandle, r.dc, r.context
	texture := r.texture
	r.windowHandle, r.dc, r.context, r.texture = 0, 0, 0, 0
	r.textureWidth, r.textureHeight = 0, 0
	r.initialized = false

	var errs []error
	if texture != 0 {
		a.deleteTextures(1, &texture)
	}
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
		purego.RegisterLibFunc(&a.genTextures, opengl32.Handle(), "glGenTextures")
		purego.RegisterLibFunc(&a.deleteTextures, opengl32.Handle(), "glDeleteTextures")
		purego.RegisterLibFunc(&a.bindTexture, opengl32.Handle(), "glBindTexture")
		purego.RegisterLibFunc(&a.texParameteri, opengl32.Handle(), "glTexParameteri")
		purego.RegisterLibFunc(&a.texImage2D, opengl32.Handle(), "glTexImage2D")
		purego.RegisterLibFunc(&a.enable, opengl32.Handle(), "glEnable")
		purego.RegisterLibFunc(&a.disable, opengl32.Handle(), "glDisable")
		purego.RegisterLibFunc(&a.blendFunc, opengl32.Handle(), "glBlendFunc")
		purego.RegisterLibFunc(&a.begin, opengl32.Handle(), "glBegin")
		purego.RegisterLibFunc(&a.end, opengl32.Handle(), "glEnd")
		purego.RegisterLibFunc(&a.texCoord2f, opengl32.Handle(), "glTexCoord2f")
		purego.RegisterLibFunc(&a.vertex2f, opengl32.Handle(), "glVertex2f")
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
