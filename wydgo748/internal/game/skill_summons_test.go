package game

import (
	"fmt"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestBMSummonAnyLearnedSkillCreatesAllTypesWithOneManaCharge(t *testing.T) {
	for _, mastery := range []int{0, 320} {
		for index := 56; index <= 63; index++ {
			t.Run(fmt.Sprintf("skill-%d/mastery-%d", index, mastery), func(t *testing.T) {
				w, owner, _ := handlerTestWorld(t)
				owner.Char.Class, owner.Char.LearnedSkill = 2, 0xff<<8
				owner.Char.Score.Int, owner.Char.Score.Con = 1000, 500
				owner.Char.Score.MaxMP, owner.Char.Score.CurMP = 10_000, 10_000
				owner.Char.Score.Mastery[2] = uint32(mastery)
				applyScore(owner.Char)
				skill := model.SkillDef{Index: index, InstanceType: 11, InstanceValue: index - 55,
					ManaSpent: 30, MaxTarget: 1, Delay: 5}
				w.skills = map[int]model.SkillDef{index: skill}
				beforeMP := playerCurMP(owner.Char)
				w.onSkillAttack(owner, skillCastRequest{Skill: index, Motion: 5})
				if len(w.summons) != 8 {
					t.Fatalf("summons=%d, want 8", len(w.summons))
				}
				wantMP := beforeMP - uint32(skillManaCost(skill, mastery, 0))
				if playerCurMP(owner.Char) != wantMP || !owner.SkillReady[index].After(w.now()) {
					t.Fatalf("MP=%d, want %d; cooldown=%v", playerCurMP(owner.Char), wantMP, owner.SkillReady[index])
				}
				faces := make(map[uint16]bool)
				for _, summon := range w.summons {
					face := summon.Def.Equip.Rosto.Index
					if faces[face] || !owner.hasVisible(summon.ID) {
						t.Fatal("summon type is duplicated or invisible to its owner")
					}
					faces[face] = true
					if face == 206 {
						if want := uint32(70 + 1500*40/60 + mastery*150/60); summon.Def.Score.Attack != want {
							t.Fatalf("Condor attack=%d, want %d", summon.Def.Score.Attack, want)
						}
					}
				}
				w.onSkillAttack(owner, skillCastRequest{Skill: index, Motion: 5})
				if len(w.summons) != 8 || playerCurMP(owner.Char) != wantMP {
					t.Fatal("cooldown replay changed summons or spent MP")
				}
			})
		}
	}
}

func TestBMSummonRecastReplacesOnlyMissingLearnedTypes(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2100, 2100)
	observer, _ := networkedTestPlayer(2, "Observer", 2101, 2100)
	w := worldWithNetworkedPlayers(owner, observer)
	owner.Char.Class, owner.Char.LearnedSkill = 2, 1<<8|1<<12|1<<15
	skill := model.SkillDef{Index: 60, InstanceValue: 5}
	if !w.castSummon(owner, skill, 100) || len(w.summons) != 3 {
		t.Fatal("cast did not create the three learned types")
	}
	first := make(map[uint16]*Mob)
	for _, summon := range w.summons {
		face := summon.Def.Equip.Rosto.Index
		if face != 206 && face != 244 && face != 396 {
			t.Fatalf("unlearned type created: face=%d", face)
		}
		first[face] = summon
		if !owner.hasVisible(summon.ID) || !observer.hasVisible(summon.ID) {
			t.Fatal("learned summon was not published to its owner and observer")
		}
	}
	first[206].HP--
	hp := first[206].HP
	first[206].Affects[0] = model.Affect{Type: 20, ExpiresAt: w.now().Add(time.Minute)}
	w.destroyPlayerSummon(first[244], owner.ID)
	if owner.hasVisible(first[244].ID) || observer.hasVisible(first[244].ID) {
		t.Fatal("dead summon remained visible")
	}
	if !w.castSummon(owner, model.SkillDef{Index: 56, InstanceValue: 1}, 100) || len(w.summons) != 3 {
		t.Fatal("another learned skill did not replace the missing type")
	}
	for _, summon := range w.summons {
		face := summon.Def.Equip.Rosto.Index
		if face == 244 {
			if summon.ID == first[244].ID {
				t.Fatal("dead summon was reused")
			}
		} else if summon != first[face] {
			t.Fatal("recast replaced a living summon")
		}
		if !observer.hasVisible(summon.ID) {
			t.Fatal("observer did not receive the replacement")
		}
	}
	if first[206].HP != hp || first[206].Affects[0].Type != 20 {
		t.Fatal("recast healed a living summon or cleared its affect")
	}
}

