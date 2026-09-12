package game

import (
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestRestoreHellGateRuntimeKeepsSelectedLichProgress(t *testing.T) {
	quadrants := make([]model.VolatileHellGateQuadrant, 0, 4)
	for id := 1; id <= 4; id++ {
		x, y := uint16(1800+id*10), uint16(3600+id*10)
		quadrants = append(quadrants, model.VolatileHellGateQuadrant{
			ID: id, X: x, Y: y, SpawnX: x, SpawnY: y, AreaRadius: 4,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "Skeleton", Count: 1, X: x, Y: y}},
			Lich:   model.VolatileInstanceSpawn{NPC: "Lich_Dragon", Count: 1, X: x, Y: y},
		})
	}
	cfg := model.VolatileInstance{
		ID: "hell", StateMachine: "hell_gate", X: 1800, Y: 3600,
		AreaRadius: 50, DurationSeconds: 240,
		Stages: []model.VolatileInstanceStage{{X: 1800, Y: 3600, SpawnX: 1800, SpawnY: 3600,
			AreaRadius: 50, Spawns: []model.VolatileInstanceSpawn{{NPC: "Hell_Spider", Count: 1}}}},
		HellGate: &model.VolatileHellGate{
			ControllerNPC: "Hell_Spider", Quadrants: quadrants,
			FinalSpawns: []model.VolatileInstanceSpawn{{NPC: "Skeleton", Count: 1}},
		},
	}
	w := testSpatialWorld(nil)
	for _, name := range []string{"Hell_Spider", "Skeleton", "Lich_Dragon"} {
		w.npcs = append(w.npcs, model.NPCDef{Name: name, Tipo: model.TipoMonstro,
			Score: testScore(model.Score{MaxHP: 10, CurHP: 10})})
	}
	inst := &ItemInstance{Config: cfg, RuntimeID: cfg.ID, State: "lich_puzzle",
		HellGateVariant: 1, HellGateValidLichMask: hellGateBit(2) | hellGateBit(4),
		MobIDs: make(map[uint16]struct{}), MobQuadrants: make(map[uint16]uint8),
		HellGateLichIDs: make(map[uint8]uint16)}
	saved := model.InstanceRuntimeState{
		State: "lich_puzzle", HellGateVariant: 1,
		HellGateValidLichMask: hellGateBit(2) | hellGateBit(4),
		HellGateLichSpawned:   hellGateBit(2),
		HellGateClearedMask:   hellGateBit(1), HellGateWrongLich: true,
	}
	if !w.restoreHellGateRuntime(inst, saved, time.Unix(2_000_000_000, 0)) {
		t.Fatal("restauracao do estado Hell Gate falhou")
	}
	if inst.State != "lich_puzzle" || inst.HellGateValidLichMask != saved.HellGateValidLichMask ||
		inst.HellGateClearedMask != saved.HellGateClearedMask || !inst.HellGateWrongLich {
		t.Fatalf("estado restaurado incorreto: %+v", inst)
	}
	if inst.HellGateLichSpawnedMask&hellGateBit(2) == 0 ||
		inst.HellGateLichIDs[2] == 0 || inst.HellGateLichSpawnedMask&hellGateBit(1) != 0 {
		t.Fatalf("Lichs restaurados incorretamente: spawned=%02x ids=%v",
			inst.HellGateLichSpawnedMask, inst.HellGateLichIDs)
	}
	if inst.Remaining != 3 {
		t.Fatalf("a restauracao deve manter apenas o Lich 2 e as ondas 3/4: remaining=%d", inst.Remaining)
	}
	for id, quadrant := range inst.MobQuadrants {
		if quadrant == 1 || (quadrant == 2 && id != inst.HellGateLichIDs[2]) {
			t.Fatalf("quadrante ja resolvido foi recriado: mob=%d quadrant=%d", id, quadrant)
		}
	}
}

