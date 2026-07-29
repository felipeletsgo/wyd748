package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func instanceTestRule() model.VolatileRule {
	return model.VolatileRule{
		Action: "instance_ticket", Consume: true,
		Instance: &model.VolatileInstance{
			ID: "water-normal-1", Name: "Water Room",
			X: 2200, Y: 2200, SpawnX: 2202, SpawnY: 2202,
			AreaRadius: 8,
			Spawns:     []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}},
			RewardItem: 3174, AllowedEvolutions: []string{"mortal"},
			DurationSeconds: 120, ExitX: 2100, ExitY: 2100,
		},
	}
}

func instanceTestWorld() (*World, *Player, *Player, *guildFlowStore, *fakeClock) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
	leader.Party = &Party{Members: []*Player{leader, member}}
	member.Party = leader.Party
	w, st := guildFlowWorld(leader, member)
	clock := newFakeClock(time.Unix(2_000_000_000, 0))
	w.clock = clock
	w.itemInstances = make(map[string]*ItemInstance)
	w.npcs = []model.NPCDef{{
		Name: "RoomMob", Tipo: model.TipoMonstro,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100},
	}}
	w.volatiles = model.VolatileCatalog{
		Default:   model.VolatileRule{Action: "generic"},
		Items:     map[uint16]model.VolatileRule{100: instanceTestRule()},
		ItemCodes: map[uint16]int{100: 21},
	}
	w.items = map[uint16]model.ItemDef{100: {Index: 100}, 3174: {Index: 3174}}
	leader.Char.Inv[0] = model.Item{Index: 100}
	return w, leader, member, st, clock
}

func TestInstanceTicketMovesPartySpawnsAndExpires(t *testing.T) {
	w, leader, member, st, clock := instanceTestWorld()
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil || len(inst.MobIDs) != 1 || inst.Remaining != 1 ||
		leader.Char.Inv[0].Index != 0 || st.gameSaves != 1 {
		t.Fatalf("instancia=%+v item=%d saves=%d", inst, leader.Char.Inv[0].Index, st.gameSaves)
	}
	for mobID := range inst.MobIDs {
		if !leader.hasVisible(mobID) || !member.hasVisible(mobID) {
			t.Fatalf("mob inicial %d nao foi publicado aos dois membros", mobID)
		}
	}
	if chebyshev(leader.X, leader.Y, 2200, 2200) > 4 ||
		chebyshev(member.X, member.Y, 2200, 2200) > 4 ||
		(leader.X == member.X && leader.Y == member.Y) {
		t.Fatalf("party nao teleportada/separada: leader=(%d,%d) member=(%d,%d)",
			leader.X, leader.Y, member.X, member.Y)
	}

	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(mob, clock.Now())
	if inst.Remaining != 0 || !inst.ExitAt.Equal(clock.Now().Add(10*time.Second)) ||
		!inst.RewardGranted || leader.Char.Inv[0].Index != 3174 ||
		st.gameSaves != 1 || st.saves != 1 {
		t.Fatalf("conclusao/recompensa incorreta: remaining=%d exit=%v reward=%v item=%d txn=%d saves=%d",
			inst.Remaining, inst.ExitAt, inst.RewardGranted, leader.Char.Inv[0].Index,
			st.gameSaves, st.saves)
	}
	clock.Advance(10 * time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances["water-normal-1"] != nil ||
		chebyshev(leader.X, leader.Y, 2100, 2100) > 3 ||
		chebyshev(member.X, member.Y, 2100, 2100) > 3 {
		t.Fatal("instancia nao encerrou/recolheu a party")
	}
}

func TestInstanceRewardRetriesAfterPersistenceFailure(t *testing.T) {
	w, leader, _, st, clock := instanceTestWorld()
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}

	st.err = errors.New("database unavailable")
	w.onItemInstanceMobKilled(mob, clock.Now())
	if inst.RewardGranted || !inst.ExitAt.IsZero() || leader.Char.Inv[0].Index != 0 {
		t.Fatal("falha de persistencia confirmou recompensa ou encerrou a sala")
	}

	st.err = nil
	clock.Advance(time.Second)
	w.tickItemInstances(clock.Now())
	if !inst.RewardGranted || inst.ExitAt.IsZero() || leader.Char.Inv[0].Index != 3174 {
		t.Fatal("recompensa pendente nao foi repetida depois da recuperacao")
	}
}

