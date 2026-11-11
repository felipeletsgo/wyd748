package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestLoadVolatilesWithInstancesRejectsMissingFilesAndGeneratorSources(t *testing.T) {
	root := t.TempDir()
	volatilesPath := filepath.Join(root, "volatiles.json")
	instancesPath := filepath.Join(root, "instances.json")
	writeVolatileCoverageFile(t, volatilesPath, `{"default":{"action":"generic"}}`)
	if _, err := LoadVolatilesWithInstances(volatilesPath, instancesPath, root, nil, nil); err == nil {
		t.Fatal("missing instances file was accepted")
	}
	writeVolatileCoverageFile(t, instancesPath, "{")
	if _, err := LoadVolatilesWithInstances(volatilesPath, instancesPath, root, nil, nil); err == nil {
		t.Fatal("malformed instances file was accepted")
	}

	generated := `{"id":"room","name":"Room","generator":"missing.txt","durationSeconds":60,"exitX":200,"exitY":200}`
	writeVolatileCoverageFile(t, instancesPath, `{"items":{"100":{"action":"instance_ticket","instance":`+generated+`}}}`)
	if _, err := LoadVolatilesWithInstances(volatilesPath, instancesPath, root, nil, nil); err == nil {
		t.Fatal("missing item instance generator was accepted")
	}
	writeVolatileCoverageFile(t, instancesPath, `{"instances":{"room":`+generated+`}}`)
	if _, err := LoadVolatilesWithInstances(volatilesPath, instancesPath, root, nil, nil); err == nil {
		t.Fatal("missing named instance generator was accepted")
	}
}

func TestLoadVolatileDataRejectsInvalidInstanceReferencesAndItemBindings(t *testing.T) {
	t.Run("unknown base reference", func(t *testing.T) {
		file := volatileDataFile{Default: model.VolatileRule{Action: "generic"}, Instances: map[string]model.VolatileInstance{
			"alias": {BaseRef: "missing"},
		}}
		if _, err := loadVolatileData(file, nil, nil); err == nil || !strings.Contains(err.Error(), "unknown instance reference") {
			t.Fatalf("unknown base reference error=%v", err)
		}
	})

	t.Run("invalid volatile code", func(t *testing.T) {
		file := volatileDataFile{Default: model.VolatileRule{Action: "generic"}, Rules: map[int]model.VolatileRule{
			256: {Action: "generic"},
		}}
		if _, err := loadVolatileData(file, nil, nil); err == nil || !strings.Contains(err.Error(), "volatile code invalid") {
			t.Fatalf("invalid volatile code error=%v", err)
		}
	})

	t.Run("item rule requires catalog effect", func(t *testing.T) {
		file := volatileDataFile{Default: model.VolatileRule{Action: "generic"}, Items: map[uint16]model.VolatileRule{
			100: {Action: "restore"},
		}}
		if _, err := loadVolatileData(file, nil, nil); err == nil || !strings.Contains(err.Error(), "has no EF_VOLATILE") {
			t.Fatalf("missing item effect error=%v", err)
		}
	})

	t.Run("item and rule cannot disagree on instance identity", func(t *testing.T) {
		ruleInstance := validVolatileInstance("shared")
		itemInstance := validVolatileInstance("shared")
		itemInstance.Name = "Different room"
		file := volatileDataFile{
			Default: model.VolatileRule{Action: "generic"},
			Rules:   map[int]model.VolatileRule{1: {Action: "instance_ticket", Instance: &ruleInstance}},
			Items:   map[uint16]model.VolatileRule{100: {Action: "instance_ticket", Instance: &itemInstance}},
		}
		items := map[uint16]model.ItemDef{100: {StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 2}}}}
		if _, err := loadVolatileData(file, items, nil); err == nil || !strings.Contains(err.Error(), "conflicting configurations") {
			t.Fatalf("conflicting item instance error=%v", err)
		}
	})
}

func TestLoadVolatileDataValidatesInstanceChains(t *testing.T) {
	t.Run("target must be an instance ticket", func(t *testing.T) {
		source := validVolatileInstance("room-1")
		source.ChainNextItem = 200
		file := volatileDataFile{Default: model.VolatileRule{Action: "generic"}, Rules: map[int]model.VolatileRule{
			1: {Action: "instance_ticket", Instance: &source},
			2: {Action: "restore"},
		}}
		items := map[uint16]model.ItemDef{200: {StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 2}}}}
		if _, err := loadVolatileData(file, items, nil); err == nil || !strings.Contains(err.Error(), "is not instance_ticket") {
			t.Fatalf("invalid chain target error=%v", err)
		}
	})

	t.Run("water chain cannot cross tiers", func(t *testing.T) {
		source := validVolatileInstance("water-normal-room1")
		source.ChainNextItem = 200
		target := validVolatileInstance("water-mystic-room1")
		file := volatileDataFile{
			Default: model.VolatileRule{Action: "generic"},
			Rules:   map[int]model.VolatileRule{1: {Action: "instance_ticket", Instance: &source}},
			Items:   map[uint16]model.VolatileRule{200: {Action: "instance_ticket", Instance: &target}},
		}
		items := map[uint16]model.ItemDef{200: {StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 2}}}}
		if _, err := loadVolatileData(file, items, nil); err == nil || !strings.Contains(err.Error(), "crosses Water tiers") {
			t.Fatalf("cross-tier chain error=%v", err)
		}
	})
}

