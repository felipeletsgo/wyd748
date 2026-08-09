package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestRuntimeIsolationMatrixMobsAndLoot(t *testing.T) {
	public, _ := networkedTestPlayer(1, "Public", 2200, 2200)
	member, _ := networkedTestPlayer(2, "Member", 2200, 2200)
	outsider, _ := networkedTestPlayer(3, "Outsider", 2200, 2200)
	w := testSpatialWorld(nil, public, member, outsider)
	w.itemInstances = map[string]*ItemInstance{
		"runtime-a": privateTestInstance("runtime-a", member.ID),
		"runtime-b": privateTestInstance("runtime-b", outsider.ID),
	}
	w.rebuildPlayerInstanceIndex()
	monster := &Mob{ID: 1400, X: 2200, Y: 2200, HP: 100,
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100})}
	w.mobs = append(w.mobs, monster)
	w.registerMobSpatial(monster)
	if !w.mobVisibleToPlayer(public, monster) || !w.playerCanInteractWithMob(public, monster) {
		t.Fatal("monstro publico deixou de pertencer ao espaco publico")
	}
	if w.mobVisibleToPlayer(member, monster) || w.playerCanInteractWithMob(member, monster) {
		t.Fatal("monstro publico vazou para runtime privado")
	}
	monster.InstanceID = "runtime-a"
	if !w.mobVisibleToPlayer(member, monster) || !w.playerCanInteractWithMob(member, monster) {
		t.Fatal("monstro do runtime nao foi acessivel ao membro")
	}
	if w.mobVisibleToPlayer(outsider, monster) || w.playerCanInteractWithMob(outsider, monster) {
		t.Fatal("monstro de runtime vazou para outro runtime")
	}

	private := &GroundItem{ID: 10001, InstanceID: "runtime-a"}
	publicDrop := &GroundItem{ID: 10002}
	permanent := &GroundItem{ID: 10003, InstanceID: "runtime-a", Permanent: true}
	if !w.groundItemVisibleToPlayer(member, private) || w.groundItemVisibleToPlayer(outsider, private) ||
		!w.groundItemVisibleToPlayer(public, publicDrop) || !w.groundItemVisibleToPlayer(outsider, permanent) {
		t.Fatal("isolamento de loot nao respeitou o espaco de gameplay")
	}
}

func TestOwnedAffectUsesCharacterUIDAndClearsOnIDReuse(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	owner.Char.UID = "owner-uid"
	target, _ := networkedTestPlayer(2, "Target", 2200, 2200)
	target.Char.UID = "target-uid"
	w := testSpatialWorld(nil, owner, target)
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	target.Char.Affects[0] = model.Affect{Type: 3, OwnerID: owner.ID,
		OwnerCharacterUID: owner.Char.UID, ExpiresAt: now.Add(time.Minute), NextTick: now}
	delete(w.playersByID, owner.ID)
	reused, _ := networkedTestPlayer(owner.ID, "Reused", 2200, 2200)
	reused.Char.UID = "different-uid"
	w.playersByID[reused.ID] = reused
	w.players[reused.Session] = reused
	w.tickPlayerAffects(now)
	if target.Char.Affects[0].Type != 0 {
		t.Fatal("affect antigo sobreviveu a reutilizacao do ClientID")
	}
}

func TestOwnedNonDotAffectCannotCrossRuntime(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	owner.Char.UID = "owner-uid"
	target, _ := networkedTestPlayer(2, "Target", 2200, 2200)
	target.Char.UID = "target-uid"
	w := testSpatialWorld(nil, owner, target)
	w.itemInstances = map[string]*ItemInstance{
		"runtime-a": privateTestInstance("runtime-a", owner.ID),
		"runtime-b": privateTestInstance("runtime-b", target.ID),
	}
	w.rebuildPlayerInstanceIndex()
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	target.Char.Affects[0] = model.Affect{Type: 12, OwnerID: owner.ID,
		OwnerCharacterUID: owner.Char.UID, ExpiresAt: now.Add(time.Minute)}
	w.tickPlayerAffects(now)
	if target.Char.Affects[0].Type != 0 {
		t.Fatal("debuff nao-DOT atravessou runtime")
	}
}

