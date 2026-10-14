package game

import (
	"strings"

	"wydgo/internal/model"
)

// W2PP GuildProcess creates the three Noatum towers explicitly during the
// Sunday castle battle. MinuteGenerate=-1 means event-owned, not permanent.
// Generator indices differ between installed files, so identify the Noatum
// tower by its template and region, never by W2PP's indices 23..25.
// The full castle battle is not implemented here; ordinary NPC generation
// must not impersonate that event, even on Sunday.
func generatorReservedForGuildWar(g model.NPCGener) bool {
	if !strings.EqualFold(generName(g.Leader), "Tower of Thor") {
		return false
	}
	start := g.Segments[0]
	return (warRect{1036, 1672, 1144, 1764}).contains(start.X, start.Y)
}
