package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func instanceContractPlayer(id uint16, uid string, x, y uint16) *Player {
	return &Player{ID: id, InWorld: true, X: x, Y: y, Char: &model.Char{
		UID: uid, Name: "P", Extended: &model.ExtendedScore{
			Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100, MaxMP: 10, CurMP: 10,
		},
	}}
}

func TestInstanceIdentityModePolicyAndDeadlineHelpers(t *testing.T) {
	if itemInstanceGameplaySpace(nil) != "" || instanceRuntimeKey(nil) != "" || expectedChainItem(nil) != 0 {
		t.Fatal("nil instance/config ganhou identidade")
	}
	cfg := model.VolatileInstance{ID: " private ", RewardItem: 10}
	inst := &ItemInstance{Config: cfg}
	if itemInstanceGameplaySpace(inst) != "private" || instanceRuntimeKey(&cfg) != " private " || expectedChainItem(&cfg) != 10 {
		t.Fatalf("identidade privada divergente space=%q runtime=%q next=%d", itemInstanceGameplaySpace(inst), instanceRuntimeKey(&cfg), expectedChainItem(&cfg))
	}
	inst.RuntimeID = " runtime "
	if itemInstanceGameplaySpace(inst) != "runtime" {
		t.Fatal("RuntimeID nao teve precedencia")
	}
	cfg.SharedGroup = " zone "
	if instanceRuntimeKey(&cfg) != "shared:zone" {
		t.Fatalf("shared runtime=%q", instanceRuntimeKey(&cfg))
	}
	cfg.ChainNextItem = 11
	if expectedChainItem(&cfg) != 11 {
		t.Fatal("ChainNextItem nao teve precedencia")
	}

	w := &World{itemInstances: map[string]*ItemInstance{}}
	if got := w.nextItemInstanceRuntimeID(&cfg); got != "shared:zone" {
		t.Fatalf("runtime inicial=%q", got)
	}
	w.itemInstances["shared:zone"] = &ItemInstance{}
	w.itemInstances["shared:zone:1"] = &ItemInstance{}
	if got := w.nextItemInstanceRuntimeID(&cfg); got != "shared:zone:2" {
		t.Fatalf("runtime collision=%q", got)
	}
	var nilWorld *World
	if nilWorld.nextItemInstanceRuntimeID(&cfg) != "shared:zone" {
		t.Fatal("nil World deveria devolver base")
	}

	if instanceMode(model.VolatileInstance{}) != "private_chain" ||
		instanceMode(model.VolatileInstance{SharedEntry: true}) != "private_shared_entry" ||
		instanceMode(model.VolatileInstance{Mode: " SHARED_TIMED_ZONE "}) != "shared_timed_zone" {
		t.Fatal("instanceMode divergente")
	}
	shared := model.VolatileInstance{Mode: "shared_timed_zone"}
	if !sharedTimedInstance(shared) || sharedTimedInstance(model.VolatileInstance{}) {
		t.Fatal("sharedTimedInstance divergente")
	}
	if instanceAdmissionOnlySchedule(nil) || !instanceAdmissionOnlySchedule(&ItemInstance{Config: model.VolatileInstance{Mode: "shared_timed_zone", StateMachine: " HELL_GATE "}}) {
		t.Fatal("admission-only schedule divergente")
	}

	if instanceFinishPolicy(model.VolatileInstance{FinishPolicy: " FINISH_ON_TIMEOUT "}) != "finish_on_timeout" ||
		instanceFinishPolicy(shared) != "respawn_until_timeout" ||
		instanceFinishPolicy(model.VolatileInstance{Stages: []model.VolatileInstanceStage{{}, {}}}) != "advance_on_clear" ||
		instanceFinishPolicy(model.VolatileInstance{}) != "finish_on_clear" {
		t.Fatal("finish policy divergente")
	}
	if instanceCompletesOnClear(nil) || instanceCompletesOnClear(&ItemInstance{Config: model.VolatileInstance{FinishPolicy: "finish_on_timeout"}}) ||
		instanceCompletesOnClear(&ItemInstance{Config: model.VolatileInstance{FinishPolicy: "respawn_until_timeout"}}) ||
		instanceCompletesOnClear(&ItemInstance{Config: model.VolatileInstance{FinishPolicy: "state_machine"}}) ||
		!instanceCompletesOnClear(&ItemInstance{Config: model.VolatileInstance{FinishPolicy: "finish_on_clear"}}) {
		t.Fatal("instanceCompletesOnClear divergente")
	}

	now := time.Unix(2_500_000_000, 0)
	if !instanceCombatDeadline(nil).IsZero() {
		t.Fatal("nil combat deadline nao zerou")
	}
	deadline := now.Add(time.Minute)
	combat := now.Add(2 * time.Minute)
	inst = &ItemInstance{Deadline: deadline, CombatDeadline: combat}
	if !instanceCombatDeadline(inst).Equal(deadline) {
		t.Fatal("alias administrativo menor nao teve precedencia")
	}
	inst.Deadline = time.Time{}
	if !instanceCombatDeadline(inst).Equal(combat) {
		t.Fatal("CombatDeadline nao usado")
	}
	inst.CombatDeadline = time.Time{}
	inst.Deadline = deadline
	if !instanceCombatDeadline(inst).Equal(deadline) {
		t.Fatal("fallback Deadline falhou")
	}
	setInstanceCombatDeadline(nil, deadline)
	setInstanceCombatDeadline(inst, combat)
	if !inst.Deadline.Equal(combat) || !inst.CombatDeadline.Equal(combat) {
		t.Fatal("setInstanceCombatDeadline nao sincronizou aliases")
	}
}

