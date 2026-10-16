package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

// trainingFieldTerrain marks the native training field (attribute 0x80) on a
// flat, walkable map.
func trainingFieldTerrain() model.TerrainMap {
	terrain := loadedFlatTerrain()
	for y := 1932; y <= 2048; y += 4 {
		for x := 2064; x <= 2156; x += 4 {
			terrain.Attribute[(y/4)*model.AttributeWidth+x/4] |= 0x80
		}
	}
	return terrain
}

func TestNewbieZoneEligibilityUsesDisplayedLevel35(t *testing.T) {
	for _, tc := range []struct {
		level     uint32
		evolution string
		want      bool
	}{
		{0, "", true},
		{33, "mortal", true},  // displayed 34
		{34, "mortal", false}, // displayed 35
		{0, "arch", false},
	} {
		ch := &model.Char{Score: &model.Score{Level: tc.level}, Evolution: tc.evolution}
		if got := newbieZoneEligible(ch); got != tc.want {
			t.Fatalf("level %d %q: eligible=%t, want %t", tc.level, tc.evolution, got, tc.want)
		}
	}
}

// Reaching displayed level 35 inside the field returns the character to its
// home city; walking or teleporting back in is refused.
func TestTrainingFieldReturnsAndBlocksLevel35(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.terrain = trainingFieldTerrain()
	p.X, p.Y = mortalBeginnerSpawnX, mortalBeginnerSpawnY
	p.Char.X, p.Char.Y = p.X, p.Y

	p.Char.Score.Level = 33
	w.tickNewbieZone(time.Now())
	if p.X != mortalBeginnerSpawnX || p.Y != mortalBeginnerSpawnY {
		t.Fatal("an eligible mortal was removed from the training field")
	}

	p.Char.Score.Level = 34
	before := p.Session.QueuedPacketsForTest()
	w.tickNewbieZone(time.Now())
	if w.terrain.NewbieZone(p.X, p.Y) || p.Session.QueuedPacketsForTest() <= before {
		t.Fatalf("level 35 stayed in the training field at (%d,%d)", p.X, p.Y)
	}

	if w.newbieZoneDestinationAllowed(p, mortalBeginnerSpawnX, mortalBeginnerSpawnY) {
		t.Fatal("level 35 may walk back into the training field")
	}
	if _, _, ok := w.resolvePlayerTeleportDestination(p, mortalBeginnerSpawnX, mortalBeginnerSpawnY); ok {
		t.Fatal("level 35 may teleport back into the training field")
	}
	if !w.newbieZoneDestinationAllowed(p, 2100, 2100) {
		t.Fatal("the city outside the field was blocked")
	}
}

// Generator-backed instance groups stay inside their StartRange; inline
// spawns keep the room radius.
func TestInstanceSpawnRadiusUsesGeneratorRange(t *testing.T) {
	generator := model.VolatileInstanceSpawn{NPC: "Meio_Orc__", Count: 25, X: 801, Y: 3684, Range: 3}
	if got := instanceSpawnRadius(generator, 24); got != 3 {
		t.Fatalf("generator radius = %d, want 3", got)
	}
	if got := instanceSpawnRadius(model.VolatileInstanceSpawn{NPC: "Skull2", Count: 100}, 24); got != 24 {
		t.Fatalf("inline radius = %d, want the room radius 24", got)
	}
}
