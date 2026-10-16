package data

import (
	"fmt"
	"path/filepath"

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
func (c *instanceGeneratorCache) spawns(file string, ids []int) ([]model.VolatileInstanceSpawn, error) {
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
		spawns = append(spawns, model.VolatileInstanceSpawn{NPC: gener.Leader, Count: gener.MaxNumMob,
			X: start.X, Y: start.Y, Range: int(start.Range)})
	}
	return spawns, nil
}

// expandInstanceGenerators fills instance and stage spawns from generator
// files. A config may not declare both inline spawns and a generator.
func (c *instanceGeneratorCache) expand(where string, inst *model.VolatileInstance) error {
	if inst == nil {
		return nil
	}
	if inst.Generator != "" {
		if len(inst.Spawns) != 0 {
			return fmt.Errorf("data: %s declares both spawns and generator", where)
		}
		spawns, err := c.spawns(inst.Generator, nil)
		if err != nil {
			return fmt.Errorf("%s: %w", where, err)
		}
		inst.Spawns = spawns
	}
	for i := range inst.Stages {
		stage := &inst.Stages[i]
		if stage.Generator == "" {
			if len(stage.GeneratorIDs) != 0 || len(stage.CompletionGeneratorIDs) != 0 {
				return fmt.Errorf("data: %s stage %d selects generator blocks without a generator", where, i)
			}
			continue
		}
		if len(stage.Spawns) != 0 || len(stage.CompletionSpawns) != 0 && len(stage.CompletionGeneratorIDs) != 0 {
			return fmt.Errorf("data: %s stage %d declares both spawns and generator blocks", where, i)
		}
		if len(stage.GeneratorIDs) == 0 {
			return fmt.Errorf("data: %s stage %d needs generatorIds", where, i)
		}
		spawns, err := c.spawns(stage.Generator, stage.GeneratorIDs)
		if err != nil {
			return fmt.Errorf("%s stage %d: %w", where, i, err)
		}
		stage.Spawns = spawns
		if len(stage.CompletionGeneratorIDs) != 0 {
			boss, err := c.spawns(stage.Generator, stage.CompletionGeneratorIDs)
			if err != nil {
				return fmt.Errorf("%s stage %d: %w", where, i, err)
			}
			stage.CompletionSpawns = boss
		}
	}
	return nil
}
