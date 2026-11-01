package game

import (
	"errors"
	"expvar"
	"fmt"
	"log"
	"slices"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

// Guild synchronization between game processes (DOCS/server/server-correction-plan.md,
// W9 phase 3). With row versions enabled, every guild write carries the version
// of each row that it changes; a row that another process changed rejects the
// write. A notification from another process, or a rejected write, reloads the
// changed rows. A row whose guild has a write in flight is reloaded after it.

type guildRowStore interface {
	LoadGuildRowStates(ids []uint16) ([]store.GuildRowState, error)
}

var metricGuildSync = expvar.NewMap("world_guild_sync_total")

// guildSyncEvent is a notification (states nil) or a reload result.
type guildSyncEvent struct {
	ids    []uint16
	all    bool
	loaded bool
	states []store.GuildRowState
	err    error
}

// WithGuildRowVersions enables versioned guild writes and reloads. Enable it
// when other game processes write the same guild rows.
func WithGuildRowVersions() WorldOption {
	return func(w *World) { w.guildVersioning = true }
}

// NotifyGuildRows queues a reload of guild rows that another process
// changed; nil ids reloads every row. It may run on any goroutine.
func (w *World) NotifyGuildRows(ids []uint16) {
	w.commands <- command{queuedAt: time.Now(), guildSync: &guildSyncEvent{ids: ids, all: ids == nil}}
}

// loadGuildRowVersions replaces the boot registry with the rows and their
// versions, read together so that no version is newer than its content.
func (w *World) loadGuildRowVersions() error {
	rows, ok := w.store.(guildRowStore)
	if !ok {
		return errors.New("guild row versions need a PostgreSQL store")
	}
	states, err := rows.LoadGuildRowStates(nil)
	if err != nil {
		return err
	}
	registry := &model.GuildRegistry{Version: model.GuildRegistryVersion}
	w.guildVersions = make(map[uint16]int64, len(states))
	for _, state := range states {
		if state.GuildID == 0 {
			registry = state.Header
			registry.Guilds = nil
			w.guildVersions[0] = state.Version
		}
	}
	for _, state := range states {
		if state.GuildID != 0 && state.Present {
			registry.Guilds = append(registry.Guilds, state.Guild)
			w.guildVersions[state.GuildID] = state.Version
		}
	}
	if registry.Guilds == nil {
		registry.Guilds = []model.Guild{}
	}
	if err := registry.Validate(); err != nil {
		return fmt.Errorf("guild rows: %w", err)
	}
	if w.guilds == nil {
		w.guilds = registry
	} else {
		*w.guilds = *registry
	}
	return nil
}

// versionedGuildKeys lists the rows that a guild write changes, each with
// the version this process holds: -1 for a guild without a row, 0 for an
// unknown registry version. A whole-registry write names every known guild.
func (w *World) versionedGuildKeys(write criticalWrite) []store.GuildParticipant {
	ids := make([]uint16, 0, len(write.guildKeys)+1)
	if write.guildKeys != nil {
		for _, key := range write.guildKeys {
			ids = append(ids, key.GuildID)
		}
	} else {
		ids = append(ids, 0)
		for _, guild := range write.guilds.Guilds {
			ids = append(ids, guild.ID)
		}
		for id := range w.guildVersions {
			ids = append(ids, id)
		}
	}
	slices.Sort(ids)
	ids = slices.Compact(ids)
	keys := make([]store.GuildParticipant, 0, len(ids))
	for _, id := range ids {
		version, known := w.guildVersions[id]
		if !known && id != 0 {
			version = -1
		}
		keys = append(keys, store.GuildParticipant{GuildID: id, Version: version})
	}
	return keys
}

// advanceGuildVersions records the versions that a committed write made.
func (w *World) advanceGuildVersions(write criticalWrite) {
	for _, key := range write.guildVersionKeys {
		if key.GuildID != 0 {
			if _, present := guildByID(write.guilds, key.GuildID); !present {
				delete(w.guildVersions, key.GuildID)
				continue
			}
		}
		next := int64(1)
		if key.Version > 0 {
			next = key.Version + 1
		}
		w.guildVersions[key.GuildID] = next
	}
}

// guildConflict reloads the rows of a write that another process changed.
func (w *World) guildConflict(write criticalWrite, err error) {
	if !errors.Is(err, store.ErrGuildRowConflict) {
		return
	}
	metricGuildSync.Add("conflicts", 1)
	ids := make([]uint16, 0, len(write.guildVersionKeys))
	for _, key := range write.guildVersionKeys {
		ids = append(ids, key.GuildID)
	}
	w.requestGuildRefresh(ids, false)
}

func (w *World) requestGuildRefresh(ids []uint16, all bool) {
	if !w.guildVersioning {
		return
	}
	if w.guildRefreshPending == nil {
		w.guildRefreshPending = make(map[uint16]bool)
	}
	w.guildRefreshAll = w.guildRefreshAll || all
	for _, id := range ids {
		w.guildRefreshPending[id] = true
	}
	w.startGuildRefresh()
}

// startGuildRefresh reads the pending rows off the actor, one read at a time.
func (w *World) startGuildRefresh() {
	if w.guildRefreshInFlight || !w.guildRefreshAll && len(w.guildRefreshPending) == 0 {
		return
	}
	rows, ok := w.store.(guildRowStore)
	if !ok {
		return
	}
	var ids []uint16
	all := w.guildRefreshAll
	if !all {
		ids = make([]uint16, 0, len(w.guildRefreshPending))
		for id := range w.guildRefreshPending {
			ids = append(ids, id)
		}
	}
	w.guildRefreshPending, w.guildRefreshAll, w.guildRefreshInFlight = nil, false, true
	go func() {
		states, err := rows.LoadGuildRowStates(ids)
		w.commands <- command{queuedAt: time.Now(), guildSync: &guildSyncEvent{ids: ids, all: all, loaded: true, states: states, err: err}}
	}()
}

func (w *World) onGuildSync(event *guildSyncEvent) {
	if !event.loaded {
		w.requestGuildRefresh(event.ids, event.all)
		return
	}
	w.guildRefreshInFlight = false
	if event.err != nil {
		log.Printf("guild sync: reload failed: %v", event.err)
		w.requestGuildRefreshLater(event.ids, event.all)
		return
	}
	seen := make(map[uint16]bool, len(event.states))
	var deferred []uint16
	for _, state := range event.states {
		seen[state.GuildID] = true
		if !w.installGuildRow(state) {
			deferred = append(deferred, state.GuildID)
		}
	}
	if event.all {
		// A whole reload also removes guilds that no longer have a row.
		for _, guild := range slices.Clone(w.guilds.Guilds) {
			if !seen[guild.ID] && !w.installGuildRow(store.GuildRowState{GuildID: guild.ID}) {
				deferred = append(deferred, guild.ID)
			}
		}
	}
	if len(deferred) > 0 {
		w.requestGuildRefreshLater(deferred, false)
	}
	w.refreshGuildMembers()
}

// refreshGuildMembers repairs the guild fields of the connected characters
// after a reload and sends the changed guild identity to their clients. A
// player with a write in flight is repaired by a later reload or at login.
func (w *World) refreshGuildMembers() {
	for _, p := range w.players {
		if p == nil || !p.InWorld || p.Char == nil || p.Session == nil || w.criticalPlayerReserved(p) {
			continue
		}
		if w.repairGuildState(p.Char) {
			metricGuildSync.Add("members_repaired", 1)
			w.publishGuildIdentity(p)
		}
	}
}

// requestGuildRefreshLater keeps rows pending; the tick retries them.
func (w *World) requestGuildRefreshLater(ids []uint16, all bool) {
	if w.guildRefreshPending == nil {
		w.guildRefreshPending = make(map[uint16]bool)
	}
	w.guildRefreshAll = w.guildRefreshAll || all
	for _, id := range ids {
		w.guildRefreshPending[id] = true
	}
}

// installGuildRow applies a newer row. It returns false while a write of the
// row is in flight; the row is then reloaded after that write.
func (w *World) installGuildRow(state store.GuildRowState) bool {
	if w.criticalGuildWholeReserved() || state.GuildID != 0 && w.criticalGuildIDReserved(state.GuildID) ||
		state.GuildID == 0 && w.criticalGuildReserved() {
		return false
	}
	local, known := w.guildVersions[state.GuildID]
	if state.GuildID == 0 {
		if state.Present && state.Version > local {
			guilds, active := w.guilds.Guilds, w.guilds.ActiveWarChannel
			*w.guilds = *state.Header
			w.guilds.Guilds, w.guilds.ActiveWarChannel = guilds, active
			w.guildVersions[0] = state.Version
			metricGuildSync.Add("installed", 1)
		}
		return true
	}
	index := slices.IndexFunc(w.guilds.Guilds, func(g model.Guild) bool { return g.ID == state.GuildID })
	switch {
	case state.Present && state.Version > local:
		if index >= 0 {
			w.guilds.Guilds[index] = state.Guild
		} else {
			w.guilds.Guilds = append(w.guilds.Guilds, state.Guild)
		}
		w.guildVersions[state.GuildID] = state.Version
		metricGuildSync.Add("installed", 1)
	case !state.Present && (known || index >= 0):
		if index >= 0 {
			w.guilds.Guilds = slices.Delete(w.guilds.Guilds, index, index+1)
		}
		delete(w.guildVersions, state.GuildID)
		metricGuildSync.Add("removed", 1)
	}
	return true
}
