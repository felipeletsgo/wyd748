package login

import (
	"crypto/rand"
	"errors"
	"fmt"
	"time"
)

// Sender é a única capacidade de transporte necessária pelo fluxo de login.
// Send deve consumir os bytes durante a chamada; o controller os apaga antes
// de retornar para não reter senha nem SecretCode em buffers temporários.
type Sender interface {
	Send(plain []byte, key byte) error
}

type TickSource func() uint32
type KeyWordSource func() (byte, error)

type ControllerOptions struct {
	Tick    TickSource
	KeyWord KeyWordSource
}

// Controller pertence à thread principal junto com SessionState. Ele
// materializa uma intenção, envia exatamente uma vez e desfaz a transição se o
// transporte não aceitar o packet.
type Controller struct {
	state   *SessionState
	sender  Sender
	tick    TickSource
	keyword KeyWordSource
}

var controllerTickEpoch = time.Now()

func NewController(state *SessionState, sender Sender, options ControllerOptions) (*Controller, error) {
	if state == nil {
		return nil, errors.New("login: session state is required")
	}
	if sender == nil {
		return nil, errors.New("login: packet sender is required")
	}
	if options.Tick == nil {
		options.Tick = func() uint32 {
			return uint32(time.Since(controllerTickEpoch) / time.Millisecond)
		}
	}
	if options.KeyWord == nil {
		options.KeyWord = randomKeyWord
	}
	return &Controller{
		state: state, sender: sender, tick: options.Tick, keyword: options.KeyWord,
	}, nil
}

// Authenticate envia o 0x20D. A senha fornecida e o packet temporário são
// apagados em todos os caminhos, inclusive validação, geração de chave e erro
// do socket.
func (c *Controller) Authenticate(account string, password []byte, adapter [4]uint32, id uint16) error {
	defer clear(password)
	if err := c.validate(); err != nil {
		return err
	}
	packet, err := c.state.BeginAuthentication(AccountLoginRequest{
		Account: account, Password: password, Adapter: adapter, ID: id, Tick: c.tick(),
	})
	if err != nil {
		return err
	}
	defer clear(packet)
	key, err := c.keyword()
	if err != nil {
		return errors.Join(fmt.Errorf("login: generate authentication keyword: %w", err), c.state.CancelAuthentication())
	}
	if err := c.sender.Send(packet, key); err != nil {
		return errors.Join(fmt.Errorf("login: send authentication: %w", err), c.state.CancelAuthentication())
	}
	return nil
}

// SelectCharacter envia o 0x213 usando exclusivamente a lista autenticada que
// pertence ao SessionState. Slot vazio, slot inválido e seleção concorrente são
// rejeitados antes de qualquer envio.
func (c *Controller) SelectCharacter(slot int32, force int32, id uint16) error {
	if err := c.validate(); err != nil {
		return err
	}
	packet, err := c.state.BeginCharacterLogin(slot, force, id, c.tick())
	if err != nil {
		return err
	}
	defer clear(packet)
	key, err := c.keyword()
	if err != nil {
		return errors.Join(fmt.Errorf("login: generate character-login keyword: %w", err), c.state.CancelCharacterLogin())
	}
	if err := c.sender.Send(packet, key); err != nil {
		return errors.Join(fmt.Errorf("login: send character login: %w", err), c.state.CancelCharacterLogin())
	}
	return nil
}

// Logout envia o 0x215 com o ClientID do snapshot 0x114 aceito. A UI não
// recebe parâmetro de ID e, portanto, não pode escolher outra identidade.
func (c *Controller) Logout() error {
	if err := c.validate(); err != nil {
		return err
	}
	packet, err := c.state.BeginCharacterLogout(c.tick())
	if err != nil {
		return err
	}
	defer clear(packet)
	key, err := c.keyword()
	if err != nil {
		return errors.Join(fmt.Errorf("login: generate character-logout keyword: %w", err), c.state.CancelCharacterLogout())
	}
	if err := c.sender.Send(packet, key); err != nil {
		return errors.Join(fmt.Errorf("login: send character logout: %w", err), c.state.CancelCharacterLogout())
	}
	return nil
}

func (c *Controller) validate() error {
	if c == nil || c.state == nil || c.sender == nil || c.tick == nil || c.keyword == nil {
		return errors.New("login: controller is not initialized")
	}
	return nil
}

func randomKeyWord() (byte, error) {
	var value [1]byte
	if _, err := rand.Read(value[:]); err != nil {
		return 0, err
	}
	return value[0], nil
}