func TestInstanceRewardDropsWhenInventoryIsFullAndCannotDuplicate(t *testing.T) {
	w, leader, _, st, clock := instanceTestWorld()
	w.nextItemID = 10000
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		leader.Char.Inv[slot] = model.Item{Index: 100}
	}

	w.onItemInstanceMobKilled(mob, clock.Now())
	if !inst.RewardGranted || len(w.groundItems) != 1 || st.saves != 0 {
		t.Fatalf("fallback no chao incorreto: granted=%v ground=%d saves=%d",
			inst.RewardGranted, len(w.groundItems), st.saves)
	}
	for _, ground := range w.groundItems {
		if ground.Item.Index != 3174 || ground.Item.UID == "" {
			t.Fatalf("recompensa no chao perdeu indice/UID: %+v", ground.Item)
		}
	}

	w.onItemInstanceMobKilled(mob, clock.Now())
	if len(w.groundItems) != 1 {
		t.Fatal("notificacao de morte repetida duplicou a recompensa")
	}
}

func TestInstanceSupportsMixedBossPopulationWithoutReward(t *testing.T) {
	w, leader, _, _, _ := instanceTestWorld()
	w.npcs = append(w.npcs, model.NPCDef{
		Name: "BossMob", Tipo: model.TipoMonstro,
		Extended: &model.ExtendedScore{
			Version: model.ExtendedScoreVersion, MaxHP: 1000, CurHP: 1000,
		},
	})
	rule := instanceTestRule()
	rule.Instance.ID = "water-boss"
	rule.Instance.Spawns = []model.VolatileInstanceSpawn{
		{NPC: "BossMob", Count: 1}, {NPC: "RoomMob", Count: 2},
	}
	rule.Instance.RewardItem = 0
	w.volatiles.Items[100] = rule

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-boss"]
	if inst == nil || inst.Remaining != 3 || len(inst.MobIDs) != 3 {
		t.Fatalf("populacao mista incorreta: %+v", inst)
	}
}

func TestInstanceRefusesPhysicalRoomOccupationAndWrongEvolution(t *testing.T) {
	w, leader, member, st, _ := instanceTestWorld()
	member.X, member.Y = 2200, 2200
	member.Char.X, member.Char.Y = member.X, member.Y
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 100 || st.gameSaves != 0 {
		t.Fatal("sala fisicamente ocupada consumiu ticket")
	}

	member.X, member.Y = 2300, 2300
	leader.Char.Evolution = "arch"
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 100 || st.gameSaves != 0 {
		t.Fatal("evolucao nao permitida abriu a sala")
	}
}

func TestInstanceTicketRollbackAndRoomExclusion(t *testing.T) {
	w, leader, _, st, _ := instanceTestWorld()
	st.err = errors.New("database unavailable")
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 100 || leader.X != 2100 ||
		len(w.itemInstances) != 0 || len(w.mobsByID) != 0 {
		t.Fatal("falha de persistencia deixou instancia parcial")
	}

	st.err = nil
	w.itemInstances["water-normal-1"] = &ItemInstance{}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 100 || st.gameSaves != 1 {
		t.Fatal("sala ocupada consumiu/persistiu ticket")
	}
}

func TestOnlyPartyLeaderCanOpenInstance(t *testing.T) {
	w, _, member, st, _ := instanceTestWorld()
	member.Char.Inv[0] = model.Item{Index: 100}
	w.onUseItem(member.Session, useItemPacket(0, 0))
	if member.Char.Inv[0].Index != 100 || st.gameSaves != 0 || len(w.itemInstances) != 0 {
		t.Fatal("membro comum abriu instancia")
	}
}

