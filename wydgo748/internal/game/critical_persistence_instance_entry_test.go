package game

import (
	"encoding/binary"
	"errors"
	"maps"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func criticalInstanceEntryFixture(t *testing.T, kind, outcome string) (*World, *Player, *Player,
	*Player, *guildFlowStore, *criticalTestStore, *criticalPersistence, model.VolatileRule, func()) {
	t.Helper()
	w, leader, member, fallback, _ := instanceTestWorld()
	peer, _ := networkedTestPlayer(3, "TradePeer", leader.X+2, leader.Y)
	w.players[peer.Session], w.playersByID[peer.ID] = peer, peer
	w.indexPlayerCharacter(peer)
	w.updatePlayerSpatial(peer)
	for _, p := range []*Player{leader, member, peer} {
		w.indexPlayerCharacter(p)
		if !w.claimAccountSession(p.Session, p.Account.Name) {
			t.Fatal("instance fixture account admission failed")
		}
	}
	leader.Trade = &TradeState{OpponentID: peer.ID, Gold: 12, Checked: true}
	peer.Trade = &TradeState{OpponentID: leader.ID, Gold: 34, Checked: true}
	leader.Char.Inv[0].UID = "entry-ticket"
	rule := instanceTestRule()
	if kind == "shared" || kind == "shared-join" {
		rule.Instance.ID, rule.Instance.Mode = "shared-entry", "shared_timed_zone"
		rule.Instance.MaxPlayers = 13
	} else if kind == "big-cube" {
		rule.Instance.ID, rule.Instance.StateMachine = "big-entry", "big_cube"
		rule.Instance.Spawns = nil
		rule.Instance.Stages = []model.VolatileInstanceStage{{Name: "Question", X: 2200, Y: 2200,
			SpawnX: 2202, SpawnY: 2202, Quiz: &model.VolatileInstanceQuiz{Question: "Test_question",
				DurationSeconds: 10, TrueX: 2204, TrueY: 2204, FalseX: 2205, FalseY: 2205}}}
	}
	if kind == "shared-join" {
		inst := &ItemInstance{Config: cfgCopy(*rule.Instance), RuntimeID: rule.Instance.ID,
			MobIDs: map[uint16]struct{}{3200: {}}, Remaining: 1,
			ScheduleEnd: w.now().Add(time.Minute), HardDeadline: w.now().Add(time.Hour)}
		// A shared encounter can still admit participants after its combat clock.
		setInstanceCombatDeadline(inst, w.now().Add(-time.Second))
		w.itemInstances[inst.RuntimeID] = inst
		mob := &Mob{ID: 3200, Def: &w.npcs[0], X: 2208, Y: 2208, HP: 100, InstanceID: inst.RuntimeID}
		w.appendMobInstance(mob)
		w.mobsByID[mob.ID] = mob
		w.registerMobSpatial(mob)
	}
	w.volatiles.Items[100] = rule
	gate, release := criticalGate(t)
	base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
	if outcome == "rejection" {
		base.failure = criticalCertifiedRejection(t)
	} else if outcome == "unknown" {
		base.failure = errors.New("instance entry commit acknowledgement lost")
	}
	q := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 4)
	t.Cleanup(release)
	w.criticalWrites = newCriticalPersistenceOwner(q)
	return w, leader, member, peer, fallback, base, q, rule, release
}

