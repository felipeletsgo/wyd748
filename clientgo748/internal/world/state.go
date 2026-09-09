// Package world materializa o estado transitório das entidades recebidas do
// servidor. O servidor continua autoritativo; este pacote apenas copia
// snapshots validados para que a cena possa renderizá-los com segurança.
package world

import (
	"encoding/binary"
	"errors"
	"fmt"
	"sync"

	"wydclient748/internal/login"
	"wydclient748/internal/protocol"
)

const (
	OpcodeCreateMob      uint16 = 0x364
	OpcodeCreateMobTrade uint16 = 0x363
	OpcodeUpdateScore    uint16 = 0x336
	OpcodeMove           uint16 = 0x366
	OpcodeActionStop     uint16 = 0x367
	OpcodeMotion         uint16 = 0x368
	OpcodeRemoveMob      uint16 = 0x165

	CreateMobPacketSize      = 328
	CreateMobTradePacketSize = 352
	UpdateScorePacketSize    = 232
	ActionPacketSize         = 52
	RemoveMobPacketSize      = 16
)

var (
	ErrInvalidPacketSize = errors.New("world: invalid packet size")
	ErrUnexpectedOpcode  = errors.New("world: unexpected opcode")
)

// Entity é uma cópia independente do snapshot coordenado CreateMob. Ações
// posteriores atualizam somente posição/ação; nunca aceitam dados do client.
type Entity struct {
	ID         uint16
	PosX       uint16
	PosY       uint16
	Name       string
	Score      login.Score
	TradeTitle string
	Action     Action
}

type Action struct {
	PosX, PosY       uint16
	Speed, Effect    uint32
	TargetX, TargetY uint16
	Opcode           uint16
}

// State possui ownership exclusivo das entidades. Métodos de leitura devolvem
// cópias para impedir que a UI mutile o snapshot recebido do transporte.
type State struct {
	mu       sync.RWMutex
	entities map[uint16]Entity
}

func NewState() *State { return &State{entities: make(map[uint16]Entity)} }

func (s *State) Reset() {
	if s == nil {
		return
	}
	s.mu.Lock()
	defer s.mu.Unlock()
	s.entities = make(map[uint16]Entity)
}

