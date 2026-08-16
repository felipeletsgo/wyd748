package model

import (
	"strings"
	"testing"
)

func validQuestContract(id int, npc string) QuestDef {
	return QuestDef{
		ID: id, NPC: npc,
		Messages: QuestMessages{Offer: "Offer", Success: "Success"},
	}
}

func TestQuestItemQuantityDefaultsToOne(t *testing.T) {
	if got := (QuestItem{Index: 1}).Quantity(); got != 1 {
		t.Fatalf("Quantity default=%d", got)
	}
	if got := (QuestItem{Index: 1, Amount: 7}).Quantity(); got != 7 {
		t.Fatalf("Quantity explicit=%d", got)
	}
}

func TestQuestValidateRejectsEveryMalformedFamily(t *testing.T) {
	longLine := strings.Repeat("x", MaxQuestLine+1)
	tests := []struct {
		name   string
		mutate func(*QuestDef)
	}{
		{"zero id", func(q *QuestDef) { q.ID = 0 }},
		{"negative id", func(q *QuestDef) { q.ID = -1 }},
		{"empty npc", func(q *QuestDef) { q.NPC = "  " }},
		{"empty offer", func(q *QuestDef) { q.Messages.Offer = " " }},
		{"empty success", func(q *QuestDef) { q.Messages.Success = " " }},
		{"level inversion", func(q *QuestDef) { q.Requires.MinLevel, q.Requires.MaxLevel = 100, 99 }},
		{"self dependency", func(q *QuestDef) { q.Requires.AfterQuest = q.ID }},
		{"zero required item", func(q *QuestDef) { q.Requires.Items = []QuestItem{{Index: 0}} }},
		{"zero consumed item", func(q *QuestDef) { q.Consumes = []QuestItem{{Index: 0}} }},
		{"zero reward item", func(q *QuestDef) { q.Rewards.Items = []QuestItem{{Index: 0}} }},
		{"invalid teleport x", func(q *QuestDef) { q.Rewards.Teleport = &QuestTeleport{X: 0, Y: 1} }},
		{"invalid teleport y", func(q *QuestDef) { q.Rewards.Teleport = &QuestTeleport{X: 1, Y: 0} }},
		{"require counter empty name", func(q *QuestDef) { q.Requires.Counters = map[string]uint32{" ": 1} }},
		{"consume counter zero", func(q *QuestDef) { q.ConsumeCounters = map[string]uint32{"kefra": 0} }},
		{"reward counter too large", func(q *QuestDef) { q.Rewards.Counters = map[string]uint32{"fame": MaxQuestCounter + 1} }},
		{"recharge missing item", func(q *QuestDef) {
			q.ConsumeCounters = map[string]uint32{"kefra": 1}
			q.Recharge = &QuestRecharge{Counter: "kefra", Amount: 100}
		}},
		{"recharge missing counter", func(q *QuestDef) {
			q.ConsumeCounters = map[string]uint32{"kefra": 1}
			q.Recharge = &QuestRecharge{Item: 4127, Amount: 100}
		}},
		{"recharge zero amount", func(q *QuestDef) {
			q.ConsumeCounters = map[string]uint32{"kefra": 1}
			q.Recharge = &QuestRecharge{Item: 4127, Counter: "kefra"}
		}},
		{"recharge too large", func(q *QuestDef) {
			q.ConsumeCounters = map[string]uint32{"kefra": 1}
			q.Recharge = &QuestRecharge{Item: 4127, Counter: "kefra", Amount: MaxQuestCounter + 1}
		}},
		{"recharge never consumed", func(q *QuestDef) {
			q.Recharge = &QuestRecharge{Item: 4127, Counter: "kefra", Amount: 100}
		}},
		{"refine sanc low", func(q *QuestDef) { q.Rewards.Refine = &QuestRefine{Slot: -1, Sanc: 0} }},
		{"refine sanc high", func(q *QuestDef) { q.Rewards.Refine = &QuestRefine{Slot: -1, Sanc: 10} }},
		{"refine slot high", func(q *QuestDef) { q.Rewards.Refine = &QuestRefine{Slot: MaxEquipSlots, Sanc: 1} }},
		{"dialogue empty", func(q *QuestDef) { q.Dialogue = []string{" "} }},
		{"dialogue too long", func(q *QuestDef) { q.Dialogue = []string{longLine} }},
	}

	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			q := validQuestContract(1, "NPC")
			tc.mutate(&q)
			if err := q.Validate(); err == nil {
				t.Fatal("quest invalida foi aceita")
			}
		})
	}

	var nilQuest *QuestDef
	if err := nilQuest.Validate(); err == nil {
		t.Fatal("nil quest foi aceita")
	}
}

