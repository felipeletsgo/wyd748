package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func installHellGatePortalTestInstance(w *World, p *Player, state string, x, y uint16) *ItemInstance {
	cfg := model.VolatileInstance{
		ID:           "hell-gate-v2",
		Name:         "Hell Gate v2",
		StateMachine: "hell_gate",
		Mode:         "shared_timed_zone",
		HellGate: &model.VolatileHellGate{
			Version: 2,
			Portals: []model.Teleport{
				{SourceX: 1756, SourceY: 3616, DestX: 1811, DestY: 3606, Price: 777},
				{SourceX: 1756, SourceY: 3644, DestX: 1876, DestY: 3606, Price: 777},
				{SourceX: 1756, SourceY: 3672, DestX: 1811, DestY: 3692, Price: 777},
			},
			Quadrants: []model.VolatileHellGateQuadrant{
				{ID: 1, X: 1811, Y: 3606, SpawnX: 1811, SpawnY: 3606, AreaRadius: 28},
				{ID: 2, X: 1876, Y: 3606, SpawnX: 1876, SpawnY: 3606, AreaRadius: 28},
				{ID: 3, X: 1876, Y: 3692, SpawnX: 1876, SpawnY: 3692, AreaRadius: 28},
				{ID: 4, X: 1811, Y: 3692, SpawnX: 1811, SpawnY: 3692, AreaRadius: 28},
			},
		},
	}
	inst := &ItemInstance{
		Config:              cfg,
		RuntimeID:           "hell-gate-v2:1",
		LeaderID:            p.ID,
		MemberIDs:           []uint16{p.ID},
		MemberCharacterUIDs: []string{p.Char.UID},
		LeaderCharacterUID:  p.Char.UID,
		State:               state,
		HardDeadline:        w.now().Add(time.Minute),
	}
	w.itemInstances[inst.RuntimeID] = inst
	w.setPlayerInstanceIndex(p.ID, inst.RuntimeID)
	p.X, p.Y = x, y
	p.Char.X, p.Char.Y = x, y
	w.updatePlayerSpatial(p)
	return inst
}

func hellGatePortalTestWorld(t *testing.T, state string, x, y uint16) (*World, *Player, *ItemInstance, *guildFlowStore, *fakeClock) {
	t.Helper()
	w, p, _, store, clock := instanceTestWorld()
	inst := installHellGatePortalTestInstance(w, p, state, x, y)
	p.Char.Gold = 1000
	return w, p, inst, store, clock
}

func hellGateGlobalFallback() []model.Teleport {
	return []model.Teleport{{SourceX: 1756, SourceY: 3616, DestX: 1900, DestY: 3600}}
}

func TestHellGatePortalRejectsClosedPhasesBeforeGlobalRoute(t *testing.T) {
	for _, state := range []string{"preparing", "entry", "corridors"} {
		t.Run(state, func(t *testing.T) {
			w, p, inst, store, _ := hellGatePortalTestWorld(t, state, 1759, 3619)
			w.teleports = hellGateGlobalFallback()
			before := accountStateSnapshot(p.Account)
			w.onReqTeleport(p.Session, make([]byte, 16))
			if p.X != 1759 || p.Y != 3619 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
				len(store.snapshots) != 0 || !itemInstanceHasMember(inst, p.ID) {
				t.Fatalf("closed phase used the global route or changed state: pos=(%d,%d) snapshots=%d", p.X, p.Y, len(store.snapshots))
			}
		})
	}
}

func TestHellGatePortalRequiresAnOpenDeadline(t *testing.T) {
	w, p, inst, store, clock := hellGatePortalTestWorld(t, "lich_puzzle", 1759, 3619)
	w.teleports = hellGateGlobalFallback()
	clock.Advance(2 * time.Minute)
	before := accountStateSnapshot(p.Account)
	w.onReqTeleport(p.Session, make([]byte, 16))
	if p.X != 1759 || p.Y != 3619 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
		len(store.snapshots) != 0 || w.playerRuntimeInstanceID(p.ID) != inst.RuntimeID {
		t.Fatal("expired Hell Gate portal was accepted or bypassed to the global route")
	}
}

