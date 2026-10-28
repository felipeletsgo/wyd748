package game

import (
	"context"
	"encoding/json"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/data"
	"wydgo/internal/store"
)

type npcBatchTestStore struct {
	*craftStore
	store.NPCDefinitionStore
	entries   map[uint8]store.NPCDefinitionRecord
	saves     int
	afterSave func()
}

func (s *npcBatchTestStore) LoadNPCDefinition(_ context.Context, channel uint8, _ string) (store.NPCDefinitionRecord, error) {
	return s.entries[channel], nil
}

func (s *npcBatchTestStore) SaveNPCDefinitions(_ context.Context, updates []store.NPCDefinitionUpdate, _, _ string) ([]store.NPCDefinitionRecord, error) {
	s.saves++
	for _, change := range updates {
		if s.entries[change.Channel].Version != change.ExpectedVersion {
			return nil, store.ErrNPCDefinitionStale
		}
	}
	var entries []store.NPCDefinitionRecord
	for _, change := range updates {
		entry := s.entries[change.Channel]
		entry.Version++
		entry.Definition = change.Definition
		if change.RestoreSource {
			entry.Definition = entry.Source
		}
		entry.Overridden = !change.RestoreSource
		entry.UpdatedAt = time.Now()
		s.entries[change.Channel] = entry
		entries = append(entries, entry)
	}
	if s.afterSave != nil {
		s.afterSave()
	}
	return entries, nil
}

func TestNPCBothChannelsCommitAndAcknowledgeSeparately(t *testing.T) {
	for _, mode := range []string{"applied", "peer_busy", "stale", "invalid_peer", "different_sources", "unavailable", "revoked"} {
		t.Run(mode, func(t *testing.T) {
			first, _, base := handlerTestWorld(t)
			second, _, _ := handlerTestWorld(t)
			defs, err := data.LoadNPCs("../../data/npcs/Aki.json")
			if err != nil {
				t.Fatal(err)
			}
			catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
			if err != nil {
				t.Fatal(err)
			}
			raw, _ := json.Marshal(defs[0])
			backend := &npcBatchTestStore{craftStore: base, entries: map[uint8]store.NPCDefinitionRecord{}}
			cluster := &ChannelCluster{store: backend, worlds: []*World{first, second}}
			for i, w := range cluster.worlds {
				channel, version := uint8(i+1), uint32(1+i*3)
				w.store, w.cluster, w.channel, w.items = backend, cluster, channel, catalog.Items
				w.npcs, w.commands = data.CloneNPCs(defs), make(chan command, 8)
				w.npcDefinitionState = &npcDefinitionRuntime{source: map[string]json.RawMessage{"Aki": raw}, active: map[string]bool{"Aki": true}, versions: map[string]uint32{"Aki": version}}
				backend.entries[channel] = store.NPCDefinitionRecord{Channel: channel, Name: "Aki", Version: version, Active: true, Definition: raw, Source: raw}
			}
			next := data.CloneNPCs(defs)[0]
			next.Gold++
			change, _ := json.Marshal(next)
			cmd := control.NPCDefinitionCommand{Version: 1, Channel: 1, ExpectedVersion: 1, Name: "Aki", Reason: "Edit both shops", Definition: change,
				Targets: []control.NPCDefinitionTarget{{Channel: 2, ExpectedVersion: 4}, {Channel: 1, ExpectedVersion: 1}}}
			wantCode, wantSaves := "ok", 1
			if mode == "peer_busy" {
				wantCode = "saved_pending"
				backend.afterSave = func() { second.staffOps.pending.Store(maxStaffInFlight) }
			}
			if mode == "stale" {
				wantCode = "stale_version"
				cmd.Targets[0].ExpectedVersion = 3
			}
			if mode == "invalid_peer" {
				wantCode, wantSaves = "unknown_npc", 0
				second.npcDefinitionState.active["Aki"] = false
			}
			if mode == "different_sources" {
				wantCode, wantSaves = "invalid_definition", 0
				changed := data.CloneNPCs(defs)[0]
				changed.Gold++
				other, _ := json.Marshal(changed)
				second.npcDefinitionState.source["Aki"] = other
				cmd.RestoreSource, cmd.Definition = true, nil
			}
			if mode == "unavailable" {
				wantCode, wantSaves = "unavailable", 0
				cluster.worlds = []*World{first}
			}
			if mode == "revoked" {
				wantCode, wantSaves = "capability_required", 0
			}
			ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
			defer cancel()
			stop := make(chan struct{})
			stopped := make(chan struct{}, 2)
			for _, w := range []*World{first, second} {
				go func() { defer func() { stopped <- struct{}{} }(); runStaffQueue(w, stop) }()
			}
			calls := 0
			result, err := first.SaveNPCDefinition(ctx, "editor", cmd, func() bool { calls++; return mode != "revoked" || calls == 1 })
			close(stop)
			<-stopped
			<-stopped
			if err != nil || result.Code != wantCode || backend.saves != wantSaves {
				t.Fatalf("code=%s saves=%d err=%v", result.Code, backend.saves, err)
			}
			if mode == "applied" || mode == "peer_busy" {
				if len(result.NPCs) != 2 || result.NPC.Channel != 1 {
					t.Fatalf("missing channel result: %+v", result)
				}
				for _, view := range result.NPCs {
					want := uint32(2)
					if view.Channel == 2 {
						want = 5
					}
					if view.Version != want {
						t.Fatal("lost per-channel version")
					}
					if mode == "peer_busy" && view.Channel == 2 {
						if view.AppliedVersion != 0 {
							t.Fatal("peer falsely confirmed")
						}
						continue
					}
					if view.AppliedVersion != want {
						t.Fatalf("channel %d unconfirmed", view.Channel)
					}
				}
			} else if backend.entries[1].Version != 1 || backend.entries[2].Version != 4 {
				t.Fatal("rejected edit changed persistence")
			}
		})
	}
}

func TestNPCApplicationMetricsKeepChannelAndClockBoundaries(t *testing.T) {
	now := time.Now()
	observeNPCDefinitionApply(1, now.Add(-2*time.Second), now)
	observeNPCDefinitionApply(2, now.Add(time.Second), now)
	if got := metricNPCDefinitionApplyDelayMicros.Get("1").String(); got != "2000000" {
		t.Fatal(got)
	}
	if got := metricNPCDefinitionApplyDelayMicros.Get("2").String(); got != "0" {
		t.Fatal(got)
	}
	observeNPCDefinitionApply(1, time.Time{}, now)
	if got := metricNPCDefinitionApplyDelayMicros.Get("1").String(); got != "2000000" {
		t.Fatal(fmt.Sprintf("zero timestamp replaced metric: %s", got))
	}
}
