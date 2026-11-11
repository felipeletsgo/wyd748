package model

import (
	"strings"
	"testing"
)

func validQuestForValidation() QuestDef {
	return QuestDef{
		ID:       1,
		NPC:      "Kibita",
		Messages: QuestMessages{Offer: "Offer", Success: "Success"},
	}
}

func TestQuestItemQuantityDefaultsToOne(t *testing.T) {
	if got := (QuestItem{}).Quantity(); got != 1 {
		t.Fatalf("zero item amount = %d, want 1", got)
	}
	if got := (QuestItem{Amount: 7}).Quantity(); got != 7 {
		t.Fatalf("explicit item amount = %d, want 7", got)
	}
}

func TestQuestValidationRejectsInvalidBaseRequirements(t *testing.T) {
	var absent *QuestDef
	if err := absent.Validate(); err == nil {
		t.Fatal("nil quest was accepted")
	}
	for _, test := range []struct {
		name   string
		mutate func(*QuestDef)
	}{
		{"nonpositive id", func(q *QuestDef) { q.ID = 0 }},
		{"blank npc", func(q *QuestDef) { q.NPC = " \t" }},
		{"blank offer", func(q *QuestDef) { q.Messages.Offer = " " }},
		{"blank success", func(q *QuestDef) { q.Messages.Success = "\n" }},
		{"max level below minimum", func(q *QuestDef) { q.Requires.MinLevel, q.Requires.MaxLevel = 10, 9 }},
		{"self prerequisite", func(q *QuestDef) { q.Requires.AfterQuest = q.ID }},
	} {
		t.Run(test.name, func(t *testing.T) {
			quest := validQuestForValidation()
			test.mutate(&quest)
			if err := quest.Validate(); err == nil {
				t.Fatal("invalid quest was accepted")
			}
		})
	}
}

func TestQuestValidationRejectsMissingItemsAndInvalidTeleport(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*QuestDef)
	}{
		{"requirement item zero", func(q *QuestDef) { q.Requires.Items = []QuestItem{{}} }},
		{"consumed item zero", func(q *QuestDef) { q.Consumes = []QuestItem{{}} }},
		{"reward item zero", func(q *QuestDef) { q.Rewards.Items = []QuestItem{{}} }},
		{"teleport x zero", func(q *QuestDef) { q.Rewards.Teleport = &QuestTeleport{Y: 1} }},
		{"teleport y zero", func(q *QuestDef) { q.Rewards.Teleport = &QuestTeleport{X: 1} }},
	} {
		t.Run(test.name, func(t *testing.T) {
			quest := validQuestForValidation()
			test.mutate(&quest)
			if err := quest.Validate(); err == nil {
				t.Fatal("invalid item or teleport was accepted")
			}
		})
	}
}

func TestQuestValidationChecksNamedCounters(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*QuestDef)
	}{
		{"requirement counter name", func(q *QuestDef) { q.Requires.Counters = map[string]uint32{" ": 1} }},
		{"requirement counter amount", func(q *QuestDef) { q.Requires.Counters = map[string]uint32{"ticket": 0} }},
		{"requirement counter limit", func(q *QuestDef) { q.Requires.Counters = map[string]uint32{"ticket": MaxQuestCounter + 1} }},
		{"consumed counter name", func(q *QuestDef) { q.ConsumeCounters = map[string]uint32{"": 1} }},
		{"consumed counter amount", func(q *QuestDef) { q.ConsumeCounters = map[string]uint32{"ticket": 0} }},
		{"reward counter limit", func(q *QuestDef) { q.Rewards.Counters = map[string]uint32{"ticket": MaxQuestCounter + 1} }},
	} {
		t.Run(test.name, func(t *testing.T) {
			quest := validQuestForValidation()
			test.mutate(&quest)
			if err := quest.Validate(); err == nil {
				t.Fatal("invalid named counter was accepted")
			}
		})
	}
}

