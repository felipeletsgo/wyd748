package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func privateTestInstance(id string, member uint16) *ItemInstance {
	return &ItemInstance{
		Config:    model.VolatileInstance{ID: id, Mode: "private_chain", Stages: []model.VolatileInstanceStage{{X: 2200, Y: 2200, AreaRadius: 12}}},
		RuntimeID: id, MemberIDs: []uint16{member},
	}
}

func TestGameplaySpaceBlocksPlayerSkillAndSupportAcrossRuntimes(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Caster", 2200, 2200)
	enemy, _ := networkedTestPlayer(2, "Enemy", 2200, 2200)
	w := testSpatialWorld(nil, caster, enemy)
	w.itemInstances = map[string]*ItemInstance{
		"water-a": privateTestInstance("water-a", caster.ID),
		"water-b": privateTestInstance("water-b", enemy.ID),
	}
	w.rebuildPlayerInstanceIndex()
	if w.playersShareGameplaySpace(caster, enemy) {
		t.Fatal("runtimes privados distintos compartilharam espaço de gameplay")
	}
	skill := model.SkillDef{Range: 6, MaxTarget: 3, Aggressive: 1}
	if got := w.skillPlayerTargets(caster, skillCastRequest{TargetID: enemy.ID}, skill); len(got) != 0 {
		t.Fatalf("skill PvP atravessou runtime: %+v", got)
	}
	if got := w.supportTargets(caster, skillCastRequest{TargetID: enemy.ID}, skill); len(got) != 0 {
		t.Fatalf("support targeting atravessou runtime: %+v", got)
	}

	// Mova ambos para o mesmo runtime sem deixar o caster duplicado no
	// runtime anterior; a produção mantém essa invariante após cada chain.
	w.itemInstances["water-a"].MemberIDs = nil
	w.itemInstances["water-b"].MemberIDs = []uint16{caster.ID, enemy.ID}
	w.rebuildPlayerInstanceIndex()
	if !w.playersShareGameplaySpace(caster, enemy) {
		t.Fatal("membros do mesmo runtime foram isolados")
	}
}

func TestSharedAndStateMachineRuntimesStayOutOfPublicGameplaySpace(t *testing.T) {
	for _, mode := range []string{"private_shared_entry", "shared_timed_zone", "state_machine"} {
		t.Run(mode, func(t *testing.T) {
			member, _ := networkedTestPlayer(1, "Member", 2200, 2200)
			other, _ := networkedTestPlayer(2, "Other", 2200, 2200)
			public, _ := networkedTestPlayer(3, "Public", 2200, 2200)
			w := testSpatialWorld(nil, member, other, public)
			w.itemInstances = map[string]*ItemInstance{
				"run-a": {RuntimeID: "run-a", Config: model.VolatileInstance{ID: "run-a", Mode: mode}, MemberIDs: []uint16{member.ID}},
				"run-b": {RuntimeID: "run-b", Config: model.VolatileInstance{ID: "run-b", Mode: mode}, MemberIDs: []uint16{other.ID}},
			}
			w.rebuildPlayerInstanceIndex()
			if w.playersShareGameplaySpace(member, public) || w.playersShareGameplaySpace(member, other) {
				t.Fatalf("runtime %q vazou para outro espaco de interacao", mode)
			}
			w.itemInstances["run-a"].MemberIDs = []uint16{member.ID, public.ID}
			w.itemInstances["run-b"].MemberIDs = nil
			w.rebuildPlayerInstanceIndex()
			if !w.playersShareGameplaySpace(member, public) {
				t.Fatalf("participantes do runtime %q nao compartilharam o espaco", mode)
			}
		})
	}
}

