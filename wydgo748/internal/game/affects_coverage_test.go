package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestMobPublicExtendedHandlesMissingAndLiveMob(t *testing.T) {
	if mobPublicExtended(nil) != nil || mobPublicExtended(&Mob{}) != nil {
		t.Fatal("missing mob definition produced public stats")
	}
	mob := &Mob{Def: &model.NPCDef{Score: &model.Score{
		MaxHP: 100, Defense: 50, Attack: 200, AttackRun: 0x31,
		ResistFire: 5, ResistIce: 6, ResistHoly: 7, ResistThunder: 8,
	}}, HP: 150}
	stats := mobPublicExtended(mob)
	if stats == nil || stats.MaxHP != 100 || stats.CurHP != 100 || stats.Defense != 50 ||
		stats.Attack != 200 || stats.AttackRun != 0x31 || stats.ResistFire != 5 ||
		stats.ResistIce != 6 || stats.ResistHoly != 7 || stats.ResistThunder != 8 {
		t.Fatalf("public extended mob stats=%+v", stats)
	}
}
