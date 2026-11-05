package game

import (
	"encoding/json"
	"fmt"
	"os"
	"regexp"
	"strconv"
	"strings"
	"testing"
	"time"
	"wydgo/internal/model"
)

func TestNativeAuditSharedEquipmentDamageFixtures(t *testing.T) {
	raw, err := os.ReadFile("../../../tmproject/TMProject748/tests/EquipmentDamageFixtures.h")
	if err != nil {
		t.Fatal(err)
	}
	rows := regexp.MustCompile(`(?m)^    \{([0-9, ]+)\},`).FindAllSubmatch(raw, -1)
	if len(rows) != 11 {
		t.Fatalf("equipment fixtures missing: %d", len(rows))
	}
	for n, row := range rows {
		t.Run(fmt.Sprint(n), func(t *testing.T) {
			fields := strings.Split(string(row[1]), ",")
			if len(fields) != 8 {
				t.Fatal("invalid equipment fixture width")
			}
			var v [8]int
			for i, field := range fields {
				v[i], err = strconv.Atoi(strings.TrimSpace(field))
				if err != nil {
					t.Fatal(err)
				}
			}
			w := &World{items: map[uint16]model.ItemDef{
				100: {Index: 100, Unique: v[1], StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: v[3]}}},
				101: {Index: 101, Unique: v[2], StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE2", Value: v[4]}}},
				102: {Index: 102, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: v[5]}}},
			}}
			ch := &model.Char{Class: byte(v[0]), LearnedSkill: uint32(v[6])}
			ch.Equip[6].Index, ch.Equip[7].Index, ch.Equip[1].Index = 100, 101, 102
			if got := w.equipmentDamage(ch); got != v[7] {
				t.Fatalf("equipment=%d want=%d", got, v[7])
			}
		})
	}
}

func TestNativeAuditSharedSkillPredictionFixtures(t *testing.T) {
	raw, err := os.ReadFile("../../../tmproject/TMProject748/tests/SkillPredictionFixtures.h")
	if err != nil {
		t.Fatal(err)
	}
	rows := regexp.MustCompile(`(?m)^    \{([0-9, ]+)\},`).FindAllSubmatch(raw, -1)
	if len(rows) < 144 {
		t.Fatalf("shared skill fixtures missing: %d", len(rows))
	}
	for n, row := range rows {
		t.Run(fmt.Sprint(n), func(t *testing.T) {
			fields := strings.Split(string(row[1]), ",")
			if len(fields) != 23 {
				t.Fatal("invalid fixture width")
			}
			var v [23]int
			for i, field := range fields {
				v[i], err = strconv.Atoi(strings.TrimSpace(field))
				if err != nil {
					t.Fatal(err)
				}
			}
			w := &World{weather: v[15], items: map[uint16]model.ItemDef{100: {
				Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_DAMAGE", Value: v[10]}},
			}}}
			ch := &model.Char{Class: byte(v[0]), X: 2000, Y: 2000, LearnedSkill: uint32(v[14]),
				Score: testScore(model.Score{Level: uint32(v[5]), Str: uint32(v[8]), Int: uint32(v[9]),
					Attack: uint32(v[11]), MagicAmp: uint32(v[12]), MagicAttack: uint32(v[13])})}
			ch.Equip[0].Index, ch.Equip[6].Index = uint16(v[7]), 100
			formula := serverSkillIndex(ch, v[1])
			if ch.Class == 3 {
				formula = v[1]
			}
			ch.Score.Mastery[formula%24/8+1] = uint32(v[6])
			skill := model.SkillDef{Index: v[1], InstanceType: v[2], InstanceValue: v[3], AffectValue: v[4],
				ManaSpent: v[17], Delay: v[19]}
			if got := w.baseSkillDamage(ch, skill); got != v[20] {
				t.Fatalf("damage=%d want=%d fixture=%v", got, v[20], v)
			}
			special := v[6]
			if v[1] >= 96 {
				special = v[5]
			}
			if got := skillManaCost(skill, special, v[18]); got != v[21] {
				t.Fatalf("mana=%d want=%d", got, v[21])
			}
			if got := skillCooldownDuration(skill); got != time.Duration(v[22])*time.Second {
				t.Fatalf("cooldown=%v want=%ds", got, v[22])
			}
		})
	}
}

