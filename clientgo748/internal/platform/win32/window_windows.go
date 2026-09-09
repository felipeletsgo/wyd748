//go:build windows

// Package win32 implementa a janela principal do client usando somente Go.
//
// Os bindings ficam deliberadamente pequenos: esta unidade possui o HWND e o
// message loop, sem importar input, scene ou regras de jogo para a plataforma.
package win32

import (
	"fmt"
	"structs"
	"sync"
	"sync/atomic"
	"unsafe"

	"github.com/ebitengine/purego"
	"golang.org/x/sys/windows"

	"wydclient748/internal/input"
)

const (
	windowClassName    = "WYDClientGo748Window"
	wsOverlappedWindow = uint32(0x00CF0000)
	cwUseDefault       = int32(-2147483648)
	swShow             = int32(5)
	pmRemove           = uint32(0x0001)
	wmClose            = uint32(0x0010)
	wmDestroy          = uint32(0x0002)
	wmQuit             = uint32(0x0012)
	wmEraseBackground  = uint32(0x0014)
	wmSize             = uint32(0x0005)
	wmSetFocus         = uint32(0x0007)
	wmKillFocus        = uint32(0x0008)
	wmKeyDown          = uint32(0x0100)
	wmKeyUp            = uint32(0x0101)
	wmMouseMove        = uint32(0x0200)
	wmLButtonDown      = uint32(0x0201)
	wmLButtonUp        = uint32(0x0202)
	wmRButtonDown      = uint32(0x0204)
	wmRButtonUp        = uint32(0x0205)
	idcArrow           = uintptr(32512)
)

type point struct {
	_    structs.HostLayout
	X, Y int32
}

type rect struct {
	_                        structs.HostLayout
	Left, Top, Right, Bottom int32
}

type message struct {
	_        structs.HostLayout
	HWnd     uintptr
	Message  uint32
	WParam   uintptr
	LParam   uintptr
	Time     uint32
	Point    point
	LPrivate uint32
}

type windowClassEx struct {
	_          structs.HostLayout
	Size       uint32
	Style      uint32
	WndProc    uintptr
	ClsExtra   int32
	WndExtra   int32
	Instance   uintptr
	Icon       uintptr
	Cursor     uintptr
	Background uintptr
	MenuName   *uint16
	ClassName  *uint16
	IconSmall  uintptr
}

type api struct {
	getModuleHandle  func(*uint16) uintptr
	loadCursor       func(uintptr, uintptr) uintptr
	registerClassEx  func(*windowClassEx) uint16
	adjustWindowRect func(*rect, uint32, int32, uint32) int32
	createWindowEx   func(uint32, *uint16, *uint16, uint32, int32, int32, int32, int32, uintptr, uintptr, uintptr, unsafe.Pointer) uintptr
	showWindow       func(uintptr, int32) int32
	updateWindow     func(uintptr) int32
	peekMessage      func(*message, uintptr, uint32, uint32, uint32) int32
	translateMessage func(*message) int32
	dispatchMessage  func(*message) uintptr
	defWindowProc    func(uintptr, uint32, uintptr, uintptr) uintptr
	destroyWindow    func(uintptr) int32
	postQuitMessage  func(int32)
}

var (
	apiOnce         sync.Once
	sharedAPI       *api
	apiErr          error
	classOnce       sync.Once
	classErr        error
	classNameUTF16  *uint16
	windowsByHandle sync.Map
)

// Window possui exatamente um HWND. O zero value é válido e Close é seguro
// mesmo quando Open falhou antes de transferir ownership.
type Window struct {
	handle   atomic.Uintptr
	closed   atomic.Bool
	eventsMu sync.Mutex
	events   []input.Event
}

// New cria um owner ainda sem recursos externos.
func New() *Window { return &Window{} }

