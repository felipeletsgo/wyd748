// Package platform concentra as dependências específicas do sistema
// operacional. Nenhuma chamada Win32 é feita nesta fundação; a interface
// permite adicionar janela, input e ciclo de mensagens sem contaminar o
// protocolo ou o domínio do jogo.
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
