package world

import (
	"crypto/rand"
	"encoding/binary"
	"errors"
	"fmt"
	"time"

	"wydclient748/internal/login"
	"wydclient748/internal/protocol"
)

// MovementOptions fornece somente as fontes temporais/chave do envelope. A
// posição de origem continua vindo do snapshot aceito pelo SessionState.
type MovementOptions struct {
	Tick    func() uint32
	KeyWord func() (byte, error)
}

// MovementController envia intenções 0x366; ele não cria nem atualiza
// entidades. A próxima posição válida só chega em um snapshot do servidor.
type MovementController struct {
	state   *login.SessionState
	sender  login.Sender
	tick    func() uint32
	keyword func() (byte, error)
}

var movementEpoch = time.Now()

func NewMovementController(state *login.SessionState, sender login.Sender, options MovementOptions) (*MovementController, error) {
	if state == nil {
		return nil, errors.New("world: session state is required")
	}
	if sender == nil {
		return nil, errors.New("world: packet sender is required")
	}
	if options.Tick == nil {
		options.Tick = func() uint32 { return uint32(time.Since(movementEpoch) / time.Millisecond) }
	}
	if options.KeyWord == nil {
		options.KeyWord = randomMovementKeyWord
	}
	return &MovementController{state: state, sender: sender, tick: options.Tick, keyword: options.KeyWord}, nil
}

// Move envia uma intenção com a origem autoritativa atual e destino solicitado.
func (c *MovementController) Move(targetX, targetY uint16) error {
	if c == nil || c.state == nil || c.sender == nil || c.tick == nil || c.keyword == nil {
		return errors.New("world: movement controller is not initialized")
	}
	if c.state.Phase() != login.InWorld {
		return fmt.Errorf("world: movement requires InWorld, got %s", c.state.Phase())
	}
	snapshot, ok := c.state.World()
	if !ok {
		return errors.New("world: authoritative snapshot is unavailable")
	}
	packet := buildMovementPacket(snapshot, targetX, targetY, c.tick())
	defer clear(packet)
	key, err := c.keyword()
	if err != nil {
		return fmt.Errorf("world: generate movement keyword: %w", err)
	}
	if err := c.sender.Send(packet, key); err != nil {
		return fmt.Errorf("world: send movement: %w", err)
	}
	return nil
}

func buildMovementPacket(snapshot login.WorldSnapshot, targetX, targetY uint16, tick uint32) []byte {
	body := make([]byte, ActionPacketSize-protocol.HeaderSize)
	binary.LittleEndian.PutUint16(body[0:2], snapshot.PosX)
	binary.LittleEndian.PutUint16(body[2:4], snapshot.PosY)
	// Native 7.48 uses these action fields for a walking segment. The server
	// remains authoritative for speed and collision; these are only intent.
	binary.LittleEndian.PutUint32(body[4:8], 2)
	binary.LittleEndian.PutUint32(body[8:12], 1)
	binary.LittleEndian.PutUint16(body[12:14], targetX)
	binary.LittleEndian.PutUint16(body[14:16], targetY)
	return protocol.NewPacket(OpcodeMove, snapshot.ClientID, tick, body)
}

func randomMovementKeyWord() (byte, error) {
	var value [1]byte
	if _, err := rand.Read(value[:]); err != nil {
		return 0, err
	}
	return value[0], nil
}