func TestHellGatePortalUsesNativeBlockAndPreservesMembership(t *testing.T) {
	for _, state := range []string{"lich_puzzle", "protection"} {
		t.Run(state, func(t *testing.T) {
			w, p, inst, store, _ := hellGatePortalTestWorld(t, state, 1759, 3619)
			w.teleports = hellGateGlobalFallback()
			beforeGold := p.Char.Gold
			beforeMembers := append([]uint16(nil), inst.MemberIDs...)
			beforeRuntime := w.playerRuntimeInstanceID(p.ID)
			p.AirMoveActive = true
			p.AirMoveRoute = 2
			p.MovePublished = true
			p.MoveAuthorityRoute = []byte{1, 2}

			w.onReqTeleport(p.Session, make([]byte, 16))
			if p.X != 1811 || p.Y != 3606 || p.Char.X != 1811 || p.Char.Y != 3606 {
				t.Fatalf("native 4x4 portal did not use its exact destination: (%d,%d)", p.X, p.Y)
			}
			if p.Char.Gold != beforeGold || !reflect.DeepEqual(inst.MemberIDs, beforeMembers) ||
				w.playerRuntimeInstanceID(p.ID) != beforeRuntime || len(store.snapshots) != 1 {
				t.Fatalf("portal changed charge or membership: gold=%d members=%v runtime=%q snapshots=%d",
					p.Char.Gold, inst.MemberIDs, w.playerRuntimeInstanceID(p.ID), len(store.snapshots))
			}
			if p.AirMoveActive || p.AirMoveRoute != 0 || p.MovePublished || len(p.MoveAuthorityRoute) != 0 {
				t.Fatal("portal publication retained movement state")
			}

			// A repeated request at the destination is still consumed as an
			// instance-member request and cannot detach the membership.
			if !w.handleHellGatePortal(p) || p.X != 1811 || p.Y != 3606 ||
				!reflect.DeepEqual(inst.MemberIDs, beforeMembers) || len(store.snapshots) != 1 {
				t.Fatal("repeated member request changed the committed instance state")
			}
		})
	}
}

func TestHellGatePortalRejectsRewardAndExitGrace(t *testing.T) {
	for _, test := range []struct {
		name  string
		apply func(*ItemInstance, time.Time)
	}{
		{name: "reward granted", apply: func(inst *ItemInstance, _ time.Time) { inst.RewardGranted = true }},
		{name: "exit scheduled", apply: func(inst *ItemInstance, now time.Time) { inst.ExitAt = now.Add(10 * time.Second) }},
	} {
		t.Run(test.name, func(t *testing.T) {
			w, p, inst, store, _ := hellGatePortalTestWorld(t, "protection", 1759, 3619)
			w.teleports = hellGateGlobalFallback()
			test.apply(inst, w.now())
			before := accountStateSnapshot(p.Account)
			w.onReqTeleport(p.Session, make([]byte, 16))
			if p.X != 1759 || p.Y != 3619 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
				len(store.snapshots) != 0 || w.playerRuntimeInstanceID(p.ID) != inst.RuntimeID {
				t.Fatal("reward or exit-grace portal request was accepted or bypassed")
			}
		})
	}
}

