package game

import (
	"path/filepath"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

func TestWaterSilverRestartProductionExitAllowsFirstScroll(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "catalog", "itemlist.csv"),
		filepath.Join(root, "catalog", "Itemname.csv"), filepath.Join(root, "catalog", "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	volatiles, err := data.LoadVolatilesWithInstances(filepath.Join(root, "content", "volatiles.json"),
		filepath.Join(root, "content", "instances.json"), filepath.Join(root, "spawns", "instances"), catalog.Items, catalog.Skills)
	if err != nil {
		t.Fatal(err)
	}
	for family, scroll := range map[string]uint16{"normal": 3173, "mystic": 777, "arcane": 3182} {
		t.Run(family, func(t *testing.T) {
			bossRule, _, ok := volatiles.Rule(scroll + 8)
			boss := bossRule.Instance
			if !ok || boss == nil || boss.ID != "water-"+family+"-boss" ||
				boss.RewardItem != 0 || boss.ChainNextItem != scroll {
				t.Fatalf("unexpected terminal room: %+v", boss)
			}
			rule, _, ok := volatiles.Rule(scroll)
			if !ok || rule.Instance == nil || !rule.Instance.ChainStart || !rule.Consume ||
				!isDurablePrivateWaterConfig(*rule.Instance) {
				t.Fatal("first scroll is not a consumable Water chain start")
			}
			area, ok := instanceEntryAreaAt(rule.Instance, boss.ExitX, boss.ExitY)
			if !ok || area.RequiresChain {
				t.Fatalf("terminal exit does not allow a fresh first scroll: %+v", area)
			}
		})
	}
}

func waterRestartFixture(t *testing.T, family string, scroll uint16) (*World, *Player, *Player, *ItemInstance, *fakeClock) {
	t.Helper()
	w, leader, member, _, clock := instanceTestWorld()
	first := instanceTestRule()
	first.Instance.ID = "water-" + family + "-1"
	w.volatiles.Items[scroll], w.volatiles.ItemCodes[scroll] = first, 21
	w.items[scroll] = model.ItemDef{Index: scroll}
	leader.Char.Inv[0] = model.Item{}
	var err error
	leader.Char.Inv[5], err = materializeItem(model.Item{Index: scroll})
	if err != nil {
		t.Fatal(err)
	}
	equipActiveSilverForWaterTest(t, w, leader)
	cfg := *first.Instance
	cfg.ID, cfg.ChainStart, cfg.RewardItem, cfg.ChainNextItem = "water-"+family+"-boss", false, 0, scroll
	inst := &ItemInstance{Config: cfg, RuntimeID: cfg.ID, LeaderID: leader.ID,
		MemberIDs: []uint16{leader.ID, member.ID}, MobIDs: make(map[uint16]struct{}),
		RewardGranted: true, ExitAt: clock.Now().Add(10 * time.Second), HardDeadline: clock.Now().Add(time.Minute)}
	w.itemInstances[inst.RuntimeID] = inst
	w.indexInstanceMembers(inst)
	for _, p := range []*Player{leader, member} {
		p.X, p.Y, p.Char.X, p.Char.Y = 2202, 2202, 2202, 2202
		w.updatePlayerSpatial(p)
	}
	return w, leader, member, inst, clock
}

func TestWaterSilverRestartsAllFamiliesAfterTerminalExit(t *testing.T) {
	for family, scroll := range map[string]uint16{"normal": 3173, "mystic": 777, "arcane": 3182} {
		t.Run(family, func(t *testing.T) {
			w, leader, member, boss, clock := waterRestartFixture(t, family, scroll)
			original := leader.Char.Inv[5]
			w.tickItemInstances(clock.Now())
			if leader.Char.Inv[5] != original {
				t.Fatal("first scroll consumed before terminal exit")
			}
			clock.Advance(10 * time.Second)
			w.tickItemInstances(clock.Now())
			next := w.itemInstanceForPlayer(leader.ID)
			if next == nil || next.Config.ID != "water-"+family+"-1" ||
				w.itemInstanceForPlayer(member.ID) != next || w.itemInstances[boss.RuntimeID] != nil ||
				leader.Char.Inv[5].Index != 0 {
				t.Fatalf("terminal exit did not restart party: next=%+v scroll=%+v", next, leader.Char.Inv[5])
			}
		})
	}
}

