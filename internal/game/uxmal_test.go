package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func uxmalTestConfig() model.VolatileInstance {
	positions := make([][]model.VolatileDestination, 7)
	runes := make([][]uint16, 7)
	maxParties := []int{2, 3, 3, 3, 3, 3, 3}
	nextSanc := []int{1, 2, 3, 4, 5, 6, 6}
	stages := make([]model.VolatileInstanceStage, 7)
	for room := 0; room < 7; room++ {
		baseX := uint16(3350 + room*10)
		baseY := uint16(1622 + room*10)
		positions[room] = []model.VolatileDestination{
			{X: baseX, Y: baseY},
			{X: baseX + 8, Y: baseY},
			{X: baseX + 16, Y: baseY},
		}
		runes[room] = []uint16{7001}
		stages[room] = model.VolatileInstanceStage{
			Name:            "Pista",
			X:               baseX,
			Y:               baseY,
			SpawnX:          baseX,
			SpawnY:          baseY,
			AreaRadius:      8,
			DurationSeconds: 900,
			Spawns:          []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}},
		}
	}
	return model.VolatileInstance{
		ID: "uxmal", Name: "Pista de Runas", StateMachine: "uxmal",
		Mode: "state_machine", MaxPlayers: 13, DurationSeconds: 900,
		ExitX: 2100, ExitY: 2100,
		Schedule: []model.VolatileInstanceWindow{{StartMinute: 16, DurationSeconds: 240}},
		Stages:   stages,
		Uxmal: &model.VolatileUxmal{
			NPC: "Uxmal", TicketItem: 5134,
			EntryAreas:    []model.VolatileInstanceEntryArea{{MinX: 3275, MinY: 1674, MaxX: 3317, MaxY: 1711}},
			RoomPositions: positions, MaxParties: maxParties, Runes: runes,
			TicketNextSanc: nextSanc,
		},
	}
}

func uxmalTestWorld() (*World, *Player, *Player, *guildFlowStore, *fakeClock) {
	leader, _ := networkedTestPlayer(1, "Leader", 3280, 1680)
	member, _ := networkedTestPlayer(2, "Member", 3281, 1680)
	party := &Party{Members: []*Player{leader, member}}
	leader.Party, member.Party = party, party
	w, store := guildFlowWorld(leader, member)
	clock := newFakeClock(time.Date(2026, 1, 1, 12, 16, 0, 0, time.UTC))
	w.clock = clock
	w.itemInstances = make(map[string]*ItemInstance)
	w.uxmal = func() *model.VolatileInstance {
		cfg := uxmalTestConfig()
		return &cfg
	}()
	w.npcs = []model.NPCDef{
		{Name: "RoomMob", Tipo: model.TipoMonstro,
			Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100}},
		{Name: "Uxmal", Tipo: model.TipoNPC,
			Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion}},
	}
	w.items = map[uint16]model.ItemDef{
		5134: {Index: 5134}, 7001: {Index: 7001},
	}
	leader.Char.Inv[0] = model.Item{Index: 5134}
	w.updatePlayerSpatial(leader)
	w.updatePlayerSpatial(member)
	return w, leader, member, store, clock
}

func TestUxmalScheduleWindowsAndEntryValidation(t *testing.T) {
	w, leader, _, _, clock := uxmalTestWorld()
	if !uxmalScheduleAllowed(w.uxmal, clock.Now()) {
		t.Fatal("janela das 16h deveria aceitar entrada")
	}
	clock.Advance(4 * time.Minute)
	if uxmalScheduleAllowed(w.uxmal, clock.Now()) {
		t.Fatal("janela deveria fechar no minuto 20")
	}
	leader.X, leader.Y = 2100, 2100
	if uxmalInsideEntry(w.uxmal, leader.X, leader.Y) {
		t.Fatal("coordenada fora da caixa foi aceita")
	}
}

func TestValidateUxmalConfigAcceptsCompleteAuthoritativeData(t *testing.T) {
	w, _, _, _, _ := uxmalTestWorld()
	if err := w.validateUxmalConfig(); err != nil {
		t.Fatalf("configuracao Uxmal valida foi recusada: %v", err)
	}
}

func TestValidateUxmalConfigRejectsBrokenReferences(t *testing.T) {
	tests := []struct {
		name        string
		breakConfig func(*World)
	}{
		{"entry npc missing", func(w *World) { w.uxmal.Uxmal.NPC = "Missing" }},
		{"ticket missing", func(w *World) { delete(w.items, w.uxmal.Uxmal.TicketItem) }},
		{"entry area inverted", func(w *World) {
			w.uxmal.Uxmal.EntryAreas[0].MinX = w.uxmal.Uxmal.EntryAreas[0].MaxX + 1
		}},
		{"rune missing", func(w *World) { delete(w.items, uint16(7001)) }},
		{"next clue sanc invalid", func(w *World) { w.uxmal.Uxmal.TicketNextSanc[3] = 7 }},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			w, _, _, _, _ := uxmalTestWorld()
			test.breakConfig(w)
			if err := w.validateUxmalConfig(); err == nil {
				t.Fatal("configuracao quebrada foi aceita")
			}
		})
	}
}

