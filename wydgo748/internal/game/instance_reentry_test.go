package game

import (
	"testing"

	"wydgo/internal/wire"
)

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
