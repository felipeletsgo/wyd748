package game

import (
	"encoding/binary"
	"testing"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestBMSummonNativeAttackProfiles(t *testing.T) {
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	w, owner, _ := handlerTestWorld(t)
	w.items, w.skills = catalog.Items, catalog.Skills
	owner.Char.Class, owner.Char.LearnedSkill = 2, 0xff<<8
	if !w.castSummon(owner, model.SkillDef{Index: 56, InstanceValue: 1}, 0) || len(w.summons) != 8 {
		t.Fatal("could not summon all eight types")
	}
	profiles := map[uint16]struct {
		reach  int
		near   summonAttackVisual
		far    summonAttackVisual
		spells map[int16]int
	}{
		206: {2, summonAttackVisual{-1, 6, 0}, summonAttackVisual{-1, 6, 0}, nil},
		225: {2, summonAttackVisual{-1, 6, 0}, summonAttackVisual{-1, 6, 0}, nil},
		226: {1, summonAttackVisual{-1, 6, 0}, summonAttackVisual{-1, 6, 0}, nil},
		227: {2, summonAttackVisual{-1, 6, 0}, summonAttackVisual{-1, 6, 0}, nil},
		244: {4, summonAttackVisual{-1, 5, 0}, summonAttackVisual{106, 4, 0}, map[int16]int{21: 15}},
		245: {4, summonAttackVisual{-1, 4, 0}, summonAttackVisual{105, 4, 1}, map[int16]int{34: 15}},
		307: {6, summonAttackVisual{-1, 6, 0}, summonAttackVisual{108, 4, 0}, map[int16]int{0: 15}},
		396: {3, summonAttackVisual{-1, 6, 0}, summonAttackVisual{108, 4, 0}, map[int16]int{39: 50, 34: 35}},
	}
	for _, summon := range w.summons {
		face := summon.Def.Equip.Rosto.Index
		want := profiles[face]
		if summon.SummonRange != want.reach {
			t.Fatalf("face %d reach = %d, want %d", face, summon.SummonRange, want.reach)
		}
		roll := 0
		if face == 396 {
			roll = 85
		}
		w.rng = &sequenceRNG{values: []int{0, roll}}
		if got := w.summonAttackVisual(summon, 2); got != want.near {
			t.Fatalf("face %d near attack = %+v, want %+v", face, got, want.near)
		}
		w.rng = &sequenceRNG{values: []int{roll}}
		if got := w.summonAttackVisual(summon, 3); got != want.far {
			t.Fatalf("face %d far attack = %+v, want %+v", face, got, want.far)
		}
		counts := make(map[int16]int)
		for roll := 0; roll < 100; roll++ {
			w.rng = fixedRNG{value: roll}
			attack := w.summonAttackVisual(summon, 3)
			if attack.skill != want.far.skill {
				counts[attack.skill]++
			}
		}
		for skill, count := range want.spells {
			if counts[skill] != count {
				t.Fatalf("face %d spell %d selected %d times, want %d", face, skill, counts[skill], count)
			}
			delete(counts, skill)
		}
		if len(counts) != 0 {
			t.Fatalf("face %d selected unexpected spells %v", face, counts)
		}
	}
}

func TestBMSummonNativeAttacksPublishToOwnerAndObserver(t *testing.T) {
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct {
		name           string
		face           uint16
		dx, dy         uint16
		reach          int
		rolls          []int
		skill          int16
		motion, effect byte
		affect         byte
		mastery        int
		miss           bool
	}{
		{"wolf melee", 226, 1, 1, 1, []int{1}, -1, 5, 0, 0, 0, false},
		{"tiger breath", 244, 2, 2, 4, nil, 106, 4, 0, 0, 0, false},
		{"tiger poison", 244, 1, 0, 4, []int{0, 85}, 21, 5, 0, 20, 4, false},
		{"gorilla stone", 245, 4, 0, 4, nil, 105, 4, 1, 0, 0, false},
		{"gorilla stone miss", 245, 4, 0, 4, []int{0, 99}, 105, 4, 1, 0, 0, true},
		{"dragon beam", 307, 6, 0, 6, nil, 108, 4, 0, 0, 0, false},
		{"succubus meteor", 396, 3, 0, 3, nil, 39, 4, 0, 0, 0, false},
		{"succubus ice", 396, 3, 0, 3, []int{50}, 34, 4, 0, 1, 108, false},
		{"succubus beam", 396, 3, 0, 3, []int{85}, 108, 4, 0, 0, 0, false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, owner, observer, summon := bmSummonPvPWorld(t, 2000)
			summon.Def.Equip.Rosto.Index = tc.face
			summon.Def.Score.Attack, summon.SummonRange = 10_000, tc.reach
			target := &Mob{ID: 1501, X: summon.X + tc.dx, Y: summon.Y + tc.dy, HP: 100_000,
				Def: testNPCDef(model.Score{MaxHP: 100_000, Dex: 4000}), GenerIndex: -1}
			target.Def.Tipo = model.TipoMonstro
			w.appendMobInstance(target)
			w.registerMobSpatial(target)
			w.skills = catalog.Skills
			w.rng = &sequenceRNG{values: tc.rolls}
			owner.CombatTargetID = target.ID
			w.tickSummonCombat(w.now())
			damage := target.Def.Score.MaxHP - target.HP
			if (damage == 0) != tc.miss {
				t.Fatalf("damage = %d, expected miss = %t", damage, tc.miss)
			}
			for _, viewer := range []*Player{owner, observer} {
				packet := requireSummonPacket(t, viewer, wire.OpAttackOne, wire.SceneField)
				if len(packet) != 60 || binary.LittleEndian.Uint16(packet[12:14]) != summon.ID ||
					int16(binary.LittleEndian.Uint16(packet[24:26])) != tc.skill ||
					packet[28] != tc.motion || packet[29] != tc.effect ||
					binary.LittleEndian.Uint16(packet[44:46]) != target.ID ||
					string(packet[48:52]) != "DMGX" || binary.LittleEndian.Uint32(packet[56:60]) != damage ||
					(int16(binary.LittleEndian.Uint16(packet[46:48])) == -3) != tc.miss {
					t.Fatalf("unexpected attack packet for viewer %d: %x", viewer.ID, packet)
				}
			}
			if tc.affect != 0 {
				affect := activeMobAffectAt(target, tc.affect, w.now())
				if affect == nil || int(affect.Level) != tc.mastery {
					t.Fatalf("affect = %+v, want type %d at mastery %d", affect, tc.affect, tc.mastery)
				}
			}
		})
	}
}

