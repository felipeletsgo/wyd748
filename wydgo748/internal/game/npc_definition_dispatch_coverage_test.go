package game

import (
	"testing"

	"wydgo/internal/store"
)

func TestNotifyNPCDefinitionRoutesAndCountsQueueDrops(t *testing.T) {
	source := &World{channel: 1, commands: make(chan command, 1)}
	target := &World{channel: 2, commands: make(chan command, 1)}
	source.cluster = &ChannelCluster{worlds: []*World{source, target}}
	entry := store.NPCDefinitionRecord{Channel: 2, ChangeID: 41, Name: "Test NPC"}

	source.notifyNPCDefinition(entry)
	select {
	case cmd := <-target.commands:
		if cmd.npcDefinitions == nil || len(cmd.npcDefinitions.entries) != 1 ||
			cmd.npcDefinitions.entries[0].Channel != entry.Channel ||
			cmd.npcDefinitions.entries[0].ChangeID != entry.ChangeID ||
			cmd.npcDefinitions.entries[0].Name != entry.Name {
			t.Fatalf("wrong channel notification: %+v", cmd.npcDefinitions)
		}
	default:
		t.Fatal("notification was not routed to the owning channel")
	}

	drops := metricNPCDefinitionQueueDrops.Value()
	source.notifyNPCDefinition(store.NPCDefinitionRecord{Channel: 3, ChangeID: 42})
	if metricNPCDefinitionQueueDrops.Value() != drops {
		t.Fatal("unknown channel incremented the queue-drop metric")
	}

	target.commands <- command{}
	source.notifyNPCDefinition(entry)
	if got := metricNPCDefinitionQueueDrops.Value(); got != drops+1 {
		t.Fatalf("full destination queue drop metric=%d, want %d", got, drops+1)
	}
}