func TestWaterSilverRestartStopsWithoutEligibleFairyOrScroll(t *testing.T) {
	for _, reason := range []string{"unequipped", "expired", "paused", "exhausted", "wrong-family", "dead", "timeout"} {
		t.Run(reason, func(t *testing.T) {
			w, leader, _, inst, clock := waterRestartFixture(t, "normal", 3173)
			switch reason {
			case "unequipped":
				leader.Char.Equip[fairySlot] = model.Item{}
			case "expired":
				leader.Char.Equip[fairySlot].ExpiresUnix = clock.Now().Unix()
			case "paused":
				leader.Char.Equip[fairySlot].PausedUnix = clock.Now().Unix()
			case "exhausted":
				leader.Char.Inv[5] = model.Item{}
			case "wrong-family":
				leader.Char.Inv[5].Index = 3182
			case "dead":
				leader.Char.Score.CurHP = 0
				leader.Char.RuntimeScore = nil
			case "timeout":
				inst.RewardGranted = false
				inst.ExitAt = time.Time{}
				inst.HardDeadline = clock.Now().Add(10 * time.Second)
			}
			original := leader.Char.Inv[5]
			clock.Advance(10 * time.Second)
			w.tickItemInstances(clock.Now())
			if w.itemInstanceForPlayer(leader.ID) != nil || leader.Char.Inv[5] != original {
				t.Fatal("ineligible restart opened a room or consumed a scroll")
			}
		})
	}
}

func TestWaterSilverRestartWaitsForDurableExitAndEntry(t *testing.T) {
	for _, outcome := range []string{"commit", "rejected"} {
		t.Run(outcome, func(t *testing.T) {
			w, leader, member, inst, clock := waterRestartFixture(t, "normal", 3173)
			claimCriticalTestPlayers(t, w, leader, member)
			for _, p := range []*Player{leader, member} {
				w.indexPlayerCharacter(p)
			}
			gate, release := criticalGate(t)
			base := &criticalTestStore{entered: make(chan criticalWrite, 4), release: gate}
			if outcome == "rejected" {
				base.failure = criticalCertifiedRejection(t)
			}
			queue := newCriticalTestQueue(t, &criticalInstanceAtomicTestStore{&criticalInstanceTestStore{base}}, 1, 4)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			original := leader.Char.Inv[5]
			clock.Advance(10 * time.Second)
			w.tickItemInstances(clock.Now())
			awaitCriticalWrite(t, base.entered)
			if w.itemInstanceForPlayer(leader.ID) != inst || leader.Char.Inv[5] != original {
				t.Fatal("pending exit exposed restart or consumption")
			}
			release()
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if outcome == "rejected" {
				if w.itemInstanceForPlayer(leader.ID) != inst || leader.Char.Inv[5] != original {
					t.Fatal("rejected exit consumed first scroll")
				}
				return
			}
			if len(queue.pending) == 0 {
				t.Fatalf("restart not admitted after exit: alive=%d fairy=%v leader=%d reward=%v next=%d exit=%v now=%v inworld=%v active=%v", playerCurHP(leader.Char), w.hasActiveSilverFairy(leader.Char), inst.LeaderID, inst.RewardGranted, inst.Config.ChainNextItem, inst.ExitAt, w.now(), leader.InWorld, w.itemInstanceForPlayer(leader.ID))
			}
			awaitCriticalWrite(t, base.entered)
			if leader.Char.Inv[5] != original || w.itemInstanceForPlayer(leader.ID) != nil {
				t.Fatal("pending restart entry published consumption or membership")
			}
			awaitReadyCriticalResults(t, queue, 1)
			w.processCriticalCompletions(1)
			if next := w.itemInstanceForPlayer(leader.ID); next == nil || next.Config.ID != "water-normal-1" ||
				leader.Char.Inv[5].Index != 0 || w.itemInstanceForPlayer(member.ID) != next {
				t.Fatal("acknowledged restart did not consume scroll and move party")
			}
		})
	}
}
