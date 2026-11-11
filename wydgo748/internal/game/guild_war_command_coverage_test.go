package game

import (
	"testing"

	"wydgo/internal/wire"
)

func TestGuildWarCommandReportsValidationFailures(t *testing.T) {
	tests := []struct {
		name string
		arg  string
		want string
	}{
		{
			name: "wrong argument count",
			arg:  "1",
			want: "Use /citywar <1..4> <total guild fame bid>. Only increases are charged.",
		},
		{
			name: "invalid number",
			arg:  "one 25",
			want: "City must be 1..4; the total bid must be an unsigned 32-bit amount.",
		},
		{
			name: "invalid bid",
			arg:  "1 -1",
			want: "City must be 1..4; the total bid must be an unsigned 32-bit amount.",
		},
		{
			name: "city out of range",
			arg:  "9 25",
			want: "invalid city challenge",
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			w, _, leader, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
			drainPlayerPackets(leader)
			w.guildWarCommand(leader, "citywar", tt.arg)
			assertCriticalPickupPacket(t, leader, wire.MessageWhisper(0, "[WAR]", tt.want, 7))
			if len(w.guilds.Wars.Cities.Entries) != 0 {
				t.Fatalf("invalid command arg %q changed city-war entries", tt.arg)
			}
		})
	}
}

func TestGuildWarCommandGuildFameAndTowerRejections(t *testing.T) {
	t.Run("guild registry unavailable", func(t *testing.T) {
		w, _, leader, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
		w.guilds = nil
		drainPlayerPackets(leader)
		w.guildWarCommand(leader, "guildfame", "")
		assertCriticalPickupPacket(t, leader, wire.MessageWhisper(0, "[WAR]", "Guild registry unavailable.", 7))
	})

	t.Run("not in a guild", func(t *testing.T) {
		w, _, leader, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
		leader.Char.Name = "NoGuild"
		drainPlayerPackets(leader)
		w.guildWarCommand(leader, "guildfame", "")
		assertCriticalPickupPacket(t, leader, wire.MessageWhisper(0, "[WAR]", "You do not belong to a guild.", 7))
	})

	t.Run("guild fame", func(t *testing.T) {
		w, _, leader, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
		w.guilds.FindByID(1).Fame = 321
		drainPlayerPackets(leader)
		w.guildWarCommand(leader, "guildfame", "")
		assertCriticalPickupPacket(t, leader, wire.MessageWhisper(0, "[WAR]", "Guild Alpha: 321 fame.", 7))
	})

	t.Run("tower entry rejected", func(t *testing.T) {
		w, _, leader, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
		drainPlayerPackets(leader)
		w.guildWarCommand(leader, "tower", "")
		assertCriticalPickupPacket(t, leader, wire.MessageWhisper(0, "[WAR]", "Tower entry requires a kingdom guild, a living character in Erion and an active tower war.", 7))
	})

	t.Run("unknown command", func(t *testing.T) {
		w, _, leader, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
		before := leader.Session.QueuedPacketsForTest()
		w.guildWarCommand(leader, "unknown", "")
		if got := leader.Session.QueuedPacketsForTest(); got != before {
			t.Fatalf("unknown command sent %d packets", got-before)
		}
	})

	t.Run("missing player context", func(t *testing.T) {
		w, _, leader, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
		before := leader.Session.QueuedPacketsForTest()
		w.guildWarCommand(nil, "guildfame", "")
		noSession := *leader
		noSession.Session = nil
		w.guildWarCommand(&noSession, "guildfame", "")
		noCharacter := *leader
		noCharacter.Char = nil
		w.guildWarCommand(&noCharacter, "guildfame", "")
		if got := leader.Session.QueuedPacketsForTest(); got != before {
			t.Fatalf("invalid player context sent %d packets", got-before)
		}
	})
}

func TestGuildWarCommandUsesCanonicalGuildMembership(t *testing.T) {
	w, _, leader, _, _ := scheduledWarWorld(t, "2026-09-12 12:00")
	leader.Char.GuildID = 999
	drainPlayerPackets(leader)
	w.guildWarCommand(leader, "guildfame", "")
	assertCriticalPickupPacket(t, leader, wire.MessageWhisper(0, "[WAR]", "Guild Alpha: 0 fame.", 7))
}
