package data

import (
	"bytes"
	"encoding/json"
	"fmt"
	"os"
	"strconv"

	"wydgo/internal/model"
)

// LoadRepliction carrega a transcricao autoritativa de SetItemBonus2 e das
// tabelas g_pBonusValue2..5 do W2PP. O formato compacto de cada bonus e
// [effect1,value1,effect2,value2].
func LoadRepliction(path string, items map[uint16]model.ItemDef) (model.ReplictionCatalog, error) {
	var raw struct {
		Comment string                          `json:"_comment"`
		Items   map[uint16]model.ReplictionItem `json:"items"`
		Pools   map[string][][]int              `json:"pools"`
	}
	body, err := os.ReadFile(path)
	if err != nil {
		return model.ReplictionCatalog{}, err
	}
	decoder := json.NewDecoder(bytes.NewReader(body))
	decoder.DisallowUnknownFields()
	if err := decoder.Decode(&raw); err != nil {
		return model.ReplictionCatalog{}, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if len(raw.Items) == 0 || len(raw.Pools) == 0 {
		return model.ReplictionCatalog{}, fmt.Errorf("data: %s exige items e pools", path)
	}
	result := model.ReplictionCatalog{
		Items: raw.Items,
		Pools: make(map[int][]model.ReplictionBonus, len(raw.Pools)),
	}
	knownEffects := make(map[byte]bool)
	for _, def := range items {
		if len(def.DynamicEffectNames) == 0 {
			continue
		}
		for id := range def.DynamicEffectNames {
			knownEffects[id] = true
		}
		break // todos os ItemDef compartilham o mesmo mapa de ItemEffect.h
	}
	for itemID, rule := range raw.Items {
		def, ok := items[itemID]
		if !ok {
			return model.ReplictionCatalog{}, fmt.Errorf("data: repliction item %d ausente do itemlist", itemID)
		}
		volatile := 0
		for _, effect := range def.StaticEffects {
			if effect.Name == "EF_VOLATILE" {
				volatile += effect.Value
			}
		}
		if volatile != 190 {
			return model.ReplictionCatalog{}, fmt.Errorf("data: repliction item %d nao possui EF_VOLATILE 190", itemID)
		}
		if rule.ItemLevel < 1 || rule.ItemLevel > 5 || rule.MaxSanc < 0 || rule.MaxSanc > 15 {
			return model.ReplictionCatalog{}, fmt.Errorf("data: repliction item %d possui regra invalida", itemID)
		}
	}
	for positionText, entries := range raw.Pools {
		position, err := strconv.Atoi(positionText)
		if err != nil || (position != 2 && position != 4 && position != 8 && position != 16 && position != 32) {
			return model.ReplictionCatalog{}, fmt.Errorf("data: pool repliction de posicao invalida %q", positionText)
		}
		if len(entries) == 0 {
			return model.ReplictionCatalog{}, fmt.Errorf("data: pool repliction %d vazio", position)
		}
		for index, values := range entries {
			if len(values) != 4 {
				return model.ReplictionCatalog{}, fmt.Errorf("data: pool %d entrada %d requer quatro valores", position, index)
			}
			for _, value := range []int{values[0], values[2]} {
				if value <= 0 || value > 255 {
					return model.ReplictionCatalog{}, fmt.Errorf("data: pool %d entrada %d possui efeito invalido", position, index)
				}
				if !knownEffects[byte(value)] {
					return model.ReplictionCatalog{}, fmt.Errorf(
						"data: pool %d entrada %d usa efeito %d ausente do ItemEffect.h",
						position, index, value)
				}
			}
			for _, value := range []int{values[1], values[3]} {
				if value < 0 || value > 255 {
					return model.ReplictionCatalog{}, fmt.Errorf("data: pool %d entrada %d possui valor invalido", position, index)
				}
			}
			result.Pools[position] = append(result.Pools[position], model.ReplictionBonus{
				Effect1: values[0], Value1: values[1], Effect2: values[2], Value2: values[3],
			})
		}
	}
	for _, position := range []int{2, 4, 8, 16, 32} {
		if len(result.Pools[position]) == 0 {
			return model.ReplictionCatalog{}, fmt.Errorf("data: %s nao possui pool da posicao %d", path, position)
		}
	}
	return result, nil
}
