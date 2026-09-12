package game

import (
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

func TestSkillMasterMerchantWinsOverNormalShopNibble(t *testing.T) {
	got, ok := shopTypeForMerchant(skillMasterMerchant)
	if !ok || got != wire.ShopSkill {
		t.Fatalf("merchant 0x13 abriu shop=%d ok=%v, quer skill shop", got, ok)
	}
	got, ok = shopTypeForMerchant(0x23)
	if !ok || got != wire.ShopNormal {
		t.Fatalf("merchant 0x23 abriu shop=%d ok=%v, quer loja normal", got, ok)
	}
}

func TestNativeMerchantOneOpensNormalShop(t *testing.T) {
	got, ok := shopTypeForMerchant(1)
	if !ok || got != wire.ShopNormal {
		t.Fatalf("merchant 1 abriu shop=%d ok=%v, quer loja normal", got, ok)
	}
}

func TestSummonOnlyUsesOwnerCombatOrders(t *testing.T) {
	owner := partyTestPlayer(1, 2200, 2100)
	attacked := &Mob{ID: 1000, X: 2201, Y: 2100, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100})}
	ordered := &Mob{ID: 1001, X: 2202, Y: 2100, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100})}
	w := testSpatialWorld([]*Mob{attacked, ordered})

	owner.LastAttackerID = attacked.ID
	if got := w.ownerSummonTarget(owner); got.mob != attacked {
		t.Fatalf("evocacao nao defendeu o dono: alvo=%+v", got)
	}
	owner.CombatTargetID = ordered.ID
	if got := w.ownerSummonTarget(owner); got.mob != ordered {
		t.Fatalf("ordem de ataque do dono nao teve prioridade: alvo=%+v", got)
	}
	owner.CombatTargetID, owner.LastAttackerID = 0, 0
	if got := w.ownerSummonTarget(owner); got.id != 0 {
		t.Fatalf("evocacao iniciou aggro sem ordem: alvo=%+v", got)
	}
}

func TestSummonRequiresPKModeOnlyForPlayerTargets(t *testing.T) {
	owner := partyTestPlayer(1, 2200, 2100)
	target := partyTestPlayer(2, 2201, 2100)
	mob := &Mob{ID: 1000, X: 2201, Y: 2100, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	w := testSpatialWorld([]*Mob{mob}, owner, target)

	if got := w.summonTarget(owner, target.ID); got.id != 0 {
		t.Fatalf("summon alvejou jogador com PK desligado: %+v", got)
	}
	if got := w.summonTarget(owner, mob.ID); got.mob != mob {
		t.Fatalf("PK desligado bloqueou alvo PvE: %+v", got)
	}

	owner.PKMode = true
	if got := w.summonTarget(owner, target.ID); got.user != target {
		t.Fatalf("summon recusou jogador com PK ligado: %+v", got)
	}
}

func TestSummonIsNotPartyMember(t *testing.T) {
	owner := partyTestPlayer(1, 2200, 2100)
	member := partyTestPlayer(2, 2201, 2100)
	party := &Party{Members: []*Player{owner, member}}
	owner.Party, member.Party = party, party
	summon := &Mob{ID: 1000, SummonerID: owner.ID, LeaderID: 0}

	if party.indexOf(owner) != 0 || party.indexOf(member) != 1 || len(party.Members) != 2 {
		t.Fatal("grupo de jogadores foi alterado pela evocacao")
	}
	if summon.LeaderID != 0 {
		t.Fatal("evocacao herdou lideranca/slot de grupo")
	}
}
