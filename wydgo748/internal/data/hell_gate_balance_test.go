package data

import (
	"path/filepath"
	"testing"
	"time"
)

func TestHellGateBossBalanceUsesMagicCastCadence(t *testing.T) {
	root := filepath.Join("..", "..", "data")
	npcs, err := LoadNPCs(filepath.Join(root, "npcs"))
	if err != nil {
		t.Fatal(err)
	}
	byName := make(map[string]struct {
		monster bool
		maxHP   uint32
		curHP   uint32
	})
	for _, npc := range npcs {
		if npc.Score == nil {
			continue
		}
		byName[npc.Name] = struct {
			monster bool
			maxHP   uint32
			curHP   uint32
		}{monster: npc.IsMonster(), maxHP: npc.Score.MaxHP, curHP: npc.Score.CurHP}
	}

	const averageMagicDamagePerCast uint32 = 10000
	const magicRotationCadence = 900 * time.Millisecond
	// This is an explicit project planning model: FelipeTr's requested average
	// MAGIC damage is 10,000 per cast, rotating spells every 900 milliseconds.
	// It is not a claim about observed FelipeTr gameplay or a change to cooldowns.
	playerBosses := []struct {
		name  string
		maxHP uint32
	}{
		{name: "HG_Spider", maxHP: 400000},
		{name: "HG_Corridor", maxHP: 400000},
		{name: "HG_Lich", maxHP: 400000},
	}
	for _, tc := range playerBosses {
		got, ok := byName[tc.name]
		if !ok || !got.monster || got.maxHP != tc.maxHP || got.curHP != tc.maxHP {
			t.Fatalf("%s calibration = %+v, want monster=true maxHP=curHP=%d", tc.name, got, tc.maxHP)
		}
		plannedCasts := (got.maxHP + averageMagicDamagePerCast - 1) / averageMagicDamagePerCast
		// The first cast is immediate; later casts follow the spell rotation.
		killTime := time.Duration(plannedCasts-1) * magicRotationCadence
		if plannedCasts != 40 || killTime != 35100*time.Millisecond {
			t.Errorf("%s solo magic estimate = %d casts / %s, want 40 casts / 35.1s", tc.name, plannedCasts, killTime)
		}
	}

	protector, ok := byName["HG_Dionys"]
	if !ok || protector.monster || protector.maxHP != 150000 {
		t.Fatalf("HG_Dionys calibration = %+v, want monster=false maxHP=150000", protector)
	}
}
