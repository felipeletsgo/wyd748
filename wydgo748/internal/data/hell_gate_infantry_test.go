package data

import (
	"path/filepath"
	"testing"

	"wydgo/internal/model"
)

func TestHellGate759InfantryUsesDedicatedHistoricalData(t *testing.T) {
	npcs, err := LoadNPCs(filepath.Join("..", "..", "data", "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	byName := map[string]model.NPCDef{}
	for _, npc := range npcs {
		byName[npc.Name] = npc
	}
	for _, tc := range []struct {
		name                                  string
		level, hp, attack, defense, exp, gold uint32
	}{
		{"HG_CorrSkel", 350, 26000, 850, 1600, 5000, 500},
		{"HG_SkelArch", 399, 24000, 3600, 1820, 5500, 500},
		{"HG_SkelMage", 399, 32000, 3600, 1850, 5500, 500},
		{"Guer_Caveira", 110, 8100, 468, 1236, 1500, 500},
		{"HG_SkelSpear", 399, 21000, 3410, 1940, 5500, 500},
		{"HG_SkelGuard", 399, 22000, 3600, 1700, 10900, 0},
	} {
		n := byName[tc.name]
		if !n.IsMonster() || n.Score == nil {
			t.Fatalf("missing infantry %s", tc.name)
		}
		s := n.Score
		if s.Level != tc.level || s.MaxHP != tc.hp || s.CurHP != tc.hp || s.Attack != tc.attack || s.Defense != tc.defense || n.ExpReward != tc.exp || n.Gold != tc.gold {
			t.Fatalf("%s does not match the imported historical score/EXP/gold", tc.name)
		}
		if s == byName["Skeleton"].Score || s == byName["HG_Skeleton"].Score {
			t.Fatal("event infantry shares an ordinary or restored-version score")
		}
		if tc.name != "Guer_Caveira" && len(n.Carry) != 0 {
			t.Fatalf("%s inherited unrelated ordinary loot", tc.name)
		}
	}
	if got := byName["HG_SkelMage"].SkillBar; got != [4]byte{32, 33, 32, 255} {
		t.Fatalf("historical mage bar = %v", got)
	}
	carry := byName["Guer_Caveira"].Carry
	if len(carry) != 64 || carry[6].Index != 4040 || carry[7].Index != 4039 || carry[33] != (model.Item{Index: 1176, Eff: [6]byte{103, 0, 0, 0, 0, 0}}) || carry[63].Index != 412 {
		t.Fatal("historical drop positions/options were not retained")
	}
	if n := byName["HG_Skeleton"]; n.ExpReward != 17000 || n.Score.MaxHP != 3300 || n.SkillBar != [4]byte{255, 255, 255, 255} {
		t.Fatal("restored version-2 template changed")
	}
}
