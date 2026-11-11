package data

import (
	"testing"

	"wydgo/internal/model"
)

func validCoverageInstance(id string) model.VolatileInstance {
	return model.VolatileInstance{
		ID: id, Name: "Room", X: 100, Y: 101, SpawnX: 102, SpawnY: 103,
		AreaRadius: 5, Spawns: []model.VolatileInstanceSpawn{{NPC: "Mob", Count: 1}},
		DurationSeconds: 60, ExitX: 200, ExitY: 201,
	}
}

func TestLoadVolatileDataResolvesAndMergesInstanceVariants(t *testing.T) {
	area := model.VolatileInstanceEntryArea{MinX: 100, MinY: 100, MaxX: 120, MaxY: 120}
	base := validCoverageInstance("water-tier-a-base")
	base.Name = "Base room"
	base.MaxPlayers = 4
	base.RewardItem = 500
	base.AllowedEvolutions = []string{"arch"}
	base.TotalDurationSeconds = 600
	base.ActiveDurationSeconds = 300
	base.TransitionSeconds = 5
	base.SharedEntry = true
	base.Mode = "shared_timed_zone"
	base.SharedGroup = "water-tier-a"
	base.ExclusiveGroup = "water-tier-a-lock"
	base.FinishPolicy = "finish_on_clear"
	base.HellGate = &model.VolatileHellGate{ControllerNPC: "Base controller"}
	base.AllowChainDuringExitGrace = true
	base.EntryAreas = []model.VolatileInstanceEntryArea{area}
	base.Schedule = []model.VolatileInstanceWindow{{StartMinute: 10, DurationSeconds: 60}}
	base.PartyRunLimit = 3
	base.NightmareTier = "normal"

	variant := model.VolatileInstance{
		BaseRef: "base", ID: "water-tier-a-variant", Name: "Variant room",
		X: 110, Y: 111, SpawnX: 112, SpawnY: 113, AreaRadius: 7, MaxPlayers: 6,
		Spawns:     []model.VolatileInstanceSpawn{{NPC: "Variant Mob", Count: 2}},
		RewardItem: 501, ChainNextItem: 101, ChainStart: true,
		AllowedEvolutions: []string{"celestial"}, DurationSeconds: 90,
		TotalDurationSeconds: 900, ActiveDurationSeconds: 450, ExitX: 210, ExitY: 211,
		TransitionSeconds: 8, SharedEntry: true, Mode: "shared_timed_zone",
		SharedGroup: "water-tier-a-variant", ExclusiveGroup: "water-tier-a-variant-lock",
		FinishPolicy: "finish_on_timeout", StateMachine: "", AllowChainDuringExitGrace: true,
		EntryAreas:    []model.VolatileInstanceEntryArea{{MinX: 110, MinY: 110, MaxX: 130, MaxY: 130}},
		Schedule:      []model.VolatileInstanceWindow{{StartMinute: 20, DurationSeconds: 90}},
		PartyRunLimit: 5, NightmareTier: "mystic",
		Stages: []model.VolatileInstanceStage{{
			Name: "Variant stage", X: 110, Y: 111, SpawnX: 112, SpawnY: 113,
			AreaRadius: 7, DurationSeconds: 90,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "Variant Mob", Count: 2}},
		}},
	}
	next := validCoverageInstance("water-tier-a-next")
	entrySetBase := validCoverageInstance("entry-set-base")
	entrySetVariant := model.VolatileInstance{BaseRef: entrySetBase.ID, ID: "entry-set-variant", EntryAreaSet: "variant-entry"}
	items := map[uint16]model.ItemDef{
		101: {Index: 101, StaticEffects: []model.StaticEffect{{Name: "EF_VOLATILE", Value: 2}}},
		500: {Index: 500}, 501: {Index: 501},
	}
	file := volatileDataFile{
		Default: model.VolatileRule{Action: "none"},
		Rules: map[int]model.VolatileRule{
			1: {Action: "instance_ticket", InstanceRef: "variant"},
			2: {Action: "instance_ticket", Instance: &next},
		},
		Instances: map[string]model.VolatileInstance{
			"base": base, "variant": variant,
			"entry-set-base": entrySetBase, "entry-set-variant": entrySetVariant,
		},
		EntryAreaSets: map[string][]model.VolatileInstanceEntryArea{"variant-entry": {
			{MinX: 111, MinY: 111, MaxX: 131, MaxY: 131},
		}},
	}

	catalog, err := loadVolatileData(file, items, nil)
	if err != nil {
		t.Fatalf("load inherited instance variants: %v", err)
	}
	got := catalog.Rules[1].Instance
	if got == nil || got.ID != variant.ID || got.BaseRef != "" || got.Name != variant.Name ||
		got.X != variant.X || got.Y != variant.Y || got.SpawnX != variant.SpawnX || got.SpawnY != variant.SpawnY ||
		got.AreaRadius != variant.AreaRadius || got.MaxPlayers != variant.MaxPlayers ||
		len(got.Spawns) != 1 || got.Spawns[0].NPC != "Variant Mob" ||
		got.RewardItem != variant.RewardItem || got.ChainNextItem != variant.ChainNextItem || !got.ChainStart ||
		len(got.AllowedEvolutions) != 1 || got.AllowedEvolutions[0] != "celestial" ||
		got.DurationSeconds != variant.DurationSeconds || got.TotalDurationSeconds != variant.TotalDurationSeconds ||
		got.ActiveDurationSeconds != variant.ActiveDurationSeconds || got.ExitX != variant.ExitX || got.ExitY != variant.ExitY ||
		got.TransitionSeconds != variant.TransitionSeconds || !got.SharedEntry || got.Mode != variant.Mode ||
		got.SharedGroup != variant.SharedGroup || got.ExclusiveGroup != variant.ExclusiveGroup ||
		got.FinishPolicy != variant.FinishPolicy || !got.AllowChainDuringExitGrace ||
		got.EntryAreaSet != "" || len(got.EntryAreas) != 1 || got.EntryAreas[0].MinX != 110 ||
		len(got.Schedule) != 1 || got.Schedule[0].StartMinute != 20 || got.PartyRunLimit != 5 ||
		got.NightmareTier != "mystic" || len(got.Stages) != 1 || got.Stages[0].Name != "Variant stage" {
		t.Fatalf("variant overrides or inherited configuration were lost: %+v", got)
	}
	if baseResult := catalog.Instances["base"]; baseResult.ID != base.ID || baseResult.Name != base.Name ||
		baseResult.RewardItem != base.RewardItem || baseResult.SharedGroup != base.SharedGroup {
		t.Fatalf("base instance was modified by its variant: %+v", baseResult)
	}
	if resolved := catalog.Instances["entry-set-variant"]; resolved.EntryAreaSet != "variant-entry" ||
		len(resolved.EntryAreas) != 1 || resolved.EntryAreas[0].MinX != 111 {
		t.Fatalf("inherited entry area set was not expanded: %+v", resolved)
	}
	if target, _, ok := catalog.Rule(101); !ok || target.Instance == nil || target.Instance.ID != next.ID {
		t.Fatalf("same-tier chain target was not resolved: %+v ok=%v", target, ok)
	}
}

