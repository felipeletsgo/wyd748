package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

func TestCombatUsesInjectedRNG(t *testing.T) {
	attacker := &Player{Char: &model.Char{Name: "rng", Extended: testExtended(model.ExtendedScore{})}}
	attacker.Char.Extended.Attack = 1_000
	mob := &Mob{Def: &model.NPCDef{Extended: &model.ExtendedScore{
		Version: model.ExtendedScoreVersion, Defense: 100, Dex: 0,
	}}}

	wLow := &World{rng: fixedRNG{value: 0}}
	wHigh := &World{rng: fixedRNG{value: 10}}
	low := wLow.playerHitsMob(attacker, mob)
	attacker.AttackProgress = 0
	high := wHigh.playerHitsMob(attacker, mob)
	if low == high {
		t.Fatalf("damage ignored injected RNG: low=%d high=%d", low, high)
	}
}

func TestDropsUseInjectedRNG(t *testing.T) {
	def := &model.NPCDef{Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, Level: 100},
		Carry: make([]model.Item, 64)}
	def.Carry[32] = model.Item{Index: 4011}
	mob := &Mob{Def: def, X: 10, Y: 10}
	newWorld := func(value int) (*World, *Player) {
		w := &World{rng: fixedRNG{value: value}}
		w.dropRates[32] = 2
		p := &Player{Char: &model.Char{Name: "drop", Extended: testExtended(model.ExtendedScore{})}}
		return w, p
	}

	wMiss, miss := newWorld(1)
	if got := wMiss.planMobDrops(miss, mob); len(got) != 0 {
		t.Fatalf("fixed miss RNG produced %d drops", len(got))
	}
	wHit, hit := newWorld(0)
	if got := wHit.planMobDrops(hit, mob); len(got) != 1 || hit.Char.Inv[0].Index != 4011 {
		t.Fatalf("fixed hit RNG did not materialize the drop: %#v", got)
	}
}

func TestMountInitializationUsesInjectedRNG(t *testing.T) {
	w := &World{rng: fixedRNG{value: 20}}
	mount := model.Item{Index: model.MountBabyBase}
	if !w.initFreshMount(&mount) || mount.MountLongev() != 30 {
		t.Fatalf("mount longevity ignored injected RNG: %d", mount.MountLongev())
	}
}

func TestEquippedMountAcceptsOnlyCanonicalSlot(t *testing.T) {
	ch := &model.Char{}
	ch.Equip[0] = model.Item{Index: model.MountAdultBase}
	if mount, slot := equippedMount(ch); mount != nil || slot != -1 {
		t.Fatalf("mount in invalid slot was accepted: slot=%d mount=%#v", slot, mount)
	}
	ch.Equip[mountSlot] = ch.Equip[0]
	ch.Equip[0] = model.Item{}
	if mount, slot := equippedMount(ch); mount == nil || slot != mountSlot {
		t.Fatalf("canonical mount was not accepted: slot=%d mount=%#v", slot, mount)
	}
}

func TestAffectLifecycleUsesInjectedClock(t *testing.T) {
	now := time.Date(2026, 8, 10, 12, 0, 0, 0, time.UTC)
	w := &World{clock: newFakeClock(now)}
	ch := &model.Char{Name: "clock", Extended: testExtended(model.ExtendedScore{})}
	rule := model.VolatileRule{AffectType: 30, AffectValue: 1, DurationUnits: 10}
	if got := w.applyVolatileBuff(ch, rule); got != volatileBuffApplied {
		t.Fatalf("applyVolatileBuff=%d", got)
	}
	if want := now.Add(80 * time.Second); !ch.Affects[0].ExpiresAt.Equal(want) {
		t.Fatalf("ExpiresAt=%v want %v", ch.Affects[0].ExpiresAt, want)
	}
}

func TestResurrectionUsesInjectedRNG(t *testing.T) {
	newDeadPlayer := func() *Player {
		ch := &model.Char{Name: "dead", Extended: testExtended(model.ExtendedScore{
			Level: 399, MaxHP: 1_000, MaxMP: 500,
		})}
		ch.Extended.CurHP = 0
		return &Player{ID: 1, InWorld: true, Char: ch, Session: &net.Session{ID: 1}}
	}
	skill := model.SkillDef{Index: 99, MaxTarget: 1}

	success := newDeadPlayer()
	wSuccess := &World{rng: fixedRNG{value: 0}, playersByID: map[uint16]*Player{1: success}}
	if got := wSuccess.applySupportSkill(success, skillCastRequest{TargetID: success.ID}, skill, 0); len(got) != 1 || playerCurHP(success.Char) == 0 {
		t.Fatalf("fixed success RNG did not resurrect: results=%v hp=%d", got, playerCurHP(success.Char))
	}

	failure := newDeadPlayer()
	wFailure := &World{rng: fixedRNG{value: 99}, playersByID: map[uint16]*Player{1: failure}}
	if got := wFailure.applySupportSkill(failure, skillCastRequest{TargetID: failure.ID}, skill, 0); len(got) != 0 || playerCurHP(failure.Char) != 0 {
		t.Fatalf("fixed failure RNG resurrected unexpectedly: results=%v hp=%d", got, playerCurHP(failure.Char))
	}
}