// Open registra a classe e cria uma janela cuja área cliente tem width x
// height. Uma instância não pode ser reaberta após adquirir seu HWND.
func (w *Window) Open(title string, width, height int) error {
	if title == "" {
		return fmt.Errorf("clientgo748: window title is required")
	}
	if width <= 0 || height <= 0 {
		return fmt.Errorf("clientgo748: invalid window size %dx%d", width, height)
	}
	if w.handle.Load() != 0 {
		return fmt.Errorf("clientgo748: window is already open")
	}

	a, err := loadAPI()
	if err != nil {
		return err
	}
	if err := registerWindowClass(a); err != nil {
		return err
	}
	titleUTF16, err := windows.UTF16PtrFromString(title)
	if err != nil {
		return fmt.Errorf("clientgo748: invalid window title: %w", err)
	}

	bounds := rect{Right: int32(width), Bottom: int32(height)}
	if a.adjustWindowRect(&bounds, wsOverlappedWindow, 0, 0) == 0 {
		return lastError("adjust the client window rectangle")
	}
	instance := a.getModuleHandle(nil)
	if instance == 0 {
		return lastError("get the application module")
	}
	hwnd := a.createWindowEx(
		0,
		classNameUTF16,
		titleUTF16,
		wsOverlappedWindow,
		cwUseDefault,
		cwUseDefault,
		bounds.Right-bounds.Left,
		bounds.Bottom-bounds.Top,
		0,
		0,
		instance,
		nil,
	)
	if hwnd == 0 {
		return lastError("create the main window")
	}

	w.closed.Store(false)
	w.handle.Store(hwnd)
	windowsByHandle.Store(hwnd, w)
	a.showWindow(hwnd, swShow)
	a.updateWindow(hwnd)
	return nil
}

// Handle retorna o HWND enquanto a janela é válida.
func (w *Window) Handle() uintptr { return w.handle.Load() }

// PollEvents consome todas as mensagens disponíveis sem bloquear o frame.
func (w *Window) PollEvents() []input.Event {
	a, err := loadAPI()
	if err != nil {
		w.closed.Store(true)
		return nil
	}
	var msg message
	for a.peekMessage(&msg, 0, 0, 0, pmRemove) != 0 {
		if msg.Message == wmQuit {
			w.closed.Store(true)
			continue
		}
		a.translateMessage(&msg)
		a.dispatchMessage(&msg)
	}
	w.eventsMu.Lock()
	events := append([]input.Event(nil), w.events...)
	w.events = w.events[:0]
	w.eventsMu.Unlock()
	return events
}

// ShouldClose informa se WM_CLOSE/WM_DESTROY/WM_QUIT encerrou a janela.
func (w *Window) ShouldClose() bool { return w.closed.Load() }

// Close destrói o HWND uma única vez. WM_DESTROY publicará WM_QUIT.
func (w *Window) Close() error {
	hwnd := w.handle.Swap(0)
	if hwnd == 0 {
		w.closed.Store(true)
		return nil
	}
	windowsByHandle.Delete(hwnd)
	w.closed.Store(true)
	w.eventsMu.Lock()
	w.events = nil
	w.eventsMu.Unlock()
	a, err := loadAPI()
	if err != nil {
		return err
	}
	if a.destroyWindow(hwnd) == 0 {
		return lastError("destroy the main window")
	}
	return nil
}

func loadAPI() (*api, error) {
	apiOnce.Do(func() {
		kernel32 := windows.NewLazySystemDLL("kernel32.dll")
		user32 := windows.NewLazySystemDLL("user32.dll")
		if err := kernel32.Load(); err != nil {
			apiErr = fmt.Errorf("clientgo748: load kernel32.dll: %w", err)
			return
		}
		if err := user32.Load(); err != nil {
			apiErr = fmt.Errorf("clientgo748: load user32.dll: %w", err)
			return
		}

		a := &api{}
		purego.RegisterLibFunc(&a.getModuleHandle, kernel32.Handle(), "GetModuleHandleW")
		purego.RegisterLibFunc(&a.loadCursor, user32.Handle(), "LoadCursorW")
		purego.RegisterLibFunc(&a.registerClassEx, user32.Handle(), "RegisterClassExW")
		purego.RegisterLibFunc(&a.adjustWindowRect, user32.Handle(), "AdjustWindowRectEx")
		purego.RegisterLibFunc(&a.createWindowEx, user32.Handle(), "CreateWindowExW")
		purego.RegisterLibFunc(&a.showWindow, user32.Handle(), "ShowWindow")
		purego.RegisterLibFunc(&a.updateWindow, user32.Handle(), "UpdateWindow")
		purego.RegisterLibFunc(&a.peekMessage, user32.Handle(), "PeekMessageW")
		purego.RegisterLibFunc(&a.translateMessage, user32.Handle(), "TranslateMessage")
		purego.RegisterLibFunc(&a.dispatchMessage, user32.Handle(), "DispatchMessageW")
		purego.RegisterLibFunc(&a.defWindowProc, user32.Handle(), "DefWindowProcW")
		purego.RegisterLibFunc(&a.destroyWindow, user32.Handle(), "DestroyWindow")
		purego.RegisterLibFunc(&a.postQuitMessage, user32.Handle(), "PostQuitMessage")
		sharedAPI = a
	})
	return sharedAPI, apiErr
}