func TestLoadVolatileDataRejectsInvalidInstanceReferencesAndDefinitions(t *testing.T) {
	for _, tc := range []struct {
		name string
		file volatileDataFile
	}{
		{
			name: "unknown reference",
			file: volatileDataFile{Default: model.VolatileRule{Action: "none"},
				Rules: map[int]model.VolatileRule{1: {Action: "instance_ticket", InstanceRef: "missing"}}},
		},
		{
			name: "cyclic base references",
			file: volatileDataFile{Default: model.VolatileRule{Action: "none"},
				Rules: map[int]model.VolatileRule{1: {Action: "instance_ticket", InstanceRef: "first"}},
				Instances: map[string]model.VolatileInstance{
					"first":  {BaseRef: "second"},
					"second": {BaseRef: "first"},
				}},
		},
		{
			name: "unknown entry area set",
			file: volatileDataFile{Default: model.VolatileRule{Action: "none"},
				Instances: map[string]model.VolatileInstance{
					"room": func() model.VolatileInstance {
						instance := validCoverageInstance("room")
						instance.EntryAreaSet = "missing"
						return instance
					}(),
				}},
		},
		{
			name: "entry area set combined with inline areas",
			file: volatileDataFile{Default: model.VolatileRule{Action: "none"},
				EntryAreaSets: map[string][]model.VolatileInstanceEntryArea{"zone": {{MinX: 1, MinY: 1, MaxX: 2, MaxY: 2}}},
				Instances: map[string]model.VolatileInstance{
					"room": func() model.VolatileInstance {
						instance := validCoverageInstance("room")
						instance.EntryAreaSet = "zone"
						instance.EntryAreas = []model.VolatileInstanceEntryArea{{MinX: 3, MinY: 3, MaxX: 4, MaxY: 4}}
						return instance
					}(),
				}},
		},
		{
			name: "chain start outside Water",
			file: volatileDataFile{Default: model.VolatileRule{Action: "none"},
				Instances: map[string]model.VolatileInstance{
					"room": func() model.VolatileInstance {
						instance := validCoverageInstance("room")
						instance.ChainStart = true
						return instance
					}(),
				}},
		},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if _, err := loadVolatileData(tc.file, nil, nil); err == nil {
				t.Fatal("invalid instance data was accepted")
			}
		})
	}
}