func TestRestoreHellGateRuntimeRollsBackPartialWave(t *testing.T) {
	quadrants := make([]model.VolatileHellGateQuadrant, 0, 4)
	for id := 1; id <= 4; id++ {
		x, y := uint16(2200+id*10), uint16(2200+id*10)
		npc := "Missing_HellGate_Wave"
		if id == 1 {
			npc = "Hell_Spider"
		}
		quadrants = append(quadrants, model.VolatileHellGateQuadrant{
			ID: id, X: x, Y: y, SpawnX: x, SpawnY: y, AreaRadius: 4,
			Spawns: []model.VolatileInstanceSpawn{{NPC: npc, Count: 1, X: x, Y: y}},
			Lich:   model.VolatileInstanceSpawn{NPC: "Lich_Dragon", Count: 1, X: x, Y: y},
		})
	}
	cfg := model.VolatileInstance{
		ID: "hell-partial", StateMachine: "hell_gate", X: 2200, Y: 2200,
		AreaRadius: 50, DurationSeconds: 240,
		HellGate: &model.VolatileHellGate{
			ControllerNPC: "Hell_Spider", Quadrants: quadrants,
		},
	}
	w := testSpatialWorld(nil)
	for _, name := range []string{"Hell_Spider", "Lich_Dragon"} {
		w.npcs = append(w.npcs, model.NPCDef{Name: name, Tipo: model.TipoMonstro,
			Score: testScore(model.Score{MaxHP: 10, CurHP: 10})})
	}
	inst := &ItemInstance{
		Config: cfg, RuntimeID: cfg.ID, State: "lich_puzzle",
		MobIDs: make(map[uint16]struct{}), MobQuadrants: make(map[uint16]uint8),
		HellGateLichIDs: make(map[uint8]uint16),
	}
	saved := model.InstanceRuntimeState{State: "lich_puzzle"}
	if w.restoreHellGateRuntime(inst, saved, time.Unix(2_000_000_000, 0)) {
		t.Fatal("restauracao deveria falhar quando um quadrante nao possui template")
	}
	if len(w.mobs) != 0 || len(w.mobsByID) != 0 {
		t.Fatalf("ondas parcialmente materializadas permaneceram no mundo: mobs=%d index=%d", len(w.mobs), len(w.mobsByID))
	}
	if len(inst.MobIDs) != 0 || len(inst.MobQuadrants) != 0 || len(inst.HellGateLichIDs) != 0 || inst.Remaining != 0 {
		t.Fatalf("estado runtime nao foi limpo apos rollback: %+v", inst)
	}
}

func TestInstanceConfigLookupPrefersExactIDOverSharedRuntimeAlias(t *testing.T) {
	w := &World{volatiles: model.VolatileCatalog{Instances: map[string]model.VolatileInstance{
		"first":  {ID: "first", SharedGroup: "nightmare", Name: "wrong"},
		"second": {ID: "second", SharedGroup: "nightmare", Name: "expected"},
	}}}
	for i := 0; i < 100; i++ {
		got, ok := w.instanceConfigByID("second", "shared:nightmare")
		if !ok || got.ID != "second" || got.Name != "expected" {
			t.Fatalf("lookup escolheu alias incorreto: %+v/%v", got, ok)
		}
	}
}

func TestInstanceStateSnapshotIsDeterministicAndFiltersNonDurableRuns(t *testing.T) {
	now := time.Unix(2_000_000_000, 0)
	w := &World{
		clock:              newFakeClock(now),
		nightmarePartyRuns: map[string]int{"kept": 2, "zero": 0},
		itemInstances: map[string]*ItemInstance{
			"z-runtime": {
				RuntimeID:    "z-runtime",
				Config:       model.VolatileInstance{ID: "z", Mode: "shared_timed_zone"},
				HardDeadline: now.Add(time.Minute),
			},
			"a-runtime": {
				RuntimeID:    "a-runtime",
				Config:       model.VolatileInstance{ID: "a", Mode: "shared_timed_zone"},
				HardDeadline: now.Add(time.Minute),
			},
			"private": {
				RuntimeID: "private",
				Config:    model.VolatileInstance{ID: "private", Mode: "private_chain"},
			},
			"expired": {
				RuntimeID:    "expired",
				Config:       model.VolatileInstance{ID: "expired", Mode: "shared_timed_zone"},
				HardDeadline: now,
			},
		},
	}
	snapshot := w.instanceStateSnapshot()
	if snapshot.Version != model.InstanceStateVersion || snapshot.NightmarePartyRuns["kept"] != 2 {
		t.Fatalf("agregado incompleto: %+v", snapshot)
	}
	if _, exists := snapshot.NightmarePartyRuns["zero"]; exists {
		t.Fatal("contador zerado foi persistido")
	}
	if len(snapshot.Instances) != 2 || snapshot.Instances[0].RuntimeID != "a-runtime" ||
		snapshot.Instances[1].RuntimeID != "z-runtime" {
		t.Fatalf("instancias nao foram filtradas/ordenadas: %+v", snapshot.Instances)
	}
}

