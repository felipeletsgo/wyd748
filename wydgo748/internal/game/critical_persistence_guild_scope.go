package game

import (
	"reflect"
	"slices"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// Per-guild ownership (DOCS/server/server-correction-plan.md, W8). A keyed
// guild plan changes only the guilds, and the registry header, that differ
// between its retained and proposed registries. A guild referenced by a
// changed guild (alliance or war target) joins the scope, so a concurrent
// dissolution of that target conflicts instead of leaving a stale reference.

type criticalGuildPlan struct {
	registry *model.GuildRegistry // live registry pointer when planned
	retained *model.GuildRegistry
	proposed *model.GuildRegistry
	guilds   map[uint16]bool
	header   bool
	keyed    bool
}

func registryHeader(r *model.GuildRegistry) model.GuildRegistry {
	header := *r
	header.Guilds = nil
	return header
}

func guildByID(r *model.GuildRegistry, id uint16) (model.Guild, bool) {
	for _, guild := range r.Guilds {
		if guild.ID == id {
			return guild, true
		}
	}
	return model.Guild{}, false
}

// newCriticalGuildPlan computes the scope of a proposal. keyed false keeps
// the whole-registry semantics for producers that read across guilds.
func newCriticalGuildPlan(registry, retained, proposed *model.GuildRegistry, keyed bool) *criticalGuildPlan {
	plan := &criticalGuildPlan{registry: registry, retained: retained, proposed: proposed,
		guilds: make(map[uint16]bool), keyed: keyed}
	if !keyed {
		return plan
	}
	for _, guild := range retained.Guilds {
		if next, ok := guildByID(proposed, guild.ID); !ok || !reflect.DeepEqual(guild, next) {
			plan.guilds[guild.ID] = true
		}
	}
	for _, guild := range proposed.Guilds {
		if _, ok := guildByID(retained, guild.ID); !ok {
			plan.guilds[guild.ID] = true
		}
	}
	for _, id := range maps2Keys(plan.guilds) {
		if guild, ok := guildByID(proposed, id); ok {
			for _, target := range []uint16{guild.Ally, guild.WarTarget} {
				if target != 0 {
					plan.guilds[target] = true
				}
			}
		}
	}
	plan.header = !reflect.DeepEqual(registryHeader(retained), registryHeader(proposed))
	return plan
}

func maps2Keys(m map[uint16]bool) []uint16 {
	keys := make([]uint16, 0, len(m))
	for key := range m {
		keys = append(keys, key)
	}
	return keys
}

// keys returns the declared participants, or nil for a whole-registry plan.
func (plan *criticalGuildPlan) keys() []store.GuildParticipant {
	if !plan.keyed {
		return nil
	}
	keys := make([]store.GuildParticipant, 0, len(plan.guilds)+1)
	if plan.header {
		keys = append(keys, store.GuildParticipant{})
	}
	for _, id := range maps2Keys(plan.guilds) {
		keys = append(keys, store.GuildParticipant{GuildID: id})
	}
	slices.SortFunc(keys, func(a, b store.GuildParticipant) int { return int(a.GuildID) - int(b.GuildID) })
	return keys
}

// matches reports whether the live registry equals want within the scope.
func (plan *criticalGuildPlan) matches(w *World, want *model.GuildRegistry) bool {
	if w.guilds != plan.registry {
		return false
	}
	live := w.criticalGuildSnapshot()
	if !plan.keyed {
		return reflect.DeepEqual(live, want)
	}
	for id := range plan.guilds {
		a, okA := guildByID(live, id)
		b, okB := guildByID(want, id)
		if okA != okB || okA && !reflect.DeepEqual(a, b) {
			return false
		}
	}
	return !plan.header || reflect.DeepEqual(registryHeader(live), registryHeader(want))
}

// unchanged checks the retained participants before installing a commit.
func (plan *criticalGuildPlan) unchanged(w *World) bool { return plan.matches(w, plan.retained) }

// installed checks the committed participants before publication.
func (plan *criticalGuildPlan) installed(w *World) bool { return plan.matches(w, plan.proposed) }

// install applies the proposal. A keyed plan changes only its participants
// and keeps every other guild's live state.
func (plan *criticalGuildPlan) install(w *World) {
	if !plan.keyed {
		*w.guilds = *plan.proposed
		return
	}
	for id := range plan.guilds {
		next, present := guildByID(plan.proposed, id)
		index := slices.IndexFunc(w.guilds.Guilds, func(g model.Guild) bool { return g.ID == id })
		switch {
		case present && index >= 0:
			w.guilds.Guilds[index] = next
		case present:
			w.guilds.Guilds = append(w.guilds.Guilds, next)
		case index >= 0:
			w.guilds.Guilds = slices.Delete(w.guilds.Guilds, index, index+1)
		}
	}
	if plan.header {
		guilds := w.guilds.Guilds
		*w.guilds = registryHeader(plan.proposed)
		w.guilds.Guilds = guilds
	}
}
