package data

import (
	"fmt"
	"path/filepath"
	"strings"

	"wydgo/internal/model"
)

// instanceGeneratorCache loads each generator file once per instances load.
type instanceGeneratorCache struct {
	dir   string
	files map[string][]model.NPCGener
}

func (c *instanceGeneratorCache) load(file string) ([]model.NPCGener, error) {
	if geners, ok := c.files[file]; ok {
		return geners, nil
	}
	geners, err := LoadNPCGener(filepath.Join(c.dir, filepath.FromSlash(file)))
	if err != nil {
		return nil, fmt.Errorf("data: generator %s: %w", file, err)
	}
	c.files[file] = geners
	return geners, nil
}

// spawns turns generator blocks into instance spawns at their native start
// point and StartRange. Empty ids select every block of the file.
func (c *instanceGeneratorCache) spawns(file string, ids []int, mode string) ([]model.VolatileInstanceSpawn, error) {
	if mode != "" && mode != "initial_group" {
		return nil, fmt.Errorf("data: generator %s has unsupported mode %q", file, mode)
	}
	geners, err := c.load(file)
	if err != nil {
		return nil, err
	}
	byIndex := make(map[int]model.NPCGener, len(geners))
	for _, gener := range geners {
		byIndex[gener.Label] = gener
	}
	selected := geners
	if len(ids) != 0 {
		selected = make([]model.NPCGener, 0, len(ids))
		for _, id := range ids {
			gener, ok := byIndex[id]
			if !ok {
				return nil, fmt.Errorf("data: generator %s has no block #[%d]", file, id)
			}
			selected = append(selected, gener)
		}
	}
	spawns := make([]model.VolatileInstanceSpawn, 0, len(selected))
	for _, gener := range selected {
		start := gener.Segments[0]
		if gener.Leader == "" || gener.MaxNumMob <= 0 || start.X == 0 || start.Y == 0 {
			return nil, fmt.Errorf("data: generator %s block #[%d] needs a leader, MaxNumMob and a start point",
				file, gener.Label)
		}
		spawn := model.VolatileInstanceSpawn{NPC: gener.Leader, Count: gener.MaxNumMob,
			X: start.X, Y: start.Y, Range: int(start.Range)}
		if mode == "initial_group" {
			if !gener.Enabled || gener.MinGroup < 0 || gener.MinGroup != gener.MaxGroup {
				return nil, fmt.Errorf("data: generator %s block #[%d] needs an enabled fixed-size initial group", file, gener.Label)
			}
			count := gener.MaxNumMob
			if gener.MinGroup < count {
				count = 1 + gener.MinGroup
			}
			spawn.Count = count
			if count > 1 && gener.Follower != gener.Leader {
				if strings.TrimSpace(gener.Follower) == "" {
					return nil, fmt.Errorf("data: generator %s block #[%d] needs a follower", file, gener.Label)
				}
				spawn.Count = 1
				spawns = append(spawns, spawn)
				spawn.NPC, spawn.Count = gener.Follower, count-1
			}
		}
		spawns = append(spawns, spawn)
	}
	return spawns, nil
}

