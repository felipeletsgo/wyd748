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

// migrateNamedCharStates converte uma unica vez os sidecars antigos baseados em
// nickname. Em caso de Mortal/Arch homonimos o legado so possuia UM arquivo; ele
// fica com o primeiro slot, sem duplicar moedas/buffs para o segundo.
func (s *JSONStore) migrateNamedCharStates(account *model.Account) error {
	if account == nil || s.charStateDir == "" {
		return nil
	}
	for i := range account.Chars {
		character := &account.Chars[i]
		if character.Name == "" || character.UID == "" ||
			!safePathElement(character.Name) || !safePathElement(character.UID) {
			continue
		}
		legacy := filepath.Join(s.charStateDir, character.Name+".json")
		current := filepath.Join(s.charStateDir, character.UID+".json")
		if _, err := os.Stat(current); err == nil {
			continue
		} else if !os.IsNotExist(err) {
			return err
		}
		if _, err := os.Stat(legacy); err != nil {
			if os.IsNotExist(err) {
				continue
			}
			return err
		}
		if err := os.MkdirAll(s.charStateDir, 0o755); err != nil {
			return err
		}
		if err := os.Rename(legacy, current); err != nil {
			return fmt.Errorf("store: migrar charstate %q para UID %s: %w",
				character.Name, character.UID, err)
		}
	}
	return nil
}

// charStatePath resolve o sidecar pela identidade estavel do personagem. O UID
// nunca se repete, mesmo quando Mortal e Arch usam o mesmo nome visual.
func (s *JSONStore) charStatePath(uid string) (string, error) {
	if !safePathElement(uid) {
		return "", fmt.Errorf("store: UID de personagem invalido %q", uid)
	}
	return filepath.Join(s.charStateDir, uid+".json"), nil
}

// charStateWrite prepara a escrita/remocao SEM tocar o disco: valida, marshala e
// devolve o closure da operacao. Roda sob s.mu, no game-loop (snapshot). Estado
// vazio (sem buffs nem moedas) vira uma remocao, evitando lixo no disco.
func (s *JSONStore) charStateWrite(uid string, state *model.CharState) (func() error, error) {
	if s.charStateDir == "" {
		return nil, fmt.Errorf("store: charStateDir nao configurado")
	}
	path, err := s.charStatePath(uid)
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
func (s *JSONStore) SaveCharState(uid string, state *model.CharState) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	run, err := s.charStateWrite(uid, state)
	if err != nil {
		return err
	}
	s.flushWrites()
	return run()
}

// SaveCharStateAsync agenda a escrita FORA do game-loop (autosave). Snapshot e
// marshal acontecem agora; so o fsync sai para a goroutine.
func (s *JSONStore) SaveCharStateAsync(uid string, state *model.CharState) error {
	s.mu.Lock()
	defer s.mu.Unlock()
	run, err := s.charStateWrite(uid, state)
	if err != nil {
		return err
	}
	s.enqueueAsyncWrite(func() {
		if err := run(); err != nil {
			log.Printf("store: autosave charstate %q: %v", uid, err)
		}
	})
	return nil
}

// LoadCharState devolve o estado de sessao do personagem, ou nil quando nao
// existe (personagem sem buffs salvos e estado valido).
func (s *JSONStore) LoadCharState(uid string) (*model.CharState, error) {
	if s.charStateDir == "" {
		return nil, fmt.Errorf("store: charStateDir nao configurado")
	}
	path, err := s.charStatePath(uid)
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
		return nil, fmt.Errorf("store: parse charstate %q: %w", uid, err)
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		if err == nil {
			return nil, fmt.Errorf("store: parse charstate %q: conteudo JSON adicional", uid)
		}
		return nil, err
	}
	if state.Version != model.CharStateVersion {
		return nil, fmt.Errorf("store: charstate %q versao %d; esperado %d",
			uid, state.Version, model.CharStateVersion)
	}
	return &state, nil
}
