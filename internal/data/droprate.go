package data

import (
	"encoding/json"
	"fmt"
	"os"

	"wydgo/internal/model"
)

// LoadDropRates le a tabela de taxa de drop POR SLOT (data/droprate.json), portada
// do g_pDropRate[64] do W2PP. Semantica nativa: na morte do mob, cada slot ocupado
// rola rand()%rate==0 (rate menor = drop mais comum; <=0 = drop garantido).
func LoadDropRates(path string) ([model.MaxCarry]int, error) {
	var rates [model.MaxCarry]int
	b, err := os.ReadFile(path)
	if err != nil {
		return rates, err
	}
	var file struct {
		Rates []int `json:"rates"`
	}
	if err := json.Unmarshal(b, &file); err != nil {
		return rates, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if len(file.Rates) != model.MaxCarry {
		return rates, fmt.Errorf("data: %s precisa de %d rates, tem %d", path, model.MaxCarry, len(file.Rates))
	}
	copy(rates[:], file.Rates)
	return rates, nil
}