func TestBMSummonRejectsInvalidOrUnlearnedSkills(t *testing.T) {
	for _, scenario := range []string{"class", "unlearned", "index-low", "index-high", "type", "nil-owner", "nil-character"} {
		t.Run(scenario, func(t *testing.T) {
			w, owner, _ := handlerTestWorld(t)
			owner.Char.Class, owner.Char.LearnedSkill = 2, 1<<8
			skill := model.SkillDef{Index: 56, InstanceValue: 1}
			switch scenario {
			case "class":
				owner.Char.Class = 1
			case "unlearned":
				owner.Char.LearnedSkill = 1 << 12
			case "index-low":
				skill.Index = 55
			case "index-high":
				skill.Index = 64
			case "type":
				skill.InstanceValue = 2
			case "nil-owner":
				owner = nil
			case "nil-character":
				owner.Char = nil
			}
			nextID := w.nextMobID
			if w.castSummon(owner, skill, 320) || len(w.summons) != 0 || w.nextMobID != nextID {
				t.Fatal("invalid cast created a summon or consumed an entity ID")
			}
		})
	}
}

func TestBMSummonRecastRemovesOwnedDuplicatesOnly(t *testing.T) {
	w, owner, _ := handlerTestWorld(t)
	owner.Char.Class, owner.Char.LearnedSkill = 2, 1<<8
	skill := model.SkillDef{Index: 56, InstanceValue: 1}
	if !w.castSummon(owner, skill, 100) {
		t.Fatal("initial summon failed")
	}
	var original *Mob
	for _, m := range w.summons {
		original = m
	}
	var duplicate *Mob
	var preserved []*Mob
	for _, kind := range []byte{summonKindBM, summonKindContract, summonKindMount, summonKindThornWall, summonKindBM} {
		m := *original
		m.ID, m.SummonKind = w.allocMobID(), kind
		if len(preserved) == 3 {
			m.SummonerID++
		}
		w.appendMobInstance(&m)
		w.publishMobSpawn(&m)
		if duplicate == nil {
			duplicate = &m
		} else {
			preserved = append(preserved, &m)
		}
	}
	if w.castSummon(owner, skill, 100) || !duplicate.Dead || w.mobsByID[duplicate.ID] != nil ||
		w.summons[duplicate.ID] != nil || owner.hasVisible(duplicate.ID) {
		t.Fatal("recast did not remove the duplicate from authoritative and visible state")
	}
	if w.summons[original.ID] != original {
		t.Fatal("recast replaced the original creature")
	}
	for _, m := range preserved {
		if m.Dead || w.summons[m.ID] != m {
			t.Fatal("recast removed another owner or another summon kind")
		}
	}
}

// A teleported owner finds its summons beside it instead of walking back.
func TestTeleportBringsSummonsBesideOwner(t *testing.T) {
	w, owner, _ := handlerTestWorld(t)
	summon := &Mob{ID: 1450, X: owner.X + 1, Y: owner.Y, HP: 100, SummonerID: owner.ID,
		Def: testNPCDef(model.Score{MaxHP: 100})}
	w.mobs = append(w.mobs, summon)
	w.registerMobSpatial(summon)
	w.summons[summon.ID] = summon
	destX, destY := owner.X+60, owner.Y
	if !w.teleportPlayer(owner, destX, destY) {
		t.Skip("fixture terrain rejects the destination")
	}
	if chebyshev(summon.X, summon.Y, owner.X, owner.Y) > summonFollowDistance {
		t.Fatalf("summon left behind at (%d,%d), owner at (%d,%d)", summon.X, summon.Y, owner.X, owner.Y)
	}
}