func TestPlayerAreaSkillAndSummonCannotCrossRuntime(t *testing.T) {
	caster, _ := networkedTestPlayer(1, "Caster", 2200, 2200)
	ally, _ := networkedTestPlayer(2, "Ally", 2200, 2200)
	other, _ := networkedTestPlayer(3, "Other", 2200, 2200)
	w := testSpatialWorld(nil, caster, ally, other)
	w.itemInstances = map[string]*ItemInstance{
		"water-a": privateTestInstance("water-a", caster.ID),
		"water-b": privateTestInstance("water-b", other.ID),
	}
	w.itemInstances["water-a"].MemberIDs = []uint16{caster.ID, ally.ID}
	w.rebuildPlayerInstanceIndex()

	skill := model.SkillDef{Range: 6, MaxTarget: 3, Aggressive: 1}
	targets := w.skillPlayerTargets(caster, skillCastRequest{TargetID: ally.ID}, skill)
	if len(targets) != 1 || targets[0] != ally {
		t.Fatalf("alvo do mesmo runtime foi recusado: %+v", targets)
	}
	for _, target := range targets {
		if target == other {
			t.Fatal("AoE incluiu jogador de outro runtime")
		}
	}
	if target := w.summonTarget(caster, other.ID); target.id != 0 {
		t.Fatal("summon conseguiu alvejar jogador de outro runtime")
	}
}

func TestPrivateRuntimeRejectsStaleDamageAffect(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	target, _ := networkedTestPlayer(2, "Target", 2200, 2200)
	w := testSpatialWorld(nil, owner, target)
	w.itemInstances = map[string]*ItemInstance{
		"water-a": privateTestInstance("water-a", owner.ID),
		"water-b": privateTestInstance("water-b", target.ID),
	}
	w.rebuildPlayerInstanceIndex()
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	w.recalcPlayer(target.Char)
	setPlayerCurHP(target.Char, playerMaxHP(target.Char))
	target.Char.Affects[0] = model.Affect{Type: 20, OwnerID: owner.ID, Value: 100,
		ExpiresAt: now.Add(time.Minute), NextTick: now}
	before := playerCurHP(target.Char)
	w.tickPlayerAffects(now)
	if playerCurHP(target.Char) != before || target.Char.Affects[0].Type != 0 {
		t.Fatalf("DoT atravessou runtime: hp=%d/%d affect=%+v", playerCurHP(target.Char), before, target.Char.Affects[0])
	}
}

func TestPrivateRuntimeRemovesDamageAffectAfterOwnerDisconnects(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	target, _ := networkedTestPlayer(2, "Target", 2200, 2200)
	w := testSpatialWorld(nil, owner, target)
	w.itemInstances = map[string]*ItemInstance{
		"water-a": privateTestInstance("water-a", owner.ID),
	}
	w.itemInstances["water-a"].MemberIDs = []uint16{owner.ID, target.ID}
	w.rebuildPlayerInstanceIndex()
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	w.recalcPlayer(target.Char)
	setPlayerCurHP(target.Char, playerMaxHP(target.Char))
	target.Char.Affects[0] = model.Affect{Type: 20, OwnerID: owner.ID, Value: 100,
		ExpiresAt: now.Add(time.Minute), NextTick: now}
	before := playerCurHP(target.Char)
	delete(w.playersByID, owner.ID)
	owner.InWorld = false
	w.tickPlayerAffects(now)
	if playerCurHP(target.Char) != before || target.Char.Affects[0].Type != 0 {
		t.Fatalf("DoT do jogador desconectado permaneceu ativo: hp=%d/%d affect=%+v", playerCurHP(target.Char), before, target.Char.Affects[0])
	}
}

func TestReconnectPositionIgnoresOtherPrivateRuntime(t *testing.T) {
	// A player from another runtime must not reserve the tile being restored.
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	other, _ := networkedTestPlayer(2, "Other", 2200, 2200)
	w := testSpatialWorld(nil, owner, other)
	w.itemInstances = map[string]*ItemInstance{
		"water-a": privateTestInstance("water-a", owner.ID),
		"water-b": privateTestInstance("water-b", other.ID),
	}
	w.rebuildPlayerInstanceIndex()
	x, y := w.findFreePlayerPositionInInstance(2200, 2200, 1, owner, "water-a")
	if x != 2200 || y != 2200 {
		t.Fatalf("runtime privado incorreto bloqueou o centro: (%d,%d)", x, y)
	}

	// A member from the same runtime must reserve it, forcing a nearby tile.
	same, _ := networkedTestPlayer(3, "Same", 2200, 2200)
	w = testSpatialWorld(nil, owner, same)
	w.itemInstances = map[string]*ItemInstance{
		"water-a": privateTestInstance("water-a", owner.ID),
	}
	w.itemInstances["water-a"].MemberIDs = []uint16{owner.ID, same.ID}
	w.rebuildPlayerInstanceIndex()
	x, y = w.findFreePlayerPositionInInstance(2200, 2200, 1, owner, "water-a")
	if x == 2200 && y == 2200 {
		t.Fatal("reconexao ignorou o membro do proprio runtime")
	}
}