func TestGeneratorReservationCoversStageEndpointsRouteAndUnrelatedGenerators(t *testing.T) {
	cfg := model.VolatileInstance{ID: "room", X: 100, Y: 100, AreaRadius: 2}
	rule := model.VolatileRule{Action: "instance_ticket", Instance: &cfg}
	w := &World{volatiles: model.VolatileCatalog{Rules: map[int]model.VolatileRule{1: rule}, Items: map[uint16]model.VolatileRule{}}}

	endpoint := model.NPCGener{}
	endpoint.Segments[0] = model.GenerSegment{X: 106, Y: 100}
	if !w.generatorReservedForItemInstance(endpoint) {
		t.Fatal("endpoint dentro da reserva minima nao foi bloqueado")
	}

	route := model.NPCGener{}
	route.Segments[0] = model.GenerSegment{X: 50, Y: 100}
	route.Segments[1] = model.GenerSegment{X: 150, Y: 100}
	if !w.generatorReservedForItemInstance(route) {
		t.Fatal("rota que cruza sala nao foi reservada")
	}

	far := model.NPCGener{}
	far.Segments[0] = model.GenerSegment{X: 300, Y: 300}
	far.Segments[1] = model.GenerSegment{X: 320, Y: 320}
	if w.generatorReservedForItemInstance(far) {
		t.Fatal("gerador distante foi reservado")
	}

	w.volatiles.Rules[2] = model.VolatileRule{Action: "other", Instance: &cfg}
	w.volatiles.Items[10] = rule
	if !w.generatorReservedForItemInstance(endpoint) {
		t.Fatal("override por item nao participou da reserva")
	}
}

