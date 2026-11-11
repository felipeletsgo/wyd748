package game

import (
	"testing"

	"wydgo/internal/model"
)

func TestGameplaySpaceAndHostilityHelpersPreserveRuntimeBoundaries(t *testing.T) {
	w := &World{}
	if mobGameplaySpace(nil) != "" || mobGameplaySpace(&Mob{InstanceID: "  uxmal:2:1  "}) != "uxmal:2:1" {
		t.Fatal("mob gameplay space was not normalized")
	}
	if w.hostileMob(nil) || w.hostileMob(&Mob{}) || w.hostileMob(&Mob{Def: &model.NPCDef{Tipo: model.TipoNPC}}) {
		t.Fatal("nil or friendly mob was classified as hostile")
	}
	if !w.hostileMob(&Mob{Def: &model.NPCDef{Tipo: model.TipoMonstro}}) {
		t.Fatal("ordinary monster was not classified as hostile")
	}
	merchantMob := &Mob{Def: &model.NPCDef{Tipo: model.TipoMonstro, Score: &model.Score{Merchant: 1}}}
	if w.hostileMob(merchantMob) {
		t.Fatal("monster template with merchant state was classified as hostile")
	}
	if isCombatSummon(nil) || isCombatSummon(&Mob{SummonerID: 1, SummonKind: summonKindBM, Dead: true}) ||
		isCombatSummon(&Mob{SummonerID: 1, SummonKind: 99}) {
		t.Fatal("invalid combat summon was accepted")
	}
	for _, kind := range []byte{summonKindBM, summonKindContract} {
		if !isCombatSummon(&Mob{SummonerID: 1, SummonKind: kind}) {
			t.Errorf("living summon kind %q was not attackable", kind)
		}
	}
}

func TestKingdomGuardAndMobParticipationHelpers(t *testing.T) {
	if mobKingdom(nil) != model.KingdomNeutral || mobKingdom(&Mob{}) != model.KingdomNeutral {
		t.Fatal("missing mob definition should be neutral")
	}
	blueGuard := &Mob{Def: &model.NPCDef{Equip: model.Equip{Capa: model.Item{Index: 545}}}}
	redGuard := &Mob{Def: &model.NPCDef{Equip: model.Equip{Capa: model.Item{Index: 546}}}}
	if mobKingdom(blueGuard) != model.KingdomHekalotia || mobKingdom(redGuard) != model.KingdomAkelonia {
		t.Fatal("mob kingdom did not follow the cape")
	}
	blue := &Player{Char: &model.Char{}}
	blue.Char.Equip[model.CapeSlot].Index = 545
	red := &Player{Char: &model.Char{}}
	red.Char.Equip[model.CapeSlot].Index = 546
	if sameKingdomGuard(blueGuard, blue) != true || sameKingdomGuard(blueGuard, red) ||
		sameKingdomGuard(redGuard, blue) || sameKingdomGuard(&Mob{}, blue) || sameKingdomGuard(blueGuard, nil) ||
		sameKingdomGuard(blueGuard, &Player{}) {
		t.Fatal("same-kingdom guard classification is incorrect")
	}

	if validPlayerMobParticipant(nil) || validPlayerMobParticipant(&Player{InWorld: true}) ||
		validPlayerMobParticipant(&Player{Char: &model.Char{Score: &model.Score{Version: model.ScoreVersion}}}) {
		t.Fatal("missing, offline, or zero-HP player passed participation validation")
	}
	player := &Player{InWorld: true, Char: &model.Char{Score: &model.Score{Version: model.ScoreVersion, CurHP: 100}}}
	if !validPlayerMobParticipant(player) {
		t.Fatal("live player failed participation validation")
	}
	player.Char.RuntimeScore = &model.Score{Version: model.ScoreVersion, CurHP: 0}
	if validPlayerMobParticipant(player) {
		t.Fatal("runtime-dead player passed participation validation")
	}
}