func TestNativeAuditRefinementDecoding(t *testing.T) {
	for _, tc := range []struct {
		item model.Item
		want int
	}{
		{model.Item{Index: 100, Eff: [6]byte{116, 234, 43, 9}}, 9},
		{model.Item{Index: 100, Eff: [6]byte{43, 254}}, 16},
		{model.Item{Index: 100, Eff: [6]byte{126, 230}}, 10},
		{model.Item{Index: 2362, Eff: [6]byte{43, 9}}, 0},
		{model.Item{Index: 3980, Eff: [6]byte{43, 9}}, 0},
		{model.Item{Index: 1936, Eff: [6]byte{43, 50}}, 50},
	} {
		if got := itemSanc(tc.item); got != tc.want {
			t.Fatalf("item=%+v got=%d want=%d", tc.item, got, tc.want)
		}
	}
	def := model.ItemDef{Index: 100, StaticEffects: []model.StaticEffect{{Name: "EF_REGENHP", Value: 7}, {Name: "EF_RUNSPEED", Value: 1}}}
	item := model.Item{Index: 100, Eff: [6]byte{43, 234}}
	if got := itemAbility(item, def, "EF_REGENHP"); got != 77 {
		t.Fatalf("native regeneration=%d want 77", got)
	}
	if got := itemAbility(item, def, "EF_RUNSPEED"); got != 3 {
		t.Fatalf("refined run speed=%d want 3", got)
	}
	if got := itemAbility(model.Item{Index: 100, Eff: [6]byte{43, 254}},
		model.ItemDef{Pos: 0x100, StaticEffects: []model.StaticEffect{{Name: "EF_CRITICAL", Value: 100}}}, "EF_CRITICAL"); got != 7 {
		t.Fatalf("native effective +17 fallback=%d want 7", got)
	}
}

func TestNativeAuditOffhandRequirementBoundary(t *testing.T) {
	def := model.ItemDef{Index: 100, Pos: 192, ReqLevel: 10, ReqStr: 100, ReqInt: 20, ReqDex: 30, ReqCon: 40,
		StaticEffects: []model.StaticEffect{{Name: "EF_WTYPE", Value: 12}}}
	w := &World{items: map[uint16]model.ItemDef{100: def}}
	ch := &model.Char{Score: testScore(model.Score{Level: 13, Str: 130, Int: 26, Dex: 39, Con: 52})}
	item := model.Item{Index: 100}
	if !w.canEquip(ch, item, 7) {
		t.Fatal("native 130% boundary rejected")
	}
	ch.Score.Str = 129
	if w.canEquip(ch, item, 7) || !w.canEquip(ch, item, 6) {
		t.Fatal("second-weapon requirement was bypassed or applied to main hand")
	}
	ch.Evolution = "arch"
	if !w.canEquip(ch, item, 7) {
		t.Fatal("higher-evolution native exemption changed")
	}
}

func TestNativeAuditDamageTransformationUsesFace(t *testing.T) {
	w := &World{}
	skill := model.SkillDef{Index: 24, InstanceType: 1, InstanceValue: 30}
	ch := &model.Char{Class: 1, Score: testScore(model.Score{Level: 100, Int: 120, Mastery: [4]uint32{0, 56}})}
	mortal := w.baseSkillDamage(ch, skill)
	ch.Equip[0].Index = 16
	advanced := w.baseSkillDamage(ch, skill)
	if mortal != 225 || advanced != 357 {
		t.Fatalf("face damage mortal=%d advanced=%d; want 225,357", mortal, advanced)
	}
}

func TestClientMountHealthMatchesAuthoritativeCatalog(t *testing.T) {
	raw, err := os.ReadFile("../../data/content/mounts.json")
	if err != nil {
		t.Fatal(err)
	}
	var catalog struct {
		Types map[string]struct {
			MaxHP int `json:"maxHP"`
		} `json:"types"`
	}
	if err = json.Unmarshal(raw, &catalog); err != nil {
		t.Fatal(err)
	}
	header, err := os.ReadFile("../../../tmproject/TMProject748/internal/application/MountHealthContract.h")
	if err != nil {
		t.Fatal(err)
	}
	table := regexp.MustCompile(`MaximumByType\[30\] = \{([^}]+)\}`).FindSubmatch(header)
	if len(table) != 2 {
		t.Fatal("client mount HP table missing")
	}
	values := regexp.MustCompile(`[0-9]+`).FindAllString(string(table[1]), -1)
	if len(values) != 30 {
		t.Fatalf("mount table size=%d", len(values))
	}
	for i, value := range values {
		got, _ := strconv.Atoi(value)
		if got != catalog.Types[strconv.Itoa(i)].MaxHP {
			t.Fatalf("mount %d maximum=%d catalog=%d", i, got, catalog.Types[strconv.Itoa(i)].MaxHP)
		}
	}
}
