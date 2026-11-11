package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestRestrictRuneBossAccessMovesIntruderButKeepsLeader(t *testing.T) {
	w, leader, member, store, clock := runeTrackTestWorld(t)
	inst := enterRuneTrack(t, w, leader, clock, 2)
	inst.MemberIDs = []uint16{leader.ID, member.ID}
	inst.RuneTrack.Parties = []runeTrackParty{{
		Leader:  runeTrackRef(leader),
		Members: []runeTrackMember{runeTrackRef(leader), runeTrackRef(member)},
	}}
	w.setPlayerInstanceIndex(leader.ID, inst.RuntimeID)
	w.setPlayerInstanceIndex(member.ID, inst.RuntimeID)

	leader.X, leader.Y, leader.Char.X, leader.Char.Y = 3400, 1437, 3400, 1437
	member.X, member.Y, member.Char.X, member.Char.Y = 3385, 1435, 3385, 1435
	w.updatePlayerSpatial(leader)
	w.updatePlayerSpatial(member)
	boss := runeMob(t, w, inst, 5798)
	boss.X, boss.Y = 3389, 1437
	w.restrictRuneBossAccess(inst)

	if runeInside(member.X, member.Y, [4]uint16{3382, 1426, 3394, 1466}) ||
		member.X != member.Char.X || member.Y != member.Char.Y {
		t.Fatalf("intruder remained in boss area or desynchronized: world=(%d,%d), character=(%d,%d)",
			member.X, member.Y, member.Char.X, member.Char.Y)
	}
	if leader.X != 3400 || leader.Y != 1437 || store.gameSaves == 0 {
		t.Fatal("boss boundary moved the party leader or skipped the intruder mutation")
	}
}

func TestRestrictRuneBossAccessReturnsEscapedBossToArena(t *testing.T) {
	w, leader, _, _, clock := runeTrackTestWorld(t)
	inst := enterRuneTrack(t, w, leader, clock, 2)
	inst.RuneTrack.Parties = nil
	boss := runeMob(t, w, inst, 5798)
	oldX, oldY := boss.X, boss.Y
	boss.X, boss.Y = 3395, 1437
	w.moveMobSpatial(boss, oldX, oldY)

	w.restrictRuneBossAccess(inst)

	if boss.X != 3389 || boss.Y != 1437 {
		t.Fatalf("escaped boss position=(%d,%d), want arena position=(3389,1437)", boss.X, boss.Y)
	}
	found := false
	for _, mob := range w.nearbyMobs(3389, 1437, 0) {
		if mob == boss {
			found = true
			break
		}
	}
	if !found {
		t.Fatal("boss relocation did not update the spatial index")
	}
	if boss.Def == nil || boss.Def.Tipo != model.TipoMonstro {
		t.Fatal("boss relocation changed its monster definition")
	}
}