func TestQuestValidateAcceptsCompleteContractAtBoundaries(t *testing.T) {
	q := validQuestContract(7, " Survivor ")
	q.Requires.MinLevel = 1
	q.Requires.MaxLevel = 1
	q.Requires.Items = []QuestItem{{Index: 1}}
	q.Requires.Counters = map[string]uint32{"kefra": MaxQuestCounter}
	q.Consumes = []QuestItem{{Index: 2, Amount: 3}}
	q.ConsumeCounters = map[string]uint32{"kefra": 1}
	q.Rewards.Items = []QuestItem{{Index: 3}}
	q.Rewards.Counters = map[string]uint32{"fame": MaxQuestCounter}
	q.Rewards.Teleport = &QuestTeleport{X: 1, Y: 1}
	q.Rewards.Refine = &QuestRefine{Slot: -1, Sanc: 9}
	q.Recharge = &QuestRecharge{Item: 4127, Counter: "kefra", Amount: MaxQuestCounter}
	q.Dialogue = []string{strings.Repeat("x", MaxQuestLine)}
	if err := q.Validate(); err != nil {
		t.Fatalf("quest valida foi recusada: %v", err)
	}
	if got := NormalizeQuestNPC("  Survivor  "); got != "survivor" {
		t.Fatalf("NormalizeQuestNPC=%q", got)
	}
}

func TestQuestFileValidationCoversVersionDuplicatesAndDependencies(t *testing.T) {
	var nilFile *QuestFile
	if err := nilFile.Validate(); err == nil {
		t.Fatal("nil QuestFile foi aceito")
	}
	if err := (&QuestFile{Version: QuestFileVersion + 1}).Validate(); err == nil {
		t.Fatal("versao invalida foi aceita")
	}

	q1 := validQuestContract(1, "NPC One")
	q2 := validQuestContract(2, "NPC Two")
	if err := (&QuestFile{Version: QuestFileVersion, Quests: []QuestDef{q1, q2}}).Validate(); err != nil {
		t.Fatalf("arquivo valido foi recusado: %v", err)
	}

	dupID := q2
	dupID.ID = q1.ID
	if err := (&QuestFile{Version: QuestFileVersion, Quests: []QuestDef{q1, dupID}}).Validate(); err == nil {
		t.Fatal("ID duplicado foi aceito")
	}

	dupNPC := q2
	dupNPC.NPC = " npc one "
	if err := (&QuestFile{Version: QuestFileVersion, Quests: []QuestDef{q1, dupNPC}}).Validate(); err == nil {
		t.Fatal("NPC duplicado normalizado foi aceito")
	}

	missingDependency := q2
	missingDependency.Requires.AfterQuest = 99
	if err := (&QuestFile{Version: QuestFileVersion, Quests: []QuestDef{q1, missingDependency}}).Validate(); err == nil {
		t.Fatal("AfterQuest inexistente foi aceito")
	}

	validDependency := q2
	validDependency.Requires.AfterQuest = q1.ID
	if err := (&QuestFile{Version: QuestFileVersion, Quests: []QuestDef{q1, validDependency}}).Validate(); err != nil {
		t.Fatalf("AfterQuest existente foi recusado: %v", err)
	}
}