// expandInstanceGenerators fills instance and stage spawns from generator
// files. A config may not declare both inline spawns and a generator.
func (c *instanceGeneratorCache) expand(where string, inst *model.VolatileInstance) error {
	if inst == nil {
		return nil
	}
	if inst.Uxmal != nil && inst.Uxmal.Version == 759 {
		if err := c.expandRuneTrack(where, inst.Uxmal); err != nil {
			return err
		}
	}
	if inst.Uxmal != nil && inst.Uxmal.Version != 0 && inst.Uxmal.Version != 759 {
		return fmt.Errorf("data: %s unsupported Rune Track version %d", where, inst.Uxmal.Version)
	}
	if inst.Generator != "" {
		if len(inst.Spawns) != 0 {
			return fmt.Errorf("data: %s declares both spawns and generator", where)
		}
		spawns, err := c.spawns(inst.Generator, nil, "")
		if err != nil {
			return fmt.Errorf("%s: %w", where, err)
		}
		inst.Spawns = spawns
	}
	if boss := inst.NightmareBoss; boss != nil {
		if !strings.EqualFold(inst.NightmareTier, "arcane") || strings.TrimSpace(boss.Spawn.NPC) == "" ||
			boss.Spawn.Count != 1 || boss.Spawn.X == 0 || boss.Spawn.Y == 0 || boss.Spawn.Range < 0 ||
			boss.SecondsRemaining <= 0 || boss.SecondsRemaining >= 900 {
			return fmt.Errorf("data: %s has an invalid Nightmare boss", where)
		}
		for _, spawn := range inst.Spawns {
			if strings.EqualFold(spawn.NPC, boss.Spawn.NPC) {
				return fmt.Errorf("data: %s repeats its Nightmare boss in ordinary spawns", where)
			}
		}
	}
	for i := range inst.Stages {
		stage := &inst.Stages[i]
		if stage.Generator == "" {
			if len(stage.GeneratorIDs) != 0 || len(stage.CompletionGeneratorIDs) != 0 || stage.GeneratorMode != "" {
				return fmt.Errorf("data: %s stage %d selects generator blocks without a generator", where, i)
			}
			continue
		}
		if len(stage.Spawns) != 0 || len(stage.CompletionSpawns) != 0 {
			return fmt.Errorf("data: %s stage %d declares both spawns and generator blocks", where, i)
		}
		if len(stage.GeneratorIDs) == 0 {
			return fmt.Errorf("data: %s stage %d needs generatorIds", where, i)
		}
		spawns, err := c.spawns(stage.Generator, stage.GeneratorIDs, stage.GeneratorMode)
		if err != nil {
			return fmt.Errorf("%s stage %d: %w", where, i, err)
		}
		stage.Spawns = spawns
		if len(stage.CompletionGeneratorIDs) != 0 {
			boss, err := c.spawns(stage.Generator, stage.CompletionGeneratorIDs, stage.GeneratorMode)
			if err != nil {
				return fmt.Errorf("%s stage %d: %w", where, i, err)
			}
			stage.CompletionSpawns = boss
		}
	}
	return c.expandHellGate(where, inst.HellGate)
}

func (c *instanceGeneratorCache) expandRuneTrack(where string, cfg *model.VolatileUxmal) error {
	if len(cfg.RoomGenerators) == 0 {
		if strings.TrimSpace(cfg.Generator) == "" {
			return fmt.Errorf("data: %s Rune Track requires generator files", where)
		}
		generators, err := c.load(cfg.Generator)
		if err != nil {
			return err
		}
		cfg.Generators = generators
		return nil
	}
	if cfg.Generator != "" || len(cfg.RoomGenerators) != 7 {
		return fmt.Errorf("data: %s Rune Track needs seven roomGenerators without a combined generator", where)
	}
	var combined []model.NPCGener
	files, labels := map[string]bool{}, map[int]bool{}
	for room, file := range cfg.RoomGenerators {
		if strings.TrimSpace(file) == "" || files[file] {
			return fmt.Errorf("data: %s Rune Track room %d needs a distinct generator file", where, room)
		}
		files[file] = true
		generators, err := c.load(file)
		if err != nil {
			return err
		}
		if len(generators) == 0 {
			return fmt.Errorf("data: %s Rune Track room %d has no generators", where, room)
		}
		for _, g := range generators {
			if model.RuneTrackGeneratorRoom(g.Label) != room || labels[g.Label] {
				return fmt.Errorf("data: %s Rune Track room %d has misplaced or duplicate generator %d", where, room, g.Label)
			}
			labels[g.Label] = true
			g.Index = len(combined)
			combined = append(combined, g)
		}
	}
	if len(combined) != 327 {
		return fmt.Errorf("data: %s Rune Track needs 327 generators, got %d", where, len(combined))
	}
	cfg.Generators = combined
	return nil
}