func TestInstanceLogoutFreesSlotAndPromotesLeader(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil {
		t.Fatal("instancia nao abriu")
	}

	w.detachPlayerFromItemInstances(leader.ID, clock.Now())

	if len(inst.MemberIDs) != 1 || inst.MemberIDs[0] != member.ID ||
		inst.LeaderID != member.ID {
		t.Fatalf("saida do lider deixou membro fantasma: members=%v leader=%d",
			inst.MemberIDs, inst.LeaderID)
	}

	w.detachPlayerFromItemInstances(member.ID, clock.Now())
	if len(inst.MemberIDs) != 0 || !inst.Deadline.Equal(clock.Now()) ||
		!inst.TransitionAt.IsZero() || !inst.QuizAt.IsZero() {
		t.Fatalf("ultima saida nao agendou cleanup: %+v", inst)
	}
}

func TestInstanceEnforcesConfiguredPlayerLimit(t *testing.T) {
	w, leader, _, st, _ := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.MaxPlayers = 1
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 100 || st.gameSaves != 0 ||
		len(w.itemInstances) != 0 {
		t.Fatal("party acima do limite abriu ou consumiu a instancia")
	}
}

func TestCubeSharedEntryConsumesOneTicketPerPlayer(t *testing.T) {
	w, leader, member, st, _ := instanceTestWorld()
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	rule.Instance.ID = "cube-shared"
	rule.Instance.SharedEntry = true
	rule.Instance.MaxPlayers = 6
	rule.Instance.RewardItem = 0
	w.volatiles.Items[100] = rule
	member.Char.Inv[0] = model.Item{Index: 100}

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["cube-shared"]
	if inst == nil || len(inst.MemberIDs) != 1 || leader.Char.Inv[0].Index != 0 ||
		member.Char.Inv[0].Index != 100 || st.gameSaves != 1 {
		t.Fatalf("abertura individual incorreta: inst=%+v itens=%d/%d saves=%d",
			inst, leader.Char.Inv[0].Index, member.Char.Inv[0].Index, st.gameSaves)
	}

	w.onUseItem(member.Session, useItemPacket(0, 0))
	if len(inst.MemberIDs) != 2 || !itemInstanceHasMember(inst, member.ID) ||
		member.Char.Inv[0].Index != 0 || st.gameSaves != 2 ||
		chebyshev(member.X, member.Y, 2200, 2200) > 4 {
		t.Fatalf("ingresso compartilhado incorreto: membros=%v item=%d saves=%d pos=(%d,%d)",
			inst.MemberIDs, member.Char.Inv[0].Index, st.gameSaves, member.X, member.Y)
	}
}

func TestCubeSharedEntryRollbackAndStageLock(t *testing.T) {
	w, leader, member, st, clock := instanceTestWorld()
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	rule.Instance.ID = "cube-shared-lock"
	rule.Instance.SharedEntry = true
	rule.Instance.MaxPlayers = 6
	rule.Instance.RewardItem = 0
	rule.Instance.TransitionSeconds = 1
	rule.Instance.Stages = []model.VolatileInstanceStage{
		{X: 2200, Y: 2200, SpawnX: 2202, SpawnY: 2202, AreaRadius: 8,
			DurationSeconds: 60,
			Spawns:          []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}}},
		{X: 2250, Y: 2250, SpawnX: 2252, SpawnY: 2252, AreaRadius: 8,
			DurationSeconds: 60,
			Spawns:          []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}}},
	}
	w.volatiles.Items[100] = rule
	member.Char.Inv[0] = model.Item{Index: 100}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["cube-shared-lock"]

	oldX, oldY := member.X, member.Y
	st.err = errors.New("database unavailable")
	w.onUseItem(member.Session, useItemPacket(0, 0))
	if member.Char.Inv[0].Index != 100 || member.X != oldX || member.Y != oldY ||
		len(inst.MemberIDs) != 1 {
		t.Fatal("falha do banco consumiu convite ou inseriu membro")
	}

	st.err = nil
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(mob, clock.Now())
	clock.Advance(time.Second)
	w.tickItemInstances(clock.Now())
	if inst.CurrentStage != 1 {
		t.Fatal("fixture nao avancou para a segunda sala")
	}
	w.onUseItem(member.Session, useItemPacket(0, 0))
	if member.Char.Inv[0].Index != 100 || len(inst.MemberIDs) != 1 {
		t.Fatal("Cube aceitou ingresso depois da primeira sala")
	}
}

