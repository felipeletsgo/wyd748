package model

import "testing"

func TestQuestZoneEntryNPCKeepsLegacyAndExplicitBindings(t *testing.T) {
	if got := (QuestZone{Name: "Training Camp"}).EntryNPC(); got != "Training Camp" {
		t.Fatalf("legacy entry NPC = %q, want zone name", got)
	}
	if got := (QuestZone{Name: "training_camp", NPC: "Camp Guard"}).EntryNPC(); got != "Camp Guard" {
		t.Fatalf("explicit entry NPC = %q, want configured guard", got)
	}
}