func TestValidateItemInstanceTemplatesCoversRulesItemsStagesHellGateAndErrors(t *testing.T) {
	monster := model.NPCDef{Name: "Monster", Tipo: model.TipoMonstro, Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 1, CurHP: 1}}
	npc := model.NPCDef{Name: "Guide", Tipo: model.TipoNPC, Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion}}
	w := &World{npcs: []model.NPCDef{monster, npc}}

	cfg := model.VolatileInstance{ID: "room", Stages: []model.VolatileInstanceStage{{
		Spawns: []model.VolatileInstanceSpawn{{NPC: "Monster", Count: 1}},
		CompletionSpawns: []model.VolatileInstanceSpawn{{NPC: "Monster", Count: 1}},
	}}}
	w.volatiles = model.VolatileCatalog{Rules: map[int]model.VolatileRule{1: {Action: "instance_ticket", Instance: &cfg}}, Items: map[uint16]model.VolatileRule{10: {Action: "instance_ticket", Instance: &cfg}}}
	if err := w.validateItemInstanceTemplates(); err != nil {
		t.Fatalf("template valido recusado: %v", err)
	}

	bad := cfg
	bad.ID = "bad"
	bad.Stages[0].Spawns = []model.VolatileInstanceSpawn{{NPC: "Missing", Count: 1}}
	w.volatiles = model.VolatileCatalog{Rules: map[int]model.VolatileRule{1: {Action: "instance_ticket", Instance: &bad}}}
	if err := w.validateItemInstanceTemplates(); err == nil {
		t.Fatal("spawn inexistente foi aceito")
	}

	hell := model.VolatileInstance{ID: "hell", HellGate: &model.VolatileHellGate{
		FinalSpawns: []model.VolatileInstanceSpawn{{NPC: "Monster", Count: 1}},
		FinalNPCs: []model.VolatileInstanceSpawn{{NPC: "Guide", Count: 1}},
		Quadrants: []model.VolatileHellGateQuadrant{{Spawns: []model.VolatileInstanceSpawn{{NPC: "Monster", Count: 1}}, Lich: model.VolatileInstanceSpawn{NPC: "Monster", Count: 1}}},
	}}
	w.volatiles = model.VolatileCatalog{Items: map[uint16]model.VolatileRule{20: {Action: "instance_ticket", Instance: &hell}}}
	if err := w.validateItemInstanceTemplates(); err != nil {
		t.Fatalf("Hell Gate valido recusado: %v", err)
	}
	hell.HellGate.FinalNPCs[0].NPC = "Monster"
	w.volatiles.Items[20] = model.VolatileRule{Action: "instance_ticket", Instance: &hell}
	if err := w.validateItemInstanceTemplates(); err == nil {
		t.Fatal("NPC final hostil foi aceito")
	}
}

