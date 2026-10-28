package game

import (
	"bytes"
	"context"
	"crypto/sha256"
	"encoding/json"
	"errors"
	"fmt"
	"log"
	"strings"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/npccontent"
	"wydgo/internal/store"
)

func npcDefinitionView(entry store.NPCDefinitionRecord) control.NPCDefinitionView {
	def, _ := npccontent.Decode(entry.Definition)
	var updatedAt *time.Time
	if !entry.UpdatedAt.IsZero() {
		updatedAt = &entry.UpdatedAt
	}
	return control.NPCDefinitionView{Name: entry.Name, DisplayName: def.VisibleName(), Channel: entry.Channel,
		Version: entry.Version, Active: entry.Active, Overridden: entry.Overridden,
		Definition: append(json.RawMessage(nil), entry.Definition...), Source: append(json.RawMessage(nil), entry.Source...),
		UpdatedBy: entry.UpdatedBy, UpdatedAt: updatedAt}
}

func (w *World) NPCDefinitions(ctx context.Context, channel uint8) (control.NPCDefinitionCatalogView, error) {
	target := w.worldForNPCChannel(channel)
	if target == nil {
		return control.NPCDefinitionCatalogView{Version: 1, Channel: channel, Available: false, NPCs: []control.NPCDefinitionView{}}, nil
	}
	backend, ok := target.store.(store.NPCDefinitionStore)
	if !ok {
		return control.NPCDefinitionCatalogView{Version: 1, Channel: channel, Available: false, NPCs: []control.NPCDefinitionView{}}, nil
	}
	entries, err := backend.LoadNPCDefinitions(ctx, channel)
	if err != nil {
		return control.NPCDefinitionCatalogView{}, err
	}
	views := make([]control.NPCDefinitionView, 0, len(entries))
	for _, entry := range entries {
		def, err := npccontent.Decode(entry.Definition)
		if err != nil || def.IsMonster() {
			continue
		}
		views = append(views, npcDefinitionView(entry))
	}
	err = target.submitStaff(ctx, "staff.npc-definitions.catalog", func() {
		for i := range views {
			views[i].Active = target.npcDefinitionState.active[views[i].Name]
			views[i].AppliedVersion = target.npcDefinitionState.versions[views[i].Name]
		}
	})
	return control.NPCDefinitionCatalogView{Version: 1, Available: true, Channel: channel, NPCs: views}, err
}

// prepareNPCDefinition captures actor-owned source data before content validation.
func (w *World) prepareNPCDefinition(ctx context.Context, cmd control.NPCDefinitionCommand, expected uint32) (store.NPCDefinitionUpdate, string, error) {
	var baselineRaw json.RawMessage
	code := "unknown_npc"
	err := w.submitStaff(ctx, "staff.npc-definitions.validate", func() {
		if w.npcDefinitionState == nil || !w.npcDefinitionState.active[cmd.Name] {
			return
		}
		baselineRaw = append(json.RawMessage(nil), w.npcDefinitionState.source[cmd.Name]...)
		code = ""
	})
	if err != nil || code != "" {
		return store.NPCDefinitionUpdate{}, code, err
	}
	baseline, err := npccontent.Decode(baselineRaw)
	if err != nil {
		return store.NPCDefinitionUpdate{}, "", fmt.Errorf("decode source NPC %q: %w", cmd.Name, err)
	}
	if baseline.IsMonster() {
		return store.NPCDefinitionUpdate{}, "not_editable", nil
	}
	raw := cmd.Definition
	if cmd.RestoreSource {
		raw = baselineRaw
	}
	validated, err := npccontent.ValidateEditable(raw, baseline, w.items)
	if err != nil {
		return store.NPCDefinitionUpdate{}, "invalid_definition", nil
	}
	definition, err := json.Marshal(validated)
	if err != nil {
		return store.NPCDefinitionUpdate{}, "", err
	}
	change := store.NPCDefinitionUpdate{Channel: w.channel, Name: cmd.Name, ExpectedVersion: expected,
		Definition: definition, RestoreSource: cmd.RestoreSource, ExpectedSourceHash: fmt.Sprintf("%x", sha256.Sum256(baselineRaw))}
	return change, "", nil
}

