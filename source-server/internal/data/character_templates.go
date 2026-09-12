package data

import (
	"bytes"
	"encoding/json"
	"fmt"
	"io"
	"os"

	"wydgo/internal/model"
)

// LoadCharacterTemplates carrega e valida os quatro layouts de personagem.
func LoadCharacterTemplates(path string, items map[uint16]model.ItemDef) (model.CharacterTemplateFile, error) {
	var cfg model.CharacterTemplateFile
	b, err := os.ReadFile(path)
	if err != nil {
		return cfg, err
	}
	decoder := json.NewDecoder(bytes.NewReader(b))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&cfg); err != nil {
		return cfg, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if err := decoder.Decode(&struct{}{}); err != io.EOF {
		if err == nil {
			err = fmt.Errorf("conteudo JSON adicional")
		}
		return cfg, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if cfg.Spawn.X == 0 || cfg.Spawn.Y == 0 {
		return cfg, fmt.Errorf("data: %s: coordenada inicial invalida (%d,%d)", path, cfg.Spawn.X, cfg.Spawn.Y)
	}
	if len(cfg.Classes) != 4 {
		return cfg, fmt.Errorf("data: %s: esperado layout das 4 classes; encontrados %d", path, len(cfg.Classes))
	}
	seen := [4]bool{}
	for i, class := range cfg.Classes {
		if class.Class >= 4 {
			return cfg, fmt.Errorf("data: %s classes[%d]: classe invalida %d", path, i, class.Class)
		}
		if seen[class.Class] {
			return cfg, fmt.Errorf("data: %s: classe %d duplicada", path, class.Class)
		}
		seen[class.Class] = true
		if err := class.Score.Validate(); err != nil {
			return cfg, fmt.Errorf("data: %s classe %d: %w", path, class.Class, err)
		}
		if class.Score.MaxHP == 0 || class.Score.MaxMP == 0 {
			return cfg, fmt.Errorf("data: %s classe %d: HP/MP base devem ser maiores que zero", path, class.Class)
		}
		if len(class.Inventory) > model.PlayerCarrySlots {
			return cfg, fmt.Errorf("data: %s classe %d: inventario inicial excede %d slots", path, class.Class, model.PlayerCarrySlots)
		}
		equip := class.Equip.Items()
		allItems := make([]model.Item, 0, len(equip)+len(class.Inventory))
		allItems = append(allItems, equip[:]...)
		allItems = append(allItems, class.Inventory...)
		for _, item := range allItems {
			if item.UID != "" {
				return cfg, fmt.Errorf("data: %s classe %d: blueprint de item %d possui UID",
					path, class.Class, item.Index)
			}
			if item.Index != 0 {
				if _, ok := items[item.Index]; !ok {
					return cfg, fmt.Errorf("data: %s classe %d: item inicial %d nao existe no catalogo", path, class.Class, item.Index)
				}
			}
		}
	}
	return cfg, nil
}