func TestInstanceFirstSpawnFailureRollsBackTicketAndPosition(t *testing.T) {
	w, leader, _, st, _ := instanceTestWorld()
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	w.volatiles.Items[100] = rule
	// Ocupa toda a faixa de IDs sem materializar dezenas de milhares de mobs.
	for id := uint32(firstMobID); id <= uint32(^uint16(0)); id++ {
		w.mobsByID[uint16(id)] = nil
	}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 100 || leader.X != 2100 || leader.Y != 2100 ||
		st.gameSaves != 0 || len(w.itemInstances) != 0 || len(w.mobs) != 0 {
		t.Fatalf("falha do primeiro spawn deixou estado: item=%d pos=(%d,%d) saves=%d inst=%d mobs=%d",
			leader.Char.Inv[0].Index, leader.X, leader.Y, st.gameSaves,
			len(w.itemInstances), len(w.mobs))
	}
}

func TestCubeQuizUsesNativeFourByFourPlatform(t *testing.T) {
	quiz := &model.VolatileInstanceQuiz{
		Answer: true, TrueX: 100, TrueY: 200, FalseX: 300, FalseY: 400,
	}
	for _, point := range [][2]uint16{{97, 197}, {97, 200}, {100, 197}, {100, 200}} {
		if !instanceQuizCorrect(&Player{X: point[0], Y: point[1]}, quiz) {
			t.Errorf("ponto nativo (%d,%d) foi recusado", point[0], point[1])
		}
	}
	for _, point := range [][2]uint16{{96, 200}, {101, 200}, {100, 196}, {100, 201}, {103, 203}} {
		if instanceQuizCorrect(&Player{X: point[0], Y: point[1]}, quiz) {
			t.Errorf("ponto fora da plataforma (%d,%d) foi aceito", point[0], point[1])
		}
	}
}

func TestInstanceRoomsReserveTheirNPCGenerPopulation(t *testing.T) {
	w, _, _, _, _ := instanceTestWorld()
	near := model.NPCGener{}
	near.Segments[0] = model.GenerSegment{X: 2208, Y: 2202}
	if !w.generatorReservedForItemInstance(near) {
		t.Fatal("gerador da sala nao foi reservado")
	}
	far := near
	far.Segments[0] = model.GenerSegment{X: 2211, Y: 2202}
	if w.generatorReservedForItemInstance(far) {
		t.Fatal("gerador fora da sala foi removido")
	}
	// Cube/Big Cube vivem em regras por volatile, nao em override por item.
	// Esses geradores tambem precisam ser reservados para nao duplicar a
	// populacao permanente quando o ticket abrir a instancia.
	w.volatiles.Rules = map[int]model.VolatileRule{54: instanceTestRule()}
	w.volatiles.Items = nil
	if !w.generatorReservedForItemInstance(near) {
		t.Fatal("regra de volatile nao reservou o NPCGener da instancia")
	}
}