func (w *World) SaveNPCDefinition(ctx context.Context, actor string, cmd control.NPCDefinitionCommand, authorize func() bool) (control.NPCDefinitionResult, error) {
	if cmd.Validate() != nil {
		return control.NPCDefinitionResult{Code: "invalid_command"}, nil
	}
	if actor == "" || authorize == nil || !authorize() {
		return control.NPCDefinitionResult{Code: "capability_required"}, nil
	}
	targets := cmd.Targets
	if len(targets) == 0 {
		targets = []control.NPCDefinitionTarget{{Channel: cmd.Channel, ExpectedVersion: cmd.ExpectedVersion}}
	}
	worlds := make(map[uint8]*World, len(targets))
	var backend store.NPCDefinitionStore
	for _, requested := range targets {
		target := w.worldForNPCChannel(requested.Channel)
		if target == nil {
			return control.NPCDefinitionResult{Code: "unavailable"}, nil
		}
		candidate, ok := target.store.(store.NPCDefinitionStore)
		if !ok {
			return control.NPCDefinitionResult{Code: "unavailable"}, nil
		}
		// One transaction requires one shared persistence authority.
		if backend != nil && candidate != backend {
			return control.NPCDefinitionResult{Code: "unavailable"}, nil
		}
		backend, worlds[requested.Channel] = candidate, target
	}
	batch, batchAvailable := backend.(store.NPCDefinitionBatchStore)
	if len(targets) > 1 && !batchAvailable {
		return control.NPCDefinitionResult{Code: "unavailable"}, nil
	}
	updates := make([]store.NPCDefinitionUpdate, 0, len(targets))
	var effective json.RawMessage
	for _, requested := range targets {
		change, code, err := worlds[requested.Channel].prepareNPCDefinition(ctx, cmd, requested.ExpectedVersion)
		if err != nil || code != "" {
			return control.NPCDefinitionResult{Code: code}, err
		}
		// Restoring differing sources must not report identical channel content.
		if effective != nil && !bytes.Equal(effective, change.Definition) {
			return control.NPCDefinitionResult{Code: "invalid_definition"}, nil
		}
		effective = change.Definition
		if change.RestoreSource {
			change.Definition = nil
		}
		updates = append(updates, change)
	}
	if !authorize() {
		return control.NPCDefinitionResult{Code: "capability_required"}, nil
	}
	var entries []store.NPCDefinitionRecord
	var err error
	if batchAvailable {
		entries, err = batch.SaveNPCDefinitions(ctx, updates, actor, strings.TrimSpace(cmd.Reason))
	} else {
		change := updates[0]
		var entry store.NPCDefinitionRecord
		entry, err = backend.SaveNPCDefinition(ctx, change.Channel, change.Name, change.ExpectedVersion, change.Definition, change.RestoreSource, actor, strings.TrimSpace(cmd.Reason))
		entries = []store.NPCDefinitionRecord{entry}
	}
	if errors.Is(err, store.ErrNPCDefinitionStale) {
		result := control.NPCDefinitionResult{Code: "stale_version"}
		for _, requested := range targets {
			entry, loadErr := backend.LoadNPCDefinition(ctx, requested.Channel, cmd.Name)
			if loadErr != nil {
				return control.NPCDefinitionResult{}, loadErr
			}
			view := npcDefinitionView(entry)
			if view.Channel == cmd.Channel {
				result.NPC = view
			}
			result.NPCs = append(result.NPCs, view)
		}
		return result, nil
	}
	if err != nil {
		return control.NPCDefinitionResult{}, err
	}
	result := control.NPCDefinitionResult{Code: "ok"}
	for _, entry := range entries {
		target := worlds[entry.Channel]
		view := npcDefinitionView(entry)
		applied := make(chan uint32, 1)
		ackErr := target.submitStaff(ctx, "staff.npc-definitions.apply", func() {
			if !target.shuttingDown {
				target.installNPCDefinitionChanges([]store.NPCDefinitionRecord{entry})
				applied <- target.npcDefinitionState.versions[entry.Name]
			} else {
				applied <- 0
			}
		})
		if ackErr == nil {
			view.AppliedVersion = <-applied
		}
		if ackErr != nil || view.AppliedVersion < entry.Version {
			target.notifyNPCDefinition(entry)
			view.AppliedVersion = 0
			result.Code = "saved_pending"
		}
		if view.Channel == cmd.Channel {
			result.NPC = view
		}
		result.NPCs = append(result.NPCs, view)
		log.Printf("[web-admin] NPC definition v%d saved by %q for %q channel=%d restore_source=%t reason=%q",
			entry.Version, actor, entry.Name, entry.Channel, cmd.RestoreSource, cmd.Reason)
	}
	return result, nil
}

var _ control.NPCDefinitionSource = (*World)(nil)
