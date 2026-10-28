// Package npccontent validates authored NPC content without owning live entities.
package npccontent

import (
	"fmt"
	"sort"
	"strings"

	"wydgo/internal/model"
)

// Sources contains only content fields with explicit NPC reference semantics.
type Sources struct {
	Generators []model.NPCGener
	Volatiles  model.VolatileCatalog
	Bosses     model.BossCatalog
	Quests     model.QuestFile
	QuestZones []model.QuestZone
	Colosseum  model.ColosseumGenerators
	RuneTrack  *model.VolatileInstance
}

type Reference struct {
	Name      string
	Source    string
	Generator bool
}

// References excludes display names, dialogue, identifiers, and other metadata.
func References(s Sources) []Reference {
	var refs []Reference
	add := func(name, source string) {
		if name != "" {
			refs = append(refs, Reference{Name: name, Source: source})
		}
	}
	generators := func(groups []model.NPCGener, source string) {
		for _, g := range groups {
			if !g.Enabled {
				continue
			}
			add(g.Leader, fmt.Sprintf("%s[%d].leader", source, g.Index))
			if g.Leader != "" {
				refs[len(refs)-1].Generator = true
			}
			add(g.Follower, fmt.Sprintf("%s[%d].follower", source, g.Index))
			if g.Follower != "" {
				refs[len(refs)-1].Generator = true
			}
		}
	}
	spawns := func(groups []model.VolatileInstanceSpawn, source string) {
		for i, g := range groups {
			add(g.NPC, fmt.Sprintf("%s[%d].npc", source, i))
		}
	}
	instance := func(inst *model.VolatileInstance, source string) {
		if inst == nil {
			return
		}
		spawns(inst.Spawns, source+".spawns")
		spawns(inst.NightmareProtectors, source+".nightmareProtectors")
		if inst.NightmareBoss != nil {
			add(inst.NightmareBoss.Spawn.NPC, source+".nightmareBoss.spawn.npc")
		}
		for i, stage := range inst.Stages {
			where := fmt.Sprintf("%s.stages[%d]", source, i)
			spawns(stage.Spawns, where+".spawns")
			spawns(stage.CompletionSpawns, where+".completionSpawns")
			spawns(stage.RuneBonusSpawns, where+".runeBonusSpawns")
		}
		if u := inst.Uxmal; u != nil {
			add(u.NPC, source+".uxmal.npc")
			generators(u.Generators, source+".uxmal.generators")
		}
		if gate := inst.HellGate; gate != nil {
			add(gate.ControllerNPC, source+".hellGate.controllerNPC")
			spawns(gate.Corridors, source+".hellGate.corridors")
			spawns(gate.FinalSpawns, source+".hellGate.finalSpawns")
			spawns(gate.FinalNPCs, source+".hellGate.finalNPCs")
			for i, q := range gate.Quadrants {
				where := fmt.Sprintf("%s.hellGate.quadrants[%d]", source, i)
				spawns(q.Spawns, where+".spawns")
				add(q.Lich.NPC, where+".lich.npc")
			}
			spawns(gate.Population759.Spawns(), source+".hellGate.population759")
		}
	}
	generators(s.Generators, "NPCGener")
	for tier, groups := range s.Colosseum {
		generators(groups, fmt.Sprintf("colosseum[%d]", tier))
	}
	for _, boss := range s.Bosses.Bosses {
		where := fmt.Sprintf("boss[%s]", boss.ID)
		add(boss.NPC, where+".npc")
		for i, summon := range boss.Summons {
			add(summon.NPC, fmt.Sprintf("%s.summons[%d].npc", where, i))
		}
	}
	for _, quest := range s.Quests.Quests {
		add(quest.NPC, fmt.Sprintf("quest[%d].npc", quest.ID))
	}
	for i, zone := range s.QuestZones {
		add(zone.NPC, fmt.Sprintf("questZones[%d].npc", i))
	}
	instance(s.Volatiles.Default.Instance, "volatiles.default.instance")
	instance(s.RuneTrack, "runeTrack")
	for code, rule := range s.Volatiles.Rules {
		instance(rule.Instance, fmt.Sprintf("volatiles.rules[%d].instance", code))
	}
	for item, rule := range s.Volatiles.Items {
		instance(rule.Instance, fmt.Sprintf("volatiles.items[%d].instance", item))
	}
	for name, inst := range s.Volatiles.Instances {
		instance(&inst, fmt.Sprintf("instances[%s]", name))
	}
	sort.Slice(refs, func(i, j int) bool {
		if refs[i].Source == refs[j].Source {
			return refs[i].Name < refs[j].Name
		}
		return refs[i].Source < refs[j].Source
	})
	return refs
}

// RequiredNames rejects missing templates before database synchronization or spawn.
func RequiredNames(npcs []model.NPCDef, sources Sources) (map[string]bool, error) {
	known := make(map[string]bool, len(npcs))
	aliases := make(map[string]string, len(npcs))
	for _, npc := range npcs {
		known[npc.Name] = true
		alias := strings.ReplaceAll(npc.Name, "_", " ")
		if previous, exists := aliases[alias]; exists && previous != npc.Name {
			aliases[alias] = ""
		} else {
			aliases[alias] = npc.Name
		}
	}
	used := make(map[string]bool)
	for _, ref := range References(sources) {
		name := ref.Name
		if !known[name] && ref.Generator {
			name = aliases[strings.ReplaceAll(name, "_", " ")]
		}
		if !known[name] {
			return nil, fmt.Errorf("%s: missing template %q", ref.Source, ref.Name)
		}
		used[name] = true
	}
	return used, nil
}