func TestPartyExperienceSharesOnlyGameplaySpace(t *testing.T) {
	killer, _ := networkedTestPlayer(1, "Killer", 2200, 2200)
	member, _ := networkedTestPlayer(2, "Member", 2200, 2200)
	outsider, _ := networkedTestPlayer(3, "Outsider", 2200, 2200)
	w := testSpatialWorld(nil, killer, member, outsider)
	w.itemInstances = map[string]*ItemInstance{
		"runtime-a": privateTestInstance("runtime-a", killer.ID),
		"runtime-b": privateTestInstance("runtime-b", outsider.ID),
	}
	w.itemInstances["runtime-a"].MemberIDs = []uint16{killer.ID, member.ID}
	w.itemInstances["runtime-b"].MemberIDs = []uint16{outsider.ID}
	w.rebuildPlayerInstanceIndex()
	party := &Party{Members: []*Player{killer, member, outsider}}
	killer.Party, member.Party, outsider.Party = party, party, party
	shares := w.partyExpShares(killer, 1000, 2)
	if len(shares) != 2 || shares[0].player != killer || shares[1].player != member {
		t.Fatalf("party EXP vazou para outro runtime: %+v", shares)
	}
}

func TestHellGateCollisionIgnoresOtherRuntimeButBlocksOwn(t *testing.T) {
	member, _ := networkedTestPlayer(1, "Member", 2200, 2200)
	w := testSpatialWorld(nil, member)
	other := &Mob{ID: 1401, X: 2200, Y: 2200, HP: 100, InstanceID: "other",
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100})}
	w.mobs = append(w.mobs, other)
	w.registerMobSpatial(other)
	if x, y, ok := findHellGatePosition(w, "target", 2200, 2200, 0, nil); !ok || x != 2200 || y != 2200 {
		t.Fatalf("colisao de outro runtime bloqueou Hell Gate: (%d,%d) ok=%v", x, y, ok)
	}
	own := &Mob{ID: 1402, X: 2200, Y: 2200, HP: 100, InstanceID: "target",
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100})}
	w.mobs = append(w.mobs, own)
	w.registerMobSpatial(own)
	if _, _, ok := findHellGatePosition(w, "target", 2200, 2200, 0, nil); ok {
		t.Fatal("Hell Gate ignorou colisao do proprio runtime")
	}
}

func TestMobInteractionAllowsAttackToBreakHideButAIRejectsHiddenTarget(t *testing.T) {
	p, _ := networkedTestPlayer(1, "Hidden", 2200, 2200)
	p.Char.Affects[0] = model.Affect{Type: 28, ExpiresAt: time.Now().Add(time.Minute)}
	w := testSpatialWorld(nil, p)
	m := &Mob{ID: 1403, X: 2201, Y: 2200, HP: 100,
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100})}
	if !w.playerCanInteractWithMob(p, m) {
		t.Fatal("ataque de jogador oculto foi bloqueado antes de quebrar Hide")
	}
	if w.mobCanTargetPlayer(m, p) {
		t.Fatal("IA adquiriu jogador oculto")
	}
}

func TestMobOwnedAffectCannotSurviveOwnerRuntimeReuse(t *testing.T) {
	target := &Mob{ID: 1404, InstanceID: "runtime-a", HP: 100,
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100})}
	owner := &Mob{ID: 1405, InstanceID: "runtime-b", HP: 100,
		Def: testNPCDef(model.ExtendedScore{MaxHP: 100, CurHP: 100})}
	w := testSpatialWorld([]*Mob{target, owner})
	w.activeMobs[target.ID] = target
	w.activeMobs[owner.ID] = owner
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	target.Affects[0] = model.Affect{Type: 12, ExpiresAt: now.Add(time.Minute)}
	target.AffectOwners[0] = owner.ID
	w.tickMobAffects(now, 0, 1)
	if target.Affects[0].Type != 0 || target.AffectOwners[0] != 0 {
		t.Fatal("affect de mob sobreviveu a troca de runtime do dono")
	}
}
