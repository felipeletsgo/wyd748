// Package lifecycle define as transições permitidas do processo do client.
//
// A máquina não cria sockets, janelas ou recursos. Ela apenas centraliza o
// estado para que login, relogin, falha e fechamento possam ser testados sem
// depender do renderer ou do servidor.
package lifecycle

import "fmt"

// State é o estado observável da sessão local.
type State uint8

const (
	StateCreated State = iota
	StateConfigured
	StateConnected
	StateAuthenticated
	StateInWorld
	StateClosed
)

// Event representa uma transição solicitada pelo orquestrador.
type Event uint8

const (
	EventConfigure Event = iota
	EventConnect
	EventAuthenticate
	EventEnterWorld
	EventLogout
	EventFail
	EventClose
)

// Machine mantém um único estado e não possui referências externas. O zero
// value é utilizável e começa em Created.
type Machine struct {
	state State
}

// State retorna o estado atual.
func (m Machine) State() State { return m.state }

// Apply tenta aplicar uma transição. Fechar é idempotente; todas as demais
// transições inválidas retornam erro sem modificar o estado.
func (m *Machine) Apply(event Event) error {
	if m.state == StateClosed {
		if event == EventClose {
			return nil
		}
		return fmt.Errorf("clientgo748: event %d after closed state", event)
	}

	var next State
	switch {
	case event == EventClose || event == EventFail:
		next = StateClosed
	case m.state == StateCreated && event == EventConfigure:
		next = StateConfigured
	case m.state == StateConfigured && event == EventConnect:
		next = StateConnected
	case m.state == StateConnected && event == EventAuthenticate:
		next = StateAuthenticated
	case m.state == StateAuthenticated && event == EventEnterWorld:
		next = StateInWorld
	case (m.state == StateAuthenticated || m.state == StateInWorld) && event == EventLogout:
		next = StateConfigured
	default:
		return fmt.Errorf("clientgo748: event %d is invalid in state %d", event, m.state)
	}

	m.state = next
	return nil
}
