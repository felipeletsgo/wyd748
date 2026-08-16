package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestInstanceScheduleHandlesNoScheduleInvalidWindowsCrossHourAndEarliestEnd(t *testing.T) {
	now := time.Date(2026, 8, 16, 12, 0, 30, 0, time.UTC)
	if end, ok := instanceScheduleEnd(nil, now); !ok || !end.IsZero() {
		t.Fatalf("nil schedule=%v/%v", end, ok)
	}
	cfg := &model.VolatileInstance{}
	if end, ok := instanceScheduleEnd(cfg, now); !ok || !end.IsZero() {
		t.Fatalf("empty schedule=%v/%v", end, ok)
	}
	cfg.Schedule = []model.VolatileInstanceWindow{{StartMinute: -1, DurationSeconds: 10}, {StartMinute: 60, DurationSeconds: 10}, {StartMinute: 0, DurationSeconds: 0}}
	if _, ok := instanceScheduleEnd(cfg, now); ok {
		t.Fatal("somente janelas invalidas abriram schedule")
	}

	// Janela iniciada 11:59 dura quatro minutos e permanece aberta depois da virada.
	cfg.Schedule = []model.VolatileInstanceWindow{{StartMinute: 59, DurationSeconds: 240}}
	end, ok := instanceScheduleEnd(cfg, now)
	if !ok || !end.Equal(time.Date(2026, 8, 16, 12, 3, 0, 0, time.UTC)) {
		t.Fatalf("cross-hour end=%v ok=%v", end, ok)
	}

	now = time.Date(2026, 8, 16, 12, 20, 10, 0, time.UTC)
	cfg.Schedule = []model.VolatileInstanceWindow{{StartMinute: 20, DurationSeconds: 240}, {StartMinute: 20, DurationSeconds: 60}}
	end, ok = instanceScheduleEnd(cfg, now)
	if !ok || !end.Equal(time.Date(2026, 8, 16, 12, 21, 0, 0, time.UTC)) {
		t.Fatalf("earliest end=%v ok=%v", end, ok)
	}
	cfg.Schedule = []model.VolatileInstanceWindow{{StartMinute: 10, DurationSeconds: 30}}
	if _, ok := instanceScheduleEnd(cfg, now); ok {
		t.Fatal("janela encerrada foi aceita")
	}
}

func TestInstanceEntryAreaAndTargetValidationCoversLegacyZeroOutsideAndInside(t *testing.T) {
	if area, ok := instanceEntryAreaAt(nil, 0, 0); !ok || area != (model.VolatileInstanceEntryArea{}) {
		t.Fatalf("legacy nil area=%+v ok=%v", area, ok)
	}
	cfg := &model.VolatileInstance{EntryAreas: []model.VolatileInstanceEntryArea{{MinX: 10, MaxX: 20, MinY: 30, MaxY: 40}}}
	if _, ok := instanceEntryAreaAt(cfg, 0, 35); ok || instanceTargetAllowedAt(cfg, 0, 35) {
		t.Fatal("coordenada zero foi aceita")
	}
	if _, ok := instanceEntryAreaAt(cfg, 9, 35); ok || instanceTargetAllowedAt(cfg, 9, 35) {
		t.Fatal("coordenada externa foi aceita")
	}
	area, ok := instanceEntryAreaAt(cfg, 10, 40)
	if !ok || area.MinX != 10 || !instanceTargetAllowedAt(cfg, 10, 40) {
		t.Fatalf("borda inclusiva nao foi aceita: %+v/%v", area, ok)
	}
}