func TestUxmalStartsPartySpawnsAndChainsClue(t *testing.T) {
	w, leader, member, store, _ := uxmalTestWorld()
	w.startUxmal(leader.Session, leader)
	inst := w.itemInstances["uxmal:0:0"]
	if inst == nil || len(inst.MemberIDs) != 2 || len(inst.MobIDs) != 1 || inst.Remaining != 1 {
		t.Fatalf("instancia nao abriu corretamente: %+v", inst)
	}
	if leader.Char.Inv[0].Index != 0 || chebyshev(leader.X, leader.Y, 3350, 1622) > 4 ||
		chebyshev(member.X, member.Y, 3350, 1622) > 4 {
		t.Fatalf("party nao foi movida/convite nao consumido: leader=(%d,%d) member=(%d,%d) item=%d",
			leader.X, leader.Y, member.X, member.Y, leader.Char.Inv[0].Index)
	}
	if store.gameSaves != 1 {
		t.Fatalf("abertura deveria ser um commit, saves=%d", store.gameSaves)
	}
	for mobID := range inst.MobIDs {
		w.onItemInstanceMobKilled(w.mobsByID[mobID], w.now())
	}
	if !inst.RewardGranted || leader.Char.Inv[1].Index != 5134 || itemSanc(leader.Char.Inv[1]) != 1 {
		t.Fatalf("recompensa nao encadeou a proxima pista: reward=%v inv=%+v", inst.RewardGranted, leader.Char.Inv[:2])
	}
	if leader.Char.Inv[0].Index != 7001 || member.Char.Inv[0].Index != 7001 {
		t.Fatalf("runa nao foi entregue aos membros: leader=%d member=%d", leader.Char.Inv[0].Index, member.Char.Inv[0].Index)
	}
	if store.gameSaves != 2 {
		t.Fatalf("recompensa deveria usar segundo commit, saves=%d", store.gameSaves)
	}
}

func TestUxmalStartRollsBackTicketPartyAndMobsOnPersistenceFailure(t *testing.T) {
	w, leader, member, store, _ := uxmalTestWorld()
	store.err = errors.New("postgres indisponivel")
	w.startUxmal(leader.Session, leader)
	if len(w.itemInstances) != 0 || len(w.mobs) != 0 || leader.Char.Inv[0].Index != 5134 {
		t.Fatalf("falha deixou estado parcial: instances=%d mobs=%d item=%d", len(w.itemInstances), len(w.mobs), leader.Char.Inv[0].Index)
	}
	if leader.X != 3280 || leader.Y != 1680 || member.X != 3281 || member.Y != 1680 {
		t.Fatalf("rollback nao restaurou entrada: leader=(%d,%d) member=(%d,%d)",
			leader.X, leader.Y, member.X, member.Y)
	}
}

func TestUxmalGroundRewardsRollbackWhenPersistenceFails(t *testing.T) {
	w, leader, member, store, clock := uxmalTestWorld()
	for _, p := range []*Player{leader, member} {
		for slot := 0; slot < model.PlayerCarrySlots; slot++ {
			p.Char.Inv[slot] = model.Item{Index: 7001}
		}
	}
	inst := &ItemInstance{
		Config:        *w.uxmal,
		RuntimeID:     "uxmal:0:0",
		LeaderID:      leader.ID,
		MemberIDs:     []uint16{leader.ID, member.ID},
		UxmalRoom:     0,
		RewardGranted: false,
	}
	store.err = errors.New("postgres indisponivel")
	if w.grantUxmalRewards(inst, clock.Now()) {
		t.Fatal("recompensa deveria permanecer pendente quando o commit falha")
	}
	if inst.RewardGranted || len(w.groundItems) != 0 {
		t.Fatalf("rollback de recompensa no chao incompleto: reward=%v ground=%d",
			inst.RewardGranted, len(w.groundItems))
	}
	for _, p := range []*Player{leader, member} {
		for slot := 0; slot < model.PlayerCarrySlots; slot++ {
			if p.Char.Inv[slot].Index != 7001 {
				t.Fatalf("inventario alterado no rollback: player=%s slot=%d item=%d",
					p.Char.Name, slot, p.Char.Inv[slot].Index)
			}
		}
	}
}

func TestUxmalFullInventoryPublishesGroundRewardsOnlyAfterCommit(t *testing.T) {
	w, leader, member, store, clock := uxmalTestWorld()
	for _, p := range []*Player{leader, member} {
		for slot := 0; slot < model.PlayerCarrySlots; slot++ {
			p.Char.Inv[slot] = model.Item{Index: 7001}
		}
	}
	inst := &ItemInstance{
		Config: *w.uxmal, RuntimeID: "uxmal:0:0", LeaderID: leader.ID,
		MemberIDs: []uint16{leader.ID, member.ID}, UxmalRoom: 0,
	}
	if !w.grantUxmalRewards(inst, clock.Now()) {
		t.Fatal("recompensa no chao deveria ser confirmada")
	}
	if !inst.RewardGranted || store.gameSaves != 1 || len(w.groundItems) != 3 {
		t.Fatalf("transicao incompleta: reward=%v saves=%d ground=%d",
			inst.RewardGranted, store.gameSaves, len(w.groundItems))
	}
	counts := map[uint16]int{}
	for _, item := range w.groundItems {
		counts[item.Item.Index]++
	}
	if counts[7001] != 2 || counts[5134] != 1 {
		t.Fatalf("recompensas no chao incorretas: %v", counts)
	}
}

func TestUxmalFirstRoomHasTwoPartySlots(t *testing.T) {
	w, _, _, _, _ := uxmalTestWorld()
	if got := w.uxmalFreeSlot(0); got != 0 {
		t.Fatalf("primeira vaga=%d, esperado 0", got)
	}
	w.itemInstances["uxmal:0:0"] = &ItemInstance{}
	if got := w.uxmalFreeSlot(0); got != 1 {
		t.Fatalf("segunda vaga=%d, esperado 1", got)
	}
	w.itemInstances["uxmal:0:1"] = &ItemInstance{}
	if got := w.uxmalFreeSlot(0); got != -1 {
		t.Fatalf("sala 0 aceitou terceira party: vaga=%d", got)
	}
}
