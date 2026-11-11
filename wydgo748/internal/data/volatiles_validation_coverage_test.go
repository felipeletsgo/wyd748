package data

import (
	"encoding/json"
	"os"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestLoadVolatileDataRejectsInvalidRulesTable(t *testing.T) {
	validInstance := `"instance":{"id":"room","name":"Room","x":100,"y":100,"spawnX":101,"spawnY":101,"areaRadius":5,"spawns":[{"npc":"Mob","count":1}],"durationSeconds":60,"exitX":200,"exitY":200}`
	withInstanceField := func(field string) string {
		return `{"action":"instance_ticket",` + validInstance[:len(validInstance)-1] + `,` + field + `}}`
	}
	tests := []struct {
		name string
		rule string
	}{
		{"unknown action", `{"action":"not_an_action"}`},
		{"custom pattern requires consumed firework", `{"action":"restore","customPattern":true}`},
		{"teleport coordinates", `{"action":"teleport"}`},
		{"hunting destinations", `{"action":"hunting_teleport"}`},
		{"learned bit range", `{"action":"learn_special_skill","learnedBit":24}`},
		{"incomplete summon", `{"action":"summon_contract","summon":{}}`},
		{"buff has exactly one form", `{"action":"buff"}`},
		{"buff single duration", `{"action":"buff","affectType":2}`},
		{"buff affect type or skill", `{"action":"buff","affects":[{"type":2,"skillId":43,"durationUnits":1}]}`},
		{"buff skill must resolve", `{"action":"buff","affects":[{"skillId":999,"durationUnits":1}]}`},
		{"accumulate cap", `{"action":"restore","accumulate":true}`},
		{"grant exp amount", `{"action":"grant_exp"}`},
		{"gem variant and consume", `{"action":"equipment_gem","variant":4,"consume":true}`},
		{"ore success chance", `{"action":"ore_upgrade","variant":0,"successPercent":101,"consume":true}`},
		{"counter list", `{"action":"grant_counter"}`},
		{"counter reward item", `{"action":"grant_counter_once","counters":{"entry":1},"rewardItems":[999]}`},
		{"loot box requires rewards", `{"action":"loot_box"}`},
		{"loot box rejects zero reward", `{"action":"loot_box","rewardItems":[0]}`},
		{"loot box reward catalog", `{"action":"loot_box","rewardItems":[999]}`},
		{"timed access data", `{"action":"timed_access"}`},
		{"party mode", `{"action":"restore","partyMode":"duo"}`},
		{"mastery reset range", `{"action":"mastery_reset","amount":1,"minLevel":10,"maxLevelExclusive":10}`},
		{"quiz arena coordinates", `{"action":"restore","quizArena":{"x":0,"y":20}}`},
		{"territory pass contract", `{"action":"territory_pass","x":1,"y":1}`},
		{"quest reward experience", `{"action":"quest_reward","minLevel":1,"maxLevelExclusive":2}`},
		{"quest reward level range", `{"action":"quest_reward","exp":1,"minLevel":2,"maxLevelExclusive":2}`},
		{"refine ceiling", `{"action":"refine","refineMax":0}`},
		{"refine ten chance range", `{"action":"refine","refineMax":1,"refineTenChance":101}`},
		{"refine chance belongs to refine", `{"action":"restore","refineChance":1}`},
		{"once quest belongs to refine set", `{"action":"restore","onceQuestId":-1}`},
		{"refine set latch", `{"action":"refine_set","refineMax":1}`},
		{"refine equipped target", `{"action":"refine_equipped","refineMax":1,"targetSlot":0}`},
		{"nightmare ticket limits", `{"action":"nightmare_ticket","amount":1}`},
		{"instance ticket template", `{"action":"instance_ticket"}`},
		{"instance tier", withInstanceField(`"nightmareTier":"nightmare"`)},
		{"instance max players", withInstanceField(`"maxPlayers":40`)},
		{"shared instance max players", withInstanceField(`"sharedEntry":true`)},
		{"instance total duration", withInstanceField(`"totalDurationSeconds":-1`)},
		{"instance active duration", withInstanceField(`"activeDurationSeconds":-1`)},
		{"instance party run limit", withInstanceField(`"partyRunLimit":1001`)},
		{"instance mode", withInstanceField(`"mode":"unknown"`)},
		{"instance finish policy", withInstanceField(`"finishPolicy":"unknown"`)},
		{"shared timed zone group", withInstanceField(`"mode":"shared_timed_zone"`)},
		{"state machine name", withInstanceField(`"stateMachine":"unknown"`)},
		{"incomplete hell gate", withInstanceField(`"stateMachine":"hell_gate"`)},
		{"incomplete big cube", withInstanceField(`"stateMachine":"big_cube"`)},
		{"incomplete uxmal", withInstanceField(`"stateMachine":"uxmal"`)},
		{"instance group collision", withInstanceField(`"mode":"state_machine","sharedGroup":"same","exclusiveGroup":"same"`)},
		{"instance entry area", withInstanceField(`"entryAreas":[{"minX":2,"minY":2,"maxX":1,"maxY":2}]`)},
		{"instance schedule", withInstanceField(`"schedule":[{"startMinute":60,"durationSeconds":10}]`)},
		{"instance single room shape", `{"action":"instance_ticket","instance":{"id":"room","name":"Room","durationSeconds":60,"exitX":200,"exitY":200}}`},
		{"instance spawn shape", withInstanceField(`"spawns":[{"npc":"Mob","count":0}]`)},
		{"instance allowed evolution", withInstanceField(`"allowedEvolutions":["god"]`)},
		{"instance reward catalog", withInstanceField(`"rewardItem":999`)},
		{"instance chain item catalog", withInstanceField(`"chainNextItem":999`)},
		{"face transform mesh", `{"action":"face_transform"}`},
		{"tint color range", `{"action":"tint","color":115}`},
		{"mount action", `{"action":"mount","mountAction":"unknown"}`},
		{"mount longevity amount", `{"action":"mount","mountAction":"longevity_restore"}`},
		{"mount level range", `{"action":"mount","mountAction":"level_set","amount":1,"mountMinLevel":1}`},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			var rule model.VolatileRule
			if err := json.Unmarshal([]byte(tc.rule), &rule); err != nil {
				t.Fatalf("invalid test rule: %v", err)
			}
			file := volatileDataFile{
				Default: model.VolatileRule{Action: "generic"},
				Rules:   map[int]model.VolatileRule{1: rule},
			}
			if _, err := loadVolatileData(file, nil, nil); err == nil {
				t.Fatalf("invalid rule was accepted: %s", tc.rule)
			}
		})
	}
}

