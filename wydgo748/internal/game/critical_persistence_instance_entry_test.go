package game

import (
	"encoding/binary"
	"errors"
	"maps"
	"reflect"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestExpiredCriticalInstanceEntryChecksEachDeadline(t *testing.T) {
	now := time.Unix(500, 0)
	tests := []struct {
		name string
		inst *ItemInstance
		want bool
	}{
		{name: "no deadlines", inst: &ItemInstance{}, want: false},
		{name: "combat deadline future", inst: &ItemInstance{CombatDeadline: now.Add(time.Second)}, want: false},
		{name: "legacy deadline elapsed", inst: &ItemInstance{Deadline: now}, want: true},
		{name: "hard deadline reached", inst: &ItemInstance{HardDeadline: now}, want: true},
		{name: "schedule end reached", inst: &ItemInstance{ScheduleEnd: now}, want: true},
		{name: "all deadlines future", inst: &ItemInstance{CombatDeadline: now.Add(time.Second), HardDeadline: now.Add(2 * time.Second), ScheduleEnd: now.Add(3 * time.Second)}, want: false},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := expiredCriticalInstanceEntry(tt.inst, now); got != tt.want {
				t.Fatalf("expiredCriticalInstanceEntry() = %v, want %v", got, tt.want)
			}
		})
	}
}

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

func TestCriticalInstanceTicketPreflightFailuresReserveNoState(t *testing.T) {
	for _, tc := range []struct {
		name          string
		kind          string
		expectedUID   string
		prepare       func(*World, *Player, *Player, *model.VolatileRule)
		wantErrorText string
	}{
		{name: "invalid player identity", wantErrorText: "participant identity changed"},
		{name: "missing instance configuration", wantErrorText: "not configured"},
		{name: "wrong action", wantErrorText: "not configured", prepare: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule) { rule.Action = "generic" }},
		{name: "stale ticket identity", expectedUID: "stale-uid", wantErrorText: "participant identity changed"},
		{name: "missing catalog item", wantErrorText: "not in the server catalog", prepare: func(w *World, _ *Player, _ *Player, _ *model.VolatileRule) { delete(w.items, 100) }},
		{name: "exclusive group occupied", wantErrorText: "already active", prepare: func(w *World, _ *Player, _ *Player, rule *model.VolatileRule) {
			rule.Instance.ExclusiveGroup = "magic-chamber"
			w.itemInstances = map[string]*ItemInstance{"occupied": {Config: model.VolatileInstance{ExclusiveGroup: "magic-chamber"}}}
		}},
		{name: "outside configured entry area", wantErrorText: "cannot be used from here", prepare: func(_ *World, p *Player, _ *Player, rule *model.VolatileRule) {
			rule.Instance.EntryAreas = []model.VolatileInstanceEntryArea{{
				MinX: p.X + 1, MaxX: p.X + 2, MinY: p.Y + 1, MaxY: p.Y + 2,
			}}
		}},
		{name: "closed shared schedule", kind: "shared-join", wantErrorText: "not open right now", prepare: func(w *World, _ *Player, _ *Player, rule *model.VolatileRule) {
			w.itemInstances[rule.Instance.ID].ScheduleEnd = w.now().Add(-time.Second)
		}},
		{name: "invalid shared entry mode", kind: "shared-join", wantErrorText: "entry mode is not configured", prepare: func(_ *World, _ *Player, _ *Player, rule *model.VolatileRule) {
			rule.PartyMode = "unsupported"
		}},
		{name: "nonleader party entry", wantErrorText: "eligible party leader", prepare: func(_ *World, p *Player, _ *Player, rule *model.VolatileRule) {
			rule.PartyMode = "party"
			p.Party = nil
		}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			kind := tc.kind
			if kind == "" {
				kind = "water"
			}
			w, leader, _, _, _, backend, queue, rule, _ := criticalInstanceEntryFixture(t, kind, "commit")
			if tc.name == "invalid player identity" {
				leader.InWorld = false
			}
			if tc.name == "missing instance configuration" {
				rule.Instance = nil
			}
			if tc.prepare != nil {
				tc.prepare(w, leader, nil, &rule)
			}
			before := accountStateSnapshot(leader.Account)
			beforeItemInstances := maps.Clone(w.itemInstances)
			err := w.queueCriticalInstanceTicket(leader, 0, rule, tc.expectedUID)
			if err == nil || !strings.Contains(err.Error(), tc.wantErrorText) {
				t.Fatalf("preflight error=%v, want text %q", err, tc.wantErrorText)
			}
			if !reflect.DeepEqual(accountStateSnapshot(leader.Account), before) ||
				!reflect.DeepEqual(w.itemInstances, beforeItemInstances) || len(queue.pending) != 0 ||
				len(backend.entered) != 0 || len(w.criticalTeleportTiles) != 0 || len(w.criticalSummonIDs) != 0 {
				t.Fatal("rejected ticket preflight changed account, membership, reservations or persistence")
			}
		})
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
