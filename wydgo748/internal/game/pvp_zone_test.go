package game

import (
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
)

// pvpZoneWorld puts the attacker at x=2000 (PK cell) and the target at the
// requested x; pvpTestTerrain marks x >= 2100 as a safe cell.
func pvpZoneWorld(t *testing.T, attackerX, targetX uint16) (*World, *Player, *Player) {
	t.Helper()
	attacker, _ := networkedTestPlayer(1, "Attacker", attackerX, 2000)
	target, _ := networkedTestPlayer(2, "Target", targetX, 2000)
	w := worldWithNetworkedPlayers(attacker, target)
	w.terrain = pvpTestTerrain()
	w.rng = fixedRNG{value: 0}
	attacker.PKMode = true
	attacker.Char.Score.Attack, attacker.Char.Score.Accuracy = 1000, 1000
	applyScore(attacker.Char)
	attacker.show(target.ID)
	return w, attacker, target
}

func TestServerPvPAuthorityRequiresBothPlayersInPKCells(t *testing.T) {
	for _, tc := range []struct {
		name              string
		attackerX, target uint16
		want              bool
	}{
		{"both in PK cells", 2096, 2097, true},
		{"target in safe cell", 2099, 2100, false},
		{"attacker in safe cell", 2100, 2099, false},
		{"both in safe cells", 2200, 2201, false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, attacker, target := pvpZoneWorld(t, tc.attackerX, tc.target)
			if got := w.canInitiatePvP(attacker, target); got != tc.want {
				t.Fatalf("canInitiatePvP=%v, want %v", got, tc.want)
			}
		})
	}
}

// A modified client may send an attack in a safe area; the server ignores it.
func TestPhysicalPvPAttackInSafeCellChangesNothing(t *testing.T) {
	for _, tc := range []struct {
		name       string
		x          uint16
		wantDamage bool
	}{{"pk cell", 2090, true}, {"safe cell", 2200, false}} {
		t.Run(tc.name, func(t *testing.T) {
			w, attacker, target := pvpZoneWorld(t, tc.x, tc.x+1)
			hp := playerCurHP(target.Char)
			w.onAttack(attacker.Session, physicalAttackPacket(1000, target.ID, target.X, target.Y))
			if damaged := playerCurHP(target.Char) < hp; damaged != tc.wantDamage {
				t.Fatalf("damaged=%v, want %v (HP %d -> %d)", damaged, tc.wantDamage, hp, playerCurHP(target.Char))
			}
		})
	}
}

func TestEquipmentPvPRatesApplyToAttackAndSkillBeforeManaControl(t *testing.T) {
	catalog := reportTestCatalog(t)
	for _, skill := range []bool{false, true} {
		for _, equipped := range []bool{false, true} {
			t.Run(fmt.Sprintf("skill=%t/equipped=%t", skill, equipped), func(t *testing.T) {
				w, attacker, target := pvpZoneWorld(t, 2090, 2091)
				w.items = catalog.Items
				w.items[6000] = model.ItemDef{Index: 6000, Pos: 64, DynamicEffectNames: catalog.ItemEffects,
					StaticEffects: []model.StaticEffect{{Name: "EF_HWORDGUILD", Value: 13}}}
				attacker.Char.Class = 0
				if skill {
					attacker.Char.LearnedSkill = 1
				}
				attacker.Char.Score.MaxMP, attacker.Char.Score.CurMP = 10_000, 10_000
				target.Char.Score.MaxHP, target.Char.Score.CurHP = 10_000, 10_000
				target.Char.Score.MaxMP, target.Char.Score.CurMP = 10_000, 10_000
				applyScore(attacker.Char)
				applyScore(target.Char)
				if equipped {
					attacker.Char.Equip[6] = model.Item{Index: 6000, Eff: [6]byte{43, 230, 56, 8}}
					target.Char.Equip[1] = model.Item{Index: 2171, Eff: [6]byte{43, 230}}
				}
				mount := model.Item{Index: model.MountAdultBase}
				mount.SetMountHP(10_000)
				target.Char.Equip[mountSlot] = mount
				target.Char.Affects[0] = model.Affect{Type: 18, ExpiresAt: w.now().Add(time.Minute)}
				w.skills = map[int]model.SkillDef{0: {
					Index: 0, InstanceType: 2, InstanceValue: 500,
					ManaSpent: 10, Delay: 1, Range: 6, MaxTarget: 1, Aggressive: 1,
				}}
				var hit uint32
				if skill {
					skillDef := w.skills[0]
					base := w.baseSkillDamage(attacker.Char, skillDef)
					value := skillPvPFinalDamageForTargetWithRNG(base, playerDefense(target.Char),
						skillDamageMastery(attacker.Char), target.Char.Class, w.intn)
					hit = uint32(applySkillResistance(value, skillDef.InstanceType, playerElementalResists(target.Char), false))
					w.onSkillAttack(attacker, skillCastRequest{Skill: 0, TargetID: target.ID, Motion: 5})
				} else {
					copy := *attacker
					hit = playerPhysicalHitPlayerWithRNG(&copy, target, 0, w.intn).Damage
					w.onAttack(attacker.Session, physicalAttackPacket(1000, target.ID, target.X, target.Y))
				}
				rider := hit * 75 / 100
				if hp := target.Char.Equip[mountSlot].MountHP(); hp != 10_000-int(hit-rider) {
					t.Fatalf("mount HP=%d, want %d", hp, 10_000-int(hit-rider))
				}
				hit = rider
				if equipped {
					hit = (hit * 1042 / 1000) * 976 / 1000
				}
				mpSpent := hit * 90 / 100
				if hp, mp := playerCurHP(target.Char), playerCurMP(target.Char); hp != 10_000-hit+mpSpent || mp != 10_000-mpSpent {
					t.Fatalf("hit=%d, remaining HP/MP=%d/%d, want %d/%d", hit, hp, mp, 10_000-hit+mpSpent, 10_000-mpSpent)
				}
			})
		}
	}
}

func TestPlayerPoisonDealsNoDamageInSafeCell(t *testing.T) {
	for _, tc := range []struct {
		name       string
		x          uint16
		wantDamage bool
	}{{"pk cell", 2090, true}, {"safe cell", 2200, false}} {
		t.Run(tc.name, func(t *testing.T) {
			w, owner, target := pvpZoneWorld(t, 2000, tc.x)
			target.Char.Affects[1] = model.Affect{Type: 20, Level: 20, Value: 10, OwnerID: owner.ID,
				OwnerCharacterUID: owner.Char.UID, ExpiresAt: w.now().Add(time.Minute), NextTick: w.now().Add(-time.Second)}
			hp := playerCurHP(target.Char)
			w.tickPlayerAffects(w.now())
			if damaged := playerCurHP(target.Char) < hp; damaged != tc.wantDamage {
				t.Fatalf("poison damaged=%v, want %v", damaged, tc.wantDamage)
			}
		})
	}
}
