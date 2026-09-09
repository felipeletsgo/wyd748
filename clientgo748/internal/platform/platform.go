// Package platform concentra as dependências específicas do sistema
// operacional. A implementação Win32 concreta permanece atrás desta interface,
// sem contaminar protocolo, assets ou domínio do jogo.
package platform

// Window representa a janela do client sem expor o toolkit gráfico escolhido.
// O handle só será válido entre Open e Close na implementação concreta.
type Window interface {
	Open(title string, width, height int) error
	Handle() uintptr
	PollEvents()
	ShouldClose() bool
	Close() error
}