func TestPrivateWaterSnapshotPersistsCharacterUIDAndExitState(t *testing.T) {
	now := time.Unix(2_100_000_000, 0)
	player, _ := networkedTestPlayer(1, "WaterMember", 2100, 2100)
	player.Char.UID = "11111111111141118111111111111111"
	w := worldWithNetworkedPlayers(player)
	w.clock = newFakeClock(now)
	w.itemInstances = map[string]*ItemInstance{
		"water-normal-8": {
			RuntimeID: "water-normal-8",
			Config:    model.VolatileInstance{ID: "water-normal-8"},
			LeaderID:  player.ID, MemberIDs: []uint16{player.ID},
			RewardGranted: true, ExitAt: now.Add(10 * time.Second),
			HardDeadline: now.Add(time.Minute),
		},
	}
	snapshot := w.instanceStateSnapshot()
	if len(snapshot.Instances) != 1 {
		t.Fatalf("Water privada nao foi persistida: %+v", snapshot)
	}
	saved := snapshot.Instances[0]
	if len(saved.MemberCharacterUIDs) != 1 || saved.MemberCharacterUIDs[0] != player.Char.UID ||
		saved.LeaderCharacterUID != player.Char.UID || !saved.RewardGranted ||
		!saved.ExitAt.Equal(now.Add(10*time.Second)) {
		t.Fatalf("snapshot Water incompleto: %+v", saved)
	}
}

func TestAttachRestoredWaterMemberUsesUIDAndSavedStage(t *testing.T) {
	w, player, _, _, clock := instanceTestWorld()
	player.Char.UID = "22222222222242228222222222222222"
	player.ID = 1
	player.X, player.Y = 2100, 2100
	w.playersByID[player.ID] = player
	w.pendingInstanceMembers = map[string]map[string]struct{}{
		"water-normal-1": {player.Char.UID: {}},
	}
	w.pendingInstanceLeaders = map[string]string{"water-normal-1": player.Char.UID}
	w.itemInstances = map[string]*ItemInstance{
		"water-normal-1": {
			RuntimeID: "water-normal-1", Config: model.VolatileInstance{
				ID: "water-normal-1", SpawnX: 2202, SpawnY: 2202,
				AreaRadius: 8, Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}},
			}, CurrentStage: 0,
		},
	}
	// The helper only needs the authoritative stage coordinates; no mob spawn
	// is required to prove that a reconnect is not placed at 2100,2100.
	w.attachRestoredInstanceMember(player)
	if !itemInstanceHasMember(w.itemInstances["water-normal-1"], player.ID) ||
		player.X == 2100 && player.Y == 2100 ||
		w.itemInstances["water-normal-1"].LeaderID != player.ID {
		t.Fatalf("reconexao nao reatachou a sala: player=(%d,%d) inst=%+v",
			player.X, player.Y, w.itemInstances["water-normal-1"])
	}
	if _, ok := w.pendingInstanceMembers["water-normal-1"]; ok {
		t.Fatal("UID reconectado permaneceu pendente")
	}
	_ = clock
}

type nonAtomicInstanceStore struct{}

func (nonAtomicInstanceStore) LoadAccount(string) (*model.Account, error) { return nil, nil }
func (nonAtomicInstanceStore) SaveAccount(*model.Account) error           { return nil }
func (nonAtomicInstanceStore) CharacterNameExists(string) (bool, error)   { return false, nil }
func (nonAtomicInstanceStore) LoadInstanceState() (*model.InstanceStateSnapshot, error) {
	return &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}, nil
}
func (nonAtomicInstanceStore) SaveInstanceState(*model.InstanceStateSnapshot) error { return nil }

func TestSaveAccountsAndInstanceStateFailsClosedWithoutAtomicStore(t *testing.T) {
	w := &World{
		store: nonAtomicInstanceStore{},
		clock: newFakeClock(time.Unix(2_100_000_000, 0)),
		itemInstances: map[string]*ItemInstance{
			"water-normal-1": {RuntimeID: "water-normal-1", Config: model.VolatileInstance{ID: "water-normal-1"}},
		},
	}
	if err := w.saveAccountsAndInstanceState(&model.Account{Name: "account"}); err == nil {
		t.Fatal("store sem transacao aceitou commit misto conta/instancia")
	}
}