func TestCriticalInstanceEntryDetachedBatchAndSettlement(t *testing.T) {
	for _, kind := range []string{"water", "shared", "shared-join", "big-cube"} {
		for _, outcome := range []string{"commit", "rejection", "unknown", "stale-party", "closed"} {
			t.Run(kind+"/"+outcome, func(t *testing.T) {
				w, leader, member, peer, fallback, base, q, rule, release := criticalInstanceEntryFixture(t, kind, outcome)
				people := []*Player{leader, member, peer}
				before := make(map[*Player]*model.Account)
				for _, p := range people {
					before[p] = accountStateSnapshot(p.Account)
				}
				membership := cloneCriticalInstanceMembership(w)
				oldMobs := maps.Clone(w.mobsByID)
				trade, peerTrade := leader.Trade, peer.Trade
				w.onUseItem(leader.Session, useItemPacket(0, 0))
				proposal := awaitCriticalWrite(t, base.entered)
				if len(proposal.accounts) != 3 || proposal.instances == nil || proposal.character != nil ||
					proposal.guilds != nil || proposal.accounts[0].Chars[0].Inv[0].Index != 0 {
					t.Fatal("entry omitted atomic participants, ticket consumption or aggregate")
				}
				for _, phase := range []string{"held", "completed-unacknowledged"} {
					if phase == "completed-unacknowledged" {
						release()
						awaitReadyCriticalResults(t, q, 1)
					}
					if !errors.Is(w.queueCriticalInstanceTicket(leader, 0, rule, ""), errCriticalPersistenceBusy) {
						t.Fatal("repeated entry was admitted while reserved")
					}
					if phase == "held" {
						// Exercise the production fence without consuming the completed result.
						w.tick()
					}
					remove := make([]byte, 16)
					binary.LittleEndian.PutUint32(remove[12:16], uint32(member.ID))
					w.onPartyRemove(leader.Session, remove)
					w.cancelTrade(peer, "competing cancellation")
					for _, p := range people {
						if !reflect.DeepEqual(accountStateSnapshot(p.Account), before[p]) || !w.criticalPlayerReserved(p) ||
							p.Session.QueuedPacketsForTest() != 0 {
							t.Fatalf("%s changed or published a retained participant", phase)
						}
					}
					if !reflect.DeepEqual(w.itemInstances, membership.itemInstances) ||
						!maps.Equal(w.playerInstance, membership.playerInstance) || !maps.Equal(w.mobsByID, oldMobs) ||
						leader.Trade != trade || peer.Trade != peerTrade || len(base.entered) != 0 {
						t.Fatalf("%s changed membership, monsters or trade", phase)
					}
				}
				if outcome == "stale-party" {
					leader.Party = &Party{Members: []*Player{leader, member}}
				} else if outcome == "closed" {
					leader.Session.Close()
				}
				w.processCriticalCompletions(1)
				if outcome == "commit" || outcome == "closed" {
					if w.shuttingDown || len(q.pending) != 0 || leader.Trade != nil || peer.Trade != nil ||
						len(w.criticalTeleportTiles) != 0 || len(w.criticalSummonIDs) != 0 {
						t.Fatal("acknowledged entry did not settle resources")
					}
					for i, p := range people {
						if !reflect.DeepEqual(accountPersistenceSnapshot(p.Account), proposal.accounts[i]) {
							t.Fatal("installed account differs from its durable proposal")
						}
					}
					inst := w.itemInstances[rule.Instance.ID]
					if inst == nil || !itemInstanceHasMember(inst, leader.ID) || !itemInstanceHasMember(inst, member.ID) ||
						itemInstanceHasMember(inst, peer.ID) || leader.X == before[leader].Chars[0].X ||
						!reflect.DeepEqual(w.instanceStateSnapshot(), proposal.instances) {
						t.Fatal("entry did not install its complete detached membership")
					}
					if kind == "big-cube" {
						if inst.State != "big_cube_waiting" || !inst.QuizDeadline.IsZero() || len(inst.MobIDs) != 0 {
							t.Fatal("prepared entry started the question before acknowledgement")
						}
					} else if len(inst.MobIDs) != 1 || inst.Remaining != 1 {
						t.Fatal("entry did not install the prepared wave")
					}
					if member.Session.QueuedPacketsForTest() == 0 {
						t.Fatal("acknowledged entry was not published to the open member")
					}
				} else {
					for _, p := range people {
						if !reflect.DeepEqual(accountStateSnapshot(p.Account), before[p]) {
							t.Fatal("unverified entry installed account state")
						}
					}
					if !reflect.DeepEqual(w.itemInstances, membership.itemInstances) || !maps.Equal(w.mobsByID, oldMobs) ||
						leader.Trade != trade || peer.Trade != peerTrade {
						t.Fatal("unverified entry installed membership, mobs or cancellation")
					}
					if outcome == "rejection" {
						if w.shuttingDown || len(q.pending) != 0 || len(w.criticalTeleportTiles) != 0 ||
							len(w.criticalSummonIDs) != 0 || w.criticalInstanceReserved() {
							t.Fatal("certified entry rejection did not release reservations")
						}
					} else if !w.shuttingDown || len(q.pending) != 1 || !w.criticalInstanceReserved() {
						t.Fatal("unknown or stale entry lost containment")
					}
				}
				if fallback.saves != 0 || fallback.gameSaves != 0 {
					t.Fatal("entry saved synchronously on World")
				}
			})
		}
	}
}

func TestCriticalBigCubeWindowStartsAfterDelayedEntryAcknowledgement(t *testing.T) {
	w, leader, member, _, _, base, q, rule, release := criticalInstanceEntryFixture(t, "big-cube", "commit")
	clock := newFakeClock(w.now())
	w.clock = clock
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	awaitCriticalWrite(t, base.entered)
	clock.Advance(20 * time.Second)
	w.tickItemInstances(clock.Now())
	if leader.Session.QueuedPacketsForTest() != 0 || member.Session.QueuedPacketsForTest() != 0 || len(w.itemInstances) != 0 {
		t.Fatal("pending entry published a question or installed the instance")
	}
	release()
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	inst := w.itemInstances[rule.Instance.ID]
	if inst == nil || !inst.QuizDeadline.IsZero() {
		t.Fatal("acknowledgement did not install a waiting question")
	}
	drainPlayerPackets(leader)
	drainPlayerPackets(member)
	w.tickItemInstances(clock.Now())
	if !inst.QuizDeadline.Equal(clock.Now().Add(10*time.Second)) || leader.Session.QueuedPacketsForTest() != 2 ||
		member.Session.QueuedPacketsForTest() != 2 {
		t.Fatal("persistence latency shortened the answer window or omitted a member")
	}
}

func TestCriticalWaterEntryChecksAutomaticReceiptIdentity(t *testing.T) {
	w, leader, _, _, _, base, q, rule, release := criticalInstanceEntryFixture(t, "water", "commit")
	item := &leader.Char.Inv[0]
	req := useItemRequest{srcType: placeInv}
	if w.useWaterScroll(leader.Session, leader, item, 0, rule, req, "another-item") || len(base.entered) != 0 {
		t.Fatal("automatic entry accepted a replaced reward item")
	}
	if !w.useWaterScroll(leader.Session, leader, item, 0, rule, req, item.UID) {
		t.Fatal("automatic entry did not admit the exact reward item")
	}
	awaitCriticalWrite(t, base.entered)
	if len(w.itemInstances) != 0 || item.Index == 0 {
		t.Fatal("automatic admission was reported as durable entry")
	}
	release()
	awaitReadyCriticalResults(t, q, 1)
	w.processCriticalCompletions(1)
	if w.itemInstances[rule.Instance.ID] == nil || item.Index != 0 {
		t.Fatal("automatic entry did not settle after durable acknowledgement")
	}
}
