package game

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"reflect"
	"sync/atomic"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/npccontent"
	"wydgo/internal/store"
)

const npcDefinitionPollInterval = 10 * time.Second
const npcDefinitionPollTimeout = 3 * time.Second

type npcDefinitionEvent struct {
	entries []store.NPCDefinitionRecord
	poll    bool
}

func (w *World) handleNPCDefinitionEvent(event *npcDefinitionEvent) {
	w.installNPCDefinitionChanges(event.entries)
	if event.poll && w.npcDefinitionState != nil {
		// Only a complete ordered poll may advance the channel checkpoint.
		for _, entry := range event.entries {
			if entry.Channel == w.channel && entry.ChangeID > w.npcDefinitionState.lastChange {
				w.npcDefinitionState.lastChange = entry.ChangeID
			}
		}
		w.npcDefinitionState.polling.Store(false)
	}
}

type npcDefinitionRuntime struct {
	source     map[string]json.RawMessage
	active     map[string]bool
	versions   map[string]uint32
	lastChange int64
	nextPoll   time.Time
	polling    atomic.Bool
}

func (w *World) loadNPCDefinitionsAtBoot(generators []model.NPCGener) error {
	state := &npcDefinitionRuntime{
		source: make(map[string]json.RawMessage, len(w.npcs)), active: make(map[string]bool),
		versions: make(map[string]uint32),
	}
	sources := npccontent.Sources{Volatiles: w.volatiles, Bosses: w.bossCatalog,
		Quests: w.questFile, QuestZones: w.questZones, RuneTrack: w.uxmal}
	for _, group := range generators {
		if group.Enabled && w.npcGeneratorReservation(group) == "" {
			sources.Generators = append(sources.Generators, group)
		}
	}
	if w.colosseum != nil {
		for tier, groups := range w.colosseum.generators {
			for _, group := range groups {
				sources.Colosseum[tier] = append(sources.Colosseum[tier], group.def)
			}
		}
	}
	used, err := npccontent.RequiredNames(w.npcs, sources)
	if err != nil {
		return err
	}
	seeds := make([]store.NPCDefinitionSeed, 0, len(used))
	for i := range w.npcs {
		def := &w.npcs[i]
		raw, err := json.Marshal(def)
		if err != nil {
			return err
		}
		state.source[def.Name] = raw
		if used[def.Name] {
			state.active[def.Name] = true
			seeds = append(seeds, store.NPCDefinitionSeed{Definition: *def, Active: true})
		}
	}
	w.npcDefinitionState = state
	backend, ok := w.store.(store.NPCDefinitionStore)
	if !ok {
		return nil
	}
	ctx, cancel := context.WithTimeout(context.Background(), npcDefinitionPollTimeout)
	defer cancel()
	if err := backend.SyncNPCDefinitions(ctx, w.channel, seeds); err != nil {
		return err
	}
	// Read the checkpoint first. Later commits may appear in the snapshot and
	// replay on the next poll; they must never fall between snapshot and cursor.
	cursor, err := backend.NPCDefinitionChangeCursor(ctx, w.channel)
	if err != nil {
		return err
	}
	entries, err := backend.LoadNPCDefinitions(ctx, w.channel)
	if err != nil {
		return err
	}
	for _, entry := range entries {
		source, err := npccontent.Decode(state.source[entry.Name])
		if err != nil {
			return err
		}
		if _, err := npccontent.ValidateEditable(entry.Definition, source, w.items); err != nil {
			return fmt.Errorf("stored NPC definition %q is invalid: %w", entry.Name, err)
		}
	}
	w.npcDefinitionState.lastChange = cursor
	w.installNPCDefinitionChanges(entries)
	return nil
}

// Share admission policy between reference registration and live population.
func (w *World) npcGeneratorReservation(g model.NPCGener) string {
	if w.generatorReservedForRuneTrack(g) {
		return "rune_track"
	}
	if w.generatorReservedForItemInstance(g) {
		return "item_instance"
	}
	if w.colosseum != nil && w.colosseum.config.Enabled && generatorReservedForColosseum(g) {
		return "colosseum"
	}
	return ""
}

