package model

import (
	"fmt"
	"strings"
)

const (
	// CelestialSealItem e o Spirit's Seal nativo (EF_VOLATILE 206).
	CelestialSealItem uint16 = 3443
	// CelestialSealEffect identifica um selo preenchido. O W2PP grava o ID em
	// EF_VALUE do primeiro e do segundo efeito, em ordem big-endian de bytes.
	CelestialSealEffect byte = 59
)

// CelestialCapsule e o agregado autoritativo criado ao retirar um Celestial da
// conta. Character e um snapshot completo da ficha; o UID da ficha nao e
// reutilizado na extracao, evitando que o charstate de um personagem apagado
// seja herdado por outro.
type CelestialCapsule struct {
	ID          uint16 `json:"id"`
	ItemUID     string `json:"itemUid"`
	SourceUID   string `json:"sourceUid"`
	Character   Char   `json:"character"`
	CreatedUnix int64  `json:"createdUnix"`
}

// CelestialSealID decodifica um Spirit's Seal preenchido. Selos vazios nao
// possuem efeito 59 e, portanto, nao apontam para nenhuma capsula.
func CelestialSealID(item Item) (uint16, bool) {
	if item.Index != CelestialSealItem ||
		item.Eff[0] != CelestialSealEffect || item.Eff[2] != CelestialSealEffect {
		return 0, false
	}
	id := uint16(item.Eff[1])<<8 | uint16(item.Eff[3])
	return id, id != 0
}

func NewCelestialSeal(uid string, id uint16) Item {
	return Item{Index: CelestialSealItem, UID: uid,
		Eff: [6]byte{CelestialSealEffect, byte(id >> 8), CelestialSealEffect, byte(id), 0, 0}}
}

func (c CelestialCapsule) Validate() error {
	if c.ID == 0 {
		return fmt.Errorf("capsula Celestial sem ID")
	}
	uid, err := NormalizeItemUID(c.ItemUID)
	if err != nil || uid == "" {
		if err == nil {
			err = fmt.Errorf("UID de item ausente")
		}
		return fmt.Errorf("capsula Celestial %d: %w", c.ID, err)
	}
	if uid != c.ItemUID {
		return fmt.Errorf("capsula Celestial %d possui UID de item nao canonico", c.ID)
	}
	source, err := NormalizeCharacterUID(c.SourceUID)
	if err != nil || source == "" {
		if err == nil {
			err = fmt.Errorf("UID de origem ausente")
		}
		return fmt.Errorf("capsula Celestial %d: %w", c.ID, err)
	}
	if source != c.SourceUID {
		return fmt.Errorf("capsula Celestial %d possui UID de origem nao canonico", c.ID)
	}
	if c.Character.Name == "" || c.Character.UID == "" {
		return fmt.Errorf("capsula Celestial %d sem personagem", c.ID)
	}
	if c.Character.UID != source {
		return fmt.Errorf("capsula Celestial %d possui SourceUID diferente da ficha", c.ID)
	}
	evolution := strings.ToLower(strings.TrimSpace(c.Character.Evolution))
	if evolution != "celestial" && evolution != "subcelestial" {
		return fmt.Errorf("capsula Celestial %d possui evolucao invalida %q", c.ID, evolution)
	}
	if err := c.Character.Extended.ValidatePlayerState(); err != nil {
		return fmt.Errorf("capsula Celestial %d: %w", c.ID, err)
	}
	return nil
}
