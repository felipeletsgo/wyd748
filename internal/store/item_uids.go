package store

import (
	"encoding/json"
	"fmt"
	"log"
	"os"
	"path/filepath"
	"strings"

	"wydgo/internal/model"
)

type itemUIDOwner struct {
	Account  string
	Location string
}

type accountItemRef struct {
	item     *model.Item
	location string
}

func accountItemRefs(acc *model.Account) []accountItemRef {
	if acc == nil {
		return nil
	}
	refs := make([]accountItemRef, 0, len(acc.Cargo)+len(acc.Chars)*(model.MaxEquipSlots+model.MaxCarry))
	for charIndex := range acc.Chars {
		ch := &acc.Chars[charIndex]
		charLocation := fmt.Sprintf("char-slot[%d]", charIndex)
		if ch.UID != "" {
			charLocation = "char[" + ch.UID + "]"
		}
		for slot := range ch.Equip {
			refs = append(refs, accountItemRef{
				item:     &ch.Equip[slot],
				location: fmt.Sprintf("%s/equip[%d]", charLocation, slot),
			})
		}
		for slot := range ch.Inv {
			refs = append(refs, accountItemRef{
				item:     &ch.Inv[slot],
				location: fmt.Sprintf("%s/inv[%d]", charLocation, slot),
			})
		}
	}
	for slot := range acc.Cargo {
		refs = append(refs, accountItemRef{
			item:     &acc.Cargo[slot],
			location: fmt.Sprintf("cargo[%d]", slot),
		})
	}
	return refs
}

func cloneItemOwners(src map[string]itemUIDOwner) map[string]itemUIDOwner {
	dst := make(map[string]itemUIDOwner, len(src))
	for uid, owner := range src {
		dst[uid] = owner
	}
	return dst
}

// prepareAccountItemUIDs monta o proximo indice sem publica-lo. Contas presentes
// no lote sao removidas primeiro para permitir moves entre slots/contas dentro
// da mesma transacao. Blueprints nunca passam por este caminho.
func (s *JSONStore) prepareAccountItemUIDs(accounts ...*model.Account) (
	map[string]itemUIDOwner, int, error,
) {
	if _, err := prepareAccountCharacterUIDs(accounts...); err != nil {
		return nil, 0, err
	}
	next := cloneItemOwners(s.itemOwners)
	targets := make(map[string]struct{}, len(accounts))
	for _, acc := range accounts {
		if acc == nil || acc.Name == "" {
			return nil, 0, fmt.Errorf("store: conta invalida ao preparar UIDs")
		}
		targets[strings.ToLower(acc.Name)] = struct{}{}
	}
	for uid, owner := range next {
		if _, replacing := targets[owner.Account]; replacing {
			delete(next, uid)
		}
	}

	changed := 0
	generated := make([]*model.Item, 0)
	rollbackGenerated := func() {
		for _, item := range generated {
			item.UID = ""
		}
	}
	for _, acc := range accounts {
		accountKey := strings.ToLower(acc.Name)
		for _, ref := range accountItemRefs(acc) {
			item := ref.item
			if item.Index == 0 {
				if item.UID != "" {
					rollbackGenerated()
					return nil, 0, fmt.Errorf("store: conta %q %s vazio possui UID %q",
						acc.Name, ref.location, item.UID)
				}
				continue
			}
			normalized, err := model.NormalizeItemUID(item.UID)
			if err != nil {
				rollbackGenerated()
				return nil, 0, fmt.Errorf("store: conta %q %s: %w",
					acc.Name, ref.location, err)
			}
			if normalized == "" {
				for {
					normalized, err = model.NewItemUID()
					if err != nil {
						rollbackGenerated()
						return nil, 0, err
					}
					if _, collision := next[normalized]; !collision {
						break
					}
				}
				item.UID = normalized
				generated = append(generated, item)
				changed++
			} else if item.UID != normalized {
				item.UID = normalized
				changed++
			}
			if previous, duplicate := next[item.UID]; duplicate {
				rollbackGenerated()
				return nil, 0, fmt.Errorf(
					"store: UID duplicado %s em %s/%s e %s/%s",
					item.UID, previous.Account, previous.Location, accountKey, ref.location)
			}
			next[item.UID] = itemUIDOwner{Account: accountKey, Location: ref.location}
		}
	}
	return next, changed, nil
}

// initializeItemUIDs faz a migracao unica das contas legadas no boot e constroi
// o indice global dos itens offline. Duplicidade preexistente interrompe o store:
// escolher silenciosamente uma copia destruiria evidencia de dupe.
func (s *JSONStore) initializeItemUIDs() error {
	s.itemOwners = make(map[string]itemUIDOwner)
	entries, err := os.ReadDir(s.dir)
	if err != nil {
		if os.IsNotExist(err) {
			return nil
		}
		return err
	}
	migratedAccounts, migratedItems, migratedCharacters := 0, 0, 0
	for _, entry := range entries {
		if entry.IsDir() || !strings.EqualFold(filepath.Ext(entry.Name()), ".json") {
			continue
		}
		path := filepath.Join(s.dir, entry.Name())
		raw, err := os.ReadFile(path)
		if err != nil {
			return fmt.Errorf("store: ler %q para indexar UIDs: %w", entry.Name(), err)
		}
		var acc model.Account
		if err := decodeAccountJSON(raw, &acc); err != nil {
			return fmt.Errorf("store: indexar UIDs em %q: %w", entry.Name(), err)
		}
		charactersChanged, err := prepareAccountCharacterUIDs(&acc)
		if err != nil {
			return err
		}
		if err := acc.Validate(); err != nil {
			return fmt.Errorf("store: indexar UIDs em %q: %w", entry.Name(), err)
		}
		next, changed, err := s.prepareAccountItemUIDs(&acc)
		if err != nil {
			return err
		}
		if charactersChanged != 0 || changed != 0 {
			encoded, err := json.MarshalIndent(&acc, "", "  ")
			if err != nil {
				return err
			}
			if err := s.writeAccountFile(path, encoded); err != nil {
				return fmt.Errorf("store: migrar UIDs em %q: %w", entry.Name(), err)
			}
			migratedAccounts++
			migratedItems += changed
			migratedCharacters += charactersChanged
		}
		if err := s.migrateNamedCharStates(&acc); err != nil {
			return err
		}
		s.itemOwners = next
	}
	if migratedItems != 0 {
		log.Printf("UID de itens: %d item(ns) migrados em %d conta(s)",
			migratedItems, migratedAccounts)
	}
	if migratedCharacters != 0 {
		log.Printf("UID de personagens: %d personagem(ns) migrados em %d conta(s)",
			migratedCharacters, migratedAccounts)
	}
	return nil
}
