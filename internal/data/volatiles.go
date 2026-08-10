package data

import (
	"encoding/json"
	"fmt"
	"os"
	"reflect"
	"strings"

	"wydgo/internal/model"
)

func validateInstanceSpawnShape(where, label string, spawns []model.VolatileInstanceSpawn) error {
	if len(spawns) == 0 {
		return fmt.Errorf("data: %s %s nao possui spawns", where, label)
	}
	for n, spawn := range spawns {
		if strings.TrimSpace(spawn.NPC) == "" || spawn.Count <= 0 ||
			(spawn.X == 0) != (spawn.Y == 0) {
			return fmt.Errorf("data: %s %s spawns[%d] invalido", where, label, n)
		}
	}
	return nil
}

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
	"no_direct_use":   true,
	"celestial_ideal": true, "celestial_fury": true, "celestial_switch": true,
	"celestial_capsule": true,
	"equipment_gem":     true, "ore_upgrade": true,
}

var validMountActions = map[string]bool{
	"feed": true, "essence": true, "longevity": true, "longevity_restore": true,
	"level_set": true, "growth": true, "hatch": true,
}

// LoadVolatiles le apenas as funcoes de data/volatiles.json e descobre os itens
// automaticamente no catalogo. Assim um item novo com EF_VOLATILE entra no
// registro sem manter uma segunda lista manual e sujeita a divergencias.
func LoadVolatiles(path string, items map[uint16]model.ItemDef,
	skills map[int]model.SkillDef) (model.VolatileCatalog, error) {
	var file struct {
		Default       model.VolatileRule                           `json:"default"`
		Rules         map[int]model.VolatileRule                   `json:"rules"`
		Items         map[uint16]model.VolatileRule                `json:"items"`
		Instances     map[string]model.VolatileInstance            `json:"instances"`
		EntryAreaSets map[string][]model.VolatileInstanceEntryArea `json:"entryAreaSets"`
	}
	b, err := os.ReadFile(path)
	if err != nil {
		return model.VolatileCatalog{}, err
	}
	if err := json.Unmarshal(b, &file); err != nil {
		return model.VolatileCatalog{}, fmt.Errorf("data: parse %s: %w", path, err)
	}
	var resolveInstance func(string, map[string]bool) (model.VolatileInstance, error)
	resolveInstance = func(ref string, stack map[string]bool) (model.VolatileInstance, error) {
		raw, exists := file.Instances[ref]
		if !exists {
			return model.VolatileInstance{}, fmt.Errorf("data: referencia instancia inexistente %q", ref)
		}
		if stack[ref] {
			return model.VolatileInstance{}, fmt.Errorf("data: ciclo em baseRef de instancia %q", ref)
		}
		if strings.TrimSpace(raw.BaseRef) == "" {
			return raw, nil
		}
		stack[ref] = true
		base, err := resolveInstance(strings.TrimSpace(raw.BaseRef), stack)
		delete(stack, ref)
		if err != nil {
			return model.VolatileInstance{}, err
		}
		// Variantes podem sobrescrever qualquer campo escalar que tenha valor e
		// qualquer tabela que tenha sido declarada. Campos zero permanecem os do
		// template base, que e o objetivo do alias de Cube.
		if raw.ID != "" {
			base.ID = raw.ID
		}
		if raw.Name != "" {
			base.Name = raw.Name
		}
		if raw.X != 0 {
			base.X = raw.X
		}
		if raw.Y != 0 {
			base.Y = raw.Y
		}
		if raw.SpawnX != 0 {
			base.SpawnX = raw.SpawnX
		}
		if raw.SpawnY != 0 {
			base.SpawnY = raw.SpawnY
		}
		if raw.AreaRadius != 0 {
			base.AreaRadius = raw.AreaRadius
		}
		if raw.MaxPlayers != 0 {
			base.MaxPlayers = raw.MaxPlayers
		}
		if len(raw.Spawns) > 0 {
			base.Spawns = raw.Spawns
		}
		if raw.RewardItem != 0 {
			base.RewardItem = raw.RewardItem
		}
		if raw.ChainNextItem != 0 {
			base.ChainNextItem = raw.ChainNextItem
		}
		if raw.ChainStart {
			base.ChainStart = true
		}
		if len(raw.AllowedEvolutions) > 0 {
			base.AllowedEvolutions = raw.AllowedEvolutions
		}
		if raw.DurationSeconds != 0 {
			base.DurationSeconds = raw.DurationSeconds
		}
		if raw.TotalDurationSeconds != 0 {
			base.TotalDurationSeconds = raw.TotalDurationSeconds
		}
		if raw.ActiveDurationSeconds != 0 {
			base.ActiveDurationSeconds = raw.ActiveDurationSeconds
		}
		if raw.ExitX != 0 {
			base.ExitX = raw.ExitX
		}
		if raw.ExitY != 0 {
			base.ExitY = raw.ExitY
		}
		if raw.TransitionSeconds != 0 {
			base.TransitionSeconds = raw.TransitionSeconds
		}
		if raw.SharedEntry {
			base.SharedEntry = true
		}
		if raw.Mode != "" {
			base.Mode = raw.Mode
		}
		if raw.SharedGroup != "" {
			base.SharedGroup = raw.SharedGroup
		}
		if raw.ExclusiveGroup != "" {
			base.ExclusiveGroup = raw.ExclusiveGroup
		}
		if raw.FinishPolicy != "" {
			base.FinishPolicy = raw.FinishPolicy
		}
		if raw.StateMachine != "" {
			base.StateMachine = raw.StateMachine
		}
		if raw.HellGate != nil {
			base.HellGate = raw.HellGate
		}
		if raw.Uxmal != nil {
			uxmal := *raw.Uxmal
			uxmal.EntryAreas = append([]model.VolatileInstanceEntryArea(nil), raw.Uxmal.EntryAreas...)
			uxmal.RoomPositions = make([][]model.VolatileDestination, len(raw.Uxmal.RoomPositions))
			for index := range raw.Uxmal.RoomPositions {
				uxmal.RoomPositions[index] = append([]model.VolatileDestination(nil), raw.Uxmal.RoomPositions[index]...)
			}
			uxmal.MaxParties = append([]int(nil), raw.Uxmal.MaxParties...)
			uxmal.Runes = make([][]uint16, len(raw.Uxmal.Runes))
			for index := range raw.Uxmal.Runes {
				uxmal.Runes[index] = append([]uint16(nil), raw.Uxmal.Runes[index]...)
			}
			uxmal.TicketNextSanc = append([]int(nil), raw.Uxmal.TicketNextSanc...)
			base.Uxmal = &uxmal
		}
		if raw.AllowChainDuringExitGrace {
			base.AllowChainDuringExitGrace = true
		}
		if len(raw.EntryAreas) > 0 {
			base.EntryAreas = raw.EntryAreas
		}
		if raw.EntryAreaSet != "" {
			base.EntryAreaSet = raw.EntryAreaSet
		}
		if len(raw.Schedule) > 0 {
			base.Schedule = raw.Schedule
		}
		if raw.PartyRunLimit != 0 {
			base.PartyRunLimit = raw.PartyRunLimit
		}
		if raw.NightmareTier != "" {
			base.NightmareTier = raw.NightmareTier
		}
		if len(raw.Stages) > 0 {
			base.Stages = raw.Stages
		}
		base.BaseRef = ""
		return base, nil
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
		rule.InstanceRef = strings.TrimSpace(rule.InstanceRef)
		if rule.Instance == nil && rule.InstanceRef != "" {
			instance, err := resolveInstance(rule.InstanceRef, make(map[string]bool))
			if err != nil {
				return rule, fmt.Errorf("data: %s: %w", where, err)
			}
			rule.Instance = &instance
		}
		if rule.Instance != nil {
			setName := strings.TrimSpace(rule.Instance.EntryAreaSet)
			if setName != "" {
				areas, exists := file.EntryAreaSets[setName]
				if !exists || len(areas) == 0 {
					return rule, fmt.Errorf(
						"data: %s referencia entryAreaSet inexistente %q", where, setName)
				}
				if len(rule.Instance.EntryAreas) != 0 {
					return rule, fmt.Errorf(
						"data: %s combina entryAreaSet com entryAreas", where)
				}
				rule.Instance.EntryAreas = append([]model.VolatileInstanceEntryArea(nil), areas...)
			}
			if rule.Instance.ChainStart &&
				!strings.HasPrefix(strings.ToLower(strings.TrimSpace(rule.Instance.ID)), "water-") {
				return rule, fmt.Errorf("data: %s chainStart so pode ser usado por instancia Water", where)
			}
		}
		if !validVolatileActions[rule.Action] {
			return rule, fmt.Errorf("data: %s possui action desconhecida %q", where, rule.Action)
		}
		if rule.CustomPattern && (rule.Action != "firework" || !rule.Consume) {
			return rule, fmt.Errorf(
				"data: %s customPattern exige firework com consume=true", where)
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
		if rule.Action == "buff" {
			hasSingle := rule.AffectType > 0
			hasPackage := len(rule.Affects) > 0
			if hasSingle == hasPackage {
				return rule, fmt.Errorf(
					"data: %s buff exige exatamente affectType ou affects", where)
			}
			if hasSingle && (rule.AffectType > 255 || rule.DurationUnits <= 0) {
				return rule, fmt.Errorf(
					"data: %s buff singular possui affectType/durationUnits invalido", where)
			}
			for index, affect := range rule.Affects {
				hasType := affect.Type > 0 && affect.Type <= 255
				hasSkill := affect.SkillID > 0
				if hasType == hasSkill || affect.DurationUnits <= 0 {
					return rule, fmt.Errorf(
						"data: %s affects[%d] invalido", where, index)
				}
				if hasSkill {
					skill, exists := skills[affect.SkillID]
					if !exists || skill.AffectType <= 0 || skill.AffectType > 255 {
						return rule, fmt.Errorf(
							"data: %s affects[%d] referencia skill %d sem affect",
							where, index, affect.SkillID)
					}
				}
			}
		}
		if rule.Accumulate && rule.MaxDurationUnits <= 0 {
			return rule, fmt.Errorf("data: %s accumulate exige maxDurationUnits positivo", where)
		}
		if rule.Action == "grant_exp" && rule.Exp == 0 {
			return rule, fmt.Errorf("data: %s grant_exp exige exp positivo", where)
		}
		if rule.Action == "equipment_gem" &&
			(rule.Variant < 0 || rule.Variant > 3 || !rule.Consume) {
			return rule, fmt.Errorf("data: %s equipment_gem exige variant 0..3 e consume=true", where)
		}
		if rule.Action == "ore_upgrade" &&
			(rule.Variant < 0 || rule.Variant > 3 || rule.SuccessPercent < 1 ||
				rule.SuccessPercent > 100 || !rule.Consume) {
			return rule, fmt.Errorf("data: %s ore_upgrade exige variant 0..3, chance 1..100 e consume=true", where)
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
		if rule.OnceQuestID != 0 && rule.Action != "refine_set" {
			return rule, fmt.Errorf("data: %s onceQuestId so pode ser usado por refine_set", where)
		}
		if rule.Action == "refine_set" {
			if !rule.MortalOnly || rule.OnceQuestID == 0 {
				return rule, fmt.Errorf("data: %s refine_set exige mortalOnly e onceQuestId", where)
			}
			if rule.MaxLevelExclusive != 0 && rule.MaxLevelExclusive <= rule.MinLevel {
				return rule, fmt.Errorf("data: %s refine_set possui faixa de nivel invalida", where)
			}
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
			switch tier := strings.ToLower(strings.TrimSpace(i.NightmareTier)); tier {
			case "", "normal", "mystic", "arcane":
			default:
				return rule, fmt.Errorf("data: %s nightmareTier invalido %q", where, i.NightmareTier)
			}
			// Nightmare allows three native parties of up to thirteen members
			// in the same timed zone. Private instances keep their own lower
			// limit in data.
			if i.MaxPlayers < 0 || i.MaxPlayers > 39 {
				return rule, fmt.Errorf(
					"data: %s instance_ticket possui maxPlayers fora de 0..39", where)
			}
			if i.SharedEntry && i.MaxPlayers <= 0 {
				return rule, fmt.Errorf(
					"data: %s instance_ticket compartilhada exige maxPlayers positivo", where)
			}
			if i.TotalDurationSeconds < 0 {
				return rule, fmt.Errorf(
					"data: %s instance_ticket possui totalDurationSeconds negativo", where)
			}
			if i.ActiveDurationSeconds < 0 {
				return rule, fmt.Errorf(
					"data: %s instance_ticket possui activeDurationSeconds negativo", where)
			}
			if i.PartyRunLimit < 0 || i.PartyRunLimit > 1000 {
				return rule, fmt.Errorf(
					"data: %s instance_ticket possui partyRunLimit invalido", where)
			}
			switch mode := strings.ToLower(strings.TrimSpace(i.Mode)); mode {
			case "", "private_chain", "private_shared_entry", "shared_timed_zone", "state_machine":
			default:
				return rule, fmt.Errorf("data: %s possui mode de instancia invalido %q", where, i.Mode)
			}
			switch policy := strings.ToLower(strings.TrimSpace(i.FinishPolicy)); policy {
			case "", "finish_on_clear", "finish_on_timeout", "advance_on_clear", "respawn_until_timeout", "state_machine":
			default:
				return rule, fmt.Errorf("data: %s possui finishPolicy invalido %q", where, i.FinishPolicy)
			}
			if i.Mode == "shared_timed_zone" && strings.TrimSpace(i.SharedGroup) == "" {
				return rule, fmt.Errorf("data: %s shared_timed_zone exige sharedGroup", where)
			}
			switch machine := strings.ToLower(strings.TrimSpace(i.StateMachine)); machine {
			case "":
			case "hell_gate":
				if i.HellGate == nil || strings.TrimSpace(i.HellGate.ControllerNPC) == "" ||
					len(i.HellGate.Quadrants) != 4 {
					return rule, fmt.Errorf("data: %s hell_gate exige controlador e quatro quadrantes", where)
				}
				seenQuadrants := make(map[int]struct{}, len(i.HellGate.Quadrants))
				for q, quadrant := range i.HellGate.Quadrants {
					if quadrant.ID < 1 || quadrant.ID > 4 || quadrant.X == 0 || quadrant.Y == 0 ||
						quadrant.SpawnX == 0 || quadrant.SpawnY == 0 || quadrant.AreaRadius <= 0 ||
						quadrant.Lich.NPC == "" || quadrant.Lich.Count != 1 {
						return rule, fmt.Errorf("data: %s hell_gate quadrante[%d] incompleto", where, q)
					}
					if _, duplicate := seenQuadrants[quadrant.ID]; duplicate {
						return rule, fmt.Errorf("data: %s hell_gate quadrante[%d] duplicado", where, q)
					}
					seenQuadrants[quadrant.ID] = struct{}{}
					if err := validateInstanceSpawnShape(where, fmt.Sprintf("hell_gate quadrants[%d]", q), quadrant.Spawns); err != nil {
						return rule, err
					}
					if err := validateInstanceSpawnShape(where,
						fmt.Sprintf("hell_gate quadrants[%d].lich", q),
						[]model.VolatileInstanceSpawn{quadrant.Lich}); err != nil {
						return rule, err
					}
				}
				for id := 1; id <= 4; id++ {
					if _, ok := seenQuadrants[id]; !ok {
						return rule, fmt.Errorf("data: %s hell_gate quadrante %d ausente", where, id)
					}
				}
				if len(i.HellGate.FinalSpawns) == 0 && len(i.HellGate.FinalNPCs) == 0 {
					return rule, fmt.Errorf("data: %s hell_gate exige onda ou NPC final", where)
				}
				if len(i.HellGate.FinalSpawns) > 0 {
					if err := validateInstanceSpawnShape(where, "hell_gate finalSpawns", i.HellGate.FinalSpawns); err != nil {
						return rule, err
					}
				}
				if len(i.HellGate.FinalNPCs) > 0 {
					if err := validateInstanceSpawnShape(where, "hell_gate finalNPCs", i.HellGate.FinalNPCs); err != nil {
						return rule, err
					}
				}
			case "big_cube":
				// Big Cube is an O/X state machine rather than a combat wave. Its
				// questions and platform coordinates are still data-driven, but a
				// stage is allowed to have zero NPC spawns; the game opens the quiz
				// immediately after the ticket commit.
				if len(i.Stages) == 0 || !i.NoCombatTimeout {
					return rule, fmt.Errorf("data: %s big_cube exige stages e noCombatTimeout", where)
				}
				for stageIndex, stage := range i.Stages {
					if stage.Quiz == nil {
						return rule, fmt.Errorf("data: %s big_cube stages[%d] exige quiz", where, stageIndex)
					}
					if len(stage.Spawns) > 0 {
						return rule, fmt.Errorf("data: %s big_cube stages[%d] nao pode possuir mobs", where, stageIndex)
					}
				}
			case "uxmal":
				if i.Uxmal == nil || strings.TrimSpace(i.Uxmal.NPC) == "" ||
					i.Uxmal.TicketItem == 0 || len(i.Uxmal.RoomPositions) != 7 ||
					len(i.Uxmal.MaxParties) != 7 || len(i.Uxmal.Runes) != 7 ||
					len(i.Uxmal.TicketNextSanc) != 7 || len(i.Stages) != 7 {
					return rule, fmt.Errorf("data: %s uxmal exige NPC, ticket e sete salas completas", where)
				}
				if _, exists := items[i.Uxmal.TicketItem]; !exists {
					return rule, fmt.Errorf("data: %s uxmal ticket %d ausente no catalogo", where, i.Uxmal.TicketItem)
				}
				for areaIndex, area := range i.Uxmal.EntryAreas {
					if area.MinX == 0 || area.MinY == 0 || area.MaxX < area.MinX || area.MaxY < area.MinY {
						return rule, fmt.Errorf("data: %s uxmal entryAreas[%d] invalida", where, areaIndex)
					}
				}
				for room := range i.Stages {
					if len(i.Uxmal.RoomPositions[room]) != 3 || i.Uxmal.MaxParties[room] < 1 || i.Uxmal.MaxParties[room] > 3 ||
						len(i.Uxmal.Runes[room]) == 0 || i.Uxmal.TicketNextSanc[room] < 0 || i.Uxmal.TicketNextSanc[room] > 6 {
						return rule, fmt.Errorf("data: %s uxmal sala[%d] possui tabela invalida", where, room)
					}
					for slot, position := range i.Uxmal.RoomPositions[room] {
						if position.X == 0 || position.Y == 0 {
							return rule, fmt.Errorf("data: %s uxmal sala[%d] posicao[%d] invalida", where, room, slot)
						}
					}
					for _, itemID := range i.Uxmal.Runes[room] {
						if itemID == 0 {
							return rule, fmt.Errorf("data: %s uxmal sala[%d] possui rune zero", where, room)
						}
						if _, exists := items[itemID]; !exists {
							return rule, fmt.Errorf("data: %s uxmal rune %d ausente no catalogo", where, itemID)
						}
					}
				}
			default:
				return rule, fmt.Errorf("data: %s stateMachine invalida %q", where, i.StateMachine)
			}
			if i.Mode == "state_machine" && strings.TrimSpace(i.SharedGroup) != "" &&
				i.ExclusiveGroup == strings.TrimSpace(i.SharedGroup) {
				return rule, fmt.Errorf("data: %s nao pode reutilizar sharedGroup como exclusiveGroup", where)
			}
			for areaIndex, area := range i.EntryAreas {
				if area.MinX == 0 || area.MinY == 0 || area.MaxX < area.MinX ||
					area.MaxY < area.MinY {
					return rule, fmt.Errorf(
						"data: %s entryAreas[%d] invalida", where, areaIndex)
				}
			}
			for windowIndex, window := range i.Schedule {
				if window.StartMinute < 0 || window.StartMinute > 59 ||
					window.DurationSeconds <= 0 || window.DurationSeconds > 3600 {
					return rule, fmt.Errorf(
						"data: %s schedule[%d] invalida", where, windowIndex)
				}
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
					if (spawn.X == 0) != (spawn.Y == 0) {
						return fmt.Errorf(
							"data: %s %s spawns[%d] possui coordenada parcial",
							where, label, n)
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
						(stage.DurationSeconds <= 0 && i.DurationSeconds <= 0 && !i.NoCombatTimeout) {
						return rule, fmt.Errorf(
							"data: %s instance_ticket stages[%d] incompleto", where, stageIndex)
					}
					if !strings.EqualFold(strings.TrimSpace(i.StateMachine), "big_cube") {
						if err := validateSpawns(
							fmt.Sprintf("instance_ticket stages[%d]", stageIndex), stage.Spawns); err != nil {
							return rule, err
						}
					}
					if len(stage.CompletionSpawns) > 0 {
						if err := validateSpawns(
							fmt.Sprintf("instance_ticket stages[%d].completionSpawns", stageIndex),
							stage.CompletionSpawns); err != nil {
							return rule, err
						}
					}
					if stage.Quiz != nil {
						q := stage.Quiz
						if strings.TrimSpace(q.Question) == "" || q.DurationSeconds <= 0 ||
							q.TrueX == 0 || q.TrueY == 0 || q.FalseX == 0 || q.FalseY == 0 {
							return rule, fmt.Errorf(
								"data: %s instance_ticket stages[%d] possui quiz incompleto",
								where, stageIndex)
						}
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
			if i.ChainNextItem != 0 {
				if _, exists := items[i.ChainNextItem]; !exists {
					return rule, fmt.Errorf(
						"data: %s instance_ticket chainNextItem inexistente %d",
						where, i.ChainNextItem)
				}
				if i.ChainNextItem == i.RewardItem {
					return rule, fmt.Errorf(
						"data: %s instance_ticket chainNextItem duplica rewardItem %d",
						where, i.ChainNextItem)
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
				return rule, fmt.Errorf("data: %s mount exige mountAction valido, veio %q", where, rule.MountAction)
			}
			if rule.MountAction == "longevity_restore" && rule.Amount <= 0 {
				return rule, fmt.Errorf("data: %s mount longevity_restore exige amount positivo", where)
			}
			if rule.MountAction == "level_set" &&
				(rule.Amount <= 0 || rule.Amount > model.MountMaxLevel ||
					rule.MountMinLevel < 0 || rule.MountMinLevel >= rule.Amount) {
				return rule, fmt.Errorf("data: %s mount level_set possui nivel alvo/minimo invalido", where)
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
		Instances: make(map[string]model.VolatileInstance),
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
	// Instancias nomeadas tambem podem ser abertas por um NPC. Materialize-as
	// com o mesmo resolvedor de BaseRef e passe-as pelo contrato de instancia
	// sem exigir que exista um item com EF_VOLATILE apontando para elas.
	for name := range file.Instances {
		instance, resolveErr := resolveInstance(name, make(map[string]bool))
		if resolveErr != nil {
			return model.VolatileCatalog{}, fmt.Errorf("data: instances[%q]: %w", name, resolveErr)
		}
		normalized, normalizeErr := normalize(fmt.Sprintf("instances[%q]", name), model.VolatileRule{
			Action: "instance_ticket", Instance: &instance,
		})
		if normalizeErr != nil {
			return model.VolatileCatalog{}, normalizeErr
		}
		result.Instances[name] = *normalized.Instance
	}
	// Vários ingressos podem compartilhar a mesma sala (Cube Mystic/Arcane,
	// Hell Gate solo/party), mas o mesmo ID nunca pode apontar para dois
	// layouts diferentes: o mapa runtime é indexado por esse ID.
	instanceDefs := make(map[string]model.VolatileInstance)
	checkInstance := func(where string, rule model.VolatileRule) error {
		if rule.Action != "instance_ticket" || rule.Instance == nil {
			return nil
		}
		if previous, exists := instanceDefs[rule.Instance.ID]; exists {
			if !reflect.DeepEqual(previous, *rule.Instance) {
				return fmt.Errorf(
					"data: instancia %q possui configuracoes divergentes (%s)",
					rule.Instance.ID, where)
			}
			return nil
		}
		instanceDefs[rule.Instance.ID] = *rule.Instance
		return nil
	}
	for code, rule := range result.Rules {
		if err := checkInstance(fmt.Sprintf("rules[%d]", code), rule); err != nil {
			return model.VolatileCatalog{}, err
		}
	}
	for id, rule := range result.Items {
		if err := checkInstance(fmt.Sprintf("items[%d]", id), rule); err != nil {
			return model.VolatileCatalog{}, err
		}
	}
	// ChainNextItem is a progression edge, not just an item existence check.
	// Resolve its target through the same catalog that the runtime uses and
	// reject cross-event/cross-tier edges at load time. This keeps a malformed
	// data edit from turning a Water boss ticket into a shortcut to another
	// tier or event.
	waterFamily := func(id string) string {
		parts := strings.Split(id, "-")
		if len(parts) >= 2 && parts[0] == "water" {
			return parts[0] + "-" + parts[1]
		}
		return ""
	}
	checkChain := func(where string, rule model.VolatileRule) error {
		if rule.Action != "instance_ticket" || rule.Instance == nil ||
			rule.Instance.ChainNextItem == 0 {
			return nil
		}
		target, _, ok := result.Rule(rule.Instance.ChainNextItem)
		if !ok || target.Action != "instance_ticket" || target.Instance == nil {
			return fmt.Errorf("data: %s chainNextItem %d nao e instance_ticket", where,
				rule.Instance.ChainNextItem)
		}
		fromFamily, toFamily := waterFamily(rule.Instance.ID), waterFamily(target.Instance.ID)
		if fromFamily != "" && fromFamily != toFamily {
			return fmt.Errorf("data: %s chainNextItem %d cruza tier Water (%s -> %s)",
				where, rule.Instance.ChainNextItem, fromFamily, toFamily)
		}
		return nil
	}
	for code, rule := range result.Rules {
		if err := checkChain(fmt.Sprintf("rules[%d]", code), rule); err != nil {
			return model.VolatileCatalog{}, err
		}
	}
	for id, rule := range result.Items {
		if err := checkChain(fmt.Sprintf("items[%d]", id), rule); err != nil {
			return model.VolatileCatalog{}, err
		}
	}
	return result, nil
}
