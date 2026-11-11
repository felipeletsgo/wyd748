package model

import "testing"

func newCoverageTerrainMap() TerrainMap {
	return TerrainMap{
		Height:    make([]byte, TerrainCells),
		Attribute: make([]byte, AttributeCells),
	}
}

func TestTerrainLoadedAndUnloadedPolicies(t *testing.T) {
	if (TerrainMap{}).Loaded() {
		t.Fatal("empty terrain reported loaded")
	}
	if (TerrainMap{Height: make([]byte, TerrainCells)}).Loaded() {
		t.Fatal("terrain without attributes reported loaded")
	}
	terrain := newCoverageTerrainMap()
	if !terrain.Loaded() {
		t.Fatal("complete terrain reported unloaded")
	}

	var unloaded TerrainMap
	if !unloaded.Walkable(1, 1) || !unloaded.PvPZone(1, 1) || !unloaded.HeightCompatible(1, 1, 2, 2) || !unloaded.LineOfSight(1, 1, 2, 2) {
		t.Fatal("unloaded test terrain should preserve permissive movement and PvP")
	}
	if unloaded.NewbieZone(1, 1) || unloaded.AttributeAt(1, 1) != 0 || unloaded.GuildArea(1, 1) {
		t.Fatal("unloaded terrain reported an attribute")
	}
}

func TestTerrainAttributeFlagsAndBounds(t *testing.T) {
	terrain := newCoverageTerrainMap()
	cell := (uint32(12)/4)*AttributeWidth + uint32(20)/4
	terrain.Attribute[cell] = 0xE0 | 0x40 | 0x20 | 0x80
	if !terrain.NewbieZone(20, 12) || !terrain.GuildArea(20, 12) || !terrain.PvPZone(20, 12) {
		t.Fatal("expected training, guild, and PvP flags in the shared 4x4 cell")
	}
	if terrain.AttributeAt(20, 12) != terrain.Attribute[cell] {
		t.Fatalf("AttributeAt = %#x, want %#x", terrain.AttributeAt(20, 12), terrain.Attribute[cell])
	}
	terrain.Attribute[cell] |= 0x01
	if terrain.PvPZone(20, 12) {
		t.Fatal("PvP flag must be suppressed by attribute 0x01")
	}
	if terrain.NewbieZone(TerrainWidth, 12) || terrain.NewbieZone(20, TerrainHeight) ||
		terrain.AttributeAt(TerrainWidth, 12) != 0 || terrain.AttributeAt(20, TerrainHeight) != 0 ||
		terrain.GuildArea(TerrainWidth, 12) || terrain.PvPZone(TerrainWidth, 12) {
		t.Fatal("out-of-map coordinates returned a terrain attribute")
	}
}

func TestTerrainHeightCompatibilityOriginsAndTargets(t *testing.T) {
	terrain := newCoverageTerrainMap()
	setHeight := func(x, y uint16, value byte) { terrain.Height[int(y)*TerrainWidth+int(x)] = value }
	if !terrain.HeightCompatible(TerrainWidth, 20, 21, 20) {
		t.Fatal("invalid source coordinate should let spawn search recover from an invalid origin")
	}
	setHeight(20, 20, TerrainBlockedByte)
	if !terrain.HeightCompatible(20, 20, 21, 20) {
		t.Fatal("blocked source height should let spawn search recover from an invalid origin")
	}
	setHeight(21, 20, TerrainBlockedByte)
	if terrain.HeightCompatible(20, 20, 21, 20) {
		t.Fatal("blocked destination should never be accepted")
	}
}

func TestTerrainLineOfSightAndClosedCorners(t *testing.T) {
	terrain := newCoverageTerrainMap()
	setHeight := func(x, y uint16, value byte) { terrain.Height[int(y)*TerrainWidth+int(x)] = value }
	if !terrain.LineOfSight(20, 20, 24, 20) || !terrain.LineOfSight(24, 20, 20, 20) ||
		!terrain.LineOfSight(20, 20, 20, 24) || !terrain.LineOfSight(20, 24, 20, 20) ||
		!terrain.LineOfSight(20, 20, 24, 24) || !terrain.LineOfSight(24, 24, 20, 20) {
		t.Fatal("clear horizontal, vertical, or diagonal segments were rejected")
	}

	setHeight(22, 20, TerrainBlockedByte)
	if terrain.LineOfSight(20, 20, 24, 20) {
		t.Fatal("line of sight crossed a blocked intermediate cell")
	}
	setHeight(22, 20, 0)
	setHeight(24, 24, TerrainBlockedByte)
	if terrain.LineOfSight(20, 20, 24, 24) {
		t.Fatal("line of sight accepted a blocked endpoint")
	}
	setHeight(24, 24, 0)
	setHeight(21, 20, TerrainBlockedByte)
	setHeight(20, 21, TerrainBlockedByte)
	if terrain.LineOfSight(20, 20, 21, 21) {
		t.Fatal("line of sight passed between two blocked cells at a closed corner")
	}
	setHeight(20, 21, 0)
	if !terrain.LineOfSight(20, 20, 21, 21) {
		t.Fatal("one open side of a diagonal corner should keep the path open")
	}
}

func TestTerrainLineOfSightUsesStrictRouteHeight(t *testing.T) {
	terrain := newCoverageTerrainMap()
	terrain.Height[30*TerrainWidth+30] = 0
	terrain.Height[30*TerrainWidth+31] = 7
	if !terrain.LineOfSight(30, 30, 31, 30) {
		t.Fatal("route height delta 7 should be accepted")
	}
	terrain.Height[30*TerrainWidth+31] = 8
	if terrain.LineOfSight(30, 30, 31, 30) {
		t.Fatal("route height delta 8 should be rejected")
	}
	if !terrain.HeightCompatible(30, 30, 31, 30) {
		t.Fatal("spawn height range should remain wider than route height range")
	}
}