func TestLoadVolatileDataAcceptsNormalizedRuleAndInstance(t *testing.T) {
	var rule model.VolatileRule
	if err := json.Unmarshal([]byte(`{"action":"buff","valueSource":" ITEM_EFFECTS ","affectType":2,"durationUnits":3}`), &rule); err != nil {
		t.Fatal(err)
	}
	file := volatileDataFile{
		Default: model.VolatileRule{Action: "generic"},
		Rules:   map[int]model.VolatileRule{1: rule},
		Instances: map[string]model.VolatileInstance{
			"room": {ID: "room", Name: "Room", X: 100, Y: 100, SpawnX: 101, SpawnY: 101,
				AreaRadius: 5, Spawns: []model.VolatileInstanceSpawn{{NPC: "Mob", Count: 1}},
				DurationSeconds: 60, ExitX: 200, ExitY: 200},
		},
	}
	catalog, err := loadVolatileData(file, nil, nil)
	if err != nil {
		t.Fatal(err)
	}
	if got := catalog.Rules[1].ValueSource; got != "item_effects" {
		t.Fatalf("normalized value source=%q", got)
	}
	if got := catalog.Instances["room"]; got.ID != "room" || got.Name != "Room" {
		t.Fatalf("resolved instance=%+v", got)
	}
}