func (w *World) tickNPCDefinitionChanges(now time.Time) {
	state := w.npcDefinitionState
	if state == nil || now.Before(state.nextPoll) || state.polling.Load() {
		return
	}
	backend, ok := w.store.(store.NPCDefinitionStore)
	if !ok {
		return
	}
	state.nextPoll = now.Add(npcDefinitionPollInterval)
	state.polling.Store(true)
	channel, cursor := w.channel, state.lastChange
	go func() {
		ctx, cancel := context.WithTimeout(context.Background(), npcDefinitionPollTimeout)
		defer cancel()
		entries, err := backend.LoadNPCDefinitionChanges(ctx, channel, cursor)
		if err != nil {
			metricNPCDefinitionPollFailures.Add(1)
			log.Printf("NPC definition change poll failed for channel %d: %v", channel, err)
			state.polling.Store(false)
			return
		}
		select {
		case w.commands <- command{npcDefinitions: &npcDefinitionEvent{entries: entries, poll: true}, queuedAt: time.Now()}:
		default:
			metricNPCDefinitionQueueDrops.Add(1)
			state.polling.Store(false)
		}
	}()
}

func (w *World) installNPCDefinitionChanges(entries []store.NPCDefinitionRecord) {
	state := w.npcDefinitionState
	if state == nil {
		return
	}
	for _, entry := range entries {
		if entry.Channel != w.channel {
			continue
		}
		if entry.Version <= state.versions[entry.Name] || !state.active[entry.Name] {
			continue
		}
		source, err := npccontent.Decode(state.source[entry.Name])
		if err != nil {
			log.Printf("NPC source definition %q is invalid in channel %d: %v", entry.Name, w.channel, err)
			continue
		}
		def, err := npccontent.ValidateEditable(entry.Definition, source, w.items)
		if err != nil {
			log.Printf("NPC definition v%d rejected for %q in channel %d: %v", entry.Version, entry.Name, w.channel, err)
			continue
		}
		var target *model.NPCDef
		for i := range w.npcs {
			if w.npcs[i].Name == entry.Name {
				target = &w.npcs[i]
				break
			}
		}
		if target == nil {
			log.Printf("NPC definition v%d ignored; %q is not in channel %d source data", entry.Version, entry.Name, w.channel)
			continue
		}
		w.prepareNPCShopMounts(&def)
		stockChanged := !reflect.DeepEqual(target.Vende, def.Vende)
		if stockChanged {
			w.invalidateNPCShopWindows(entry.Name)
		}
		*target = def // Preserve the address used by every live Mob.Def pointer.
		state.versions[entry.Name] = entry.Version
		w.refreshNPCDefinitionInstances(target)
		observeNPCDefinitionApply(w.channel, entry.UpdatedAt, time.Now())
		log.Printf("NPC definition v%d applied to %q in channel %d", entry.Version, entry.Name, w.channel)
	}
}

func (w *World) refreshNPCDefinitionInstances(def *model.NPCDef) {
	for _, mob := range w.mobsByID {
		if mob == nil || mob.Def != def {
			continue
		}
		if def.Score != nil && mob.HP > def.Score.MaxHP {
			mob.HP = def.Score.MaxHP
		}
		for _, player := range w.nearbyWorldPlayers(mob.X, mob.Y, viewHalfX) {
			if player.hasVisible(mob.ID) {
				w.hideMob(player, mob, 0)
				w.showMob(player, mob)
			}
		}
	}
}

func (w *World) invalidateNPCShopWindows(npcName string) {
	for _, player := range w.players {
		if player == nil || player.ShopNPC == 0 || player.Session == nil || player.Session.IsClosed() {
			continue
		}
		mob := w.mobsByID[player.ShopNPC]
		if mob == nil || mob.Def == nil || mob.Def.Name != npcName || mob.Def.Score == nil {
			continue
		}
		// Keep the advertised grid intact until an in-flight purchase settles.
		// TCP orders old requests before any later explicit shop reopen.
		player.ShopNPC, player.ShopTax = 0, 0
	}
}

func (w *World) worldForNPCChannel(channel uint8) *World {
	if w.channel == channel {
		return w
	}
	if w.cluster != nil {
		for _, candidate := range w.cluster.worlds {
			if candidate != nil && candidate.channel == channel {
				return candidate
			}
		}
	}
	return nil
}

func (w *World) notifyNPCDefinition(entry store.NPCDefinitionRecord) {
	target := w.worldForNPCChannel(entry.Channel)
	if target == nil {
		return
	}
	select {
	case target.commands <- command{npcDefinitions: &npcDefinitionEvent{entries: []store.NPCDefinitionRecord{entry}}, queuedAt: time.Now()}:
	default:
		metricNPCDefinitionQueueDrops.Add(1)
	}
}
