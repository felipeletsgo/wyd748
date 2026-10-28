package game

import (
	"context"
	"encoding/json"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/data"
	"wydgo/internal/store"
)

func TestNPCServiceFaceEditPreservesFlightAndRejectsBeforeSave(t *testing.T) {
	w, p, clock := airMoveTestWorld(t)
	defs, err := data.LoadNPCs("../../data/npcs/Griphon_Master.json")
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w.npcs, w.items, w.channel = defs, catalog.Items, 1
	w.commands = make(chan command, 8)
	mob := w.mobsByID[1100]
	mob.Def = &w.npcs[0]
	raw, err := json.Marshal(defs[0])
	if err != nil {
		t.Fatal(err)
	}
	name := defs[0].Name
	w.npcDefinitionState = &npcDefinitionRuntime{source: map[string]json.RawMessage{name: raw}, active: map[string]bool{name: true}, versions: map[string]uint32{name: 1}}
	backend := &npcBatchTestStore{craftStore: w.store.(*craftStore), entries: map[uint8]store.NPCDefinitionRecord{1: {Channel: 1, Name: name, Version: 1, Active: true, Definition: raw, Source: raw}}}
	w.store = backend
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()
	stop, stopped := make(chan struct{}), make(chan struct{})
	go func() { defer close(stopped); runStaffQueue(w, stop) }()
	changed := data.CloneNPCs(defs)[0]
	changed.Equip.Rosto.Index = 60
	invalid, err := json.Marshal(changed)
	if err != nil {
		t.Fatal(err)
	}
	cmd := control.NPCDefinitionCommand{Version: 1, Channel: 1, ExpectedVersion: 1, Name: name, Reason: "Edit service appearance", Definition: invalid}
	rejected, rejectErr := w.SaveNPCDefinition(ctx, "editor", cmd, func() bool { return true })
	// A supported content edit must still commit and update the same live entity.
	changed.Equip.Rosto.Index = 63
	changed.DisplayName = "Air Service"
	changed.Score.Defense++
	cmd.Definition, err = json.Marshal(changed)
	if err != nil {
		t.Fatal(err)
	}
	accepted, acceptErr := w.SaveNPCDefinition(ctx, "editor", cmd, func() bool { return true })
	close(stop)
	<-stopped
	if rejectErr != nil || rejected.Code != "invalid_definition" || acceptErr != nil || accepted.Code != "ok" || backend.saves != 1 {
		t.Fatalf("rejected=%s err=%v accepted=%s err=%v saves=%d", rejected.Code, rejectErr, accepted.Code, acceptErr, backend.saves)
	}
	if mob.Def != &w.npcs[0] || mob.Def.VisibleName() != "Air Service" || !isAirMoveNPC(mob.Def) || accepted.NPC.AppliedVersion != 2 {
		t.Fatal("accepted edit lost live identity or service selector")
	}
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 0, 1)})
	if !p.AirMoveActive {
		t.Fatal("flight unavailable after content edit")
	}
	clock.Advance(airMoveMinimumDuration)
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 0, 2)})
	if p.AirMoveActive || p.X != airMoveDestinations[0].x || p.Y != airMoveDestinations[0].y {
		t.Fatal("flight did not complete after content edit")
	}
}

func TestNPCServiceFaceRevisionRejectedAtRuntime(t *testing.T) {
	w, p, _ := airMoveTestWorld(t)
	defs, err := data.LoadNPCs("../../data/npcs/Griphon_Master.json")
	if err != nil {
		t.Fatal(err)
	}
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w.npcs, w.items, w.channel = defs, catalog.Items, 1
	mob := w.mobsByID[1100]
	mob.Def = &w.npcs[0]
	raw, _ := json.Marshal(defs[0])
	name := defs[0].Name
	w.npcDefinitionState = &npcDefinitionRuntime{source: map[string]json.RawMessage{name: raw}, active: map[string]bool{name: true}, versions: map[string]uint32{name: 1}}
	changed := data.CloneNPCs(defs)[0]
	changed.Equip.Rosto.Index = 60
	invalid, _ := json.Marshal(changed)
	w.installNPCDefinitionChanges([]store.NPCDefinitionRecord{{Channel: 1, Name: name, Version: 2, Definition: invalid}})
	if w.npcDefinitionState.versions[name] != 1 || !isAirMoveNPC(mob.Def) {
		t.Fatal("invalid revision changed live service")
	}
	w.handle(command{s: p.Session, pkt: airMovePacket(p.ID, 0, 1)})
	if !p.AirMoveActive {
		t.Fatal("rejected revision disabled flight")
	}
}
