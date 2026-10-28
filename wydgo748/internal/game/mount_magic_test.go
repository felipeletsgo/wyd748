package game

import (
	"encoding/binary"
	"fmt"
	"testing"
	"time"

	"wydgo/internal/data"
	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func mountMagicChar() *model.Char {
	return &model.Char{Class: 1, Score: &model.Score{
		Version: model.ScoreVersion, Level: 100, Int: 300,
		MagicAttack: 200, MagicAmp: 2, Mastery: [4]uint32{0, 80},
		MaxHP: 100, CurHP: 100, MaxMP: 100, CurMP: 100,
	}}
}

func TestMountMagicAmplifiesScoreAndSpellsAtEveryLevel(t *testing.T) {
	catalog, err := data.LoadMounts("../../data/content/mounts.json")
	if err != nil || len(catalog.Types) != 30 {
		t.Fatalf("mount catalog: count=%d err=%v", len(catalog.Types), err)
	}
	w := &World{mounts: catalog}
	skill := model.SkillDef{Index: 24, InstanceType: 1, InstanceValue: 60}
	for kind, stats := range catalog.Types {
		for level := 1; level <= 120; level++ {
			t.Run(fmt.Sprintf("%d/%d", kind, level), func(t *testing.T) {
				ch := mountMagicChar()
				mount := model.Item{Index: uint16(model.MountAdultBase + kind)}
				mount.SetMountHP(100)
				mount.SetMountLevel(level)
				ch.Equip[mountSlot] = mount
				w.recalcPlayer(ch)
				wantAmp := 2 + ((level+15)*stats.Magic/100+1)/4
				// Independent operands: 200 base + 80 mastery + 50 level +
				// 100 INT/3 + 10 INT/30; the spell adds its own 60 base.
				wantScore := 440 * (100 + 4*wantAmp) / 100 * 5 / 4
				wantSpell := 500 * (100 + 4*wantAmp) / 100 * 5 / 4
				if got := effectiveScore(ch); int(got.MagicAmp) != wantAmp || int(got.MagicAttack) != wantScore {
					t.Fatalf("score magic=%d amp=%d want=%d/%d", got.MagicAttack, got.MagicAmp, wantScore, wantAmp)
				}
				if got := w.baseSkillDamage(ch, skill); got != wantSpell {
					t.Fatalf("spell=%d want=%d", got, wantSpell)
				}
				packet := wire.UpdateScore(1, *ch)
				if binary.LittleEndian.Uint32(packet[12+3*4:]) != uint32(wantScore) ||
					binary.LittleEndian.Uint32(packet[12+23*4:]) != uint32(wantAmp) {
					t.Fatal("published score differs from authoritative magic")
				}
				w.recalcPlayer(ch)
				if int(effectiveScore(ch).MagicAttack) != wantScore {
					t.Fatal("recalculation stacked the mount bonus")
				}
			})
		}
	}
}

func TestMountMagicRequiresLivingAdultInCanonicalSlot(t *testing.T) {
	w := &World{mounts: model.MountCatalog{Types: map[int]model.MountStats{19: {Magic: 100}}}}
	plain := mountMagicChar()
	w.recalcPlayer(plain)
	skill := model.SkillDef{Index: 24, InstanceType: 1, InstanceValue: 60}
	for _, tc := range []struct {
		name     string
		index    uint16
		slot, hp int
	}{
		{"dead", 2379, mountSlot, 0},
		{"baby", 2349, mountSlot, 100},
		{"egg", 2319, mountSlot, 100},
		{"wrong-slot", 2379, 13, 100},
		{"missing-catalog", 2378, mountSlot, 100},
	} {
		t.Run(tc.name, func(t *testing.T) {
			ch := mountMagicChar()
			ch.Equip[tc.slot] = model.Item{Index: tc.index}
			ch.Equip[tc.slot].SetMountHP(tc.hp)
			ch.Equip[tc.slot].SetMountLevel(120)
			w.recalcPlayer(ch)
			if playerMagicAttack(ch) != playerMagicAttack(plain) ||
				w.baseSkillDamage(ch, skill) != w.baseSkillDamage(plain, skill) {
				t.Fatal("ineligible mount granted magic")
			}
		})
	}
	ch := mountMagicChar()
	ch.Equip[mountSlot] = model.Item{Index: 2379}
	ch.Equip[mountSlot].SetMountHP(100)
	ch.Equip[mountSlot].SetMountLevel(120)
	w.recalcPlayer(ch)
	if playerMagicAttack(ch) <= playerMagicAttack(plain) {
		t.Fatal("living mount granted no magic")
	}
	ch.Equip[mountSlot].SetMountHP(0)
	w.recalcPlayer(ch)
	if playerMagicAttack(ch) != playerMagicAttack(plain) {
		t.Fatal("death kept mount magic")
	}
	ch.Equip[mountSlot] = model.Item{}
	w.recalcPlayer(ch)
	if w.baseSkillDamage(ch, skill) != w.baseSkillDamage(plain, skill) {
		t.Fatal("unequip kept spell bonus")
	}
}

func TestPremiumMountMagicAlsoAppliesToSpells(t *testing.T) {
	now := time.Unix(1700000000, 0)
	w := &World{items: map[uint16]model.ItemDef{4200: premiumTestDef(4200, mountSlot)}, clock: newFakeClock(now)}
	plain, mounted := mountMagicChar(), mountMagicChar()
	mounted.Equip[mountSlot] = model.Item{Index: 4200, ActivatedUnix: now.Unix(), ExpiresUnix: now.Add(time.Hour).Unix()}
	w.recalcPlayer(plain)
	w.recalcPlayer(mounted)
	skill := model.SkillDef{Index: 24, InstanceType: 1, InstanceValue: 60}
	if got, want := w.baseSkillDamage(mounted, skill), w.baseSkillDamage(plain, skill)*165/100; got != want {
		t.Fatalf("timed mount spell=%d want=%d", got, want)
	}
	w.clock = newFakeClock(now.Add(2 * time.Hour))
	w.recalcPlayer(mounted)
	if w.baseSkillDamage(mounted, skill) != w.baseSkillDamage(plain, skill) {
		t.Fatal("expired mount kept spell bonus")
	}
}

func TestPremiumAndKRMountMagicWithProductionCatalog(t *testing.T) {
	catalog, err := data.LoadCatalog("../../data/catalog/itemlist.csv", "../../data/catalog/Itemname.csv", "../../data/catalog/SkillData.csv")
	if err != nil {
		t.Fatal(err)
	}
	mounts, err := data.LoadMounts("../../data/content/mounts.json")
	if err != nil {
		t.Fatal(err)
	}
	now := time.Unix(1700000000, 0)
	w := &World{items: catalog.Items, mounts: mounts, clock: newFakeClock(now)}
	for index := uint16(3980); index <= 6014; index++ {
		if index > 3989 && index < 6000 {
			continue
		}
		t.Run(fmt.Sprint(index), func(t *testing.T) {
			for class := byte(0); class < 3; class++ {
				t.Run(fmt.Sprintf("class-%d", class), func(t *testing.T) {
					plain, mounted := mountMagicChar(), mountMagicChar()
					plain.Class, mounted.Class = class, class
					mounted.Equip[mountSlot] = model.Item{Index: index, UID: "33333333333343338333333333333333"}
					if _, err := w.activateTimedItem(&mounted.Equip[mountSlot], mountSlot, now); err != nil {
						t.Fatal(err)
					}
					w.recalcPlayer(plain)
					w.recalcPlayer(mounted)
					percent := staticAbility(catalog.Items[index], "EF_MAGIC")
					if stats, ok := mounts.PremiumStats(index); ok {
						percent = stats.MagicPercent
					}
					if percent <= 0 {
						t.Fatal("production mount has no expected magic bonus")
					}
					if got, want := playerMagicAttack(mounted), playerMagicAttack(plain)*(100+percent)/100; got != want {
						t.Fatalf("score=%d want=%d", got, want)
					}
					checkedSkills := 0
					for _, skill := range catalog.Skills {
						if skillUsesMagic(mounted, skill) && skill.Index != 97 && skill.Index >= int(class)*24 && skill.Index < (int(class)+1)*24 {
							checkedSkills++
							if got, want := w.baseSkillDamage(mounted, skill), w.baseSkillDamage(plain, skill)*(100+percent)/100; got != want {
								t.Fatalf("skill %d damage=%d want=%d", skill.Index, got, want)
							}
						}
					}
					if checkedSkills == 0 {
						t.Fatal("production catalog did not exercise any damaging magic skills")
					}
				})
			}
		})
	}
}
