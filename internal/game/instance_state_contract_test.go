package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

type recordingInstanceStore struct {
	loaded *model.InstanceStateSnapshot
	loadErr error
	saveErr error
	saved *model.InstanceStateSnapshot
}

func (s *recordingInstanceStore) LoadAccount(string) (*model.Account, error) { return nil, nil }
func (s *recordingInstanceStore) SaveAccount(*model.Account) error { return nil }
func (s *recordingInstanceStore) CharacterNameExists(string) (bool, error) { return false, nil }
func (s *recordingInstanceStore) LoadInstanceState() (*model.InstanceStateSnapshot, error) {
	return s.loaded, s.loadErr
}
func (s *recordingInstanceStore) SaveInstanceState(snapshot *model.InstanceStateSnapshot) error {
	s.saved = snapshot
	return s.saveErr
}

func TestResumableInstanceConfigContracts(t *testing.T) {
	water := model.VolatileInstance{ID: " water-normal-1 "}
	shared := model.VolatileInstance{ID: "nightmare", Mode: "shared_timed_zone"}
	hell := model.VolatileInstance{ID: "hell", StateMachine: " HELL_GATE "}
	private := model.VolatileInstance{ID: "cube", Mode: "private_chain"}

	if resumableInstance(nil) || isDurablePrivateWaterInstance(nil) {
		t.Fatal("nil instance foi marcada resumable")
	}
	for _, cfg := range []model.VolatileInstance{water, shared, hell} {
		inst := &ItemInstance{Config: cfg}
		if !resumableInstance(inst) || !resumableConfigOrWater(cfg) {
			t.Fatalf("config resumable foi recusada: %+v", cfg)
		}
	}
	if resumableInstance(&ItemInstance{Config: private}) || resumableConfigOrWater(private) {
		t.Fatal("private chain comum foi marcada resumable")
	}
	if !isDurablePrivateWaterConfig(water) || isDurablePrivateWaterConfig(shared) || isDurablePrivateWaterConfig(hell) {
		t.Fatal("classificacao durable Water divergiu")
	}
}

func TestAppendUniqueStringTrimsSkipsAndDeduplicates(t *testing.T) {
	values := []string{"a"}
	values = appendUniqueString(values, " ")
	values = appendUniqueString(values, " a ")
	values = appendUniqueString(values, " b ")
	if len(values) != 2 || values[0] != "a" || values[1] != "b" {
		t.Fatalf("appendUniqueString=%v", values)
	}
}

func TestInstanceCharacterUIDsMergesDurableLiveAndPendingMembers(t *testing.T) {
	leaderUID := "11111111111141118111111111111111"
	memberUID := "22222222222242228222222222222222"
	pendingUID := "33333333333343338333333333333333"
	p1 := &Player{ID: 1, Char: &model.Char{UID: leaderUID}}
	p2 := &Player{ID: 2, Char: &model.Char{UID: memberUID}}
	w := &World{
		playersByID: map[uint16]*Player{1: p1, 2: p2},
		pendingInstanceMembers: map[string]map[string]struct{}{"runtime": {pendingUID: {}}},
		pendingInstanceLeaders: map[string]string{"runtime": leaderUID},
	}
	inst := &ItemInstance{
		RuntimeID: "runtime", LeaderID: 1, MemberIDs: []uint16{1, 2, 99},
		MemberCharacterUIDs: []string{memberUID, " "}, LeaderCharacterUID: memberUID,
	}
	uids, leader := w.instanceCharacterUIDs("runtime", inst)
	if leader != leaderUID || len(uids) != 3 || uids[0] != leaderUID || uids[1] != memberUID || uids[2] != pendingUID {
		t.Fatalf("uids=%v leader=%q", uids, leader)
	}
	if uids, leader := w.instanceCharacterUIDs("runtime", nil); uids != nil || leader != "" {
		t.Fatalf("nil instance => %v/%q", uids, leader)
	}
}