func (s *State) Entity(id uint16) (Entity, bool) {
	if s == nil {
		return Entity{}, false
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	e, ok := s.entities[id]
	return e, ok
}

func (s *State) Snapshot() []Entity {
	if s == nil {
		return nil
	}
	s.mu.RLock()
	defer s.mu.RUnlock()
	out := make([]Entity, 0, len(s.entities))
	for _, e := range s.entities {
		out = append(out, e)
	}
	return out
}

// Dispatcher consome apenas respostas servidor->client desta fronteira.
type Dispatcher struct{ state *State }

func NewDispatcher(state *State) (*Dispatcher, error) {
	if state == nil {
		return nil, errors.New("world: state is required")
	}
	return &Dispatcher{state: state}, nil
}

func (d *Dispatcher) HandlePacket(packet protocol.Packet) (bool, error) {
	if d == nil || d.state == nil {
		return false, errors.New("world: dispatcher is not initialized")
	}
	switch packet.Header.Type {
	case OpcodeCreateMob:
		entity, err := parseCreate(packet, OpcodeCreateMob, CreateMobPacketSize)
		if err != nil {
			return true, err
		}
		d.upsert(entity)
	case OpcodeCreateMobTrade:
		entity, err := parseCreate(packet, OpcodeCreateMobTrade, CreateMobTradePacketSize)
		if err != nil {
			return true, err
		}
		d.upsert(entity)
	case OpcodeUpdateScore:
		if err := d.updateScore(packet); err != nil {
			return true, err
		}
	case OpcodeMove, OpcodeActionStop, OpcodeMotion:
		action, err := parseAction(packet)
		if err != nil {
			return true, err
		}
		d.updateAction(packet.Header.ID, action)
	case OpcodeRemoveMob:
		if err := validate(packet, OpcodeRemoveMob, RemoveMobPacketSize); err != nil {
			return true, err
		}
		d.remove(packet.Header.ID)
	default:
		return false, nil
	}
	return true, nil
}

func (d *Dispatcher) upsert(entity Entity) {
	d.state.mu.Lock()
	defer d.state.mu.Unlock()
	if prior, ok := d.state.entities[entity.ID]; ok && entity.TradeTitle == "" {
		entity.TradeTitle = prior.TradeTitle
	}
	d.state.entities[entity.ID] = entity
}
func (d *Dispatcher) updateScore(packet protocol.Packet) error {
	if err := validate(packet, OpcodeUpdateScore, UpdateScorePacketSize); err != nil {
		return err
	}
	score, err := parseScore(packet.Raw[12:152])
	if err != nil {
		return err
	}
	d.state.mu.Lock()
	defer d.state.mu.Unlock()
	e, ok := d.state.entities[packet.Header.ID]
	if !ok {
		return nil
	}
	e.Score = score
	d.state.entities[e.ID] = e
	return nil
}
func (d *Dispatcher) updateAction(id uint16, action Action) {
	d.state.mu.Lock()
	defer d.state.mu.Unlock()
	e, ok := d.state.entities[id]
	if !ok {
		return
	}
	e.PosX, e.PosY, e.Action = action.PosX, action.PosY, action
	d.state.entities[id] = e
}
func (d *Dispatcher) remove(id uint16) {
	d.state.mu.Lock()
	defer d.state.mu.Unlock()
	delete(d.state.entities, id)
}

func parseCreate(packet protocol.Packet, opcode uint16, size int) (Entity, error) {
	if err := validate(packet, opcode, size); err != nil {
		return Entity{}, err
	}
	raw := packet.Raw
	e := Entity{ID: binary.LittleEndian.Uint16(raw[16:18]), PosX: binary.LittleEndian.Uint16(raw[12:14]), PosY: binary.LittleEndian.Uint16(raw[14:16]), Name: fixedString(raw[18:34])}
	if e.ID != packet.Header.ID {
		return Entity{}, fmt.Errorf("world: entity id mismatch header=%d body=%d", packet.Header.ID, e.ID)
	}
	score, err := parseScore(raw[140:280])
	if err != nil {
		return Entity{}, err
	}
	e.Score = score
	if opcode == OpcodeCreateMobTrade {
		e.TradeTitle = fixedString(raw[326:350])
	}
	return e, nil
}
func parseAction(packet protocol.Packet) (Action, error) {
	if err := validate(packet, packet.Header.Type, ActionPacketSize); err != nil {
		return Action{}, err
	}
	raw := packet.Raw
	return Action{PosX: binary.LittleEndian.Uint16(raw[12:14]), PosY: binary.LittleEndian.Uint16(raw[14:16]), Speed: binary.LittleEndian.Uint32(raw[16:20]), Effect: binary.LittleEndian.Uint32(raw[20:24]), TargetX: binary.LittleEndian.Uint16(raw[24:26]), TargetY: binary.LittleEndian.Uint16(raw[26:28]), Opcode: packet.Header.Type}, nil
}
func parseScore(raw []byte) (login.Score, error) {
	if len(raw) != login.ScoreSize {
		return login.Score{}, fmt.Errorf("%w: score got %d want %d", ErrInvalidPacketSize, len(raw), login.ScoreSize)
	}
	// ParseScore is intentionally kept in login as the single canonical 35-word mapping.
	return login.ParseScore(raw)
}
func validate(packet protocol.Packet, opcode uint16, size int) error {
	if len(packet.Raw) != size || len(packet.Body) != size-protocol.HeaderSize || int(packet.Header.Size) != size {
		return fmt.Errorf("%w: opcode 0x%03X raw=%d body=%d header=%d want=%d", ErrInvalidPacketSize, opcode, len(packet.Raw), len(packet.Body), packet.Header.Size, size)
	}
	if binary.LittleEndian.Uint16(packet.Raw[0:2]) != uint16(size) || binary.LittleEndian.Uint16(packet.Raw[4:6]) != opcode || packet.Header.Type != opcode || !equal(packet.Body, packet.Raw[protocol.HeaderSize:]) {
		return fmt.Errorf("%w: opcode 0x%03X", ErrUnexpectedOpcode, opcode)
	}
	return nil
}
func equal(a, b []byte) bool {
	if len(a) != len(b) {
		return false
	}
	for i := range a {
		if a[i] != b[i] {
			return false
		}
	}
	return true
}
func fixedString(raw []byte) string {
	n := 0
	for n < len(raw) && raw[n] != 0 {
		n++
	}
	return string(raw[:n])
}