func TestStagedInstanceTransitionsAndSpawnsNextRoom(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.ID = "staged"
	rule.Instance.X, rule.Instance.Y = 0, 0
	rule.Instance.SpawnX, rule.Instance.SpawnY = 0, 0
	rule.Instance.AreaRadius = 0
	rule.Instance.Spawns = nil
	rule.Instance.DurationSeconds = 0
	rule.Instance.TransitionSeconds = 10
	rule.Instance.RewardItem = 0
	rule.Instance.Stages = []model.VolatileInstanceStage{
		{Name: "Room 1", X: 2200, Y: 2200, SpawnX: 2202, SpawnY: 2202,
			AreaRadius: 8, DurationSeconds: 60,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}}},
		{Name: "Room 2", X: 2250, Y: 2250, SpawnX: 2252, SpawnY: 2252,
			AreaRadius: 8, DurationSeconds: 90,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 2}}},
	}
	w.volatiles.Items[100] = rule

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["staged"]
	if inst == nil || inst.CurrentStage != 0 || inst.Remaining != 1 {
		t.Fatalf("primeira sala incorreta: %+v", inst)
	}
	var first *Mob
	for id := range inst.MobIDs {
		first = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(first, clock.Now())
	if inst.TransitionAt.IsZero() || !inst.ExitAt.IsZero() {
		t.Fatalf("morte da sala 1 nao iniciou transicao: %+v", inst)
	}

	clock.Advance(10 * time.Second)
	w.tickItemInstances(clock.Now())
	if inst.CurrentStage != 1 || inst.Remaining != 2 || len(inst.MobIDs) != 2 {
		t.Fatalf("segunda sala incorreta: stage=%d remaining=%d mobs=%d",
			inst.CurrentStage, inst.Remaining, len(inst.MobIDs))
	}
	if chebyshev(leader.X, leader.Y, 2250, 2250) > 4 ||
		chebyshev(member.X, member.Y, 2250, 2250) > 4 {
		t.Fatalf("grupo nao foi movido para sala 2: leader=(%d,%d) member=(%d,%d)",
			leader.X, leader.Y, member.X, member.Y)
	}
	if want := clock.Now().Add(90 * time.Second); !inst.Deadline.Equal(want) {
		t.Fatalf("deadline=%v, quer %v", inst.Deadline, want)
	}
}

func TestStagedInstanceTransitionRetriesAtomically(t *testing.T) {
	w, leader, member, st, clock := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.ID = "staged-retry"
	rule.Instance.Spawns = nil
	rule.Instance.X, rule.Instance.Y = 0, 0
	rule.Instance.SpawnX, rule.Instance.SpawnY = 0, 0
	rule.Instance.AreaRadius = 0
	rule.Instance.DurationSeconds = 0
	rule.Instance.TransitionSeconds = 10
	rule.Instance.RewardItem = 0
	rule.Instance.Stages = []model.VolatileInstanceStage{
		{X: 2200, Y: 2200, SpawnX: 2202, SpawnY: 2202, AreaRadius: 8,
			DurationSeconds: 60,
			Spawns:          []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}}},
		{X: 2250, Y: 2250, SpawnX: 2252, SpawnY: 2252, AreaRadius: 8,
			DurationSeconds: 60,
			Spawns:          []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}}},
	}
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["staged-retry"]
	var first *Mob
	for id := range inst.MobIDs {
		first = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(first, clock.Now())
	oldLeaderX, oldLeaderY := leader.X, leader.Y
	oldMemberX, oldMemberY := member.X, member.Y
	clock.Advance(10 * time.Second)

	st.err = errors.New("database unavailable")
	w.tickItemInstances(clock.Now())
	if inst.CurrentStage != 0 || inst.TransitionAt.IsZero() ||
		leader.X != oldLeaderX || leader.Y != oldLeaderY ||
		member.X != oldMemberX || member.Y != oldMemberY {
		t.Fatal("falha de transicao dividiu o grupo ou perdeu o retry")
	}

	st.err = nil
	w.tickItemInstances(clock.Now())
	if inst.CurrentStage != 1 || !inst.TransitionAt.IsZero() ||
		chebyshev(leader.X, leader.Y, 2250, 2250) > 4 ||
		chebyshev(member.X, member.Y, 2250, 2250) > 4 {
		t.Fatal("transicao nao foi repetida atomicamente")
	}
}

