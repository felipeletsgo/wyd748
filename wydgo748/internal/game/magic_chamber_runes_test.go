package game

import (
	"encoding/binary"
	"math/rand"
	"path/filepath"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
)

// Rune slates show, per run, the rune rolled when each room starts: cell k of
// every block lights with room k's color; other viewers see hidden slates.
func TestMagicChamberRuneSlatesFollowTheViewersRun(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	catalog, err := data.LoadCatalog(filepath.Join(root, "itemlist.csv"),
		filepath.Join(root, "Itemname.csv"), filepath.Join(root, "SkillData.csv"))
	if err != nil {
		t.Fatal(err)
	}
	objects, err := data.LoadInitItems(filepath.Join(root, "init_items.csv"), catalog.Items)
	if err != nil {
		t.Fatal(err)
	}
	w := &World{groundItems: map[uint16]*GroundItem{}, items: catalog.Items, nextItemID: 10000,
		initItems: objects, rng: rand.New(rand.NewSource(7))}
	if err := w.spawnInitItems(); err != nil {
		t.Fatal(err)
	}
	slates := map[[2]uint16]*GroundItem{}
	for _, g := range w.groundItems {
		if _, ok := runeSlateCell(g.X, g.Y); ok {
			slates[[2]uint16{g.X, g.Y}] = g
		}
	}
	if len(slates) != 16 {
		t.Fatalf("found %d rune slates; want 16", len(slates))
	}
	inst := &ItemInstance{Config: model.VolatileInstance{ID: "magic-chamber-normal"},
		RuntimeID: "magic-chamber-normal#1", MemberIDs: []uint16{5}}
	w.itemInstances = map[string]*ItemInstance{inst.RuntimeID: inst}
	member, outsider := &Player{ID: 5}, &Player{ID: 6}

	view := func(p *Player, x, y uint16) (uint16, byte) {
		pkt := w.groundItemCreatePacketFor(p, slates[[2]uint16{x, y}])
		return binary.LittleEndian.Uint16(pkt[18:20]), pkt[27]
	}
	hidden := func(p *Player, x, y uint16) bool {
		index, state := view(p, x, y)
		return index == slates[[2]uint16{x, y}].Item.Index && state == clientGateOpen
	}
	if index, state := view(member, 786, 3676); !hidden(member, 786, 3676) {
		t.Fatalf("slate before any roll = %d/%d; want hidden", index, state)
	}
	w.rollMagicChamberRune(inst, 0)
	first := w.magicChamberRunes[inst.RuntimeID].Runes[0]
	if first < 1 || first > 4 {
		t.Fatalf("room 1 rune = %d; want 1..4", first)
	}
	for _, block := range runeSlateBlocks {
		if index, state := view(member, block[0], block[1]); index != runeSlateBase+uint16(first) || state != clientGateLocked {
			t.Fatalf("cell 0 at %v = %d/%d; want rune %d locked", block, index, state, runeSlateBase+uint16(first))
		}
		if !hidden(member, block[0], block[1]+1) {
			t.Fatalf("cell 1 at %v is lit before room 2", block)
		}
		if _, state := view(outsider, block[0], block[1]); state != clientGateOpen {
			t.Fatalf("a viewer outside the run sees a lit slate at %v", block)
		}
	}
	w.rollMagicChamberRune(inst, 0)
	if w.magicChamberRunes[inst.RuntimeID].Runes[0] != first {
		t.Fatal("re-entering a room rolled its rune again")
	}
	w.rollMagicChamberRune(inst, 1)
	second := w.magicChamberRunes[inst.RuntimeID].Runes[1]
	if index, state := view(member, 844, 3677); index != runeSlateBase+uint16(second) || state != clientGateLocked || pkt28(w, member, slates[[2]uint16{844, 3677}]) != 0 {
		t.Fatalf("room 2 cell 1 = %d/%d; want rune %d locked without collision height", index, state, second)
	}
	w.rollMagicChamberRune(&ItemInstance{Config: model.VolatileInstance{ID: "uxmal"}}, 0)
	delete(w.itemInstances, inst.RuntimeID)
	w.rollMagicChamberRune(&ItemInstance{Config: model.VolatileInstance{ID: "magic-chamber-mystic"}, RuntimeID: "m#2"}, 0)
	if _, kept := w.magicChamberRunes[inst.RuntimeID]; kept {
		t.Fatal("an ended run kept its runes")
	}
}

func TestMagicChamberRunStateIsReleasedWhenInstanceCloses(t *testing.T) {
	const id = "magic-chamber-normal#1"
	w := &World{
		itemInstances:          map[string]*ItemInstance{},
		pendingInstanceMembers: map[string]map[string]struct{}{},
		pendingInstanceLeaders: map[string]string{},
		magicChamberRunes:      map[string]magicChamberRun{},
	}
	w.itemInstances[id] = &ItemInstance{RuntimeID: id,
		Config: model.VolatileInstance{ID: "magic-chamber-normal"}}
	w.magicChamberRunes[id] = magicChamberRun{Runes: [4]byte{1, 2, 3, 4}, BonusSpawned: true}

	w.closeItemInstance(id)

	if _, ok := w.magicChamberRunes[id]; ok {
		t.Fatal("closed Magic Chamber kept its rune state")
	}
}

func pkt28(w *World, p *Player, g *GroundItem) byte {
	return w.groundItemCreatePacketFor(p, g)[28]
}