func TestLoadVolatileDataValidatesStateMachineShapes(t *testing.T) {
	t.Run("Hell Gate quadrant and final waves", func(t *testing.T) {
		cases := []struct {
			name   string
			want   string
			mutate func(*model.VolatileInstance)
		}{
			{name: "incomplete quadrant", want: "quadrant[0] incomplete", mutate: func(i *model.VolatileInstance) { i.HellGate.Quadrants[0].ID = 0 }},
			{name: "duplicate quadrant", want: "quadrant[1] duplicate", mutate: func(i *model.VolatileInstance) { i.HellGate.Quadrants[1].ID = 1 }},
			{name: "empty quadrant spawns", want: "has no spawns", mutate: func(i *model.VolatileInstance) { i.HellGate.Quadrants[0].Spawns = nil }},
			{name: "partial lich coordinates", want: "quadrants[0].lich spawns[0] invalid", mutate: func(i *model.VolatileInstance) { i.HellGate.Quadrants[0].Lich.X = 10 }},
			{name: "no final wave", want: "requires a final wave", mutate: func(i *model.VolatileInstance) { i.HellGate.FinalSpawns = nil }},
			{name: "invalid final spawn", want: "finalSpawns spawns[0] invalid", mutate: func(i *model.VolatileInstance) { i.HellGate.FinalSpawns[0].Count = 0 }},
			{name: "invalid final NPC", want: "finalNPCs spawns[0] invalid", mutate: func(i *model.VolatileInstance) {
				i.HellGate.FinalSpawns = nil
				i.HellGate.FinalNPCs = []model.VolatileInstanceSpawn{{NPC: "Boss", Count: 0}}
			}},
		}
		for _, test := range cases {
			t.Run(test.name, func(t *testing.T) {
				instance := validHellGateInstance()
				test.mutate(&instance)
				assertVolatileInstanceRejected(t, instance, nil, test.want)
			})
		}
	})

	t.Run("Big Cube stages", func(t *testing.T) {
		for _, test := range []struct {
			name   string
			want   string
			mutate func(*model.VolatileInstance)
		}{
			{name: "quiz required", want: "requires quiz", mutate: func(i *model.VolatileInstance) { i.Stages[0].Quiz = nil }},
			{name: "mobs are forbidden", want: "must not contain mobs", mutate: func(i *model.VolatileInstance) {
				i.Stages[0].Spawns = []model.VolatileInstanceSpawn{{NPC: "Mob", Count: 1}}
			}},
		} {
			t.Run(test.name, func(t *testing.T) {
				instance := validBigCubeInstance()
				test.mutate(&instance)
				assertVolatileInstanceRejected(t, instance, nil, test.want)
			})
		}
	})

	t.Run("Uxmal ticket and room tables", func(t *testing.T) {
		for _, test := range []struct {
			name   string
			want   string
			items  map[uint16]model.ItemDef
			mutate func(*model.VolatileInstance)
		}{
			{name: "ticket must exist", want: "ticket 100 missing", items: map[uint16]model.ItemDef{101: {}}, mutate: func(*model.VolatileInstance) {}},
			{name: "entry area coordinates", want: "entryAreas[0] invalid", items: validUxmalItems(), mutate: func(i *model.VolatileInstance) {
				i.Uxmal.EntryAreas = []model.VolatileInstanceEntryArea{{MinX: 0, MinY: 2, MaxX: 3, MaxY: 4}}
			}},
			{name: "room party limit", want: "room[0] has a table", items: validUxmalItems(), mutate: func(i *model.VolatileInstance) { i.Uxmal.MaxParties[0] = 0 }},
			{name: "room position", want: "room[0] position[0] invalid", items: validUxmalItems(), mutate: func(i *model.VolatileInstance) { i.Uxmal.RoomPositions[0][0].X = 0 }},
			{name: "zero rune", want: "contains a zero rune", items: validUxmalItems(), mutate: func(i *model.VolatileInstance) { i.Uxmal.Runes[0][0] = 0 }},
			{name: "unknown rune", want: "rune 102 missing from the catalog", items: validUxmalItems(), mutate: func(i *model.VolatileInstance) { i.Uxmal.Runes[0][0] = 102 }},
		} {
			t.Run(test.name, func(t *testing.T) {
				instance := validUxmalInstance()
				test.mutate(&instance)
				assertVolatileInstanceRejected(t, instance, test.items, test.want)
			})
		}
	})
}

