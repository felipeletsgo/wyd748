package data

import (
	"encoding/json"
	"fmt"
	"os"
	"strings"

	"wydgo/internal/model"
)

var validVolatileActions = map[string]bool{
	"none": true, "restore": true, "ground_gold": true, "gold": true,
	"teleport": true, "generic": true, "buff": true, "grant_exp": true,
	"disabled": true, "save_position": true, "warp_saved": true, "refine": true,
	"refine_set": true, "face_transform": true, "face_restore": true,
	"tint": true, "untint": true, "repliction": true, "mount": true,
}

var validMountActions = map[string]bool{
	"feed": true, "essence": true, "longevity": true, "growth": true, "invuln": true,
	"hatch": true,
}

// LoadVolatiles le apenas as funcoes de data/volatiles.json e descobre os itens
// automaticamente no catalogo. Assim um item novo com EF_VOLATILE entra no
// registro sem manter uma segunda lista manual e sujeita a divergencias.
func LoadVolatiles(path string, items map[uint16]model.ItemDef) (model.VolatileCatalog, error) {
	var file struct {
		Default model.VolatileRule            `json:"default"`
		Rules   map[int]model.VolatileRule    `json:"rules"`
		Items   map[uint16]model.VolatileRule `json:"items"`
	}
	b, err := os.ReadFile(path)
	if err != nil {
		return model.VolatileCatalog{}, err
	}
	if err := json.Unmarshal(b, &file); err != nil {
		return model.VolatileCatalog{}, fmt.Errorf("data: parse %s: %w", path, err)
	}
	if file.Rules == nil {
		file.Rules = make(map[int]model.VolatileRule)
	}
	if file.Items == nil {
		file.Items = make(map[uint16]model.VolatileRule)
	}
	normalize := func(where string, rule model.VolatileRule) (model.VolatileRule, error) {
		rule.Action = strings.ToLower(strings.TrimSpace(rule.Action))
		rule.ValueSource = strings.ToLower(strings.TrimSpace(rule.ValueSource))
		if !validVolatileActions[rule.Action] {
			return rule, fmt.Errorf("data: %s possui action desconhecida %q", where, rule.Action)
		}
		if rule.Action == "teleport" && (rule.X == 0 || rule.Y == 0) {
			return rule, fmt.Errorf("data: %s teleport exige x/y positivos", where)
		}
		if rule.Action == "buff" && rule.AffectType <= 0 {
			return rule, fmt.Errorf("data: %s buff exige affectType positivo", where)
		}
		if rule.Accumulate && rule.MaxDurationUnits <= 0 {
			return rule, fmt.Errorf("data: %s accumulate exige maxDurationUnits positivo", where)
		}
		if rule.Action == "grant_exp" && rule.Exp == 0 {
			return rule, fmt.Errorf("data: %s grant_exp exige exp positivo", where)
		}
		if (rule.Action == "refine" || rule.Action == "refine_set") &&
			(rule.RefineMax <= 0 || rule.RefineMax > 15) {
			return rule, fmt.Errorf("data: %s %s exige refineMax entre 1 e 15", where, rule.Action)
		}
		if rule.Action == "face_transform" && rule.FaceMesh <= 0 {
			return rule, fmt.Errorf("data: %s face_transform exige faceMesh positivo", where)
		}
		if rule.Action == "tint" && (rule.Color < 116 || rule.Color > 125) {
			return rule, fmt.Errorf("data: %s tint exige color entre 116 e 125", where)
		}
		if rule.Action == "repliction" && len(rule.AddPool) == 0 {
			return rule, fmt.Errorf("data: %s repliction exige addPool nao-vazio", where)
		}
		if rule.Action == "mount" {
			if !validMountActions[rule.MountAction] {
				return rule, fmt.Errorf("data: %s mount exige mountAction valido (feed/essence/longevity/growth/invuln), veio %q", where, rule.MountAction)
			}
			if rule.MountAction == "invuln" && rule.DurationUnits <= 0 {
				return rule, fmt.Errorf("data: %s mount invuln exige durationUnits positivo", where)
			}
		}
		return rule, nil
	}
	file.Default, err = normalize("default", file.Default)
	if err != nil {
		return model.VolatileCatalog{}, err
	}
	for code, rule := range file.Rules {
		if code < 0 || code > 255 {
			return model.VolatileCatalog{}, fmt.Errorf("data: codigo volatile invalido %d", code)
		}
		rule, err = normalize(fmt.Sprintf("rules[%d]", code), rule)
		if err != nil {
			return model.VolatileCatalog{}, err
		}
		file.Rules[code] = rule
	}

	result := model.VolatileCatalog{
		Default: file.Default, Rules: file.Rules, Items: file.Items,
		ItemCodes: make(map[uint16]int), Codes: make(map[int]int),
	}
	for id, def := range items {
		found := false
		code := 0
		for _, effect := range def.StaticEffects {
			if effect.Name == "EF_VOLATILE" {
				code += effect.Value
				found = true
			}
		}
		if found {
			result.ItemCodes[id] = code
			result.Codes[code]++
		}
	}
	for id, rule := range file.Items {
		if _, ok := result.ItemCodes[id]; !ok {
			return model.VolatileCatalog{}, fmt.Errorf("data: items[%d] nao possui EF_VOLATILE no itemlist", id)
		}
		rule, err = normalize(fmt.Sprintf("items[%d]", id), rule)
		if err != nil {
			return model.VolatileCatalog{}, err
		}
		result.Items[id] = rule
	}
	return result, nil
}
