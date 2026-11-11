package game

import (
	"testing"

	"wydgo/internal/store"
)

func TestCommandLabelCoversActorControlCommands(t *testing.T) {
	for _, tc := range []struct {
		name  string
		cmd   command
		label string
	}{
		{name: "kick", cmd: command{kick: &kickRequest{}}, label: "control.kick"},
		{name: "teleport", cmd: command{teleport: &teleportRequest{}}, label: "control.teleport"},
		{name: "bosses", cmd: command{bosses: &bossesRequest{}}, label: "control.bosses"},
		{name: "global drop", cmd: command{globalDrop: &globalDropRequest{}}, label: "control.global-drop"},
		{name: "quiz", cmd: command{quiz: &quizRequest{}}, label: "control.quiz"},
		{name: "pvp door", cmd: command{pvpDoor: &pvpDoorRequest{}}, label: "control.pvp-door"},
		{name: "persistence recovery", cmd: command{recovery: &persistenceRecoveryRequest{}}, label: "control.persistence-recovery"},
		{name: "ox event", cmd: command{oxEvent: &oxEventRequest{}}, label: "control.ox-event"},
		{name: "colosseum", cmd: command{colosseum: &colosseumRequest{}}, label: "control.colosseum"},
		{name: "premium poll", cmd: command{premium: &premiumEvent{}}, label: "premium.poll"},
		{name: "NPC definitions", cmd: command{npcDefinitions: &npcDefinitionEvent{}}, label: "npc-definitions.poll"},
		{name: "control read", cmd: command{control: &controlRequest{}}, label: "control.read"},
		{name: "staff", cmd: command{staff: &staffRequest{label: "staff.test"}}, label: "staff.test"},
		{name: "login reload", cmd: command{loginReload: &loginReloadResult{}}, label: "login.reload"},
		{name: "channel ticket", cmd: command{channelTicket: &channelTicketIssued{}}, label: "channel.ticket"},
		{name: "guild sync", cmd: command{guildSync: &guildSyncEvent{}}, label: "guild.sync"},
		{name: "channel message", cmd: command{channelMessage: &store.ChannelMessage{}}, label: "channel.message"},
		{name: "actor apply", cmd: command{apply: func() {}}, label: "actor.apply"},
		{name: "quarantine", cmd: command{quarantine: []quarantineStatus{}}, label: "quarantine.status"},
		{name: "entry read", cmd: command{entryRead: &entryReadResult{}}, label: "entry.read"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if got := commandLabel(tc.cmd); got != tc.label {
				t.Fatalf("commandLabel=%q, want %q", got, tc.label)
			}
		})
	}
}

func TestPendingCommandCompactionAndPrepend(t *testing.T) {
	w := &World{pendingCommands: []command{{pkt: []byte{1}}, {pkt: []byte{2}}, {pkt: []byte{3}}}, pendingCommandHead: 1}
	w.compactPendingCommands()
	if w.pendingCommandHead != 0 || len(w.pendingCommands) != 2 || w.pendingCommands[0].pkt[0] != 2 || w.pendingCommands[1].pkt[0] != 3 {
		t.Fatalf("compaction lost or reordered pending commands: head=%d commands=%+v", w.pendingCommandHead, w.pendingCommands)
	}

	w.prependPendingCommands([]command{{pkt: []byte{1}}})
	if len(w.pendingCommands) != 3 || w.pendingCommands[0].pkt[0] != 1 || w.pendingCommands[1].pkt[0] != 2 {
		t.Fatalf("prepend changed pending order: %+v", w.pendingCommands)
	}
	if got := w.reservePendingCommandFront(0); got != nil {
		t.Fatal("zero-sized front reservation should return nil")
	}

	w.pendingCommandHead = len(w.pendingCommands)
	w.compactPendingCommands()
	if w.pendingCommandHead != 0 || len(w.pendingCommands) != 0 {
		t.Fatalf("fully consumed queue was not cleared: head=%d len=%d", w.pendingCommandHead, len(w.pendingCommands))
	}
}

func TestRunRejectsClusterWorld(t *testing.T) {
	w := &World{cluster: &ChannelCluster{}}
	defer func() {
		if got := recover(); got != "cluster worlds must run through ChannelCluster.Run" {
			t.Fatalf("Run panic=%v", got)
		}
	}()
	w.Run()
}
