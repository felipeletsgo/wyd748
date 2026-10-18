package game

import (
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestCriticalInstanceFirstWaveRespectsExplicitRoomBounds(t *testing.T) {
	w, owner, _, _, clock := instanceTestWorld()
	stage := model.VolatileInstanceStage{
		X: 2199, Y: 2200, AreaX: 2200, AreaY: 2200, AreaRadius: 1,
		SpawnX: 2201, SpawnY: 2200,
		Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 5}},
	}
	cfg := *instanceTestRule().Instance
	cfg.Stages = []model.VolatileInstanceStage{stage}
	inst := &ItemInstance{Config: cfg, RuntimeID: cfg.ID + "#detached", MemberIDs: []uint16{owner.ID}}
	tiles := make(map[criticalTeleportTile]*Player)
	mobs, err := w.planCriticalInstanceFirstWave(inst, stage, clock.Now(), tiles, owner)
	if err != nil || len(mobs) != 5 || inst.Remaining != 5 {
		t.Fatalf("detached wave: count=%d remaining=%d err=%v", len(mobs), inst.Remaining, err)
	}
	seen := make(map[criticalTeleportTile]bool)
	for _, mob := range mobs {
		tile := criticalTile(mob.X, mob.Y, itemInstanceGameplaySpace(inst))
		if !instanceStageContains(stage, mob.X, mob.Y) || seen[tile] || tiles[tile] != owner {
			t.Fatalf("detached mob has invalid placement at %d,%d", mob.X, mob.Y)
		}
		seen[tile] = true
		if w.mobsByID[mob.ID] != nil {
			t.Fatal("detached admission published a monster before settlement")
		}
	}
}

// Crossing a logical room edge changes visibility even when every mob stays
// inside both 32-tile view windows. No mob movement is needed for recovery.
func TestInstanceVisibilityReconcilesRoomBoundaryCrossing(t *testing.T) {
	w, p, outsider, _, _ := instanceTestWorld()
	p.Party, outsider.Party = nil, nil
	rule := w.volatiles.Items[100]
	rule.Instance.Spawns[0].Count = 5
	w.volatiles.Items[100] = rule
	w.onUseItem(p.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil || inst.Remaining != 5 {
		t.Fatal("five-monster instance did not start")
	}
	drain := func() map[uint16]int {
		t.Helper()
		counts := make(map[uint16]int)
		for {
			packet, ok := p.Session.DequeuePacketForTest()
			if !ok {
				return counts
			}
			if !wire.Decrypt(packet) {
				t.Fatal("invalid outbound checksum")
			}
			counts[wire.ParseHeader(packet).Type]++
		}
	}
	p.X, p.Y = 2208, 2200 // The inclusive edge of this room.
	p.Char.X, p.Char.Y = p.X, p.Y
	w.refreshPlayerVisibility(p)
	drain()
	for _, step := range []struct {
		x                uint16
		visible          bool
		creates, removes int
	}{
		{2209, false, 0, 5},
		{2208, true, 5, 0},
		{2207, true, 0, 0},
		{2209, false, 0, 5},
		{2208, true, 5, 0},
	} {
		oldX, oldY := p.X, p.Y
		p.X, p.Char.X = step.x, step.x
		w.refreshPlayerVisibilityAfterMove(p, oldX, oldY)
		packets := drain()
		if packets[wire.OpCreateMob] != step.creates || packets[wire.OpRemoveMob] != step.removes {
			t.Fatalf("x=%d: creates=%d removes=%d; want %d/%d", step.x,
				packets[wire.OpCreateMob], packets[wire.OpRemoveMob], step.creates, step.removes)
		}
		for id := range inst.MobIDs {
			mob := w.mobsByID[id]
			if mob == nil || mob.Dead || mob.HP == 0 || p.hasVisible(id) != step.visible ||
				w.playerCanInteractWithMob(p, mob) != step.visible {
				t.Fatalf("mob %d has inconsistent visibility/combat state at x=%d", id, step.x)
			}
		}
		if inst.Remaining != 5 || len(inst.MobIDs) != 5 {
			t.Fatal("room crossing changed the authoritative population")
		}
	}
}

// A client can lose an actor after ignoring its reentry while the old actor
// awaits deletion. Small movements do not send another CreateMob: the server
// correctly retains that actor in Visible until another view-boundary crossing.
func TestInstanceLastFiveMobsRemainAuthoritativeAcrossReentry(t *testing.T) {
	w, p, outsider, _, _ := instanceTestWorld()
	p.Party, outsider.Party = nil, nil
	rule := w.volatiles.Items[100]
	rule.Instance.AreaRadius = 64
	rule.Instance.Spawns[0].Count = 5
	w.volatiles.Items[100] = rule
	w.onUseItem(p.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil || len(inst.MobIDs) != 5 || inst.Remaining != 5 {
		t.Fatal("five-monster instance did not start")
	}
	drain := func(player *Player) map[uint16]int {
		t.Helper()
		counts := make(map[uint16]int)
		for {
			packet, ok := player.Session.DequeuePacketForTest()
			if !ok {
				return counts
			}
			if !wire.Decrypt(packet) {
				t.Fatal("invalid outbound checksum")
			}
			counts[wire.ParseHeader(packet).Type]++
		}
	}
	drain(p)
	p.X, p.Y = 2200, 2200
	p.Char.X, p.Char.Y = p.X, p.Y
	row := uint16(2198)
	for id := range inst.MobIDs {
		m := w.mobsByID[id]
		oldX, oldY := m.X, m.Y
		m.X, m.Y = 2232, row
		row++
		w.moveMobSpatial(m, oldX, oldY)
	}
	w.refreshPlayerVisibility(p)
	drain(p)
	for _, step := range []struct {
		x                    uint16
		wantCreate, wantDrop int
	}{
		{2199, 0, 5}, // All five leave the 32-tile window.
		{2200, 5, 0}, // Reenter before the client's ten-second deletion expires.
		{2201, 0, 0}, // Walking within the window does not resend known actors.
	} {
		oldX, oldY := p.X, p.Y
		p.X = step.x
		p.Char.X = p.X
		w.refreshPlayerVisibilityAfterMove(p, oldX, oldY)
		counts := drain(p)
		if counts[wire.OpCreateMob] != step.wantCreate || counts[wire.OpRemoveMob] != step.wantDrop {
			t.Fatalf("x=%d: create=%d remove=%d; want create=%d remove=%d", step.x,
				counts[wire.OpCreateMob], counts[wire.OpRemoveMob], step.wantCreate, step.wantDrop)
		}
		if inst.Remaining != 5 || len(inst.MobIDs) != 5 {
			t.Fatal("visibility changed the authoritative remaining count")
		}
		for id := range inst.MobIDs {
			m := w.mobsByID[id]
			if m == nil || m.Dead || m.HP == 0 || p.hasVisible(id) != (step.wantDrop == 0) {
				t.Fatalf("mob %d lost authoritative state or visibility", id)
			}
		}
	}
	outsider.X, outsider.Y = p.X, p.Y
	outsider.Char.X, outsider.Char.Y = outsider.X, outsider.Y
	w.refreshPlayerVisibility(outsider)
	for id := range inst.MobIDs {
		if outsider.hasVisible(id) {
			t.Fatalf("instance mob %d leaked to a non-member", id)
		}
	}
}