func TestInstancePartyStagesEvolutionAndMembershipHelpers(t *testing.T) {
	leader := instanceContractPlayer(1, "u1", 100, 100)
	member := instanceContractPlayer(2, "u2", 101, 100)
	dead := instanceContractPlayer(3, "u3", 102, 100)
	setPlayerCurHP(dead.Char, 0)
	if members, ok := instancePartyMembers(nil, "solo"); ok || members != nil {
		t.Fatal("leader nil foi aceito")
	}
	if members, ok := instancePartyMembers(leader, "solo"); !ok || len(members) != 1 {
		t.Fatal("solo nao retornou leader")
	}
	if _, ok := instancePartyMembers(leader, "party"); ok {
		t.Fatal("party sem grupo foi aceita")
	}
	if members, ok := instancePartyMembers(leader, ""); !ok || len(members) != 1 {
		t.Fatal("modo legado sem grupo falhou")
	}
	leader.Party = &Party{Members: []*Player{member, leader}}
	if _, ok := instancePartyMembers(leader, "party"); ok {
		t.Fatal("nao-lider iniciou party instance")
	}
	leader.Party = &Party{Members: []*Player{leader, nil, member, dead}}
	if members, ok := instancePartyMembers(leader, "party"); !ok || len(members) != 2 || members[1] != member {
		t.Fatalf("filtro de membros vivos=%v ok=%v", members, ok)
	}

	if instanceStages(nil) != nil || instanceSpawnCount(nil) != 0 {
		t.Fatal("nil stages/spawn count divergente")
	}
	cfg := model.VolatileInstance{Name: "legacy", X: 10, Y: 20, SpawnX: 11, SpawnY: 21, AreaRadius: 3, DurationSeconds: 4,
		Spawns: []model.VolatileInstanceSpawn{{NPC: "A", Count: 2}, {NPC: "B", Count: 3}}}
	stages := instanceStages(&cfg)
	if len(stages) != 1 || stages[0].Name != "legacy" || instanceSpawnCount(&cfg) != 5 {
		t.Fatalf("legacy stage=%+v count=%d", stages, instanceSpawnCount(&cfg))
	}
	cfg.Stages = []model.VolatileInstanceStage{{Name: "explicit", Spawns: []model.VolatileInstanceSpawn{{Count: 7}}}}
	if instanceStages(&cfg)[0].Name != "explicit" || instanceSpawnCount(&cfg) != 7 {
		t.Fatal("stages explicitas nao tiveram precedencia")
	}

	if !instanceAllowsEvolution(nil, []string{"arch"}) || !instanceAllowsEvolution(&model.Char{}, nil) ||
		!instanceAllowsEvolution(&model.Char{}, []string{"mortal"}) ||
		!instanceAllowsEvolution(&model.Char{Evolution: "arch"}, []string{"arch"}) ||
		!instanceAllowsEvolution(&model.Char{Evolution: "subcelestial"}, []string{"subcelestial"}) ||
		instanceAllowsEvolution(&model.Char{Evolution: "arch"}, []string{"mortal"}) {
		t.Fatal("instanceAllowsEvolution divergente")
	}

	inst := &ItemInstance{MemberIDs: []uint16{1, 2}, LeaderID: 99}
	if !itemInstanceHasMember(inst, 2) || itemInstanceHasMember(inst, 3) || itemInstanceHasMember(nil, 1) {
		t.Fatal("itemInstanceHasMember divergente")
	}
	ensureItemInstanceLeader(inst)
	if inst.LeaderID != 1 {
		t.Fatal("leader nao foi promovido")
	}
	inst.MemberIDs = nil
	ensureItemInstanceLeader(inst)
	if inst.LeaderID != 0 {
		t.Fatal("leader nao zerou sem membros")
	}
	ensureItemInstanceLeader(nil)
}

func TestInstanceParticipantReservationIndexAndExitGraceContracts(t *testing.T) {
	p1 := instanceContractPlayer(1, " uid1 ", 100, 100)
	p2 := &Player{ID: 2, Char: &model.Char{Name: "legacy", Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 1, CurHP: 1}}}
	if instanceParticipantKey(nil) != "" || instanceParticipantKey(&Player{}) != "" ||
		instanceParticipantKey(p1) != "uid:uid1" || instanceParticipantKey(p2) != "id:2" {
		t.Fatal("participant key divergente")
	}
	w := &World{
		playersByID: map[uint16]*Player{1: p1, 2: p2},
		playerInstance: make(map[uint16]string),
		pendingInstanceMembers: map[string]map[string]struct{}{"r": {"uid3": {}, " ": {}}},
	}
	inst := &ItemInstance{RuntimeID: "r", MemberIDs: []uint16{1, 2, 99}, MemberCharacterUIDs: []string{"uid1", "uid4", " "}}
	keys := w.instanceReservedParticipantKeys(inst)
	for _, key := range []string{"uid:uid1", "id:2", "id:99", "uid:uid3", "uid:uid4"} {
		if _, ok := keys[key]; !ok {
			t.Fatalf("reserved key ausente %q: %v", key, keys)
		}
	}
	if len(w.instanceReservedParticipantKeys(nil)) != 0 || (&World{}).instanceReservedParticipantCount(nil) != 0 {
		t.Fatal("nil reservation nao zerou")
	}
	if w.instanceReservedParticipantCount(inst) != len(keys) || w.instanceReservedParticipantCountWith(inst, []*Player{p1, instanceContractPlayer(5, "uid5", 0, 0)}) != len(keys)+1 {
		t.Fatal("participant count divergente")
	}
	w.indexInstanceMembers(inst)
	if w.playerInstance[1] != "r" || w.playerInstance[2] != "r" || w.playerInstance[99] != "r" {
		t.Fatalf("indexInstanceMembers=%v", w.playerInstance)
	}
	w.indexInstanceMembers(nil)

	now := time.Unix(2_500_000_000, 0)
	grace := &ItemInstance{Config: model.VolatileInstance{AllowChainDuringExitGrace: true, RewardItem: 20}, RewardGranted: true, ExitAt: now.Add(time.Second)}
	if !itemInstanceInExitGraceAt(grace, now) || itemInstanceInExitGraceAt(grace, now.Add(2*time.Second)) || itemInstanceInExitGraceAt(nil, now) {
		t.Fatal("exit grace temporal divergente")
	}
	grace.Config.AllowChainDuringExitGrace = false
	if itemInstanceInExitGraceAt(grace, now) {
		t.Fatal("exit grace sem opt-in foi aceita")
	}
}