func TestLoadVolatileDataMergesHellGateBaseReference(t *testing.T) {
	base := validHellGateInstance()
	base.ID = "base"
	base.Name = "Base"
	variant := validHellGateInstance()
	variant.BaseRef = "base"
	variant.ID = "variant"
	variant.Name = "Variant"
	variant.HellGate.ControllerNPC = "Variant Controller"
	file := volatileDataFile{Default: model.VolatileRule{Action: "generic"}, Instances: map[string]model.VolatileInstance{
		"base": base, "variant": variant,
	}}
	catalog, err := loadVolatileData(file, nil, nil)
	if err != nil {
		t.Fatal(err)
	}
	if got := catalog.Instances["variant"].HellGate.ControllerNPC; got != "Variant Controller" {
		t.Fatalf("merged Hell Gate controller=%q", got)
	}
}

func validVolatileInstance(id string) model.VolatileInstance {
	return model.VolatileInstance{
		ID: id, Name: "Room", X: 100, Y: 100, SpawnX: 101, SpawnY: 101, AreaRadius: 5,
		Spawns:          []model.VolatileInstanceSpawn{{NPC: "Mob", Count: 1}},
		DurationSeconds: 60, ExitX: 200, ExitY: 200,
	}
}

func validHellGateInstance() model.VolatileInstance {
	instance := validVolatileInstance("hell-gate")
	instance.StateMachine = "hell_gate"
	instance.HellGate = &model.VolatileHellGate{ControllerNPC: "Controller",
		FinalSpawns: []model.VolatileInstanceSpawn{{NPC: "Boss", Count: 1}}}
	for id := 1; id <= 4; id++ {
		instance.HellGate.Quadrants = append(instance.HellGate.Quadrants, model.VolatileHellGateQuadrant{
			ID: id, X: 100, Y: 100, SpawnX: 101, SpawnY: 101, AreaRadius: 5,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "Guard", Count: 1}},
			Lich:   model.VolatileInstanceSpawn{NPC: "Lich", Count: 1},
		})
	}
	return instance
}

func validBigCubeInstance() model.VolatileInstance {
	instance := validVolatileInstance("big-cube")
	instance.StateMachine = "big_cube"
	instance.NoCombatTimeout = true
	instance.Stages = []model.VolatileInstanceStage{{
		X: 100, Y: 100, SpawnX: 101, SpawnY: 101, AreaRadius: 5,
		Quiz: &model.VolatileInstanceQuiz{Question: "Question?", TrueX: 10, TrueY: 10,
			FalseX: 20, FalseY: 20, DurationSeconds: 10},
	}}
	return instance
}

func validUxmalInstance() model.VolatileInstance {
	instance := validVolatileInstance("uxmal")
	instance.StateMachine = "uxmal"
	instance.Uxmal = &model.VolatileUxmal{NPC: "Uxmal", TicketItem: 100,
		RoomPositions: make([][]model.VolatileDestination, 7), MaxParties: make([]int, 7),
		Runes: make([][]uint16, 7), TicketNextSanc: make([]int, 7)}
	instance.Stages = make([]model.VolatileInstanceStage, 7)
	for room := range instance.Stages {
		instance.Uxmal.RoomPositions[room] = []model.VolatileDestination{{X: 10, Y: 10}}
		instance.Uxmal.MaxParties[room] = 1
		instance.Uxmal.Runes[room] = []uint16{101}
		instance.Stages[room] = model.VolatileInstanceStage{
			X: 100, Y: 100, SpawnX: 101, SpawnY: 101, AreaRadius: 5, DurationSeconds: 60,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "Mob", Count: 1}},
		}
	}
	return instance
}

func validUxmalItems() map[uint16]model.ItemDef {
	return map[uint16]model.ItemDef{100: {}, 101: {}}
}

func assertVolatileInstanceRejected(t *testing.T, instance model.VolatileInstance, items map[uint16]model.ItemDef, want string) {
	t.Helper()
	file := volatileDataFile{Default: model.VolatileRule{Action: "generic"}, Instances: map[string]model.VolatileInstance{
		"fixture": instance,
	}}
	if _, err := loadVolatileData(file, items, nil); err == nil || !strings.Contains(err.Error(), want) {
		t.Fatalf("invalid %s instance error=%v, want substring %q", instance.StateMachine, err, want)
	}
}

func writeVolatileCoverageFile(t *testing.T, path, content string) {
	t.Helper()
	if err := os.WriteFile(path, []byte(content), 0o600); err != nil {
		t.Fatal(err)
	}
}
