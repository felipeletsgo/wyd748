package store

import (
	"fmt"

	"wydgo/internal/model"
)

// prepareAccountCharacterUIDs materializa/canonicaliza a identidade server-side
// antes de qualquer snapshot. O UID pertence ao personagem, nao ao slot nem ao
// nickname, e por isso sobrevive a Mortal/Arch homonimos.
func prepareAccountCharacterUIDs(accounts ...*model.Account) (int, error) {
	changed := 0
	seen := make(map[string]string)
	for _, account := range accounts {
		if account == nil {
			return 0, fmt.Errorf("store: conta ausente ao preparar UIDs de personagem")
		}
		for slot := range account.Chars {
			character := &account.Chars[slot]
			if character.Name == "" {
				continue
			}
			uid, err := model.NormalizeCharacterUID(character.UID)
			if err != nil {
				return 0, fmt.Errorf("store: conta %q personagem[%d]: %w",
					account.Name, slot, err)
			}
			if uid == "" {
				uid, err = model.NewCharacterUID()
				if err != nil {
					return 0, err
				}
				changed++
			} else if uid != character.UID {
				changed++
			}
			character.UID = uid
			location := fmt.Sprintf("%s/char[%d]", account.Name, slot)
			if previous, duplicate := seen[uid]; duplicate {
				return 0, fmt.Errorf("store: UID de personagem %s duplicado em %s e %s",
					uid, previous, location)
			}
			seen[uid] = location

			if character.ArchMortalUID != "" {
				origin, err := model.NormalizeCharacterUID(character.ArchMortalUID)
				if err != nil {
					return 0, fmt.Errorf("store: conta %q Arch[%d]: %w",
						account.Name, slot, err)
				}
				if origin != character.ArchMortalUID {
					character.ArchMortalUID = origin
					changed++
				}
			}
		}
	}
	return changed, nil
}
