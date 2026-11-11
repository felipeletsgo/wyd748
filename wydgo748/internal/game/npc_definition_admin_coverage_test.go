package game

import (
	"context"
	"encoding/json"
	"testing"

	"wydgo/internal/control"
	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

type npcDefinitionCatalogCoverageStore struct {
	*craftStore
	store.NPCDefinitionStore
	entries []store.NPCDefinitionRecord
}

func (s *npcDefinitionCatalogCoverageStore) LoadNPCDefinitions(context.Context, uint8) ([]store.NPCDefinitionRecord, error) {
	return s.entries, nil
}

func TestNPCDefinitionsCatalogFiltersAndAppliesWorldState(t *testing.T) {
	defs, err := data.LoadNPCs("../../data/npcs/Aki.json")
	if err != nil {
		t.Fatal(err)
	}
	definition, err := json.Marshal(defs[0])
	if err != nil {
		t.Fatal(err)
	}
	monster := defs[0]
	monster.Name, monster.Tipo = "Hostile", model.TipoMonstro
	monster.Score.Merchant = 0
	monsterRaw, err := json.Marshal(monster)
	if err != nil {
		t.Fatal(err)
	}
	w, _, base := handlerTestWorld(t)
	w.channel = 1
	w.commands = make(chan command, 1)
	w.npcDefinitionState = &npcDefinitionRuntime{
		active: map[string]bool{"Aki": true}, versions: map[string]uint32{"Aki": 7},
	}
	w.store = &npcDefinitionCatalogCoverageStore{craftStore: base, entries: []store.NPCDefinitionRecord{
		{Channel: 1, Name: "Aki", Version: 6, Active: false, Definition: definition, Source: definition},
		{Channel: 1, Name: "Hostile", Version: 1, Definition: monsterRaw},
		{Channel: 1, Name: "Broken", Version: 1, Definition: json.RawMessage(`{"invalid":`)},
	}}
	type result struct {
		view control.NPCDefinitionCatalogView
		err  error
	}
	done := make(chan result, 1)
	go func() {
		view, err := w.NPCDefinitions(context.Background(), 1)
		done <- result{view: view, err: err}
	}()
	w.handle(<-w.commands)
	got := <-done
	if got.err != nil || !got.view.Available || got.view.Channel != 1 || len(got.view.NPCs) != 1 {
		t.Fatalf("catalog=%+v err=%v", got.view, got.err)
	}
	npc := got.view.NPCs[0]
	if npc.Name != "Aki" || !npc.Active || npc.AppliedVersion != 7 || npc.Version != 6 {
		t.Fatalf("NPC world state was not attached: %+v", npc)
	}

	if view, err := (&World{}).NPCDefinitions(context.Background(), 1); err != nil || view.Available || len(view.NPCs) != 0 {
		t.Fatalf("missing channel view=%+v err=%v", view, err)
	}
	w.store = base
	if view, err := w.NPCDefinitions(context.Background(), 1); err != nil || view.Available {
		t.Fatalf("missing backend view=%+v err=%v", view, err)
	}
}
