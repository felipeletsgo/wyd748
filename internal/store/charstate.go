package store

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"log"
	"os"
	"path/filepath"

	"wydgo/internal/model"
)

// charStatePath resolve o arquivo do estado de sessao de um personagem. O nome
// vem de um Char (unico global), validado como elemento de caminho simples para
// nunca escapar da pasta.
func (s *JSONStore) charStatePath(name string) (string, error) {
	if !safePathElement(name) {
		return "", fmt.Errorf("store: nome de personagem invalido %q", name)
	}
	return filepath.Join(s.charStateDir, name+".json"), nil
}

// charStateWrite prepara a escrita/remocao SEM tocar o disco: valida, marshala e
// devolve o closure da operacao. Roda sob s.mu, no game-loop (snapshot). Estado
// vazio (sem buffs nem moedas) vira uma remocao, evitando lixo no disco.
func (s *JSONStore) charStateWrite(name string, state *model.CharState) (func() error, error) {
	if s.charStateDir == "" {
		return nil, fmt.Errorf("store: charStateDir nao configurado")
	}
	path, err := s.charStatePath(name)
	if err != nil {
		return nil, err
	}
	if state == nil || (len(state.Affects) == 0 && len(state.SpecialCoins) == 0) {
		return func() error {
			if err := os.Remove(path); err != nil && !os.IsNotExist(err) {
				return err
			}
			return nil
		}, nil
	}
	state.Version = model.CharStateVersion
	b, err := json.MarshalIndent(state, "", "  ")
	if err != nil {
		return nil, err
	}
	return func() error { return writeFileAtomic(path, b) }, nil
}

// SaveCharState grava o estado de sessao de forma atomica e SINCRONA (flush antes
// para nao ser sobrescrito por um autosave async pendente).
func (s *JSONStore) SaveCharState(name string, state *model.CharState) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	run, err := s.charStateWrite(name, state)
	if err != nil {
		return err
	}
	s.flushWrites()
	return run()
}

// SaveCharStateAsync agenda a escrita FORA do game-loop (autosave). Snapshot e
// marshal acontecem agora; so o fsync sai para a goroutine.
func (s *JSONStore) SaveCharStateAsync(name string, state *model.CharState) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	run, err := s.charStateWrite(name, state)
	if err != nil {
		return err
	}
	s.enqueueAsyncWrite(func() {
		if err := run(); err != nil {
			log.Printf("store: autosave charstate %q: %v", name, err)
		}
	})
	return nil
}

// LoadCharState devolve o estado de sessao do personagem, ou nil quando nao
// existe (personagem sem buffs salvos e estado valido).
func (s *JSONStore) LoadCharState(name string) (*model.CharState, error) {
	if s.charStateDir == "" {
		return nil, fmt.Errorf("store: charStateDir nao configurado")
	}
	path, err := s.charStatePath(name)
	if err != nil {
		return nil, err
	}
	b, err := os.ReadFile(path)
	if err != nil {
		if os.IsNotExist(err) {
			return nil, nil
		}
		return nil, err
	}
	decoder := json.NewDecoder(bytes.NewReader(b))
	decoder.DisallowUnknownFields()
	var state model.CharState
	if err := decoder.Decode(&state); err != nil {
		return nil, fmt.Errorf("store: parse charstate %q: %w", name, err)
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		if err == nil {
			return nil, fmt.Errorf("store: parse charstate %q: conteudo JSON adicional", name)
		}
		return nil, err
	}
	if state.Version != model.CharStateVersion {
		return nil, fmt.Errorf("store: charstate %q versao %d; esperado %d",
			name, state.Version, model.CharStateVersion)
	}
	return &state, nil
}