func registerWindowClass(a *api) error {
	classOnce.Do(func() {
		var err error
		classNameUTF16, err = windows.UTF16PtrFromString(windowClassName)
		if err != nil {
			classErr = fmt.Errorf("clientgo748: encode window class: %w", err)
			return
		}
		instance := a.getModuleHandle(nil)
		if instance == 0 {
			classErr = lastError("get the application module")
			return
		}
		// LoadCursorW também aceita MAKEINTRESOURCE; na ABI isso é o ID no
		// mesmo slot de ponteiro, sem fabricar um ponteiro Go inválido.
		cursor := a.loadCursor(0, idcArrow)
		if cursor == 0 {
			classErr = lastError("load the arrow cursor")
			return
		}
		wc := windowClassEx{
			Size:      uint32(unsafe.Sizeof(windowClassEx{})),
			WndProc:   purego.NewCallback(windowProc),
			Instance:  instance,
			Cursor:    cursor,
			ClassName: classNameUTF16,
		}
		if a.registerClassEx(&wc) == 0 {
			classErr = lastError("register the main window class")
		}
	})
	return classErr
}

func windowProc(hwnd uintptr, msg uint32, wParam, lParam uintptr) uintptr {
	a := sharedAPI
	if a == nil {
		return 0
	}
	switch msg {
	case wmEraseBackground:
		// OpenGL cobre toda a área cliente; evita apagar o fundo entre frames.
		return 1
	case wmClose:
		// O pedido apenas encerra o loop. O orquestrador desmonta primeiro o
		// renderer e só então chama Close para destruir a janela.
		if value, ok := windowsByHandle.Load(hwnd); ok {
			w := value.(*Window)
			w.closed.Store(true)
			w.pushEvent(input.Event{Kind: input.KindWindowClose})
		}
		return 0
	case wmSize:
		if value, ok := windowsByHandle.Load(hwnd); ok {
			value.(*Window).pushEvent(input.Event{
				Kind:   input.KindWindowResize,
				Width:  int32(uint16(lParam)),
				Height: int32(uint16(lParam >> 16)),
			})
		}
		return 0
	case wmKeyDown, wmKeyUp:
		if value, ok := windowsByHandle.Load(hwnd); ok {
			kind := input.KindKeyDown
			if msg == wmKeyUp {
				kind = input.KindKeyUp
			}
			value.(*Window).pushEvent(input.Event{Kind: kind, Key: uint32(wParam)})
		}
		return 0
	case wmMouseMove:
		if value, ok := windowsByHandle.Load(hwnd); ok {
			value.(*Window).pushEvent(input.Event{
				Kind: input.KindMouseMove,
				X:    int32(int16(uint16(lParam))),
				Y:    int32(int16(uint16(lParam >> 16))),
			})
		}
		return 0
	case wmLButtonDown, wmLButtonUp, wmRButtonDown, wmRButtonUp:
		if value, ok := windowsByHandle.Load(hwnd); ok {
			kind := input.KindMouseButtonDown
			if msg == wmLButtonUp || msg == wmRButtonUp {
				kind = input.KindMouseButtonUp
			}
			button := uint8(1)
			if msg == wmRButtonDown || msg == wmRButtonUp {
				button = 2
			}
			value.(*Window).pushEvent(input.Event{
				Kind:   kind,
				Button: button,
				X:      int32(int16(uint16(lParam))),
				Y:      int32(int16(uint16(lParam >> 16))),
			})
		}
		return 0
	case wmSetFocus, wmKillFocus:
		if value, ok := windowsByHandle.Load(hwnd); ok {
			kind := input.KindFocusGained
			if msg == wmKillFocus {
				kind = input.KindFocusLost
			}
			value.(*Window).pushEvent(input.Event{Kind: kind})
		}
		return 0
	case wmDestroy:
		if value, ok := windowsByHandle.LoadAndDelete(hwnd); ok {
			w := value.(*Window)
			w.handle.Store(0)
			w.closed.Store(true)
		}
		a.postQuitMessage(0)
		return 0
	default:
		return a.defWindowProc(hwnd, msg, wParam, lParam)
	}
}

func (w *Window) pushEvent(event input.Event) {
	w.eventsMu.Lock()
	w.events = append(w.events, event)
	w.eventsMu.Unlock()
}

func lastError(action string) error {
	err := windows.GetLastError()
	if err == windows.ERROR_SUCCESS {
		return fmt.Errorf("clientgo748: could not %s", action)
	}
	return fmt.Errorf("clientgo748: could not %s: %w", action, err)
}
