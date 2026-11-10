package game

import (
	"encoding/json"
	"errors"
	"testing"

	"wydgo/internal/model"
)

func TestIdealStoneKeepsEquippedHelmet(t *testing.T) {
	helmet := model.Item{Index: 1416, UID: "11111111111141118111111111111112"}
	stone := model.Item{Index: idealStoneItem, UID: "11111111111141118111111111111113"}
	t.Run("direct", func(t *testing.T) {
		w, p, store := newCelestialWorld(t, 399)
		p.Char.Equip[cytheraEquipSlot], p.Char.Inv[0] = helmet, stone
		w.useCelestialIdeal(p.Session, p, &p.Char.Inv[0], 0)
		if store.saves != 0 || p.Char.Equip[cytheraEquipSlot] != helmet ||
			p.Char.Evolution != archEvolution || p.Char.Inv[0] != stone {
			t.Fatal("the rejected conversion changed the character")
		}
	})
	t.Run("queued", func(t *testing.T) {
		w, p, _, backend, _, _, _, _ := criticalTinyFixture(t, true, true)
		p.Char.Evolution, p.Char.Score.Level, p.Char.ArchMortalLevel = archEvolution, 399, 399
		p.Char.Equip[cytheraEquipSlot], p.Char.Inv[0] = helmet, stone
		var rejected *nativeConsumableRejection
		if err := w.queueCelestialConsumable(p, 0, "celestial_ideal"); !errors.As(err, &rejected) {
			t.Fatalf("expected rejection before saving, got %v", err)
		}
		if p.Char.Equip[cytheraEquipSlot] != helmet || p.Char.Inv[0] != stone ||
			p.Char.Evolution != archEvolution || len(backend.entered) != 0 {
			t.Fatal("the rejected conversion changed state")
		}
	})
}

func TestExistingExpAboveLevelLockCannotAdvance(t *testing.T) {
	for _, evolution := range []string{"celestial", "subcelestial"} {
		ch := celestialCharacter(evolution, 38)
		ch.Exp = celestialNextLevel[40]
		levels, exp := grantExp(ch, 1)
		if levels != 0 || exp != 0 || ch.Score.Level != 38 || ch.Exp != celestialNextLevel[40] {
			t.Fatalf("%s advanced across the lock: level=%d applied=%d", evolution, ch.Score.Level, exp)
		}
	}
}

func TestSecondaryMasteryCapAffectsOnlyItsBranch(t *testing.T) {
	ch := celestialCharacter("celestial", 199)
	ch.SecondaryLearnedSkill = 1 << 4
	for branch, want := range [4]int{200, 200, 320, 200} {
		if got := masteryPointLimit(ch, branch); got != want {
			t.Errorf("branch %d cap=%d, want %d", branch, got, want)
		}
	}
}

func TestNativeBaseStatsMigrationKeepsAllocations(t *testing.T) {
	ch := testChar()
	ch.Score.Str += 20
	ch.Score.Con += 10
	syncStatusPoints(&ch)
	if !ch.Score.NativeBaseStats || ch.Score.Str != 32 || ch.Score.Con != 23 || ch.Score.StatusPts != 20 {
		t.Fatalf("migration changed allocation: %+v", ch.Score)
	}
	encoded, err := json.Marshal(ch.Score)
	if err != nil {
		t.Fatal(err)
	}
	loaded := ch
	loaded.Score = &model.Score{}
	if err := json.Unmarshal(encoded, loaded.Score); err != nil {
		t.Fatal(err)
	}
	syncStatusPoints(&loaded)
	if *loaded.Score != *ch.Score {
		t.Fatal("recalculating the migrated score changed the allocation")
	}
}