func TestLoadVolatileDataClonesAndValidatesUxmalTables(t *testing.T) {
	makeUxmal := func(id, baseRef string) model.VolatileInstance {
		uxmal := &model.VolatileUxmal{
			Version: 759, RoomGenerators: []string{"room-generator"}, NPC: "RuneGuide", TicketItem: 9004,
			EntryAreas:    []model.VolatileInstanceEntryArea{{MinX: 10, MinY: 10, MaxX: 20, MaxY: 20}},
			RoomPositions: make([][]model.VolatileDestination, 7),
			MaxParties:    make([]int, 7), Runes: make([][]uint16, 7), TicketNextSanc: make([]int, 7),
		}
		stages := make([]model.VolatileInstanceStage, 7)
		for room := range stages {
			parties := 3
			if room == 4 {
				parties = 4
			}
			uxmal.MaxParties[room] = parties
			uxmal.RoomPositions[room] = make([]model.VolatileDestination, parties)
			for slot := range uxmal.RoomPositions[room] {
				uxmal.RoomPositions[room][slot] = model.VolatileDestination{X: uint16(100 + room), Y: uint16(200 + slot)}
			}
			uxmal.Runes[room] = []uint16{9005}
			stages[room] = model.VolatileInstanceStage{
				Name: "Rune room", X: uint16(300 + room), Y: 400, SpawnX: 301, SpawnY: 401,
				AreaRadius: 5, DurationSeconds: 60,
			}
		}
		instance := validCoverageInstance(id)
		instance.BaseRef = baseRef
		instance.Mode = "state_machine"
		instance.StateMachine = "uxmal"
		instance.Uxmal = uxmal
		instance.Stages = stages
		return instance
	}
	base := makeUxmal("uxmal-base", "")
	variant := makeUxmal("uxmal-variant", "base")
	variant.Name = "Variant Rune Track"
	file := volatileDataFile{
		Default:   model.VolatileRule{Action: "none"},
		Instances: map[string]model.VolatileInstance{"base": base, "variant": variant},
	}
	items := map[uint16]model.ItemDef{9004: {Index: 9004}, 9005: {Index: 9005}}
	catalog, err := loadVolatileData(file, items, nil)
	if err != nil {
		t.Fatalf("load valid Uxmal tables: %v", err)
	}
	got := catalog.Instances["variant"]
	if got.ID != variant.ID || got.Name != variant.Name || got.Uxmal == nil || len(got.Uxmal.RoomPositions) != 7 ||
		len(got.Uxmal.MaxParties) != 7 || len(got.Uxmal.Runes) != 7 || len(got.Uxmal.TicketNextSanc) != 7 {
		t.Fatalf("inherited Uxmal tables are incomplete: %+v", got)
	}
	variant.Uxmal.RoomGenerators[0] = "mutated"
	variant.Uxmal.RoomPositions[0][0].X = 999
	variant.Uxmal.Runes[0][0] = 999
	variant.Uxmal.TicketNextSanc[0] = 6
	file.Instances["variant"] = variant
	got = catalog.Instances["variant"]
	if got.Uxmal.RoomGenerators[0] != "room-generator" || got.Uxmal.RoomPositions[0][0].X == 999 ||
		got.Uxmal.Runes[0][0] != 9005 || got.Uxmal.TicketNextSanc[0] != 0 {
		t.Fatal("resolved Uxmal data aliases the source variant")
	}
}
