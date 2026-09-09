package login

import (
	"errors"
	"fmt"

	"wydclient748/internal/protocol"
)

var ErrUnexpectedPacketDirection = errors.New("login: unexpected packet direction")

// Dispatcher encaminha somente contratos pertencentes ao fluxo de login. Um
// opcode desconhecido retorna handled=false para permitir que dispatchers de
// mundo e UI sejam compostos depois, sem duplicar um switch global.
type Dispatcher struct {
	state *SessionState
}

func NewDispatcher(state *SessionState) (*Dispatcher, error) {
	if state == nil {
		return nil, errors.New("login: session state is required")
	}
	return &Dispatcher{state: state}, nil
}

// HandleSessionEvent deve ser chamado pela thread principal. Desconexão é uma
// transição válida e sempre limpa a prova de sessão, a lista e o mundo.
func (d *Dispatcher) HandleSessionEvent(event protocol.SessionEvent) (bool, error) {
	if d == nil || d.state == nil {
		return false, errors.New("login: dispatcher is not initialized")
	}
	switch event.Kind {
	case protocol.SessionPacket:
		return d.HandlePacket(event.Packet)
	case protocol.SessionDisconnected:
		d.state.Disconnect()
		return true, nil
	default:
		return false, fmt.Errorf("login: unknown session event kind %d", event.Kind)
	}
}

func (d *Dispatcher) HandlePacket(packet protocol.Packet) (bool, error) {
	if d == nil || d.state == nil {
		return false, errors.New("login: dispatcher is not initialized")
	}
	switch packet.Header.Type {
	case OpcodeCharacterList:
		return true, d.state.AcceptCharacterList(packet)
	case OpcodeEnterWorld:
		return true, d.state.AcceptEnterWorld(packet)
	case OpcodeCharacterLogoutConfirmed:
		return true, d.state.AcceptCharacterLogout(packet)
	case OpcodeAccountLogin, OpcodeCharacterLogin, OpcodeCharacterLogout:
		return true, fmt.Errorf("%w: server sent client opcode 0x%03X", ErrUnexpectedPacketDirection, packet.Header.Type)
	default:
		return false, nil
	}
}