func TestHellGatePortalUsesNearestReachableFreeTileWhenAnchorIsPopulated(t *testing.T) {
	w, p, inst, store, _ := hellGatePortalTestWorld(t, "protection", 1759, 3619)
	w.teleports = hellGateGlobalFallback()
	anchorMob := &Mob{ID: 900, X: 1811, Y: 3606, HP: 1, InstanceID: inst.RuntimeID}
	w.appendMobInstance(anchorMob)
	w.registerMobSpatial(anchorMob)

	w.onReqTeleport(p.Session, make([]byte, 16))
	if p.X == 1811 && p.Y == 3606 {
		t.Fatal("portal correction selected the occupied room spawn anchor")
	}
	if p.X != 1812 || p.Y != 3606 {
		t.Fatalf("portal correction did not choose the nearest deterministic free tile: (%d,%d)", p.X, p.Y)
	}
	if !w.terrain.Walkable(p.X, p.Y) || chebyshev(p.X, p.Y, 1811, 3606) > 28 ||
		w.mobsByID[anchorMob.ID] != anchorMob || len(store.snapshots) != 1 ||
		w.playerRuntimeInstanceID(p.ID) != inst.RuntimeID {
		t.Fatal("populated portal correction left the room component or instance state invalid")
	}
}

func TestHellGatePortalConsumesUnknownAndForeignMemberRequests(t *testing.T) {
	// Two tiles beyond the instance block: outside the one-tile margin too.
	w, p, inst, store, _ := hellGatePortalTestWorld(t, "lich_puzzle", 1761, 3616)
	w.teleports = []model.Teleport{{SourceX: 1761, SourceY: 3616, DestX: 1900, DestY: 3600}}
	before := accountStateSnapshot(p.Account)
	w.onReqTeleport(p.Session, make([]byte, 16))
	if p.X != 1761 || p.Y != 3616 || !reflect.DeepEqual(accountStateSnapshot(p.Account), before) ||
		len(store.snapshots) != 0 || w.playerRuntimeInstanceID(p.ID) != inst.RuntimeID {
		t.Fatal("unknown member portal request fell through to the global route")
	}

	foreign, _ := networkedTestPlayer(3, "Foreign", 1759, 3619)
	w.players[foreign.Session], w.playersByID[foreign.ID] = foreign, foreign
	w.indexPlayerCharacter(foreign)
	w.updatePlayerSpatial(foreign)
	foreignCfg := cfgCopy(inst.Config)
	foreignCfg.ID = "hell-gate-foreign-v2"
	foreignCfg.HellGate.Portals = []model.Teleport{{SourceX: 1600, SourceY: 3600, DestX: 1604, DestY: 3600}}
	foreignInst := &ItemInstance{
		Config:              foreignCfg,
		RuntimeID:           "hell-gate-foreign-v2:1",
		LeaderID:            foreign.ID,
		MemberIDs:           []uint16{foreign.ID},
		MemberCharacterUIDs: []string{foreign.Char.UID},
		LeaderCharacterUID:  foreign.Char.UID,
		State:               "lich_puzzle",
		HardDeadline:        w.now().Add(time.Minute),
	}
	w.itemInstances[foreignInst.RuntimeID] = foreignInst
	w.setPlayerInstanceIndex(foreign.ID, foreignInst.RuntimeID)
	foreignBefore := accountStateSnapshot(foreign.Account)
	w.onReqTeleport(foreign.Session, make([]byte, 16))
	if foreign.X != 1759 || foreign.Y != 3619 || !reflect.DeepEqual(accountStateSnapshot(foreign.Account), foreignBefore) ||
		w.playerRuntimeInstanceID(foreign.ID) != foreignInst.RuntimeID || len(store.snapshots) != 0 {
		t.Fatal("foreign instance member request fell through to the global route")
	}
}

func TestHellGatePortalNonmemberFallsThroughToGlobalRoute(t *testing.T) {
	w, _, _, _, _ := hellGatePortalTestWorld(t, "lich_puzzle", 1759, 3619)
	nonmember, _ := networkedTestPlayer(3, "Nonmember", 1759, 3619)
	w.players[nonmember.Session], w.playersByID[nonmember.ID] = nonmember, nonmember
	w.indexPlayerCharacter(nonmember)
	w.updatePlayerSpatial(nonmember)
	w.teleports = hellGateGlobalFallback()
	w.onReqTeleport(nonmember.Session, make([]byte, 16))
	if nonmember.X == 1759 && nonmember.Y == 3619 || w.playerRuntimeInstanceID(nonmember.ID) != "" {
		t.Fatal("nonmember did not retain the ordinary global teleport path")
	}
}