// expandHellGate keeps every phase in the same NPCGener file, while allowing
// the state machine to select a room guard, its Lich, or the final wave.
func (c *instanceGeneratorCache) expandHellGate(where string, gate *model.VolatileHellGate) error {
	if gate == nil {
		return nil
	}
	resolve := func(spawn *model.VolatileInstanceSpawn) error {
		if spawn.GeneratorID == 0 {
			if gate.Generator != "" {
				return fmt.Errorf("data: %s Hell Gate spawn needs generatorId", where)
			}
			return nil
		}
		if gate.Generator == "" || spawn.GeneratorID < 0 {
			return fmt.Errorf("data: %s Hell Gate selects a block without a generator", where)
		}
		if spawn.NPC != "" || spawn.Count != 0 || spawn.X != 0 || spawn.Y != 0 || spawn.Range != 0 {
			return fmt.Errorf("data: %s Hell Gate declares both an inline spawn and generatorId", where)
		}
		resolved, err := c.spawns(gate.Generator, []int{spawn.GeneratorID}, "")
		if err != nil {
			return fmt.Errorf("%s Hell Gate: %w", where, err)
		}
		id := spawn.GeneratorID
		*spawn = resolved[0]
		spawn.GeneratorID = id
		return nil
	}
	for i := range gate.Corridors {
		if err := resolve(&gate.Corridors[i]); err != nil {
			return err
		}
	}
	for i := range gate.Quadrants {
		q := &gate.Quadrants[i]
		for j := range q.Spawns {
			if err := resolve(&q.Spawns[j]); err != nil {
				return err
			}
		}
		if err := resolve(&q.Lich); err != nil {
			return err
		}
	}
	for i := range gate.FinalSpawns {
		if err := resolve(&gate.FinalSpawns[i]); err != nil {
			return err
		}
	}
	// Friendly actors have their own configuration and are not hostile waves.
	return validateHellGateVersion(where, gate)
}

func validateHellGateVersion(where string, gate *model.VolatileHellGate) error {
	if population := gate.Population759; population != nil {
		if gate.Version != 3 || len(population.Rooms) != 4 || len(population.LichEscorts) != 4 ||
			len(population.CorridorEscorts) == 0 || len(population.FinalSpawns) == 0 {
			return fmt.Errorf("data: %s Hell Gate historical population requires version 3, four rooms, escorts and attackers", where)
		}
		total := 0
		for i := range population.Rooms {
			if len(population.Rooms[i]) == 0 || len(population.LichEscorts[i]) == 0 {
				return fmt.Errorf("data: %s Hell Gate historical room %d lacks guards or Lich escorts", where, i+1)
			}
		}
		for _, spawn := range population.Spawns() {
			total += spawn.Count
			if spawn.GeneratorID != 0 || strings.TrimSpace(spawn.NPC) == "" || spawn.Count <= 0 ||
				spawn.Count > 64 || spawn.X == 0 || spawn.Y == 0 || spawn.Range < 1 || spawn.Range > 8 ||
				spawn.Formation759 < 0 || spawn.Formation759 > 2 || spawn.Formation759 != 0 && spawn.Count > 12 {
				return fmt.Errorf("data: %s Hell Gate historical population has an invalid inline group", where)
			}
		}
		if total > 1024 {
			return fmt.Errorf("data: %s Hell Gate historical population exceeds the actor budget", where)
		}
	}
	if gate.Version == 0 {
		return nil // Previously persisted configurations retain their old contract.
	}
	if (gate.Version != 2 && gate.Version != 3) || gate.RewardEXP == 0 || len(gate.Corridors) != 3 ||
		len(gate.Quadrants) != 4 || len(gate.FinalSpawns) == 0 ||
		len(gate.FinalNPCs) != 1 || gate.FinalNPCs[0].Count != 1 || len(gate.Portals) != 3 {
		return fmt.Errorf("data: %s Hell Gate timed encounter requires three corridor bosses/portals, four rooms, attackers, one protector and EXP", where)
	}
	for _, spawn := range gate.Corridors {
		if strings.TrimSpace(spawn.NPC) == "" || spawn.Count != 1 || spawn.X == 0 || spawn.Y == 0 || spawn.Range < 0 {
			return fmt.Errorf("data: %s Hell Gate timed encounter has an invalid corridor boss", where)
		}
	}
	seen := make(map[[2]uint16]bool)
	for _, portal := range gate.Portals {
		block := [2]uint16{portal.SourceX / 4, portal.SourceY / 4}
		if portal.SourceX == 0 || portal.SourceY == 0 || portal.DestX == 0 || portal.DestY == 0 ||
			portal.Price != 0 || seen[block] {
			return fmt.Errorf("data: %s Hell Gate timed encounter has an invalid or duplicate portal block", where)
		}
		seen[block] = true
	}
	return nil
}
