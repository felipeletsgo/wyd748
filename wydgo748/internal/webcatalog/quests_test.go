package webcatalog

import (
	"encoding/json"
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestQuestPublicProjection(t *testing.T) {
	encoded, err := Export(filepath.Join("..", "..", "data"))
	if err != nil {
		t.Fatal(err)
	}
	var catalog Catalog
	if err := json.Unmarshal(encoded, &catalog); err != nil {
		t.Fatal(err)
	}
	q := catalog.Quests[0]
	if q.ID != 1 || q.MinLevel != 40 || q.MaxLevel == nil || *q.MaxLevel != 116 || !q.Destination.GlobalReset {
		t.Fatalf("incorrect display levels or global reset: %+v", q)
	}
	if catalog.Quests[1].GoldCost != 400000 || !catalog.Quests[1].Citizenship {
		t.Fatal("citizenship cost missing")
	}
	if catalog.Quests[14].AfterQuestID != 9 || catalog.Quests[14].Refinement.Slot != 6 {
		t.Fatal("prerequisite or refinement missing")
	}
	if catalog.Quests[17].Recharge.Quantity != 50 || catalog.Quests[17].ConsumedCounters[0].Quantity != 1 || catalog.Quests[17].Destination.GlobalReset {
		t.Fatal("Kefra recharge semantics lost")
	}
}

func TestQuestProjectionRejectsIncompleteReferences(t *testing.T) {
	for _, scenario := range []string{"missing item", "unknown counter", "cycle", "invalid name", "missing recharge item"} {
		t.Run(scenario, func(t *testing.T) {
			file := model.QuestFile{Version: 1, Quests: []model.QuestDef{{ID: 1, NPC: "Trainer", Messages: model.QuestMessages{Offer: "Bring a key.", Success: "Done."}}}}
			switch scenario {
			case "missing item":
				file.Quests[0].Requires.Items = []model.QuestItem{{Index: 451}}
			case "unknown counter":
				file.Quests[0].Requires.Counters = map[string]uint32{"secret": 1}
			case "cycle":
				file.Quests[0].Requires.AfterQuest = 2
				file.Quests = append(file.Quests, model.QuestDef{ID: 2, NPC: "Other", Requires: model.QuestRequirements{AfterQuest: 1}, Messages: model.QuestMessages{Offer: "Offer", Success: "Done"}})
			case "invalid name":
				file.Quests[0].NPC = "Bad\nname"
			case "missing recharge item":
				file.Quests[0].Recharge = &model.QuestRecharge{Item: 4127, Counter: "kefra_ticket", Amount: 50}
				file.Quests[0].ConsumeCounters = map[string]uint32{"kefra_ticket": 1}
			}
			if _, err := ProjectQuests(file, model.QuestZoneFile{Version: 1}, nil); err == nil {
				t.Fatal("invalid projection accepted")
			}
		})
	}
}

func TestQuestDefaultQuantityAndUnboundedLevel(t *testing.T) {
	file := model.QuestFile{Version: 1, Quests: []model.QuestDef{{ID: 1, NPC: "Trainer", Requires: model.QuestRequirements{Items: []model.QuestItem{{Index: 451}}}, Messages: model.QuestMessages{Offer: "Bring a key.", Success: "Done."}}}}
	quests, err := ProjectQuests(file, model.QuestZoneFile{Version: 1}, []Item{{ID: 451}})
	if err != nil {
		t.Fatal(err)
	}
	if quests[0].MinLevel != 1 || quests[0].MaxLevel != nil || quests[0].RequiredItems[0].Quantity != 1 {
		t.Fatal("defaults do not match authoritative semantics")
	}
}