func TestSummonsInheritRuntimeAndCannotTargetAnotherRuntime(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	other, _ := networkedTestPlayer(2, "Other", 2200, 2200)
	w := testSpatialWorld(nil, owner, other)
	w.itemInstances = map[string]*ItemInstance{
		"water-a": privateTestInstance("water-a", owner.ID),
		"water-b": privateTestInstance("water-b", other.ID),
	}
	w.rebuildPlayerInstanceIndex()
	otherMob := &Mob{ID: 1500, X: owner.X, Y: owner.Y, HP: 100,
		InstanceID: "water-b", Def: &model.NPCDef{Tipo: model.TipoMonstro,
			Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100}}}
	w.mobs = append(w.mobs, otherMob)
	w.mobsByID[otherMob.ID] = otherMob
	w.registerMobSpatial(otherMob)
	if target := w.summonTarget(owner, otherMob.ID); target.id != 0 {
		t.Fatal("summon conseguiu alvejar mob de outro runtime")
	}
	contract := &model.VolatileSummon{Name: "Knight", Face: 300, HP: 500,
		Attack: 100, Defense: 100, MoveSpeed: 4, AttackRange: 1}
	if !w.replaceContractSummon(owner, contract) {
		t.Fatal("summon de contrato nao foi criado")
	}
	for _, summon := range w.summons {
		if summon.SummonerID == owner.ID && summon.InstanceID != "water-a" {
			t.Fatalf("summon nao herdou runtime do dono: %q", summon.InstanceID)
		}
		if !w.mobVisibleToPlayer(other, summon) {
			continue
		}
		t.Fatal("summon privado vazou para outro runtime")
	}
}

func TestExistingSummonRebindsWhenOwnerEntersPrivateRuntime(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	w := testSpatialWorld(nil, owner)
	oldDef := &model.NPCDef{Tipo: model.TipoMonstro,
		Equip: model.Equip{Rosto: model.Item{Index: summonTemplates[0].face}},
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion,
			MaxHP: 100, CurHP: 100, AttackRun: 4}}
	existing := &Mob{ID: 1501, Def: oldDef, X: 2201, Y: 2200, HP: 100,
		SummonerID: owner.ID, SummonKind: summonKindBM}
	w.mobs = append(w.mobs, existing)
	w.registerMobSpatial(existing)
	w.itemInstances = map[string]*ItemInstance{
		"water-a": privateTestInstance("water-a", owner.ID),
	}
	w.rebuildPlayerInstanceIndex()
	if !w.castSummon(owner, model.SkillDef{InstanceValue: 1}, 0) {
		t.Fatal("recaste do summon existente falhou")
	}
	if existing.InstanceID != "water-a" {
		t.Fatalf("summon existente manteve runtime antigo: %q", existing.InstanceID)
	}
}

func TestWaterChainSnapshotContainsCharacterOnce(t *testing.T) {
	w, leader, _, store, clock := instanceTestWorld()
	secondRule := instanceTestRule()
	secondRule.Instance.ID = "water-normal-2"
	secondRule.Instance.RewardItem = 0
	w.volatiles.Items[3174] = secondRule
	w.volatiles.ItemCodes[3174] = 21
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	first := w.itemInstances["water-normal-1"]
	var mob *Mob
	for id := range first.MobIDs {
		mob = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(mob, clock.Now())
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if len(store.snapshots) < 2 {
		t.Fatalf("transicao nao persistiu dois snapshots: %d", len(store.snapshots))
	}
	snapshot := store.snapshots[len(store.snapshots)-1]
	count := 0
	for _, inst := range snapshot.Instances {
		for _, uid := range inst.MemberCharacterUIDs {
			if uid == leader.Char.UID {
				count++
			}
		}
	}
	if count != 1 {
		t.Fatalf("UID do personagem apareceu %d vezes no snapshot da transicao", count)
	}
}