func TestStageSpawnPositionCoversFallbackBoundsAndMissingCoordinates(t *testing.T) {
	w := &World{}
	if _, _, _, ok := w.stageSpawnPosition(nil); ok {
		t.Fatal("nil instance retornou spawn")
	}
	inst := &ItemInstance{Config: model.VolatileInstance{Stages: []model.VolatileInstanceStage{{X: 10, Y: 20, AreaRadius: 3}}}, CurrentStage: -1}
	if _, _, _, ok := w.stageSpawnPosition(inst); ok {
		t.Fatal("stage negativa retornou spawn")
	}
	inst.CurrentStage = 1
	if _, _, _, ok := w.stageSpawnPosition(inst); ok {
		t.Fatal("stage fora do limite retornou spawn")
	}
	inst.CurrentStage = 0
	x, y, radius, ok := w.stageSpawnPosition(inst)
	if !ok || x != 10 || y != 20 || radius != 3 {
		t.Fatalf("fallback stage=(%d,%d r=%d ok=%v)", x, y, radius, ok)
	}
	inst.Config.Stages[0].SpawnX, inst.Config.Stages[0].SpawnY = 30, 40
	x, y, _, ok = w.stageSpawnPosition(inst)
	if !ok || x != 30 || y != 40 {
		t.Fatalf("spawn explicito=(%d,%d ok=%v)", x, y, ok)
	}
	inst.Config.Stages[0] = model.VolatileInstanceStage{}
	if _, _, _, ok := w.stageSpawnPosition(inst); ok {
		t.Fatal("stage sem coordenadas retornou spawn valido")
	}
}

func TestInstanceConfigLookupCoversEveryCatalogSourceAndRuntimeFallback(t *testing.T) {
	mk := func(id, group, name string) model.VolatileInstance {
		return model.VolatileInstance{ID: id, SharedGroup: group, Name: name}
	}
	w := &World{volatiles: model.VolatileCatalog{
		Instances: map[string]model.VolatileInstance{"named": mk("named-id", "g1", "instances")},
		Items: map[uint16]model.VolatileRule{1: {Instance: func() *model.VolatileInstance { v := mk("item-id", "g2", "items"); return &v }()}},
		Rules: map[int]model.VolatileRule{2: {Instance: func() *model.VolatileInstance { v := mk("rule-id", "g3", "rules"); return &v }()}},
	}}
	for _, tc := range []struct{ configID, runtimeID, want string }{
		{"named-id", "", "instances"},
		{"item-id", "", "items"},
		{"rule-id", "", "rules"},
		{"missing", "shared:g1", "instances"},
		{"missing", "shared:g2", "items"},
		{"missing", "shared:g3", "rules"},
	} {
		got, ok := w.instanceConfigByID(tc.configID, tc.runtimeID)
		if !ok || got.Name != tc.want {
			t.Fatalf("lookup config=%q runtime=%q => %+v/%v", tc.configID, tc.runtimeID, got, ok)
		}
	}
	if _, ok := w.instanceConfigByID("missing", ""); ok {
		t.Fatal("lookup inexistente sem runtime foi aceito")
	}
	if _, ok := w.instanceConfigByID("missing", "shared:missing"); ok {
		t.Fatal("lookup inexistente por runtime foi aceito")
	}
}

func TestPersistAndDirtyInstanceStateContracts(t *testing.T) {
	plain := &World{}
	if err := plain.persistInstanceState(); err != nil {
		t.Fatalf("store sem interface de instancia deveria ser no-op: %v", err)
	}
	plain.markInstanceStateDirty()
	plain.flushInstanceStateIfDirty()
	if plain.instanceStateDirty {
		t.Fatal("store sem interface deveria limpar dirty apos no-op persistido")
	}
	var nilWorld *World
	nilWorld.markInstanceStateDirty()
	nilWorld.flushInstanceStateIfDirty()

	st := &recordingInstanceStore{}
	w := &World{store: st, nightmarePartyRuns: map[string]int{"window": 2}, itemInstances: map[string]*ItemInstance{}}
	w.markInstanceStateDirty()
	w.flushInstanceStateIfDirty()
	if w.instanceStateDirty || st.saved == nil || st.saved.NightmarePartyRuns["window"] != 2 {
		t.Fatalf("persistencia dirty falhou: dirty=%v saved=%+v", w.instanceStateDirty, st.saved)
	}

	st.saveErr = errors.New("disk")
	w.markInstanceStateDirty()
	w.flushInstanceStateIfDirty()
	if !w.instanceStateDirty {
		t.Fatal("falha de persistencia limpou dirty")
	}
}