func TestInstanceSpatialOwnershipStageAreaTargetAndNearestMember(t *testing.T) {
	w := testSpatialWorld(nil)
	now := time.Unix(2_500_000_000, 0)
	w.clock = newFakeClock(now)
	p1 := instanceContractPlayer(1, "u1", 102, 100)
	p2 := instanceContractPlayer(2, "u2", 105, 100)
	w.playersByID = map[uint16]*Player{1: p1, 2: p2}
	cfg := model.VolatileInstance{ID: "room", Stages: []model.VolatileInstanceStage{{X: 100, Y: 100, AreaRadius: 4}}}
	inst := &ItemInstance{RuntimeID: "room", Config: cfg, CurrentStage: 0, MemberIDs: []uint16{1, 2}}
	w.itemInstances = map[string]*ItemInstance{"room": inst}
	mob := bossContractMob(1000)
	mob.InstanceID = "room"
	mob.X, mob.Y = 100, 100

	if w.instanceForMob(nil) != nil || w.instanceForMob(&Mob{}) != nil || w.instanceForMob(mob) != inst {
		t.Fatal("instanceForMob divergente")
	}
	if _, ok := instanceStageForMob(nil); ok {
		t.Fatal("stage de nil retornou ok")
	}
	stage, ok := instanceStageForMob(inst)
	if !ok || stage.AreaRadius != 4 {
		t.Fatalf("stage=%+v ok=%v", stage, ok)
	}
	inst.CurrentStage = 9
	if _, ok := instanceStageForMob(inst); ok {
		t.Fatal("stage fora do range retornou ok")
	}
	inst.CurrentStage = 0
	if x, y, radius, ok := instanceMobArea(inst, mob); !ok || x != 100 || y != 100 || radius != 4 {
		t.Fatalf("mob area=%d,%d r=%d ok=%v", x, y, radius, ok)
	}
	if _, _, _, ok := instanceMobArea(nil, mob); ok {
		t.Fatal("area nil retornou ok")
	}
	if !instanceMemberInStage(inst, p1) || instanceMemberInStage(inst, p2) || instanceMemberInStage(nil, p1) || instanceMemberInStage(inst, nil) {
		t.Fatal("instanceMemberInStage divergente")
	}
	if !w.instanceMobInsideArea(mob) || !w.instanceMobStepAllowed(mob, 104, 100) || w.instanceMobStepAllowed(mob, 105, 100) {
		t.Fatal("limites de mob da instancia divergiram")
	}
	if !w.instanceMobTargetAllowed(mob, p1) || w.instanceMobTargetAllowed(mob, p2) || w.instanceMobTargetAllowed(mob, nil) {
		t.Fatal("target isolation divergente")
	}
	if got := w.nearestInstanceMember(mob); got != p1 {
		t.Fatalf("nearest=%p want=%p", got, p1)
	}
	if !w.instanceMobHasNearbyMember(mob, 3) || w.instanceMobHasNearbyMember(mob, -1) || w.instanceMobHasNearbyMember(nil, 3) {
		t.Fatal("nearby member divergente")
	}
	mob.InstanceID = "missing"
	if w.nearestInstanceMember(mob) != nil || w.instanceMobHasNearbyMember(mob, 3) {
		t.Fatal("mob sem runtime encontrou membro")
	}
}