func runeBonusChamber(t *testing.T, runes [4]byte) (*World, *ItemInstance, func(*Mob), func() *Mob) {
	t.Helper()
	w, leader, _, _, clock := instanceTestWorld()
	w.npcs = append(w.npcs,
		model.NPCDef{Name: "ChamberBoss", Tipo: model.TipoMonstro,
			Score: &model.Score{Version: model.ScoreVersion, MaxHP: 500, CurHP: 500}},
		model.NPCDef{Name: "RuneGuardian", Tipo: model.TipoMonstro,
			Score: &model.Score{Version: model.ScoreVersion, MaxHP: 900, CurHP: 900}})
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	rule.Instance.ID = "magic-chamber-normal"
	rule.Instance.ExclusiveGroup = "magic-chamber"
	rule.Instance.TransitionSeconds = 1
	rule.Instance.RewardItem = 0
	rule.Instance.Stages = make([]model.VolatileInstanceStage, 4)
	for i := range rule.Instance.Stages {
		rule.Instance.Stages[i] = model.VolatileInstanceStage{Name: "Room",
			X: uint16(2200 + i*20), Y: 2200, SpawnX: uint16(2200 + i*20), SpawnY: 2200,
			AreaRadius: 8, DurationSeconds: 60,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}}}
	}
	rule.Instance.Stages[3].CompletionSpawns = []model.VolatileInstanceSpawn{{NPC: "ChamberBoss", Count: 1}}
	rule.Instance.Stages[3].RuneBonusSpawns = []model.VolatileInstanceSpawn{{NPC: "RuneGuardian", Count: 1}}
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances[rule.Instance.ID]
	if inst == nil {
		t.Fatal("the test Magic Chamber did not open")
	}
	kill := func(m *Mob) {
		w.onItemInstanceMobKilled(m, clock.Now())
		w.removeMobInstance(m)
	}
	only := func() *Mob {
		for id := range inst.MobIDs {
			return w.mobsByID[id]
		}
		return nil
	}
	for stage := 0; stage < 4; stage++ {
		kill(only())
		if stage < 3 {
			clock.Advance(time.Second)
			w.tickItemInstances(clock.Now())
		}
	}
	run := w.magicChamberRunes[itemInstanceKey(inst)]
	run.Runes = runes
	w.magicChamberRunes[itemInstanceKey(inst)] = run
	return w, inst, kill, only
}

// After the fourth room's boss dies, runes with colors 1, 2 and 3 summon the
// bonus boss once, and the run finishes only when it dies.
func TestMagicChamberRuneBonusBossFollowsTheCompletionBoss(t *testing.T) {
	w, inst, kill, only := runeBonusChamber(t, [4]byte{3, 1, 4, 2})
	if !inst.BossSpawned {
		t.Fatal("the fourth room's boss did not appear")
	}
	kill(only())
	bonus := only()
	if bonus == nil || bonus.Def.Name != "RuneGuardian" || !inst.ExitAt.IsZero() ||
		!w.magicChamberRunes[itemInstanceKey(inst)].BonusSpawned {
		t.Fatalf("rune bonus did not follow the completion boss: bonus=%v exit=%v", bonus, inst.ExitAt)
	}
	kill(bonus)
	if inst.ExitAt.IsZero() || only() != nil {
		t.Fatal("the rune bonus boss's death did not finish the chamber")
	}
}

func TestMagicChamberWithoutTheThreeRunesFinishesAfterTheBoss(t *testing.T) {
	_, inst, kill, only := runeBonusChamber(t, [4]byte{1, 1, 2, 4})
	kill(only())
	if inst.ExitAt.IsZero() || only() != nil {
		t.Fatal("a run without colors 1, 2 and 3 spawned a bonus or did not finish")
	}
}

// 7.59 roll: Voucher_Courage limits colors to 1..3; without it the Arcane
// chamber always rolls 4 and the others roll 1..4.
func TestMagicChamberRuneRollRules(t *testing.T) {
	w := &World{rng: rand.New(rand.NewSource(3)), playersByID: map[uint16]*Player{}}
	leader := &Player{ID: 9, Char: &model.Char{}}
	w.playersByID[9] = leader
	normal := &ItemInstance{Config: model.VolatileInstance{ID: "magic-chamber-normal"}, LeaderID: 9}
	arcane := &ItemInstance{Config: model.VolatileInstance{ID: "magic-chamber-arcane"}, LeaderID: 9}
	seen := map[byte]bool{}
	for i := 0; i < 400; i++ {
		seen[w.rollRuneColor(normal)] = true
		if c := w.rollRuneColor(arcane); c != runeNoBonus {
			t.Fatalf("Arcane without a voucher rolled %d", c)
		}
	}
	if len(seen) != 4 || !seen[1] || !seen[4] {
		t.Fatalf("Normal rolls = %v; want 1..4", seen)
	}
	leader.Char.Equip[runeVoucherSlot] = model.Item{Index: runeVoucherItem}
	for i := 0; i < 400; i++ {
		for _, inst := range []*ItemInstance{normal, arcane} {
			if c := w.rollRuneColor(inst); c < 1 || c > 3 {
				t.Fatalf("voucher roll = %d; want 1..3", c)
			}
		}
	}
	if !runeBonusEarned([4]byte{2, 3, 1, 4}) || runeBonusEarned([4]byte{1, 2, 2, 4}) ||
		runeBonusEarned([4]byte{4, 4, 4, 4}) {
		t.Fatal("rune bonus condition does not require colors 1, 2 and 3")
	}
}
