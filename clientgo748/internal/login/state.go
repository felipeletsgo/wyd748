package login

import (
	"errors"
	"fmt"

	"wydclient748/internal/protocol"
)

var (
	ErrInvalidState = errors.New("login: invalid session state")
	ErrEmptySlot    = errors.New("login: empty character slot")
	ErrSlotMismatch = errors.New("login: selected slot does not match response")
)

// Phase é o estado observável do primeiro fluxo vertical. LoggingOut existe
// explicitamente para bloquear um segundo pedido enquanto a persistência
// autoritativa do servidor ainda não foi confirmada.
type Phase uint8

const (
	Disconnected Phase = iota
	Connecting
	Authenticating
	CharacterSelect
	EnteringWorld
	InWorld
	LoggingOut
)

func (p Phase) String() string {
	switch p {
	case Disconnected:
		return "Disconnected"
	case Connecting:
		return "Connecting"
	case Authenticating:
		return "Authenticating"
	case CharacterSelect:
		return "CharacterSelect"
	case EnteringWorld:
		return "EnteringWorld"
	case InWorld:
		return "InWorld"
	case LoggingOut:
		return "LoggingOut"
	default:
		return fmt.Sprintf("Phase(%d)", p)
	}
}

// SessionState pertence à thread principal. Ele não cria goroutines nem usa
// locks; a futura integração de rede deverá entregar eventos ao owner em vez
// de mutar este valor a partir da rotina do socket.
type SessionState struct {
	phase       Phase
	list        CharacterList
	hasList     bool
	pendingSlot int32
	world       WorldSnapshot
	hasWorld    bool
}

func NewSessionState() *SessionState {
	return &SessionState{phase: Disconnected, pendingSlot: -1}
}

func (s *SessionState) Phase() Phase {
	if s == nil {
		return Disconnected
	}
	return s.phase
}

// BeginConnect inicia uma nova sessão somente depois de qualquer estado antigo
// ter sido integralmente descartado por Disconnect.
func (s *SessionState) BeginConnect() error {
	if s == nil || s.phase != Disconnected {
		return stateError(s, Disconnected)
	}
	s.phase = Connecting
	return nil
}

// BeginAuthentication constrói o packet antes de alterar o estado. Credencial
// inválida, portanto, não deixa uma tentativa fantasma como pendente.
func (s *SessionState) BeginAuthentication(request AccountLoginRequest) ([]byte, error) {
	if s == nil || s.phase != Connecting {
		return nil, stateError(s, Connecting)
	}
	packet, err := BuildAccountLogin(request)
	if err != nil {
		return nil, err
	}
	s.phase = Authenticating
	return packet, nil
}

// CancelAuthentication desfaz somente uma tentativa que não chegou ao
// servidor. Uma resposta inválida mantém Authenticating para que a política de
// erro da sessão decida entre nova resposta e desconexão.
func (s *SessionState) CancelAuthentication() error {
	if s == nil || s.phase != Authenticating {
		return stateError(s, Authenticating)
	}
	s.phase = Connecting
	return nil
}

// AcceptCharacterList interpreta em temporário e só então substitui a lista.
func (s *SessionState) AcceptCharacterList(packet protocol.Packet) error {
	if s == nil || s.phase != Authenticating {
		return stateError(s, Authenticating)
	}
	list, err := ParseCharacterList(packet)
	if err != nil {
		return err
	}
	s.list = list
	s.hasList = true
	s.pendingSlot = -1
	s.world = WorldSnapshot{}
	s.hasWorld = false
	s.phase = CharacterSelect
	return nil
}

// BeginCharacterLogin usa a prova opaca da lista atual e impede seleção de um
// slot vazio. O estado muda somente quando o packet foi construído.
func (s *SessionState) BeginCharacterLogin(slot int32, force int32, id uint16, tick uint32) ([]byte, error) {
	if s == nil || s.phase != CharacterSelect || !s.hasList {
		return nil, stateError(s, CharacterSelect)
	}
	if slot < 0 || slot >= SelectionCharacterCount {
		return nil, fmt.Errorf("%w: %d", ErrInvalidSlot, slot)
	}
	if !s.list.Characters[slot].Occupied() {
		return nil, fmt.Errorf("%w: %d", ErrEmptySlot, slot)
	}
	packet, err := BuildCharacterLogin(CharacterLoginRequest{
		Slot: slot, Force: force, SecretCode: s.list.SecretCode, ID: id, Tick: tick,
	})
	if err != nil {
		return nil, err
	}
	s.pendingSlot = slot
	s.phase = EnteringWorld
	return packet, nil
}