func TestRestoreInstanceStateCoversNoStoreErrorsVersionsSkipsAndSharedRuntime(t *testing.T) {
	if err := (&World{}).restoreInstanceState(); err != nil {
		t.Fatalf("store sem interface deveria ser no-op: %v", err)
	}
	st := &recordingInstanceStore{loadErr: errors.New("load")}
	if err := (&World{store: st}).restoreInstanceState(); err == nil {
		t.Fatal("erro de load foi engolido")
	}
	for _, snapshot := range []*model.InstanceStateSnapshot{nil, {Version: model.InstanceStateVersion + 1}} {
		st := &recordingInstanceStore{loaded: snapshot}
		w := &World{store: st}
		if err := w.restoreInstanceState(); err != nil {
			t.Fatalf("snapshot ignoravel retornou erro: %v", err)
		}
	}

	now := time.Unix(2_200_000_000, 0)
	cfg := model.VolatileInstance{
		ID: "nightmare-normal", Mode: "shared_timed_zone", SharedGroup: "nightmare-normal",
		Stages: []model.VolatileInstanceStage{{X: 100, Y: 100, SpawnX: 100, SpawnY: 100, AreaRadius: 4}},
	}
	snapshot := &model.InstanceStateSnapshot{
		Version: model.InstanceStateVersion,
		NightmarePartyRuns: map[string]int{"keep": 2, "zero": 0},
		Instances: []model.InstanceRuntimeState{
			{RuntimeID: "", ConfigID: cfg.ID},
			{RuntimeID: "expired", ConfigID: cfg.ID, HardDeadline: now},
			{RuntimeID: "missing-config", ConfigID: "nope", HardDeadline: now.Add(time.Minute)},
			{RuntimeID: "shared:nightmare-normal", ConfigID: cfg.ID, State: "entry", CurrentStage: -1, HardDeadline: now.Add(time.Minute)},
		},
	}
	st = &recordingInstanceStore{loaded: snapshot}
	w := &World{
		store: st, clock: newFakeClock(now),
		nightmarePartyRuns: make(map[string]int), itemInstances: make(map[string]*ItemInstance),
		pendingInstanceMembers: make(map[string]map[string]struct{}), pendingInstanceLeaders: make(map[string]string),
		playerInstance: make(map[uint16]string), mobsByID: make(map[uint16]*Mob), mobListIndex: make(map[uint16]int),
		mobCells: make(map[uint32]map[uint16]*Mob), mobCell: make(map[uint16]uint32),
		volatiles: model.VolatileCatalog{Instances: map[string]model.VolatileInstance{cfg.ID: cfg}},
	}
	if err := w.restoreInstanceState(); err != nil {
		t.Fatal(err)
	}
	if w.nightmarePartyRuns["keep"] != 2 {
		t.Fatalf("contador Nightmare nao restaurado: %+v", w.nightmarePartyRuns)
	}
	if _, exists := w.nightmarePartyRuns["zero"]; exists {
		t.Fatal("contador zero foi restaurado")
	}
	inst := w.itemInstances["shared:nightmare-normal"]
	if inst == nil || inst.CurrentStage != 0 || inst.RuntimeID != "shared:nightmare-normal" {
		t.Fatalf("shared runtime nao restaurado: %+v", inst)
	}
	if len(w.itemInstances) != 1 {
		t.Fatalf("snapshots invalidos nao foram filtrados: %+v", w.itemInstances)
	}
}
