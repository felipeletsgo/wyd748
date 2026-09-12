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
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
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
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	w.mobs = append(w.mobs, other)
	w.registerMobSpatial(other)
	if x, y, ok := findHellGatePosition(w, "target", 2200, 2200, 0, nil); !ok || x != 2200 || y != 2200 {
		t.Fatalf("colisao de outro runtime bloqueou Hell Gate: (%d,%d) ok=%v", x, y, ok)
	}
	own := &Mob{ID: 1402, X: 2200, Y: 2200, HP: 100, InstanceID: "target",
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
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
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	if !w.playerCanInteractWithMob(p, m) {
		t.Fatal("ataque de jogador oculto foi bloqueado antes de quebrar Hide")
	}
	if w.mobCanTargetPlayer(m, p) {
		t.Fatal("IA adquiriu jogador oculto")
	}
}

func TestMobOwnedAffectCannotSurviveOwnerRuntimeReuse(t *testing.T) {
	target := &Mob{ID: 1404, InstanceID: "runtime-a", HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	owner := &Mob{ID: 1405, InstanceID: "runtime-b", HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	w := testSpatialWorld([]*Mob{target, owner})
	w.activeMobs[target.ID] = target
	w.activeMobs[owner.ID] = owner
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	target.Affects[0] = model.Affect{Type: 12, ExpiresAt: now.Add(time.Minute)}
	target.Affects[0].OwnerID = owner.ID
	w.tickMobAffects(now, 0, 1)
	if target.Affects[0].Type != 0 || target.Affects[0].OwnerID != 0 {
		t.Fatal("affect de mob sobreviveu a troca de runtime do dono")
	}
}

func TestPlayerOwnedMobAffectDoesNotFollowReusedClientID(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	targetPlayer, _ := networkedTestPlayer(2, "Target", 2200, 2200)
	targetMob := &Mob{ID: 1415, X: 2200, Y: 2200, HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	w := testSpatialWorld([]*Mob{targetMob}, owner, targetPlayer)
	w.activeMobs[targetMob.ID] = targetMob
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	targetMob.Affects[0] = model.Affect{Type: 20, Level: 20, Value: 10,
		OwnerID: owner.ID, OwnerCharacterUID: owner.Char.UID,
		ExpiresAt: now.Add(time.Minute), NextTick: now}
	delete(w.playersByID, owner.ID)
	reused, _ := networkedTestPlayer(owner.ID, "Reused", 2200, 2200)
	w.playersByID[reused.ID] = reused
	w.players[reused.Session] = reused
	if w.playerByCharacterUID(owner.Char.UID) != nil {
		t.Fatal("indice de CharacterUID manteve o dono desconectado")
	}
	oldHP := targetMob.HP
	w.tickMobAffects(now, 0, 1)
	if targetMob.Affects[0].Type != 0 {
		t.Fatal("DoT de jogador sobreviveu ao reuso do ClientID")
	}
	if targetMob.HP != oldHP || reused.Char.Exp != 0 {
		t.Fatalf("ID reutilizado recebeu efeito/credito: hp=%d/%d exp=%d", targetMob.HP, oldHP, reused.Char.Exp)
	}
}

func TestLethalMobDotCannotCreditReusedClientID(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	targetMob := &Mob{ID: 1416, X: 2200, Y: 2200, HP: 1,
		Def: testNPCDef(model.Score{MaxHP: 1, CurHP: 1})}
	w := testSpatialWorld([]*Mob{targetMob}, owner)
	w.activeMobs[targetMob.ID] = targetMob
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	targetMob.Affects[0] = model.Affect{Type: 20, Level: 20, Value: 10,
		OwnerID: owner.ID, OwnerCharacterUID: owner.Char.UID,
		ExpiresAt: now.Add(time.Minute), NextTick: now}
	owner.InWorld = false
	w.unindexPlayerCharacter(owner)
	delete(w.playersByID, owner.ID)
	reused, _ := networkedTestPlayer(owner.ID, "Reused", 2200, 2200)
	w.playersByID[reused.ID] = reused
	w.players[reused.Session] = reused
	w.tickMobAffects(now, 0, 1)
	if targetMob.Affects[0].Type != 0 || targetMob.Dead || reused.Char.Exp != 0 {
		t.Fatalf("DoT letal transferiu kill credit ao ID reciclado: affect=%+v dead=%v exp=%d",
			targetMob.Affects[0], targetMob.Dead, reused.Char.Exp)
	}
}

func TestOwnerIDOnlyPlayerAffectCannotRebind(t *testing.T) {
	owner, _ := networkedTestPlayer(1, "Owner", 2200, 2200)
	target, _ := networkedTestPlayer(2, "Target", 2200, 2200)
	w := testSpatialWorld(nil, owner, target)
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	w.recalcPlayer(target.Char)
	setPlayerCurHP(target.Char, playerMaxHP(target.Char))
	target.Char.Affects[0] = model.Affect{Type: 20, Level: 20, Value: 10,
		OwnerID: owner.ID, ExpiresAt: now.Add(time.Minute), NextTick: now}
	owner.InWorld = false
	w.unindexPlayerCharacter(owner)
	delete(w.playersByID, owner.ID)
	reused, _ := networkedTestPlayer(owner.ID, "Reused", 2200, 2200)
	w.playersByID[reused.ID] = reused
	w.players[reused.Session] = reused
	before := playerCurHP(target.Char)
	w.tickPlayerAffects(now)
	if target.Char.Affects[0].Type != 0 || playerCurHP(target.Char) != before || reused.Char.Exp != 0 {
		t.Fatalf("affect legado por OwnerID foi associado ao ID reciclado: affect=%+v hp=%d/%d exp=%d",
			target.Char.Affects[0], playerCurHP(target.Char), before, reused.Char.Exp)
	}
}

func TestMobOwnedAffectKillsWithoutPlayerCredit(t *testing.T) {
	target := &Mob{ID: 1406, InstanceID: "runtime-a", HP: 1,
		Def: testNPCDef(model.Score{MaxHP: 1, CurHP: 1})}
	source := &Mob{ID: 1407, InstanceID: "runtime-a", HP: 100,
		Def: testNPCDef(model.Score{MaxHP: 100, CurHP: 100})}
	player, _ := networkedTestPlayer(1, "Observer", 2200, 2200)
	w := testSpatialWorld([]*Mob{target, source}, player)
	w.activeMobs[target.ID] = target
	now := time.Unix(1_700_000_000, 0)
	w.clock = newFakeClock(now)
	target.Affects[0] = model.Affect{Type: 20, Level: 20, Value: 10,
		OwnerID: source.ID, ExpiresAt: now.Add(time.Minute), NextTick: now}
	w.tickMobAffects(now, 0, 1)
	if !target.Dead || w.mobsByID[target.ID] != nil || player.Char.Exp != 0 {
		t.Fatalf("morte de efeito de mob concedeu estado/credito indevido: dead=%v mob=%v exp=%d",
			target.Dead, w.mobsByID[target.ID], player.Char.Exp)
	}
}

func TestGameplayCollisionMatrixSeparatesDynamicSpacesButSharesStatics(t *testing.T) {
	member, _ := networkedTestPlayer(1, "Member", 2200, 2200)
	public, _ := networkedTestPlayer(2, "Public", 2200, 2200)
	w := worldWithNetworkedPlayers(member, public)
	w.itemInstances = map[string]*ItemInstance{
		"runtime-a": {RuntimeID: "runtime-a", MemberIDs: []uint16{member.ID}},
	}
	w.rebuildPlayerInstanceIndex()
	public.X, public.Y = 2304, 2300
	w.updatePlayerSpatial(public)
	publicMob := &Mob{ID: 1410, X: 2300, Y: 2300, HP: 1,
		Def: testNPCDef(model.Score{MaxHP: 1, CurHP: 1})}
	foreignMob := &Mob{ID: 1411, X: 2301, Y: 2300, HP: 1, InstanceID: "runtime-b",
		Def: testNPCDef(model.Score{MaxHP: 1, CurHP: 1})}
	ownMob := &Mob{ID: 1412, X: 2302, Y: 2300, HP: 1, InstanceID: "runtime-a",
		Def: testNPCDef(model.Score{MaxHP: 1, CurHP: 1})}
	staticNPC := &Mob{ID: 1413, X: 2303, Y: 2300, HP: 1,
		Def: &model.NPCDef{Tipo: model.TipoNPC, Score: &model.Score{MaxHP: 1, CurHP: 1}}}
	w.mobs = append(w.mobs, publicMob, foreignMob, ownMob, staticNPC)
	for _, mob := range []*Mob{publicMob, foreignMob, ownMob, staticNPC} {
		w.registerMobSpatial(mob)
	}
	if !w.positionOccupiedInGameplaySpace(2300, 2300, "", nil, nil, nil) ||
		w.positionOccupiedInGameplaySpace(2300, 2300, "runtime-a", nil, nil, nil) {
		t.Fatal("mob publico nao foi isolado do runtime")
	}
	if w.positionOccupiedInGameplaySpace(2301, 2300, "runtime-a", nil, nil, nil) ||
		!w.positionOccupiedInGameplaySpace(2302, 2300, "runtime-a", nil, nil, nil) ||
		!w.positionOccupiedInGameplaySpace(2303, 2300, "runtime-a", nil, nil, nil) ||
		w.positionOccupiedInGameplaySpace(2304, 2300, "runtime-a", nil, nil, nil) ||
		!w.positionOccupiedInGameplaySpace(2304, 2300, "", nil, nil, nil) {
		t.Fatal("matriz de colisao runtime/global incorreta")
	}
}

func TestPendingInstanceMembersReserveCapacityAndReconnectInPlace(t *testing.T) {
	member, _ := networkedTestPlayer(1, "Member", 2200, 2200)
	offline, _ := networkedTestPlayer(2, "Offline", 2200, 2200)
	offline.Char.UID = "offline-stable-uid"
	w := worldWithNetworkedPlayers(member)
	inst := &ItemInstance{
		RuntimeID: "shared-runtime", Config: model.VolatileInstance{ID: "shared-runtime"},
		MemberIDs: []uint16{member.ID}, MemberCharacterUIDs: []string{offline.Char.UID},
	}
	w.itemInstances = map[string]*ItemInstance{inst.RuntimeID: inst}
	w.pendingInstanceMembers = map[string]map[string]struct{}{
		inst.RuntimeID: {offline.Char.UID: {}},
	}
	if got := w.instanceReservedParticipantCount(inst); got != 2 {
		t.Fatalf("membro pendente nao reservou vaga: got=%d", got)
	}
	candidate, _ := networkedTestPlayer(3, "Candidate", 2200, 2200)
	if got := w.instanceReservedParticipantCountWith(inst, []*Player{candidate}); got != 3 {
		t.Fatalf("capacidade nao contou nova identidade: got=%d", got)
	}
	offline.InWorld = true
	w.playersByID[offline.ID] = offline
	w.players[offline.Session] = offline
	w.attachRestoredInstanceMember(offline)
	if !itemInstanceHasMember(inst, offline.ID) || len(w.pendingInstanceMembers[inst.RuntimeID]) != 0 ||
		w.instanceReservedParticipantCount(inst) != 2 {
		t.Fatalf("reconexao abriu nova vaga em vez de converter a pendente: members=%v pending=%v count=%d",
			inst.MemberIDs, w.pendingInstanceMembers[inst.RuntimeID], w.instanceReservedParticipantCount(inst))
	}
}