// CancelCharacterLogin desfaz apenas a tentativa cujo envio falhou. A lista
// autenticada continua válida para uma nova escolha.
func (s *SessionState) CancelCharacterLogin() error {
	if s == nil || s.phase != EnteringWorld {
		return stateError(s, EnteringWorld)
	}
	s.pendingSlot = -1
	s.phase = CharacterSelect
	return nil
}

// AcceptEnterWorld exige que o servidor confirme o mesmo slot pedido.
func (s *SessionState) AcceptEnterWorld(packet protocol.Packet) error {
	if s == nil || s.phase != EnteringWorld {
		return stateError(s, EnteringWorld)
	}
	snapshot, err := ParseEnterWorld(packet)
	if err != nil {
		return err
	}
	if int32(snapshot.Slot) != s.pendingSlot {
		return fmt.Errorf("%w: requested=%d received=%d", ErrSlotMismatch, s.pendingSlot, snapshot.Slot)
	}
	s.world = snapshot
	s.hasWorld = true
	s.pendingSlot = -1
	s.phase = InWorld
	return nil
}

// BeginCharacterLogout usa o ClientID aceito no 0x114; a camada visual não
// pode escolher o ID enviado ao servidor.
func (s *SessionState) BeginCharacterLogout(tick uint32) ([]byte, error) {
	if s == nil || s.phase != InWorld || !s.hasWorld {
		return nil, stateError(s, InWorld)
	}
	packet := BuildCharacterLogout(s.world.ClientID, tick)
	s.phase = LoggingOut
	return packet, nil
}

// CancelCharacterLogout restaura InWorld quando o envio não chegou ao
// servidor. Uma queda do socket deve usar Disconnect, nunca este método.
func (s *SessionState) CancelCharacterLogout() error {
	if s == nil || s.phase != LoggingOut || !s.hasWorld {
		return stateError(s, LoggingOut)
	}
	s.phase = InWorld
	return nil
}

// AcceptCharacterLogout só libera o snapshot depois da confirmação do mesmo
// personagem. A lista autenticada permanece disponível para relogin.
func (s *SessionState) AcceptCharacterLogout(packet protocol.Packet) error {
	if s == nil || s.phase != LoggingOut || !s.hasWorld {
		return stateError(s, LoggingOut)
	}
	if err := ParseCharacterLogoutConfirmation(packet); err != nil {
		return err
	}
	if packet.Header.ID != s.world.ClientID {
		return fmt.Errorf("login: logout confirmation id=%d want=%d", packet.Header.ID, s.world.ClientID)
	}
	s.world = WorldSnapshot{}
	s.hasWorld = false
	s.pendingSlot = -1
	s.phase = CharacterSelect
	return nil
}

// Disconnect apaga conta, prova de sessão, seleção e mundo. É idempotente e
// também serve ao rollback de conexão ou autenticação interrompida.
func (s *SessionState) Disconnect() {
	if s == nil {
		return
	}
	*s = SessionState{phase: Disconnected, pendingSlot: -1}
}

// CharacterList devolve uma cópia; o chamador nunca recebe a prova opaca por
// referência nem pode modificar o estado interno da sessão.
func (s *SessionState) CharacterList() (CharacterList, bool) {
	if s == nil || !s.hasList {
		return CharacterList{}, false
	}
	return s.list, true
}

// World devolve uma cópia do snapshot aceito.
func (s *SessionState) World() (WorldSnapshot, bool) {
	if s == nil || !s.hasWorld {
		return WorldSnapshot{}, false
	}
	return s.world, true
}

func (s *SessionState) PendingSlot() (int32, bool) {
	if s == nil || s.pendingSlot < 0 {
		return 0, false
	}
	return s.pendingSlot, true
}

func stateError(s *SessionState, expected Phase) error {
	got := Disconnected
	if s != nil {
		got = s.phase
	}
	return fmt.Errorf("%w: got %s want %s", ErrInvalidState, got, expected)
}