func TestQuestValidationChecksRechargeAndRefine(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func(*QuestDef)
	}{
		{"incomplete recharge", func(q *QuestDef) { q.Recharge = &QuestRecharge{Item: 1, Counter: "ticket"} }},
		{"recharge above limit", func(q *QuestDef) {
			q.ConsumeCounters = map[string]uint32{"ticket": 1}
			q.Recharge = &QuestRecharge{Item: 1, Counter: "ticket", Amount: MaxQuestCounter + 1}
		}},
		{"recharge counter not consumed", func(q *QuestDef) { q.Recharge = &QuestRecharge{Item: 1, Counter: "ticket", Amount: 1} }},
		{"refine zero", func(q *QuestDef) { q.Rewards.Refine = &QuestRefine{Slot: -1, Sanc: 0} }},
		{"refine above nine", func(q *QuestDef) { q.Rewards.Refine = &QuestRefine{Slot: 6, Sanc: 10} }},
		{"refine slot out of range", func(q *QuestDef) { q.Rewards.Refine = &QuestRefine{Slot: MaxEquipSlots, Sanc: 1} }},
	} {
		t.Run(test.name, func(t *testing.T) {
			quest := validQuestForValidation()
			test.mutate(&quest)
			if err := quest.Validate(); err == nil {
				t.Fatal("invalid recharge or refine reward was accepted")
			}
		})
	}

	quest := validQuestForValidation()
	quest.ConsumeCounters = map[string]uint32{"ticket": 1}
	quest.Recharge = &QuestRecharge{Item: 4127, Counter: "ticket", Amount: 100}
	quest.Rewards.Refine = &QuestRefine{Slot: -1, Sanc: 9}
	if err := quest.Validate(); err != nil {
		t.Fatalf("valid recharge and all-equipped refine were rejected: %v", err)
	}
}

func TestQuestValidationBoundsDialogue(t *testing.T) {
	for _, line := range []string{"", " \t"} {
		quest := validQuestForValidation()
		quest.Dialogue = []string{line}
		if err := quest.Validate(); err == nil {
			t.Fatalf("empty dialogue %q was accepted", line)
		}
	}
	quest := validQuestForValidation()
	quest.Dialogue = []string{strings.Repeat("x", MaxQuestLine+1)}
	if err := quest.Validate(); err == nil {
		t.Fatal("dialogue longer than the client line was accepted")
	}
	quest.Dialogue = []string{strings.Repeat("x", MaxQuestLine)}
	if err := quest.Validate(); err != nil {
		t.Fatalf("dialogue at the client line limit was rejected: %v", err)
	}
}

func TestQuestFileValidationAndCrossReferences(t *testing.T) {
	var absent *QuestFile
	if err := absent.Validate(); err == nil {
		t.Fatal("nil quest file was accepted")
	}
	if err := (&QuestFile{Version: QuestFileVersion + 1}).Validate(); err == nil {
		t.Fatal("unsupported quest file version was accepted")
	}

	first := validQuestForValidation()
	first.ID, first.NPC = 1, "Kibita"
	second := validQuestForValidation()
	second.ID, second.NPC = 2, "Survivor"
	second.Requires.AfterQuest = first.ID
	if err := (&QuestFile{Version: QuestFileVersion, Quests: []QuestDef{first, second}}).Validate(); err != nil {
		t.Fatalf("valid quest dependency was rejected: %v", err)
	}

	duplicateID := first
	duplicateID.NPC = "Other NPC"
	if err := (&QuestFile{Version: QuestFileVersion, Quests: []QuestDef{first, duplicateID}}).Validate(); err == nil {
		t.Fatal("duplicate quest ID was accepted")
	}
	duplicateNPC := second
	duplicateNPC.NPC = " kibita "
	if err := (&QuestFile{Version: QuestFileVersion, Quests: []QuestDef{first, duplicateNPC}}).Validate(); err == nil {
		t.Fatal("NPC duplicate after normalization was accepted")
	}
	second.Requires.AfterQuest = 99
	if err := (&QuestFile{Version: QuestFileVersion, Quests: []QuestDef{first, second}}).Validate(); err == nil {
		t.Fatal("missing prerequisite quest was accepted")
	}
	if got := NormalizeQuestNPC("  KiBiTa\t"); got != "kibita" {
		t.Fatalf("NormalizeQuestNPC = %q, want %q", got, "kibita")
	}
}