func TestLoadVolatilesWithInstancesEnforcesSplitAndMergeRules(t *testing.T) {
	root := t.TempDir()
	volPath := filepath.Join(root, "volatiles.json")
	instancePath := filepath.Join(root, "instances.json")
	write := func(path, content string) {
		t.Helper()
		if err := os.WriteFile(path, []byte(content), 0o644); err != nil {
			t.Fatal(err)
		}
	}
	baseVolatiles := `{"default":{"action":"generic"},"rules":{},"items":{}}`
	baseInstances := `{"instances":{},"rules":{},"items":{}}`
	validInstance := `{"id":"room","name":"Room","x":100,"y":100,"spawnX":101,"spawnY":101,"areaRadius":5,"spawns":[{"npc":"Mob","count":1}],"durationSeconds":60,"exitX":200,"exitY":200}`
	tests := []struct {
		name, volatiles, instances string
	}{
		{"instance templates stay in instances file", `{"default":{"action":"generic"},"instances":{"room":{}}}`, baseInstances},
		{"entry area sets stay in instances file", `{"default":{"action":"generic"},"entryAreaSets":{"water":[]}}`, baseInstances},
		{"ordinary rule cannot configure instance by ref", `{"default":{"action":"generic"},"rules":{"1":{"action":"restore","instanceRef":"room"}}}`, baseInstances},
		{"ordinary rule cannot configure inline instance", `{"default":{"action":"generic"},"rules":{"1":{"action":"restore","instance":{"id":"room"}}}}`, baseInstances},
		{"ordinary item cannot configure instance", `{"default":{"action":"generic"},"items":{"100":{"action":"restore","instanceRef":"room"}}}`, baseInstances},
		{"instance file cannot override default", baseVolatiles, `{"default":{"action":"generic"}}`},
		{"instance rule requires ticket action", baseVolatiles, `{"rules":{"1":{"action":"restore"}}}`},
		{"instance rule cannot duplicate volatile code", `{"default":{"action":"generic"},"rules":{"1":{"action":"restore"}}}`, `{"rules":{"1":{"action":"instance_ticket"}}}`},
		{"instance item requires ticket action", baseVolatiles, `{"items":{"100":{"action":"restore"}}}`},
		{"instance item cannot duplicate volatile item", `{"default":{"action":"generic"},"items":{"100":{"action":"restore"}}}`, `{"items":{"100":{"action":"instance_ticket"}}}`},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			write(volPath, tc.volatiles)
			write(instancePath, tc.instances)
			if _, err := LoadVolatilesWithInstances(volPath, instancePath, root, nil, nil); err == nil {
				t.Fatalf("split-file violation was accepted: volatile=%s instances=%s", tc.volatiles, tc.instances)
			}
		})
	}

	write(volPath, baseVolatiles)
	write(instancePath, `{"rules":{"7":{"action":"instance_ticket","instance":`+validInstance+`}}}`)
	catalog, err := LoadVolatilesWithInstances(volPath, instancePath, root, nil, nil)
	if err != nil || catalog.Rules[7].Instance == nil || catalog.Rules[7].Action != "instance_ticket" {
		t.Fatalf("valid instance merge failed: catalog=%+v err=%v", catalog, err)
	}

	write(instancePath, `{"rules":{"7":{"action":"instance_ticket","instance":{"id":"room","name":"Room","x":100,"y":100,"spawnX":101,"spawnY":101,"areaRadius":5,"generator":"missing.txt","durationSeconds":60,"exitX":200,"exitY":200}}}}`)
	if _, err := LoadVolatilesWithInstances(volPath, instancePath, root, nil, nil); err == nil {
		t.Fatal("missing instance generator was accepted")
	}
	if _, err := LoadVolatiles(volPath+".missing", nil, nil); err == nil {
		t.Fatal("missing volatile file was accepted")
	}
	write(volPath, "{")
	if _, err := LoadVolatiles(volPath, nil, nil); err == nil {
		t.Fatal("invalid volatile JSON was accepted")
	}
}