func TestBMSummonMovesIntoNativeDiagonalRangeBeforeAttacking(t *testing.T) {
	w, owner, _, summon := bmSummonPvPWorld(t, 2000)
	clock := newFakeClock(w.now())
	w.clock, w.rng = clock, fixedRNG{}
	summon.Def.Equip.Rosto.Index = 396
	summon.Def.Score.Attack, summon.SummonRange = 100, 3
	target := &Mob{ID: 1501, X: summon.X + 3, Y: summon.Y + 3, HP: 1000,
		Def: testNPCDef(model.Score{MaxHP: 1000}), GenerIndex: -1}
	target.Def.Tipo = model.TipoMonstro
	w.appendMobInstance(target)
	w.registerMobSpatial(target)
	owner.CombatTargetID = target.ID
	w.tickSummonCombat(clock.Now())
	if summon.X != target.X-2 || summon.Y != target.Y-2 || !summon.NextAttack.IsZero() || target.HP != 1000 {
		t.Fatalf("summon at (%d, %d), deadline=%v, target HP=%d", summon.X, summon.Y, summon.NextAttack, target.HP)
	}
	clock.current = summon.NextMove
	w.tickSummonCombat(clock.Now())
	if target.HP >= 1000 {
		t.Fatal("summon did not attack after entering native range")
	}
}

func TestBMSummonNativeSpellRespectsPlayerImmunity(t *testing.T) {
	for _, immune := range []bool{false, true} {
		w, owner, target, summon := bmSummonPvPWorld(t, 2000)
		now := w.now()
		w.rng = &sequenceRNG{values: []int{0, 50}}
		w.skills = map[int]model.SkillDef{
			34: {Index: 34, InstanceType: 3, AffectType: 1, AffectValue: 2, AffectTime: 1},
		}
		summon.Def.Equip.Rosto.Index = 396
		summon.Def.Score.Attack, summon.SummonRange = 600, 3
		owner.CombatTargetID = target.ID
		if immune {
			setAffectAt(target.Char, 19, 1, 0, 4, now)
		}
		before := playerCurHP(target.Char)
		w.tickSummonCombat(now)
		if hp := playerCurHP(target.Char); hp == 0 || hp >= before {
			t.Fatalf("spell did not land on a living target: HP %d -> %d", before, hp)
		}
		if applied := hasActiveAffectAt(target.Char, 1, now); applied == immune {
			t.Fatalf("ice affect = %t with immunity = %t", applied, immune)
		}
		for _, affect := range target.Char.Affects {
			if affect.Type == 1 && affect.OwnerCharacterUID != owner.Char.UID {
				t.Fatalf("ice affect source = %q, want %q", affect.OwnerCharacterUID, owner.Char.UID)
			}
		}
	}
}
