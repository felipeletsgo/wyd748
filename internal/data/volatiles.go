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
	"grant_next_level": true, "firework": true, "chaos_remission": true,
	"mastery_reset":    true,
	"territory_pass":   true,
	"refine_equipped":  true,
	"nightmare_ticket": true,
	"instance_ticket":  true,
	"disabled":         true, "save_position": true, "warp_saved": true, "refine": true,
	"refine_set": true, "face_transform": true, "face_restore": true,
	"tint": true, "untint": true, "repliction": true, "mount": true,
	"magical_pill": true, "hunting_teleport": true,
	"summon_contract": true, "learn_special_skill": true,
	"quest_reward": true, "gate_key": true, "grant_counter": true,
	"grant_counter_once": true, "arch_crystal": true,
	"loot_box": true, "mount_revive": true, "timed_access": true,
	"dungeon_teleport": true, "no_direct_use": true, "celestial_pending": true,
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
		if rule.Action == "hunting_teleport" && len(rule.Destinations) != 10 {
			return rule, fmt.Errorf("data: %s hunting_teleport exige exatamente 10 destinos", where)
		}
		if rule.Action == "learn_special_skill" && (rule.LearnedBit < 25 || rule.LearnedBit > 29) {
			return rule, fmt.Errorf("data: %s learn_special_skill exige learnedBit 25..29", where)
		}
		if rule.Action == "summon_contract" {
			if rule.Summon == nil || rule.Summon.Name == "" || rule.Summon.Group == "" ||
				rule.Summon.Face == 0 || rule.Summon.HP == 0 || rule.Summon.AttackRange <= 0 {
				return rule, fmt.Errorf("data: %s summon_contract possui template incompleto", where)
			}
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
		if (rule.Action == "grant_counter" || rule.Action == "grant_counter_once") &&
			len(rule.Counters) == 0 {
			return rule, fmt.Errorf("data: %s %s exige counters", where, rule.Action)
		}
		if rule.Action == "grant_counter_once" {
			for _, itemID := range rule.RewardItems {
				if _, exists := items[itemID]; !exists {
					return rule, fmt.Errorf(
						"data: %s grant_counter_once recompensa item inexistente %d",
						where, itemID)
				}
			}
		}
		if rule.Action == "loot_box" {
			if len(rule.RewardItems) == 0 {
				return rule, fmt.Errorf("data: %s loot_box exige rewardItems", where)
			}
			for _, itemID := range rule.RewardItems {
				if itemID == 0 {
					return rule, fmt.Errorf("data: %s loot_box possui recompensa zero", where)
				}
				if _, exists := items[itemID]; !exists {
					return rule, fmt.Errorf(
						"data: %s loot_box recompensa item inexistente %d", where, itemID)
				}
			}
		}
		if rule.Action == "timed_access" &&
			(strings.TrimSpace(rule.AccessKey) == "" || rule.DurationSeconds <= 0) {
			return rule, fmt.Errorf(
				"data: %s timed_access exige accessKey e durationSeconds positivos", where)
		}
		if rule.Action == "dungeon_teleport" &&
			(rule.X == 0 || rule.Y == 0 || rule.DurationSeconds <= 0) {
			return rule, fmt.Errorf(
				"data: %s dungeon_teleport exige x/y e durationSeconds positivos", where)
		}
		if rule.PartyMode != "" && rule.PartyMode != "party" && rule.PartyMode != "solo" {
			return rule, fmt.Errorf("data: %s possui partyMode invalido %q", where, rule.PartyMode)
		}
		if rule.Action == "mastery_reset" &&
			(rule.Amount <= 0 || rule.MaxLevelExclusive <= rule.MinLevel) {
			return rule, fmt.Errorf("data: %s mastery_reset exige amount e faixa de nivel validos", where)
		}
		if rule.Action == "territory_pass" &&
			(rule.X == 0 || rule.Y == 0 || rule.RequiredEvolution == "") {
			return rule, fmt.Errorf("data: %s territory_pass exige x/y e requiredEvolution", where)
		}
		if rule.Action == "quest_reward" {
			if rule.Exp == 0 {
				return rule, fmt.Errorf("data: %s quest_reward exige exp positivo", where)
			}
			if rule.MaxLevelExclusive <= rule.MinLevel {
				return rule, fmt.Errorf(
					"data: %s quest_reward exige maxLevelExclusive maior que minLevel", where)
			}
		}
		if (rule.Action == "refine" || rule.Action == "refine_set") &&
			(rule.RefineMax <= 0 || rule.RefineMax > 15) {
			return rule, fmt.Errorf("data: %s %s exige refineMax entre 1 e 15", where, rule.Action)
		}
		if rule.Action == "refine_equipped" &&
			(rule.RefineMax <= 0 || rule.RefineMax > 15 ||
				rule.TargetSlot < 0 || rule.TargetSlot >= 16 || len(rule.TargetItems) == 0) {
			return rule, fmt.Errorf("data: %s refine_equipped possui alvo/teto invalidos", where)
		}
		if rule.Action == "nightmare_ticket" &&
			(rule.Amount <= 0 || rule.CooldownSeconds <= 0) {
			return rule, fmt.Errorf("data: %s nightmare_ticket exige amount e cooldownSeconds positivos", where)
		}
		if rule.Action == "instance_ticket" {
			i := rule.Instance
			if i == nil || i.ID == "" || i.Name == "" ||
				i.ExitX == 0 || i.ExitY == 0 ||
				(len(i.Stages) == 0 && i.DurationSeconds <= 0) {
				return rule, fmt.Errorf("data: %s instance_ticket possui template incompleto", where)
			}
			if len(i.Stages) == 0 {
				if i.X == 0 || i.Y == 0 || i.SpawnX == 0 || i.SpawnY == 0 ||
					i.AreaRadius <= 0 || len(i.Spawns) == 0 {
					return rule, fmt.Errorf(
						"data: %s instance_ticket possui sala unica incompleta", where)
				}
			}
			validateSpawns := func(label string, spawns []model.VolatileInstanceSpawn) error {
				if len(spawns) == 0 {
					return fmt.Errorf("data: %s %s nao possui spawns", where, label)
				}
				for n, spawn := range spawns {
					if strings.TrimSpace(spawn.NPC) == "" || spawn.Count <= 0 {
						return fmt.Errorf(
							"data: %s %s spawns[%d] invalido", where, label, n)
					}
				}
				return nil
			}
			if len(i.Stages) == 0 {
				if err := validateSpawns("instance_ticket", i.Spawns); err != nil {
					return rule, err
				}
			} else {
				for stageIndex, stage := range i.Stages {
					if stage.X == 0 || stage.Y == 0 || stage.SpawnX == 0 ||
						stage.SpawnY == 0 || stage.AreaRadius <= 0 ||
						(stage.DurationSeconds <= 0 && i.DurationSeconds <= 0) {
						return rule, fmt.Errorf(
							"data: %s instance_ticket stages[%d] incompleto", where, stageIndex)
					}
					if err := validateSpawns(
						fmt.Sprintf("instance_ticket stages[%d]", stageIndex), stage.Spawns); err != nil {
						return rule, err
					}
				}
			}
			for _, evolution := range i.AllowedEvolutions {
				switch strings.ToLower(strings.TrimSpace(evolution)) {
				case "mortal", "arch", "celestial", "subcelestial":
				default:
					return rule, fmt.Errorf(
						"data: %s instance_ticket possui evolucao invalida %q",
						where, evolution)
				}
			}
			if i.RewardItem != 0 {
				if _, exists := items[i.RewardItem]; !exists {
					return rule, fmt.Errorf(
						"data: %s instance_ticket recompensa item inexistente %d",
						where, i.RewardItem)
				}
			}
		}
		if rule.Action == "face_transform" && rule.FaceMesh <= 0 {
			return rule, fmt.Errorf("data: %s face_transform exige faceMesh positivo", where)
		}
		if rule.Action == "tint" && (rule.Color < 116 || rule.Color > 125) {
			return rule, fmt.Errorf("data: %s tint exige color entre 116 e 125", where)
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
	instanceIDs := make(map[string]uint16)
	for itemID, rule := range result.Items {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			continue
		}
		if previous, exists := instanceIDs[rule.Instance.ID]; exists {
			return model.VolatileCatalog{}, fmt.Errorf(
				"data: instancia %q repetida nos itens %d e %d",
				rule.Instance.ID, previous, itemID)
		}
		instanceIDs[rule.Instance.ID] = itemID
	}
	return result, nil
}
