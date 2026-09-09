// Package protocol será a fronteira de transporte do client Go.
//
// Os opcodes e estruturas não são duplicados nesta fundação. Antes de copiá-
// los, cada contrato deve ser correlacionado com internal/wire, a ficha nativa
// 7.48 e os testes byte a byte existentes no servidor.
package protocol

// Version identifica a família de protocolo esperada pelo client.
// Ele é apenas um rótulo de configuração; não autoriza aceitar packets de uma
// versão diferente sem validação explícita.
const Version = "7.48"

// Session representa a futura sessão de transporte. O lifecycle real (login,
// relogin, falha e fechamento) será implementado após a auditoria da fronteira
// nativa e dos contratos atuais de internal/wire.
type Session interface {
	Connect() error
	Close() error
}
