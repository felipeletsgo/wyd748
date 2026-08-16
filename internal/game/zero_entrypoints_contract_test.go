package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestLegacyCombatWrappersAreSafeOnMissingEntities(t *testing.T) {
	if playerHitsPlayer(nil, nil) != 0 {
		t.Fatal("playerHitsPlayer nil deveria ser zero")
	}
	if (&World{}).playerHitsPlayer(nil, nil) != 0 {
		t.Fatal("World.playerHitsPlayer nil deveria ser zero")
	}
	if mobHitsPlayer(nil, nil) != 0 {
		t.Fatal("mobHitsPlayer nil deveria ser zero")
	}
	if (&World{}).mobHitsPlayer(nil, nil) != 0 {
		t.Fatal("World.mobHitsPlayer nil deveria ser zero")
	}
	if bossSkillDamage(nil, nil, model.SkillDef{}) != 0 {
		t.Fatal("bossSkillDamage nil deveria ser zero")
	}
	if (&World{}).bossSkillDamage(nil, nil, model.SkillDef{}) != 0 {
		t.Fatal("World.bossSkillDamage nil deveria ser zero")
	}
}

func TestSetOwnedAffectForPlayerAtRequiresStableOwnerAndPersistsBothIdentities(t *testing.T) {
	now := time.Unix(2_300_000_000, 0)
	target := &model.Char{Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100}}
	if setOwnedAffectForPlayerAt(target, nil, 7, 1, 2, 3, now) {
		t.Fatal("owner nil foi aceito")
	}
	if setOwnedAffectForPlayerAt(target, &Player{ID: 9, Char: &model.Char{}}, 7, 1, 2, 3, now) {
		t.Fatal("owner sem UID estavel foi aceito")
	}
	owner := &Player{ID: 9, Char: &model.Char{UID: " 11111111111141118111111111111111 "}}
	if !setOwnedAffectForPlayerAt(target, owner, 7, 11, 12, 3, now) {
		t.Fatal("affect valido foi recusado")
	}
	found := false
	for _, affect := range target.Affects {
		if affect.Type != 7 {
			continue
		}
		found = true
		if affect.OwnerID != 9 || affect.OwnerCharacterUID != "11111111111141118111111111111111" ||
			affect.Value != 11 || affect.Level != 12 {
			t.Fatalf("ownership do affect incompleto: %+v", affect)
		}
	}
	if !found {
		t.Fatal("affect nao materializado")
	}
}

func TestRemoveContractSummonsRemovesOnlyContractEntities(t *testing.T) {
	contract := &Mob{ID: 1000, SummonerID: 7, SummonKind: summonKindContract}
	regular := &Mob{ID: 1001, SummonerID: 7, SummonKind: summonKindBM}
	otherOwner := &Mob{ID: 1002, SummonerID: 8, SummonKind: summonKindContract}
	w := &World{
		summons: map[uint16]*Mob{1000: contract, 1001: regular, 1002: otherOwner},
		mobs: []*Mob{contract, regular, otherOwner},
		mobsByID: map[uint16]*Mob{1000: contract, 1001: regular, 1002: otherOwner},
		mobListIndex: map[uint16]int{1000: 0, 1001: 1, 1002: 2},
		mobCells: make(map[uint32]map[uint16]*Mob), mobCell: make(map[uint16]uint32),
	}
	w.removeContractSummons(7)
	if !contract.Dead || regular.Dead || otherOwner.Dead {
		t.Fatalf("remoção seletiva divergente contract=%v regular=%v other=%v", contract.Dead, regular.Dead, otherOwner.Dead)
	}
	for _, mob := range w.mobs {
		if mob == contract {
			t.Fatal("contract summon permaneceu na lista viva")
		}
	}
}
