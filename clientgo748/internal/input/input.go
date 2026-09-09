// Package input define os eventos observáveis que atravessam a fronteira da
// plataforma. Nenhuma cena precisa conhecer mensagens Win32 ou outro toolkit.
package input

// Kind identifica a origem de um evento de janela ou dispositivo.
type Kind uint8

const (
	KindWindowClose Kind = iota
	KindWindowResize
	KindKeyDown
	KindKeyUp
	KindMouseButtonDown
	KindMouseButtonUp
	KindMouseMove
	KindFocusGained
	KindFocusLost
)

// Event é uma cópia de dados; não contém ponteiros para estruturas da
// plataforma. Campos não aplicáveis ao Kind permanecem com valor zero.
type Event struct {
	Kind   Kind
	Key    uint32
	Button uint8
	X      int32
	Y      int32
	Width  int32
	Height int32
}
