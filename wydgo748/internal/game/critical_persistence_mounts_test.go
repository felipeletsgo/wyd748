package game

import (
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func criticalBabyMount() model.Item {
	item := model.Item{Index: model.MountBabyBase}
	item.SetMountHP(100)
	item.SetMountLongev(30)
	item.SetMountFood(10)
	return item
}

// These are actual mount consumers, exercised during execution and after the
// store has completed but before World acknowledges its detached write.
func TestCriticalMountConsumersWaitForOwnerAcknowledgement(t *testing.T) {
	for _, scenario := range []string{
		"food-due", "food-init", "food-reset", "egg-due", "egg-tick-direct",
		"egg-reset", "egg-reset-direct", "egg-start-direct", "pet-spawn",
		"pet-spawn-direct", "pet-remove", "pet-rebind", "hunt-exp",
		"absorb-damage", "rider-death", "feed", "incubate", "accelerate",
	} {
		t.Run(scenario, func(t *testing.T) {
			w, p, mob, st := killRewardTestWorld(t, nil)
			now := w.now()
			w.rng = fixedRNG{value: 3}
			p.Char.Equip[mountSlot] = equippedAdultMount(10)
			p.NextMountTick = now.Add(-time.Second)
			var pet *Mob
			var action func()
			var resumed func() bool
			switch scenario {
			case "food-due", "food-init", "food-reset":
				if scenario == "food-init" {
					p.NextMountTick = time.Time{}
				} else if scenario == "food-reset" {
					p.Char.Equip[mountSlot] = model.Item{}
				}
				action = func() { w.tickPlayerMounts(now) }
				resumed = func() bool {
					if scenario == "food-reset" {
						return p.NextMountTick.IsZero()
					}
					wantFood := 9
					if scenario == "food-init" {
						wantFood = 10
					}
					return p.Char.Equip[mountSlot].MountFood() == wantFood && p.NextMountTick.Equal(now.Add(time.Hour))
				}
			case "egg-due", "egg-tick-direct", "egg-reset", "egg-reset-direct", "egg-start-direct":
				p.Char.Equip[mountSlot] = model.Item{Index: model.MountEggBase, UID: "egg-clock"}
				setEggDelay(&p.Char.Equip[mountSlot], 2)
				p.EggIncubationUID, p.NextEggIncubationTick = "egg-clock", now.Add(-time.Second)
				switch scenario {
				case "egg-reset", "egg-reset-direct":
					p.Char.Equip[mountSlot] = model.Item{}
				case "egg-start-direct":
					p.EggIncubationUID, p.NextEggIncubationTick = "old-egg", time.Time{}
				}
				action = func() {
					switch scenario {
					case "egg-due", "egg-reset":
						w.tickPlayerMounts(now)
					case "egg-reset-direct":
						w.resetEggIncubationClock(p)
					case "egg-start-direct":
						w.startEggIncubationClock(p, &p.Char.Equip[mountSlot], now)
					default:
						w.tickEquippedEggIncubation(p, now)
					}
				}
				resumed = func() bool {
					switch scenario {
					case "egg-reset", "egg-reset-direct":
						return p.EggIncubationUID == "" && p.NextEggIncubationTick.IsZero()
					case "egg-start-direct":
						return p.EggIncubationUID == "egg-clock" && p.NextEggIncubationTick.Equal(now.Add(time.Hour))
					default:
						return eggDelay(p.Char.Equip[mountSlot]) == 1 && st.saves == 0 && p.NextEggIncubationTick.After(now)
					}
				}
			case "pet-spawn", "pet-spawn-direct", "pet-remove", "pet-rebind":
				p.Char.Equip[mountSlot] = criticalBabyMount()
				if scenario == "pet-remove" || scenario == "pet-rebind" {
					pet = criticalSummonFixture(w, p)
					pet.SummonKind, pet.Def.Equip.Rosto.Index, pet.InstanceID = summonKindMount, criaPetFaceBase, "previous-runtime"
					if scenario == "pet-remove" {
						p.Char.Equip[mountSlot] = model.Item{}
					}
				}
				action = func() {
					if scenario == "pet-spawn-direct" {
						w.spawnCriaPet(p, &p.Char.Equip[mountSlot])
					} else {
						w.syncCriaPet(p)
					}
				}
				resumed = func() bool {
					current := w.findCriaPet(p.ID)
					switch scenario {
					case "pet-remove":
						return current == nil && pet.Dead && w.mobsByID[pet.ID] == nil
					case "pet-rebind":
						return current == pet && pet.InstanceID == w.playerRuntimeInstanceID(p.ID)
					default:
						return current != nil && current.Def.Name == "Hatchling^" && current.Def.Equip.Rosto.Index == criaPetFaceBase
					}
				}
			case "hunt-exp":
				p.Char.Equip[mountSlot] = criticalBabyMount()
				p.Char.Equip[mountSlot].Eff[5] = 1
				action = func() { w.grantMountHuntExp(p, mob) }
				resumed = func() bool { return p.Char.Equip[mountSlot].MountLevel() == 1 && p.Char.Equip[mountSlot].Eff[5] == 0 }
			case "absorb-damage":
				damage := -1
				action = func() { damage = w.absorbMountDamage(p, 1000) }
				resumed = func() bool { return damage == 750 && p.Char.Equip[mountSlot].MountHP() == 750 }
			case "rider-death":
				action = func() { w.mountRiderDied(p) }
				resumed = func() bool { return p.Char.Equip[mountSlot].MountLongev() == 27 }
			case "feed":
				p.Char.Inv[0] = model.Item{Index: 2420}
				action = func() {
					w.applyMountItem(p, p.Session, &p.Char.Inv[0], 0,
						model.VolatileRule{MountAction: "feed", Consume: true}, 0, useItemRequest{})
				}
				resumed = func() bool {
					return p.Char.Inv[0].Index == 0 && p.Char.Equip[mountSlot].MountFood() > 10 && st.saves == 0
				}
			case "incubate", "accelerate":
				p.Char.Equip[mountSlot] = model.Item{Index: model.MountEggBase}
				w.items[model.MountEggBase] = model.ItemDef{Index: model.MountEggBase,
					StaticEffects: []model.StaticEffect{{Name: "EF_INCUBATE", Value: 3}}}
				p.Char.Inv[0] = model.Item{Index: 100}
				w.rng = fixedRNG{value: 0}
				action = func() {
					if scenario == "accelerate" {
						w.accelerateHatch(p, p.Session, &p.Char.Inv[0], 0,
							useItemRequest{dstType: placeEquip, dstPos: mountSlot})
					} else {
						w.incubateEgg(p, p.Session, &p.Char.Inv[0], 0, &p.Char.Equip[mountSlot], placeEquip, mountSlot, 4)
					}
				}
				resumed = func() bool {
					if p.Char.Inv[0].Index != 0 || st.saves != 0 {
						return false
					}
					if scenario == "accelerate" {
						return model.IsMountBaby(p.Char.Equip[mountSlot].Index) && w.findCriaPet(p.ID) != nil
					}
					return eggProgress(p.Char.Equip[mountSlot]) == 1 && eggDelay(p.Char.Equip[mountSlot]) == 6 && !p.NextEggIncubationTick.IsZero()
				}
			}
			snapshot := cloneCharacterState(p.Char)
			foodDeadline, eggDeadline, eggUID := p.NextMountTick, p.NextEggIncubationTick, p.EggIncubationUID
			mobCount, summonCount, nextID := len(w.mobs), len(w.summons), w.nextMobID
			var petSnapshot Mob
			if pet != nil {
				petSnapshot = *pet
			}
			drainPlayerPackets(p)
			release, queue := holdCriticalCombatParticipant(t, w, p)
			for _, phase := range []string{"worker-held", "completed-unacknowledged"} {
				if phase == "completed-unacknowledged" {
					release()
					awaitReadyCriticalResults(t, queue, 1)
				}
				action()
				assertCriticalCombatUnchanged(t, p, snapshot, 0, time.Time{})
				if !p.NextMountTick.Equal(foodDeadline) || !p.NextEggIncubationTick.Equal(eggDeadline) ||
					p.EggIncubationUID != eggUID || len(w.mobs) != mobCount || len(w.summons) != summonCount ||
					w.nextMobID != nextID || st.saves != 0 || (pet != nil && !reflect.DeepEqual(*pet, petSnapshot)) {
					t.Fatalf("%s mutated mount clocks, follower state, allocation or persistence", phase)
				}
			}
			w.processCriticalCompletions(1)
			action()
			switch scenario {
			case "egg-due", "egg-tick-direct", "feed", "incubate", "accelerate":
				// These producers submit their own detached write; it settles
				// through the coordinator, never through the legacy store.
				awaitReadyCriticalResults(t, queue, 1)
				w.processCriticalCompletions(1)
			}
			if !resumed() {
				t.Fatal("actual mount operation did not resume after acknowledgement")
			}
		})
	}
}

func TestMountFeedMessagesUseEnglish(t *testing.T) {
	for _, dead := range []bool{false, true} {
		mount := equippedAdultMount(10)
		want := "Mount fed."
		if dead {
			mount.SetMountHP(0)
			want = "Mount revived (100 HP, 5 food)."
		}
		ok, message := mountFeed(&mount, 2420)
		if !ok || message != want {
			t.Fatalf("feed result = %v, %q; want true, %q", ok, message, want)
		}
	}
}