func TestHellGatePortalRollsBackAccountAndPositionOnPersistenceFailure(t *testing.T) {
	w, p, inst, store, _ := hellGatePortalTestWorld(t, "protection", 1759, 3619)
	w.teleports = hellGateGlobalFallback()
	store.err = errors.New("Hell Gate portal persistence failure")
	before := accountStateSnapshot(p.Account)
	beforeMembers := append([]uint16(nil), inst.MemberIDs...)
	w.onReqTeleport(p.Session, make([]byte, 16))
	if p.X != 1759 || p.Y != 3619 || p.Char.X != 1759 || p.Char.Y != 3619 ||
		!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(inst.MemberIDs, beforeMembers) ||
		w.playerRuntimeInstanceID(p.ID) != inst.RuntimeID {
		t.Fatal("failed portal persistence did not restore account, position or membership")
	}
}

func TestHellGatePortalCriticalRejectionRestoresPositionAndMembership(t *testing.T) {
	w, leader, _, peer, _, base, queue, _, release := criticalInstanceEntryFixture(t, "water", "rejection")
	leader.Trade = nil
	peer.Trade = nil
	inst := installHellGatePortalTestInstance(w, leader, "lich_puzzle", 1759, 3619)
	w.teleports = hellGateGlobalFallback()
	before := accountStateSnapshot(leader.Account)
	w.onReqTeleport(leader.Session, make([]byte, 16))
	proposal := awaitCriticalWrite(t, base.entered)
	if len(proposal.accounts) != 1 || len(proposal.accounts[0].Chars) == 0 || proposal.instances == nil {
		t.Fatalf("critical portal proposal shape: accounts=%d instances=%v", len(proposal.accounts), proposal.instances != nil)
	}
	if proposal.accounts[0] == leader.Account {
		t.Fatal("critical portal proposal retained the live account pointer")
	}
	if got := proposal.accounts[0].Chars[0]; got.X != playerEntryX || got.Y != playerEntryY {
		t.Fatalf("critical portal proposal durable position: got=(%d,%d) want=(%d,%d)", got.X, got.Y, playerEntryX, playerEntryY)
	}
	if leader.X != 1759 || leader.Y != 3619 || !reflect.DeepEqual(accountStateSnapshot(leader.Account), before) ||
		!itemInstanceHasMember(inst, leader.ID) {
		t.Fatalf("critical portal live state changed during admission: pos=(%d,%d) accountUnchanged=%t member=%t",
			leader.X, leader.Y, reflect.DeepEqual(accountStateSnapshot(leader.Account), before), itemInstanceHasMember(inst, leader.ID))
	}
	if owner := w.criticalTeleportTiles[criticalTile(1811, 3606, inst.RuntimeID)]; owner != leader {
		t.Fatalf("critical portal reservation: got=%p want=%p all=%v", owner, leader, w.criticalTeleportTiles)
	}
	if !w.handleHellGatePortal(leader) || leader.X != 1759 || leader.Y != 3619 ||
		len(w.criticalTeleportTiles) != 1 || !w.criticalInstanceReserved() {
		t.Fatal("critical instance fencing did not consume the repeated portal request")
	}

	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if leader.X != 1759 || leader.Y != 3619 || leader.Char.X != 1759 || leader.Char.Y != 3619 ||
		!reflect.DeepEqual(accountStateSnapshot(leader.Account), before) || !itemInstanceHasMember(inst, leader.ID) ||
		w.playerRuntimeInstanceID(leader.ID) != inst.RuntimeID || len(w.criticalTeleportTiles) != 0 || w.criticalInstanceReserved() {
		t.Fatal("certified critical rejection did not restore the portal request atomically")
	}
}