func TestInstanceAreaOccupiedRespectsGameplaySpaceAndEntryIgnoredMembers(t *testing.T) {
	cfg := &model.VolatileInstance{Stages: []model.VolatileInstanceStage{{X: 100, Y: 100, AreaRadius: 4}}}
	inside := instanceContractPlayer(1, "u1", 102, 100)
	outside := instanceContractPlayer(2, "u2", 200, 200)
	otherSpace := instanceContractPlayer(3, "u3", 101, 100)
	w := &World{
		playersByID: map[uint16]*Player{1: inside, 2: outside, 3: otherSpace},
		playerInstance: map[uint16]string{3: "other"},
	}
	if !w.instanceAreaOccupied(cfg, "") {
		t.Fatal("jogador no mesmo space nao bloqueou area")
	}
	if !w.instanceAreaOccupied(cfg, "other", otherSpace) {
		t.Fatal("room legacy sem EntryAreas deve continuar ocupada pelo membro ignorado")
	}

	cfg.EntryAreas = []model.VolatileInstanceEntryArea{{MinX: 95, MaxX: 105, MinY: 95, MaxY: 105}}
	if w.instanceAreaOccupied(cfg, "other", otherSpace) {
		t.Fatal("membro entrando em EntryArea deveria ser ignorado")
	}
	if w.instanceAreaOccupied(nil, "") != true {
		t.Fatal("config nil deveria falhar fechado")
	}
	inside.InWorld = false
	if w.instanceAreaOccupied(cfg, "") {
		t.Fatal("jogador fora do mundo bloqueou area")
	}
}

func TestPlanInstancePositionsAllocatesDistinctTilesAndRejectsFullyBlockedArea(t *testing.T) {
	w := testSpatialWorld(nil)
	members := []*Player{instanceContractPlayer(1, "u1", 0, 0), instanceContractPlayer(2, "u2", 0, 0), instanceContractPlayer(3, "u3", 0, 0)}
	positions, ok := w.planInstancePositionsIgnoringForInstance(members, 100, 100, nil, "room")
	if !ok || len(positions) != 3 || positions[0] == positions[1] || positions[1] == positions[2] {
		t.Fatalf("positions=%v ok=%v", positions, ok)
	}

	blocked := testSpatialWorld(nil)
	for y := uint16(1); y <= 5; y++ {
		for x := uint16(1); x <= 5; x++ {
			m := &Mob{ID: blocked.allocMobID(), X: x, Y: y, Def: &model.NPCDef{Name: "Block", Tipo: model.TipoNPC, Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion}}}
			blocked.appendMobInstance(m)
			blocked.registerMobSpatial(m)
		}
	}
	if _, ok := blocked.planInstancePositionsIgnoringForInstance([]*Player{members[0]}, 1, 1, nil, ""); ok {
		t.Fatal("area totalmente bloqueada retornou vaga")
	}
}

func TestItemInstanceLookupExitGraceAndChainAdmissionContracts(t *testing.T) {
	now := time.Unix(2_600_000_000, 0)
	active := &ItemInstance{RuntimeID: "active", Config: model.VolatileInstance{ID: "active"}, MemberIDs: []uint16{1}}
	graceOld := &ItemInstance{RuntimeID: "grace-a", Config: model.VolatileInstance{ID: "water-normal-1", AllowChainDuringExitGrace: true, RewardItem: 20}, MemberIDs: []uint16{1}, RewardGranted: true, ExitAt: now.Add(5 * time.Second)}
	graceNew := &ItemInstance{RuntimeID: "grace-b", Config: model.VolatileInstance{ID: "water-normal-2", AllowChainDuringExitGrace: true, ChainNextItem: 30}, MemberIDs: []uint16{1}, RewardGranted: true, ExitAt: now.Add(8 * time.Second)}
	w := &World{
		clock: newFakeClock(now),
		itemInstances: map[string]*ItemInstance{"active": active, "grace-a": graceOld, "grace-b": graceNew},
		playerInstance: map[uint16]string{1: "active"},
	}
	if w.itemInstanceForPlayer(0) != nil || w.itemInstanceForPlayer(1) != active {
		t.Fatal("lookup pelo indice ativo falhou")
	}
	delete(w.playerInstance, 1)
	if w.itemInstanceForPlayer(1) != active || w.playerInstance[1] != "active" {
		t.Fatal("repair scan nao reconstruiu indice")
	}
	delete(w.itemInstances, "active")
	w.clearPlayerInstanceIndex(1, "active")
	if got := w.itemInstanceExitGraceForPlayer(1); got != graceNew {
		t.Fatalf("latest grace=%p want=%p", got, graceNew)
	}
	if !w.itemInstanceExitGraceAllowsItem(1, 30) || w.itemInstanceExitGraceAllowsItem(1, 20) {
		t.Fatal("exit grace aceitou ticket fora da cadeia")
	}

	w.itemInstances = map[string]*ItemInstance{}
	root := model.VolatileInstance{ID: "water-normal-1", ChainStart: true}
	later := model.VolatileInstance{ID: "water-normal-2", ChainStart: false}
	w.volatiles = model.VolatileCatalog{
		ItemCodes: map[uint16]int{10: 1, 20: 2},
		Items: map[uint16]model.VolatileRule{10: {Instance: &root}, 20: {Instance: &later}},
	}
	if !w.itemInstanceExitGraceAllowsItem(1, 10) || w.itemInstanceExitGraceAllowsItem(1, 20) || !w.itemInstanceExitGraceAllowsItem(1, 99) {
		t.Fatal("chain root/data admission divergente")
	}
	w.itemInstances["stale"] = &ItemInstance{MemberIDs: []uint16{1}}
	if w.itemInstanceExitGraceAllowsItem(1, 99) {
		t.Fatal("stale membership permitiu ticket no gap de cleanup")
	}
}