func TestInstanceExitRetriesAfterPersistenceFailure(t *testing.T) {
	w, leader, _, st, clock := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.RewardItem = 0
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(mob, clock.Now())
	clock.Advance(10 * time.Second)

	st.err = errors.New("database unavailable")
	w.tickItemInstances(clock.Now())
	if w.itemInstances["water-normal-1"] == nil ||
		chebyshev(leader.X, leader.Y, 2200, 2200) > 4 {
		t.Fatal("falha de saida encerrou a instancia ou moveu o lider")
	}

	st.err = nil
	w.tickItemInstances(clock.Now())
	if w.itemInstances["water-normal-1"] != nil ||
		chebyshev(leader.X, leader.Y, 2100, 2100) > 3 {
		t.Fatal("saida nao foi repetida depois da recuperacao")
	}
}

func TestCubeQuizKeepsCorrectMemberAndRemovesWrongMember(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.ID = "cube-test"
	rule.Instance.Spawns = nil
	rule.Instance.X, rule.Instance.Y = 0, 0
	rule.Instance.SpawnX, rule.Instance.SpawnY = 0, 0
	rule.Instance.AreaRadius = 0
	rule.Instance.TransitionSeconds = 0
	rule.Instance.RewardItem = 0
	rule.Instance.Stages = []model.VolatileInstanceStage{
		{Name: "Question", X: 2200, Y: 2200, SpawnX: 2202, SpawnY: 2202,
			AreaRadius: 8, DurationSeconds: 60,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}},
			Quiz: &model.VolatileInstanceQuiz{
				Question: "True or false?", Answer: true,
				TrueX: 2200, TrueY: 2200, FalseX: 2210, FalseY: 2210,
				DurationSeconds: 14, RewardExp: 8000,
			}},
		{Name: "Next", X: 2250, Y: 2250, SpawnX: 2252, SpawnY: 2252,
			AreaRadius: 8, DurationSeconds: 60,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}}},
	}
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["cube-test"]
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	leader.X, leader.Y = 2200, 2200
	leader.Char.X, leader.Char.Y = leader.X, leader.Y
	member.X, member.Y = 2210, 2210
	member.Char.X, member.Char.Y = member.X, member.Y
	oldExp := leader.Char.Exp
	w.onItemInstanceMobKilled(mob, clock.Now())
	if inst.QuizAt.IsZero() || !inst.TransitionAt.IsZero() {
		t.Fatalf("quiz nao aguardou resposta: %+v", inst)
	}

	clock.Advance(14 * time.Second)
	w.tickItemInstances(clock.Now())
	if len(inst.MemberIDs) != 1 || inst.MemberIDs[0] != leader.ID {
		t.Fatalf("resultado do quiz incorreto: membros=%v", inst.MemberIDs)
	}
	if leader.Char.Exp <= oldExp {
		t.Fatalf("acerto nao concedeu EXP: antes=%d depois=%d", oldExp, leader.Char.Exp)
	}
	if chebyshev(member.X, member.Y, 2100, 2100) > 6 {
		t.Fatalf("membro que errou nao saiu: (%d,%d)", member.X, member.Y)
	}
	if inst.CurrentStage != 1 || inst.Remaining != 1 {
		t.Fatalf("proxima sala nao abriu: stage=%d remaining=%d",
			inst.CurrentStage, inst.Remaining)
	}
}

func TestInstanceHardDeadlineDoesNotResetBetweenStages(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.TotalDurationSeconds = 5
	rule.Instance.RewardItem = 0
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst.HardDeadline.IsZero() {
		t.Fatal("prazo total nao foi armado")
	}
	clock.Advance(5 * time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances["water-normal-1"] != nil {
		t.Fatal("instancia permaneceu aberta depois do prazo absoluto")
	}
}
