// Package scene concentra o ownership e o lifecycle das telas do client.
//
// Uma cena é criada por uma Factory e passa a ser propriedade do Manager
// somente quando Enter retorna com sucesso. Em uma troca, a cena nova entra
// primeiro; a anterior só é liberada depois disso. Assim, uma falha parcial
// não deixa a aplicação sem uma cena válida nem mantém uma cena não inicializada.
package scene

import (
	"errors"
	"fmt"
	"time"

	"wydclient748/internal/input"
)

// ID identifica uma cena sem expor o layout ou a enumeração do client nativo.
type ID string

// Factory cria uma cena ainda não inicializada. O Manager fecha o resultado
// mesmo quando Enter falha.
type Factory func() (Scene, error)

// Scene representa uma tela completa. Exit encerra a participação lógica; Close
// libera recursos próprios. Ambas devem ser seguras para chamada única.
type Scene interface {
	ID() ID
	Enter() error
	HandleEvent(input.Event) error
	Update(time.Duration) error
	Render() error
	Exit() error
	Close() error
}

// Manager possui a única referência à cena ativa e às transições pendentes.
// Transitions requested during a frame are applied at the next Update.
type Manager struct {
	factories map[ID]Factory
	active    Scene
	pending   *ID
	closed    bool
}

// New cria um manager copiando o mapa de factories; alterações posteriores do
// chamador não alteram as opções do lifecycle.
func New(factories map[ID]Factory) *Manager {
	copyFactories := make(map[ID]Factory, len(factories))
	for id, factory := range factories {
		copyFactories[id] = factory
	}
	return &Manager{factories: copyFactories}
}

// Current retorna a cena ativa, se houver.
func (m *Manager) Current() (ID, bool) {
	if m.active == nil {
		return "", false
	}
	return m.active.ID(), true
}

// Start cria a primeira cena imediatamente. Só é permitido antes de existir
// uma cena ativa; trocas posteriores devem usar Request.
func (m *Manager) Start(id ID) error {
	if m.closed {
		return errors.New("clientgo748: scene manager is closed")
	}
	if m.active != nil {
		return fmt.Errorf("clientgo748: scene %q is already active", m.active.ID())
	}
	return m.activate(id)
}

// Request agenda uma troca para o próximo Update. Não substitui uma solicitação
// anterior silenciosamente, evitando que um evento perdido altere a tela.
func (m *Manager) Request(id ID) error {
	if m.closed {
		return errors.New("clientgo748: scene manager is closed")
	}
	if id == "" {
		return errors.New("clientgo748: scene id is required")
	}
	if m.pending != nil {
		return errors.New("clientgo748: a scene transition is already pending")
	}
	m.pending = &id
	return nil
}

// Dispatch entrega eventos somente à cena ativa. Eventos são valores copiados
// e a ordem recebida da plataforma é preservada.
func (m *Manager) Dispatch(events []input.Event) error {
	if m.closed {
		return errors.New("clientgo748: scene manager is closed")
	}
	if m.active == nil {
		return errors.New("clientgo748: no active scene")
	}
	for _, event := range events {
		if err := m.active.HandleEvent(event); err != nil {
			return fmt.Errorf("clientgo748: scene %q handle event: %w", m.active.ID(), err)
		}
	}
	return nil
}

// Update aplica uma transição pendente na fronteira do frame e atualiza a
// cena resultante. O delta negativo é rejeitado para preservar invariantes de
// relógio nos testes e no runtime.
func (m *Manager) Update(delta time.Duration) error {
	if m.closed {
		return errors.New("clientgo748: scene manager is closed")
	}
	if delta < 0 {
		return errors.New("clientgo748: negative scene delta")
	}
	if m.pending != nil {
		id := *m.pending
		m.pending = nil
		if err := m.activate(id); err != nil {
			return err
		}
	}
	if m.active == nil {
		return errors.New("clientgo748: no active scene")
	}
	if err := m.active.Update(delta); err != nil {
		return fmt.Errorf("clientgo748: scene %q update: %w", m.active.ID(), err)
	}
	return nil
}

// Render desenha somente a cena ativa; a cena não recebe o renderer para não
// permitir que ela retenha ownership de contexto ou janela.
func (m *Manager) Render() error {
	if m.closed {
		return errors.New("clientgo748: scene manager is closed")
	}
	if m.active == nil {
		return errors.New("clientgo748: no active scene")
	}
	if err := m.active.Render(); err != nil {
		return fmt.Errorf("clientgo748: scene %q render: %w", m.active.ID(), err)
	}
	return nil
}

// Close encerra a cena ativa e torna o manager terminal. Exit sempre é
// seguido de Close, inclusive quando Exit retorna erro.
func (m *Manager) Close() error {
	if m.closed {
		return nil
	}
	m.closed = true
	m.pending = nil
	if m.active == nil {
		return nil
	}
	active := m.active
	m.active = nil
	return errors.Join(active.Exit(), active.Close())
}

func (m *Manager) activate(id ID) error {
	if id == "" {
		return errors.New("clientgo748: scene id is required")
	}
	factory, ok := m.factories[id]
	if !ok || factory == nil {
		return fmt.Errorf("clientgo748: scene %q is not registered", id)
	}
	next, err := factory()
	if err != nil {
		return fmt.Errorf("clientgo748: create scene %q: %w", id, err)
	}
	if next == nil {
		return fmt.Errorf("clientgo748: factory for scene %q returned nil", id)
	}
	if next.ID() != id {
		_ = next.Close()
		return fmt.Errorf("clientgo748: scene factory returned %q, expected %q", next.ID(), id)
	}
	if err := next.Enter(); err != nil {
		return errors.Join(fmt.Errorf("clientgo748: enter scene %q: %w", id, err), next.Close())
	}
	previous := m.active
	m.active = next
	if previous == nil {
		return nil
	}
	return errors.Join(previous.Exit(), previous.Close())
}