func TestDetachItemInstanceMemberPreservesOrAbandonsDurableIdentity(t *testing.T) {
	now := time.Unix(2_600_000_000, 0)
	uid := "11111111111141118111111111111111"
	p := instanceContractPlayer(1, uid, 100, 100)
	makeWorld := func() (*World, *ItemInstance) {
		inst := &ItemInstance{
			RuntimeID: "water-normal-1", Config: model.VolatileInstance{ID: "water-normal-1"},
			LeaderID: 1, MemberIDs: []uint16{1}, MemberCharacterUIDs: []string{uid}, LeaderCharacterUID: uid,
		}
		w := &World{
			playersByID: map[uint16]*Player{1: p}, itemInstances: map[string]*ItemInstance{inst.RuntimeID: inst},
			playerInstance: map[uint16]string{1: inst.RuntimeID},
			pendingInstanceMembers: make(map[string]map[string]struct{}), pendingInstanceLeaders: make(map[string]string),
		}
		return w, inst
	}

	w, inst := makeWorld()
	w.detachPlayerFromItemInstances(0, now)
	w.detachPlayerFromItemInstances(1, now)
	if len(inst.MemberIDs) != 0 || inst.LeaderID != 0 || w.pendingInstanceMembers[inst.RuntimeID] == nil {
		t.Fatalf("logout resumable nao preservou UID: inst=%+v pending=%v", inst, w.pendingInstanceMembers)
	}
	if _, ok := w.pendingInstanceMembers[inst.RuntimeID][uid]; !ok || w.pendingInstanceLeaders[inst.RuntimeID] != uid || !w.instanceStateDirty {
		t.Fatalf("UID/leader pendente ausente: %v/%v", w.pendingInstanceMembers, w.pendingInstanceLeaders)
	}

	w, inst = makeWorld()
	w.pendingInstanceMembers[inst.RuntimeID] = map[string]struct{}{uid: {}}
	w.pendingInstanceLeaders[inst.RuntimeID] = uid
	w.detachPlayerFromItemInstancesMode(1, now, false)
	if len(inst.MemberCharacterUIDs) != 0 || inst.LeaderCharacterUID != "" || w.pendingInstanceMembers[inst.RuntimeID] != nil ||
		w.pendingInstanceLeaders[inst.RuntimeID] != "" || !instanceCombatDeadline(inst).Equal(now) {
		t.Fatalf("abandono nao limpou identidade/deadline: inst=%+v pending=%v leaders=%v", inst, w.pendingInstanceMembers, w.pendingInstanceLeaders)
	}
}

func TestRemainingInstanceSecondsRoundsPositiveSubsecondAndExpired(t *testing.T) {
	now := time.Unix(2_600_000_000, 0)
	if remainingInstanceSeconds(time.Time{}, now) != 0 || remainingInstanceSeconds(now, now) != 0 ||
		remainingInstanceSeconds(now.Add(-time.Second), now) != 0 {
		t.Fatal("deadline vazio/expirado nao retornou zero")
	}
	if remainingInstanceSeconds(now.Add(500*time.Millisecond), now) != 1 || remainingInstanceSeconds(now.Add(5*time.Second), now) != 5 {
		t.Fatal("arredondamento de remaining seconds divergente")
	}
}
