package game

import (
	"encoding/binary"
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
			ChainStart:                true,
			AllowChainDuringExitGrace: true,
		},
	}
}

func instanceTestWorld() (*World, *Player, *Player, *guildFlowStore, *fakeClock) {
	leader, _ := networkedTestPlayer(1, "Leader", 2100, 2100)
	member, _ := networkedTestPlayer(2, "Member", 2101, 2100)
	// Production stores assign stable CharacterUIDs before a character can
	// enter an instance. Keep the fixture faithful so the durable Water
	// snapshot exercises the same ownership path.
	leader.Char.UID = "11111111111141118111111111111111"
	member.Char.UID = "22222222222242228222222222222222"
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
		mob := w.mobsByID[mobID]
		if mob == nil || !mob.Awake {
			t.Fatalf("mob inicial %d nao foi acordado apos publicar a instancia", mobID)
		}
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

func TestWaterChainCanStartDuringExitGrace(t *testing.T) {
	w, leader, member, st, clock := instanceTestWorld()
	secondRule := instanceTestRule()
	secondRule.Instance.ID = "water-normal-2"
	secondRule.Instance.X, secondRule.Instance.Y = 2250, 2250
	secondRule.Instance.SpawnX, secondRule.Instance.SpawnY = 2252, 2252
	w.volatiles.Items[3174] = secondRule
	w.volatiles.ItemCodes[3174] = 21
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	first := w.itemInstances["water-normal-1"]
	if first == nil {
		t.Fatal("primeira sala nao abriu")
	}
	var mob *Mob
	for id := range first.MobIDs {
		mob = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(mob, clock.Now())
	if !itemInstanceInExitGraceAt(first, clock.Now()) || leader.Char.Inv[0].Index != 3174 {
		t.Fatalf("sala concluida nao entrou na janela de transicao: inst=%+v item=%d",
			first, leader.Char.Inv[0].Index)
	}

	// O lider usa o pergaminho seguinte antes do recall de 10 segundos. A
	// party inteira deve ser movida para a nova sala sem consumir novamente a
	// entrada antiga nem sofrer um teleport posterior pelo cleanup da primeira.
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	second := w.itemInstances["water-normal-2"]
	if second == nil || len(second.MemberIDs) != 2 ||
		leader.Char.Inv[0].Index != 0 || st.gameSaves != 2 {
		t.Fatalf("cadeia Water nao iniciou na janela nativa: second=%+v item=%d saves=%d",
			second, leader.Char.Inv[0].Index, st.gameSaves)
	}
	if itemInstanceFor := w.itemInstanceForPlayer(leader.ID); itemInstanceFor != second {
		t.Fatalf("lookup do jogador retornou sala antiga: got=%v want=%v",
			itemInstanceFor, second)
	}
	oldLeaderX, oldLeaderY := leader.X, leader.Y
	oldMemberX, oldMemberY := member.X, member.Y
	clock.Advance(10 * time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances["water-normal-1"] != nil ||
		leader.X != oldLeaderX || leader.Y != oldLeaderY ||
		member.X != oldMemberX || member.Y != oldMemberY {
		// O jogador deve permanecer na sala 2; somente a sala 1 desaparece.
		t.Fatalf("cleanup da sala antiga interferiu na nova: old=%v pos=(%d,%d)",
			w.itemInstances["water-normal-1"], leader.X, leader.Y)
	}
	if w.itemInstances["water-normal-2"] == nil ||
		w.itemInstanceForPlayer(member.ID) != second {
		t.Fatal("membro nao permaneceu na segunda sala")
	}
}

func TestPrivateWaterAllocatesIndependentConcurrentRuntime(t *testing.T) {
	w, leader, _, st, _ := instanceTestWorld()
	solo, _ := networkedTestPlayer(3, "Solo", 2105, 2100)
	solo.Char.UID = "33333333333343338333333333333333"
	w.players[solo.Session] = solo
	w.playersByID[solo.ID] = solo
	w.updatePlayerSpatial(solo)

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if w.itemInstances["water-normal-1"] == nil {
		t.Fatal("primeira sala privada nao abriu")
	}
	solo.Char.Inv[0] = model.Item{Index: 100}
	w.onUseItem(solo.Session, useItemPacket(0, 0))

	if w.itemInstances["water-normal-1:1"] == nil ||
		len(w.itemInstances) != 2 || st.gameSaves != 2 ||
		!itemInstanceHasMember(w.itemInstances["water-normal-1:1"], solo.ID) {
		t.Fatalf("salas privadas concorrentes nao foram isoladas: instances=%v saves=%d",
			w.itemInstances, st.gameSaves)
	}
	if w.itemInstances["water-normal-1"].RuntimeID ==
		w.itemInstances["water-normal-1:1"].RuntimeID {
		t.Fatal("RuntimeID privado foi reutilizado")
	}
}

func TestWaterExitGraceRejectsNonNextTicket(t *testing.T) {
	w, leader, _, st, clock := instanceTestWorld()
	wrong := instanceTestRule()
	wrong.Instance.ID = "water-normal-3"
	wrong.Instance.X, wrong.Instance.Y = 2300, 2300
	wrong.Instance.SpawnX, wrong.Instance.SpawnY = 2302, 2302
	wrong.Instance.RewardItem = 0
	w.volatiles.Items[3175] = wrong
	w.volatiles.ItemCodes[3175] = 21
	w.items[3175] = model.ItemDef{Index: 3175}
	leader.Char.Inv[1] = model.Item{Index: 3175}

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	first := w.itemInstances["water-normal-1"]
	if first == nil {
		t.Fatal("primeira sala nao abriu")
	}
	var mob *Mob
	for id := range first.MobIDs {
		mob = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(mob, clock.Now())
	savesBefore := st.gameSaves
	oldX, oldY := leader.X, leader.Y
	w.onUseItem(leader.Session, useItemPacket(1, 0))
	if leader.Char.Inv[1].Index != 3175 || st.gameSaves != savesBefore ||
		w.itemInstances["water-normal-3"] != nil || leader.X != oldX || leader.Y != oldY {
		t.Fatalf("ticket fora da cadeia foi aceito durante exit grace: item=%d saves=%d inst=%v pos=(%d,%d)",
			leader.Char.Inv[1].Index, st.gameSaves, w.itemInstances["water-normal-3"], leader.X, leader.Y)
	}
}

func TestExitGraceOnlyAppliesToRewardingWaterRooms(t *testing.T) {
	now := time.Now()
	for _, test := range []struct {
		name   string
		reward uint16
		want   bool
	}{
		{name: "water regular", reward: 3174, want: true},
		{name: "final/no reward", reward: 0, want: false},
	} {
		t.Run(test.name, func(t *testing.T) {
			inst := &ItemInstance{
				Config: model.VolatileInstance{RewardItem: test.reward,
					AllowChainDuringExitGrace: test.want},
				RewardGranted: true, ExitAt: now.Add(time.Second),
			}
			if got := itemInstanceInExitGrace(inst); got != test.want {
				t.Fatalf("janela de saida=%v, want %v", got, test.want)
			}
		})
	}
}

func TestWaterBossChainsToRoomOneWithoutRewardOrRuntimeReuse(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	// The boss accepts the Room 1 scroll but does not grant it. The ticket is
	// already in carry slot 1, exactly as in the native boss->Room 1 cycle.
	boss := instanceTestRule()
	boss.Instance.ID = "water-normal-boss"
	boss.Instance.RewardItem = 0
	boss.Instance.ChainNextItem = 3173
	w.volatiles.Items[3181] = boss
	w.volatiles.ItemCodes[3181] = 21
	room1 := instanceTestRule()
	room1.Instance.ID = "water-normal-1"
	room1.Instance.X, room1.Instance.Y = 2300, 2300
	room1.Instance.SpawnX, room1.Instance.SpawnY = 2302, 2302
	room1.Instance.RewardItem = 3174
	w.volatiles.Items[3173] = room1
	w.volatiles.ItemCodes[3173] = 21
	w.items[3181] = model.ItemDef{Index: 3181}
	w.items[3173] = model.ItemDef{Index: 3173}
	leader.Char.Inv[0] = model.Item{Index: 3181}
	leader.Char.Inv[1] = model.Item{Index: 3173}

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	old := w.itemInstances["water-normal-boss"]
	if old == nil {
		t.Fatal("boss nao abriu")
	}
	var bossMob *Mob
	for id := range old.MobIDs {
		bossMob = w.mobsByID[id]
	}
	if bossMob == nil {
		t.Fatal("boss sem mob")
	}
	w.onItemInstanceMobKilled(bossMob, clock.Now())
	if !itemInstanceInExitGraceAt(old, clock.Now()) || leader.Char.Inv[0].Index != 0 || leader.Char.Inv[1].Index != 3173 {
		t.Fatalf("boss nao entrou em grace sem conceder Room1: inst=%+v inv=%d/%d",
			old, leader.Char.Inv[0].Index, leader.Char.Inv[1].Index)
	}

	w.onUseItem(leader.Session, useItemPacket(1, 0))
	next := w.itemInstances["water-normal-1"]
	if next == nil || next.RuntimeID == old.RuntimeID || leader.Char.Inv[1].Index != 0 {
		t.Fatalf("ciclo boss->Room1 incorreto: old=%q next=%+v inv=%d",
			old.RuntimeID, next, leader.Char.Inv[1].Index)
	}
	if w.itemInstanceForPlayer(leader.ID) != next {
		t.Fatal("jogador nao foi associado a nova Room1")
	}
	if itemInstanceHasMember(old, leader.ID) {
		t.Fatal("associacao do boss concluido nao foi removida apos o commit")
	}

	clock.Advance(10 * time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances[old.RuntimeID] != nil || w.itemInstances[next.RuntimeID] == nil {
		t.Fatalf("cleanup do boss removeu a sala errada: old=%v next=%v",
			w.itemInstances[old.RuntimeID], w.itemInstances[next.RuntimeID])
	}
}

func TestInstanceMobDeathDoesNotConsumeNPCGeneratorCapacity(t *testing.T) {
	w, leader, _, _, _ := instanceTestWorld()
	w.generators = []generState{{def: model.NPCGener{Index: 0}, current: 1}}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil || len(inst.MobIDs) != 1 {
		t.Fatalf("instancia nao criou o mob de teste: %+v", inst)
	}
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	if mob == nil {
		t.Fatal("mob de instancia nao registrado")
	}
	w.killMobState(leader, mob, mob.HP, mob.HP)
	if w.generators[0].current != 1 {
		t.Fatalf("morte de mob de instancia alterou capacidade do NPCGener: current=%d", w.generators[0].current)
	}
}

func TestInstanceTicketIgnoresForgedPacketCoordinates(t *testing.T) {
	w, leader, _, st, _ := instanceTestWorld()
	rule := w.volatiles.Items[100]
	rule.Instance.EntryAreas = []model.VolatileInstanceEntryArea{{
		MinX: 2192, MinY: 2192, MaxX: 2208, MaxY: 2208,
	}}
	w.volatiles.Items[100] = rule

	// The packet claims to originate inside the room, but the authoritative
	// character position remains outside it.  GridX/GridY are client hints and
	// must never open an instance or consume the ticket.
	pkt := useItemPacket(0, 0)
	binary.LittleEndian.PutUint16(pkt[28:30], 2200)
	binary.LittleEndian.PutUint16(pkt[30:32], 2200)
	w.onUseItem(leader.Session, pkt)
	if w.itemInstances["water-normal-1"] != nil {
		t.Fatal("coordenadas forjadas abriram uma instancia fora da sala")
	}
	if leader.Char.Inv[0].Index != 100 || st.gameSaves != 0 {
		t.Fatalf("ticket foi consumido/persistido com posicao invalida: item=%d saves=%d",
			leader.Char.Inv[0].Index, st.gameSaves)
	}
}

func TestInstanceAreaOccupationIgnoresMembersEnteringRoom(t *testing.T) {
	w, leader, member, _, _ := instanceTestWorld()
	rule := w.volatiles.Items[100]
	rule.Instance.EntryAreas = []model.VolatileInstanceEntryArea{{
		MinX: 2192, MinY: 2192, MaxX: 2208, MaxY: 2208,
	}}
	w.volatiles.Items[100] = rule
	leader.X, leader.Y = 2200, 2200
	leader.Char.X, leader.Char.Y = leader.X, leader.Y
	member.X, member.Y = 2201, 2200
	member.Char.X, member.Char.Y = member.X, member.Y
	if w.instanceAreaOccupied(rule.Instance, "", leader, member) {
		t.Fatal("membros que estao entrando foram tratados como ocupantes externos")
	}
}

func TestInstanceMobTargetsOnlyMembersAndStaysInsideRoom(t *testing.T) {
	member, _ := networkedTestPlayer(1, "Member", 2202, 2200)
	outsider, _ := networkedTestPlayer(2, "Outsider", 2201, 2200)
	w := worldWithNetworkedPlayers(member, outsider)
	inst := &ItemInstance{
		Config: model.VolatileInstance{
			ID: "private-room", X: 2200, Y: 2200, AreaRadius: 8,
			Stages: []model.VolatileInstanceStage{{
				X: 2200, Y: 2200, AreaRadius: 8,
				Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}},
			}},
		},
		MemberIDs: []uint16{member.ID}, CurrentStage: 0,
	}
	w.itemInstances = map[string]*ItemInstance{inst.Config.ID: inst}
	def := &model.NPCDef{
		Name: "RoomMob", Tipo: model.TipoMonstro,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100, AttackRun: 4},
	}
	mob := &Mob{ID: 1000, Def: def, X: 2200, Y: 2200, HP: 100, InstanceID: inst.Config.ID}
	w.mobs = []*Mob{mob}
	w.registerMobSpatial(mob)
	if !mob.Awake {
		t.Fatal("mob nao acordou com membro da instancia dentro do raio")
	}
	w.tickMobCombat(time.Now(), 0, 1, false)
	if mob.TargetID != member.ID {
		t.Fatalf("mob de instancia adquiriu alvo externo: target=%d want=%d", mob.TargetID, member.ID)
	}
	if w.instanceMobTargetAllowed(mob, outsider) {
		t.Fatal("jogador externo foi aceito como alvo da instancia")
	}
	w.publishMobSpawn(mob)
	if !member.hasVisible(mob.ID) {
		t.Fatal("membro da instancia nao recebeu o mob")
	}
	if outsider.hasVisible(mob.ID) {
		t.Fatal("mob de instancia vazou para jogador externo")
	}

	// Uma referencia antiga fora do raio deve ser recolhida, sem permitir que
	// o mob caminhe em direcao ao jogador externo.
	mob.X, mob.Y = 2210, 2200
	w.moveMobSpatial(mob, 2200, 2200)
	w.tickMobCombat(time.Now().Add(time.Second), 0, 1, true)
	if mob.X >= 2210 || chebyshev(mob.X, mob.Y, 2200, 2200) > 8 {
		t.Fatalf("mob saiu/nao retornou ao limite da sala: (%d,%d)", mob.X, mob.Y)
	}
	if w.instanceMobStepAllowed(mob, 2209, 2200) {
		t.Fatal("passo para fora da area privada foi permitido")
	}
}

func nightmareTestRule(tier string) model.VolatileRule {
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	rule.Instance.ID = "nightmare-" + tier
	rule.Instance.NightmareTier = tier
	rule.Instance.AllowedEvolutions = []string{"mortal", "arch", "celestial", "subcelestial"}
	return rule
}

func TestNightmareCelestialGatesAndEntryCharge(t *testing.T) {
	// Normal/Mystic reject a Celestial at the native level boundary before any
	// item mutation, including the personal-ticket path.
	w, leader, _, st, _ := instanceTestWorld()
	rule := nightmareTestRule("normal")
	w.volatiles.Items[100] = rule
	leader.Char.Evolution = "celestial"
	leader.Char.Extended.Level = 180
	leader.Char.ExtendedRuntime = nil
	applyExtendedScore(leader.Char)
	leader.Char.NightmareTickets = 3
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if w.itemInstances[rule.Instance.ID] != nil || leader.Char.Inv[0].Index != 100 ||
		leader.Char.NightmareTickets != 3 || st.gameSaves != 0 {
		t.Fatalf("Normal aceitou Celestial bloqueado: inst=%v item=%d nt=%d saves=%d",
			w.itemInstances[rule.Instance.ID], leader.Char.Inv[0].Index,
			leader.Char.NightmareTickets, st.gameSaves)
	}

	// Arcane accepts a Celestial below 239 only when one NT is available, and
	// consumes exactly one entry after the room has been created.
	w, leader, _, st, _ = instanceTestWorld()
	rule = nightmareTestRule("arcane")
	w.volatiles.Items[100] = rule
	leader.Char.Evolution = "celestial"
	leader.Char.Extended.Level = 238
	leader.Char.ExtendedRuntime = nil
	applyExtendedScore(leader.Char)
	leader.Char.NightmareTickets = 1
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if w.itemInstances[rule.Instance.ID] == nil || leader.Char.Inv[0].Index != 0 ||
		leader.Char.NightmareTickets != 0 || st.gameSaves != 1 {
		t.Fatalf("Arcane nao debitou NT/abriu sala: inst=%v item=%d nt=%d saves=%d",
			w.itemInstances[rule.Instance.ID], leader.Char.Inv[0].Index,
			leader.Char.NightmareTickets, st.gameSaves)
	}

	// Without NT, the authoritative character remains untouched.
	w, leader, _, st, _ = instanceTestWorld()
	rule = nightmareTestRule("arcane")
	w.volatiles.Items[100] = rule
	leader.Char.Evolution = "celestial"
	leader.Char.Extended.Level = 238
	leader.Char.ExtendedRuntime = nil
	applyExtendedScore(leader.Char)
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if w.itemInstances[rule.Instance.ID] != nil || leader.Char.Inv[0].Index != 100 ||
		st.gameSaves != 0 {
		t.Fatalf("Arcane sem NT consumiu ticket: inst=%v item=%d saves=%d",
			w.itemInstances[rule.Instance.ID], leader.Char.Inv[0].Index, st.gameSaves)
	}
}

func TestNightmareSkipsIneligiblePartyCelestial(t *testing.T) {
	w, leader, member, _, _ := instanceTestWorld()
	rule := nightmareTestRule("normal")
	rule.PartyMode = "party"
	w.volatiles.Items[100] = rule
	leader.Char.Evolution = "mortal"
	member.Char.Evolution = "celestial"
	member.Char.Extended.Level = 180
	member.Char.ExtendedRuntime = nil
	applyExtendedScore(member.Char)
	oldX, oldY := member.X, member.Y
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances[rule.Instance.ID]
	if inst == nil || len(inst.MemberIDs) != 1 || inst.MemberIDs[0] != leader.ID {
		t.Fatalf("Celestial inelegivel foi levado pela party: inst=%+v", inst)
	}
	if member.X != oldX || member.Y != oldY {
		t.Fatalf("Celestial inelegivel foi teleportado: (%d,%d) -> (%d,%d)",
			oldX, oldY, member.X, member.Y)
	}
}

func TestNightmareEntryChargeRollsBackOnSpawnFailure(t *testing.T) {
	w, leader, _, st, _ := instanceTestWorld()
	rule := nightmareTestRule("arcane")
	w.volatiles.Items[100] = rule
	leader.Char.Evolution = "celestial"
	leader.Char.Extended.Level = 238
	leader.Char.ExtendedRuntime = nil
	applyExtendedScore(leader.Char)
	leader.Char.NightmareTickets = 1
	for id := uint32(firstMobID); id <= uint32(^uint16(0)); id++ {
		w.mobsByID[uint16(id)] = nil
	}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 100 || leader.Char.NightmareTickets != 1 ||
		st.gameSaves != 0 || w.itemInstances[rule.Instance.ID] != nil {
		t.Fatalf("falha de spawn perdeu NT: item=%d nt=%d saves=%d inst=%v",
			leader.Char.Inv[0].Index, leader.Char.NightmareTickets, st.gameSaves,
			w.itemInstances[rule.Instance.ID])
	}
}

func TestNightmarePartyRunLimitResetsByWindow(t *testing.T) {
	w, _, _, _, clock := instanceTestWorld()
	now := clock.Now()
	cfg := &model.VolatileInstance{
		ID: "nightmare-limit", PartyRunLimit: 2,
		Schedule: []model.VolatileInstanceWindow{{StartMinute: now.Minute(), DurationSeconds: 60}},
	}
	key, allowed := w.nightmarePartyRunAllowed(cfg, "party", now)
	if !allowed || key == "" {
		t.Fatalf("primeira execucao foi recusada: key=%q allowed=%v", key, allowed)
	}
	w.recordNightmarePartyRun(key)
	w.recordNightmarePartyRun(key)
	if _, allowed = w.nightmarePartyRunAllowed(cfg, "party", now); allowed {
		t.Fatal("limite de execucoes compartilhadas nao foi aplicado")
	}
	other := now.Add(time.Hour)
	if _, allowed = w.nightmarePartyRunAllowed(cfg, "party", other); !allowed {
		t.Fatal("contador da janela anterior vazou para a proxima janela")
	}
}

func TestNightmarePartyRunLimitAppliesToOpening(t *testing.T) {
	w, leader, member, st, clock := instanceTestWorld()
	rule := nightmareTestRule("normal")
	rule.PartyMode = "party"
	rule.Instance.PartyRunLimit = 1
	rule.Instance.DurationSeconds = 1
	rule.Instance.Schedule = []model.VolatileInstanceWindow{{
		StartMinute: clock.Now().Minute(), DurationSeconds: 60,
	}}
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if w.itemInstances[rule.Instance.ID] == nil {
		t.Fatal("primeira execucao do Nightmare foi recusada")
	}
	clock.Advance(2 * time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances[rule.Instance.ID] != nil {
		t.Fatal("primeira execucao nao encerrou para liberar o teste")
	}
	leader.Char.Inv[1] = model.Item{Index: 100}
	savesBefore := st.gameSaves
	oldLeaderX, oldLeaderY := leader.X, leader.Y
	oldMemberX, oldMemberY := member.X, member.Y
	w.onUseItem(leader.Session, useItemPacket(1, 0))
	if leader.Char.Inv[1].Index != 100 || w.itemInstances[rule.Instance.ID] != nil ||
		st.gameSaves != savesBefore || leader.X != oldLeaderX || member.X != oldMemberX ||
		leader.Y != oldLeaderY || member.Y != oldMemberY {
		t.Fatalf("segunda execucao ultrapassou partyRunLimit: item=%d inst=%v saves=%d",
			leader.Char.Inv[1].Index, w.itemInstances[rule.Instance.ID], st.gameSaves)
	}
}

func TestInstanceRejectsPartyAlreadyInsideAnotherRoom(t *testing.T) {
	w, leader, member, st, _ := instanceTestWorld()
	second := instanceTestRule()
	second.Instance.ID = "water-normal-2"
	second.Instance.X, second.Instance.Y = 2300, 2300
	second.Instance.SpawnX, second.Instance.SpawnY = 2302, 2302
	w.volatiles.Items[101] = second
	w.items[101] = model.ItemDef{Index: 101}
	leader.Char.Inv[1] = model.Item{Index: 101}

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if w.itemInstances["water-normal-1"] == nil {
		t.Fatal("primeira sala nao abriu")
	}
	oldLeaderX, oldLeaderY := leader.X, leader.Y
	oldMemberX, oldMemberY := member.X, member.Y
	beforePackets := leader.Session.QueuedPacketsForTest()

	w.onUseItem(leader.Session, useItemPacket(1, 0))
	if leader.Char.Inv[1].Index != 101 || st.gameSaves != 1 {
		t.Fatalf("segundo ticket foi consumido/persistido: item=%d saves=%d",
			leader.Char.Inv[1].Index, st.gameSaves)
	}
	if w.itemInstances["water-normal-2"] != nil || leader.X != oldLeaderX ||
		leader.Y != oldLeaderY || member.X != oldMemberX || member.Y != oldMemberY {
		t.Fatal("jogador/party foi inserido em duas instancias")
	}
	if leader.Session.QueuedPacketsForTest() <= beforePackets {
		t.Fatal("recusa da segunda sala nao confirmou o ticket/motivo")
	}
}

func TestExpiredOlderInstanceDoesNotTeleportNewerMembership(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	old := w.itemInstances["water-normal-1"]
	if old == nil {
		t.Fatal("primeira sala nao abriu")
	}
	// Simula um mundo criado antes da trava de unicidade: o mesmo jogador
	// aparece em uma sala nova enquanto a antiga ainda aguarda cleanup.
	newer := &ItemInstance{
		Config:    cfgCopy(old.Config),
		LeaderID:  leader.ID,
		MemberIDs: []uint16{leader.ID},
		Deadline:  clock.Now().Add(time.Minute),
	}
	newer.Config.ID = "water-normal-3"
	w.itemInstances[newer.Config.ID] = newer
	old.Deadline = clock.Now()
	oldX, oldY := leader.X, leader.Y

	w.tickItemInstances(clock.Now())
	if w.itemInstances[old.Config.ID] != nil || w.itemInstances[newer.Config.ID] == nil {
		t.Fatal("cleanup nao removeu somente a sala antiga")
	}
	if leader.X != oldX || leader.Y != oldY {
		t.Fatalf("sala antiga teleportou membro da sala nova: (%d,%d) -> (%d,%d)",
			oldX, oldY, leader.X, leader.Y)
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
	if !inst.RewardGranted || len(w.groundItems) != 1 || st.saves != 1 {
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

func TestInstanceAdmissionUsesGameplaySpaceAndChecksEvolution(t *testing.T) {
	w, leader, member, st, _ := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.ID = "private-room"
	w.volatiles.Items[100] = rule
	member.X, member.Y = 2200, 2200
	member.Char.X, member.Char.Y = member.X, member.Y
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 0 || st.gameSaves != 1 {
		t.Fatal("jogador do espaco publico bloqueou uma instancia privada")
	}

	w2, leader2, _, st2, _ := instanceTestWorld()
	rule2 := instanceTestRule()
	rule2.Instance.ID = "private-room-evolution"
	w2.volatiles.Items[100] = rule2
	leader2.Char.Evolution = "arch"
	w2.onUseItem(leader2.Session, useItemPacket(0, 0))
	if leader2.Char.Inv[0].Index != 100 || st2.gameSaves != 0 {
		t.Fatal("evolucao nao permitida abriu a sala")
	}
}

func TestInstanceTicketRollbackAndRoomExclusion(t *testing.T) {
	w, leader, _, st, _ := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.ID = "private-room"
	w.volatiles.Items[100] = rule
	st.err = errors.New("database unavailable")
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if leader.Char.Inv[0].Index != 100 || leader.X != 2100 ||
		len(w.itemInstances) != 0 || len(w.mobsByID) != 0 {
		t.Fatal("falha de persistencia deixou instancia parcial")
	}

	st.err = nil
	w.itemInstances["private-room"] = &ItemInstance{}
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
	if len(inst.MemberIDs) != 0 || !inst.Deadline.After(clock.Now()) ||
		!inst.TransitionAt.IsZero() || !inst.QuizAt.IsZero() ||
		len(w.pendingInstanceMembers[inst.RuntimeID]) != 2 {
		t.Fatalf("ultima saida nao preservou a sala Water: %+v pendentes=%v",
			inst, w.pendingInstanceMembers[inst.RuntimeID])
	}
}

func TestInstanceRecallDetachesDeadPlayer(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil {
		t.Fatal("instancia nao abriu")
	}
	setPlayerCurHP(leader.Char, 0)
	if !w.recallPlayer(leader, "test restart") {
		t.Fatal("recall do jogador morto falhou")
	}
	if itemInstanceHasMember(inst, leader.ID) || len(inst.MemberIDs) != 1 ||
		inst.MemberIDs[0] != member.ID {
		t.Fatalf("recall manteve membro morto: members=%v", inst.MemberIDs)
	}
	if playerCurHP(leader.Char) == 0 {
		t.Fatal("recall nao reviveu o jogador")
	}
	if pending := w.pendingInstanceMembers[inst.RuntimeID]; len(pending) != 0 {
		if _, kept := pending[leader.Char.UID]; kept {
			t.Fatal("recall deixou o UID do jogador morto pendente")
		}
	}
	// A sala continua viva para o membro remanescente; somente um logout real
	// do ultimo membro deve preservar a identidade offline.
	w.detachPlayerFromItemInstances(member.ID, clock.Now())
	pending := w.pendingInstanceMembers[inst.RuntimeID]
	if !inst.Deadline.After(clock.Now()) || len(pending) != 1 {
		t.Fatalf("sala Water perdeu membros offline: deadline=%v pendentes=%v",
			inst.Deadline, pending)
	}
	if _, kept := pending[member.Char.UID]; !kept {
		t.Fatal("logout do ultimo membro nao preservou seu UID")
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

func TestSharedNightmareEntryReusesZoneAndRespawnsWave(t *testing.T) {
	w, leader, member, st, clock := instanceTestWorld()
	rule := nightmareTestRule("normal")
	rule.PartyMode = "solo"
	rule.Instance.ID = "nightmare-normal-personal"
	rule.Instance.Mode = "shared_timed_zone"
	rule.Instance.SharedGroup = "nightmare-normal-test"
	rule.Instance.FinishPolicy = "respawn_until_timeout"
	rule.Instance.MaxPlayers = 13
	rule.Instance.RewardItem = 0
	w.volatiles.Items[100] = rule
	member.Char.Inv[0] = model.Item{Index: 100}
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["shared:nightmare-normal-test"]
	if inst == nil || len(inst.MobIDs) != 1 || st.gameSaves != 1 {
		t.Fatalf("zona Nightmare nao abriu: inst=%+v saves=%d", inst, st.gameSaves)
	}
	first := inst.MobIDs
	for id := range first {
		mob := w.mobsByID[id]
		w.onItemInstanceMobKilled(mob, clock.Now())
		w.removeMobInstance(mob)
	}
	if inst.ExitAt != (time.Time{}) || inst.Remaining != 1 || len(inst.MobIDs) != 1 {
		t.Fatalf("Nightmare encerrou em vez de regenerar: remaining=%d mobs=%d exit=%v",
			inst.Remaining, len(inst.MobIDs), inst.ExitAt)
	}
	w.onUseItem(member.Session, useItemPacket(0, 0))
	if w.itemInstances["shared:nightmare-normal-test"] != inst || len(inst.MemberIDs) != 2 ||
		member.Char.Inv[0].Index != 0 || st.gameSaves != 2 {
		t.Fatalf("segunda entrada nao aderiu a mesma zona: members=%v item=%d saves=%d",
			inst.MemberIDs, member.Char.Inv[0].Index, st.gameSaves)
	}
}

func TestSharedNightmareCapacityCoversThreeNativeParties(t *testing.T) {
	cfg := model.VolatileInstance{Mode: "shared_timed_zone", MaxPlayers: 13, PartyRunLimit: 3}
	if got := instancePlayerCapacity(&cfg); got != 39 {
		t.Fatalf("capacidade Nightmare=%d, want 39", got)
	}
	cfg.Mode = "private_chain"
	if got := instancePlayerCapacity(&cfg); got != 13 {
		t.Fatalf("capacidade privada=%d, want 13", got)
	}
}

func TestCubeHasNoArtificialCombatDeadline(t *testing.T) {
	cfg := model.VolatileInstance{NoCombatTimeout: true, DurationSeconds: 180}
	if got := stageDuration(cfg, model.VolatileInstanceStage{DurationSeconds: 90}); got != 0 {
		t.Fatalf("Cube recebeu deadline de combate=%d", got)
	}
}

func TestHellGateStateMachineRequiresConfiguredLichPair(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	w.rng = fixedRNG{value: 0} // variante 1: quadrantes 2 e 4
	for _, name := range []string{"Hell_Spider", "Skeleton", "Lich_Dragon"} {
		w.npcs = append(w.npcs, model.NPCDef{Name: name, Tipo: model.TipoMonstro,
			Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100}})
	}
	w.npcs = append(w.npcs, model.NPCDef{Name: "Aki", Tipo: model.TipoNPC,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 100, CurHP: 100}})
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	rule.Instance.ID = "hell-gate-test"
	rule.Instance.StateMachine = "hell_gate"
	rule.Instance.Mode = "state_machine"
	rule.Instance.DurationSeconds = 240
	rule.Instance.RewardItem = 0
	rule.Instance.Stages = []model.VolatileInstanceStage{{
		Name: "Hell Gate", X: 2200, Y: 2200, SpawnX: 2200, SpawnY: 2200,
		AreaRadius: 30, DurationSeconds: 240,
		Spawns: []model.VolatileInstanceSpawn{{NPC: "Hell_Spider", Count: 1, X: 2200, Y: 2200}},
	}}
	rule.Instance.HellGate = &model.VolatileHellGate{
		ControllerNPC: "Hell_Spider",
		Quadrants: []model.VolatileHellGateQuadrant{
			{ID: 1, X: 2200, Y: 2200, SpawnX: 2200, SpawnY: 2200, AreaRadius: 8,
				Spawns: []model.VolatileInstanceSpawn{{NPC: "Skeleton", Count: 1, X: 2200, Y: 2200}},
				Lich:   model.VolatileInstanceSpawn{NPC: "Lich_Dragon", Count: 1, X: 2200, Y: 2200}},
			{ID: 2, X: 2210, Y: 2200, SpawnX: 2210, SpawnY: 2200, AreaRadius: 8,
				Spawns: []model.VolatileInstanceSpawn{{NPC: "Skeleton", Count: 1, X: 2210, Y: 2200}},
				Lich:   model.VolatileInstanceSpawn{NPC: "Lich_Dragon", Count: 1, X: 2210, Y: 2200}},
			{ID: 3, X: 2220, Y: 2200, SpawnX: 2220, SpawnY: 2200, AreaRadius: 8,
				Spawns: []model.VolatileInstanceSpawn{{NPC: "Skeleton", Count: 1, X: 2220, Y: 2200}},
				Lich:   model.VolatileInstanceSpawn{NPC: "Lich_Dragon", Count: 1, X: 2220, Y: 2200}},
			{ID: 4, X: 2230, Y: 2200, SpawnX: 2230, SpawnY: 2200, AreaRadius: 8,
				Spawns: []model.VolatileInstanceSpawn{{NPC: "Skeleton", Count: 1, X: 2230, Y: 2200}},
				Lich:   model.VolatileInstanceSpawn{NPC: "Lich_Dragon", Count: 1, X: 2230, Y: 2200}},
		},
		FinalSpawns: []model.VolatileInstanceSpawn{{NPC: "Skeleton", Count: 1, X: 2240, Y: 2200}},
		FinalNPCs:   []model.VolatileInstanceSpawn{{NPC: "Aki", Count: 1, X: 2241, Y: 2200}},
	}
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["hell-gate-test"]
	if inst == nil {
		t.Fatal("Hell Gate de teste nao abriu")
	}
	var controller *Mob
	for id := range inst.MobIDs {
		controller = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(controller, clock.Now())
	w.removeMobInstance(controller)
	if inst.State != "lich_puzzle" || inst.HellGateValidLichMask != hellGateBit(2)|hellGateBit(4) ||
		len(inst.MobIDs) != 4 {
		t.Fatalf("TarÃ¢ntula nao abriu quadrantes: state=%q mask=%d mobs=%d",
			inst.State, inst.HellGateValidLichMask, len(inst.MobIDs))
	}
	// Limpa cada quadrante e confirma que os quatro Lichs so nascem depois da
	// sua propria guarda, nunca pelo pacote de morte de outra sala.
	for id, mob := range func() map[uint16]*Mob {
		result := make(map[uint16]*Mob)
		for id, mob := range w.mobsByID {
			if inst.MobQuadrants[id] != 0 {
				result[id] = mob
			}
		}
		return result
	}() {
		w.onItemInstanceMobKilled(mob, clock.Now())
		w.removeMobInstance(mob)
		_ = id
	}
	if inst.HellGateLichSpawnedMask != 0x0F || len(inst.HellGateLichIDs) != 4 {
		t.Fatalf("Lichs nao nasceram por quadrante: spawned=%X ids=%v", inst.HellGateLichSpawnedMask, inst.HellGateLichIDs)
	}
	for _, q := range []uint8{2, 4} {
		id := inst.HellGateLichIDs[q]
		mob := w.mobsByID[id]
		w.onItemInstanceMobKilled(mob, clock.Now())
		w.removeMobInstance(mob)
	}
	if inst.State != "final" || len(inst.NPCIDs) != 1 || inst.Remaining != 1 {
		t.Fatalf("par correto nao abriu corredor final: state=%q npcs=%d remaining=%d",
			inst.State, len(inst.NPCIDs), inst.Remaining)
	}
	for id := range inst.MobIDs {
		mob := w.mobsByID[id]
		w.onItemInstanceMobKilled(mob, clock.Now())
		w.removeMobInstance(mob)
	}
	if inst.ExitAt.IsZero() {
		t.Fatal("morte do final nao concluiu o Hell Gate")
	}
}

func TestHellGateWrongLichDoesNotBlockSelectedPair(t *testing.T) {
	w, _, _, _, clock := instanceTestWorld()
	inst := &ItemInstance{
		RuntimeID:             "hell-gate-wrong",
		Config:                model.VolatileInstance{StateMachine: "hell_gate", HellGate: &model.VolatileHellGate{}},
		State:                 "lich_puzzle",
		MobIDs:                map[uint16]struct{}{10: {}, 11: {}, 12: {}},
		MobQuadrants:          map[uint16]uint8{10: 1, 11: 2, 12: 4},
		HellGateLichIDs:       map[uint8]uint16{1: 10, 2: 11, 4: 12},
		HellGateValidLichMask: hellGateBit(2) | hellGateBit(4),
	}
	w.itemInstances[inst.RuntimeID] = inst
	w.onHellGateMobKilled(inst, &Mob{ID: 10}, clock.Now())
	w.onHellGateMobKilled(inst, &Mob{ID: 11}, clock.Now())
	w.onHellGateMobKilled(inst, &Mob{ID: 12}, clock.Now())
	if !inst.HellGateWrongLich || inst.State != "final" {
		t.Fatalf("par correto nao liberou o corredor apos Lich incorreto: wrong=%v state=%q", inst.HellGateWrongLich, inst.State)
	}
}

func TestHellGateEntryWindowDoesNotEndActiveEncounter(t *testing.T) {
	w, _, _, _, clock := instanceTestWorld()
	now := clock.Now()
	inst := &ItemInstance{
		RuntimeID: "shared:hell-gate",
		Config: model.VolatileInstance{
			ID: "hell-gate", Mode: "shared_timed_zone", StateMachine: "hell_gate",
			FinishPolicy: "state_machine", HellGate: &model.VolatileHellGate{},
		},
		State: "entry", ScheduleEnd: now.Add(time.Second),
		HardDeadline: now.Add(10 * time.Second), CombatDeadline: now.Add(10 * time.Second),
		MobIDs: make(map[uint16]struct{}), NPCIDs: make(map[uint16]struct{}),
	}
	w.itemInstances[inst.RuntimeID] = inst
	clock.Advance(2 * time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances[inst.RuntimeID] == nil {
		t.Fatal("Hell Gate foi encerrado no fim da janela de entrada")
	}
	clock.Advance(9 * time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances[inst.RuntimeID] != nil {
		t.Fatal("Hell Gate nao foi encerrado no deadline ativo")
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

func TestSharedInstanceRejectsExpiredAbsoluteDeadlines(t *testing.T) {
	tests := []struct {
		name string
		set  func(*ItemInstance, time.Time)
	}{
		{
			name: "hard deadline",
			set: func(inst *ItemInstance, now time.Time) {
				inst.HardDeadline = now.Add(-time.Second)
			},
		},
		{
			name: "schedule window",
			set: func(inst *ItemInstance, now time.Time) {
				inst.ScheduleEnd = now.Add(-time.Second)
			},
		},
	}
	for _, test := range tests {
		t.Run(test.name, func(t *testing.T) {
			w, leader, member, st, clock := instanceTestWorld()
			rule := instanceTestRule()
			rule.PartyMode = "solo"
			rule.Instance.ID = "shared-expired-" + test.name
			rule.Instance.SharedEntry = true
			rule.Instance.MaxPlayers = 6
			rule.Instance.RewardItem = 0
			w.volatiles.Items[100] = rule
			member.Char.Inv[0] = model.Item{Index: 100}

			w.onUseItem(leader.Session, useItemPacket(0, 0))
			inst := w.itemInstances[rule.Instance.ID]
			if inst == nil || len(inst.MemberIDs) != 1 || st.gameSaves != 1 {
				t.Fatalf("abertura inicial incorreta: inst=%+v saves=%d", inst, st.gameSaves)
			}
			oldX, oldY := member.X, member.Y
			test.set(inst, clock.Now())
			w.onUseItem(member.Session, useItemPacket(0, 0))
			if len(inst.MemberIDs) != 1 || member.Char.Inv[0].Index != 100 ||
				member.X != oldX || member.Y != oldY || st.gameSaves != 1 {
				t.Fatalf("ingresso apos prazo alterou estado: members=%v item=%d pos=(%d,%d) saves=%d",
					inst.MemberIDs, member.Char.Inv[0].Index, member.X, member.Y, st.gameSaves)
			}
		})
	}
}

func TestExclusiveInstanceLockIncludesExitGrace(t *testing.T) {
	w, leader, _, st, clock := instanceTestWorld()
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	rule.Instance.ID = "magic-chamber-normal-test"
	rule.Instance.ExclusiveGroup = "magic-chamber"
	rule.Instance.RewardItem = 0
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	active := w.itemInstances[rule.Instance.ID]
	if active == nil || st.gameSaves != 1 {
		t.Fatal("Magic Chamber de teste nao abriu")
	}
	active.RewardGranted = true
	active.ExitAt = clock.Now().Add(10 * time.Second)
	active.ExitDeadline = active.ExitAt
	active.Config.ID = "magic-chamber-normal-test"
	leader.Char.Inv[0] = model.Item{Index: 100}
	second := rule
	secondCfg := cfgCopy(*rule.Instance)
	second.Instance = &secondCfg
	second.Instance.ID = "magic-chamber-arcane-test"
	w.volatiles.Items[100] = second
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	if len(w.itemInstances) != 1 || leader.Char.Inv[0].Index != 100 || st.gameSaves != 1 {
		t.Fatal("lock exclusivo foi liberado durante a janela de saida")
	}
}

func TestMagicChamberBossUsesFourthRoomDeadline(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	w.npcs = append(w.npcs, model.NPCDef{Name: "ChamberBoss", Tipo: model.TipoMonstro,
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion, MaxHP: 500, CurHP: 500}})
	rule := instanceTestRule()
	rule.PartyMode = "solo"
	rule.Instance.ID = "magic-chamber-fourth"
	rule.Instance.ExclusiveGroup = "magic-chamber"
	rule.Instance.TransitionSeconds = 1
	rule.Instance.RewardItem = 0
	rule.Instance.Stages = make([]model.VolatileInstanceStage, 4)
	for i := range rule.Instance.Stages {
		rule.Instance.Stages[i] = model.VolatileInstanceStage{
			Name: "Room",
			X:    uint16(2200 + i*20), Y: 2200,
			SpawnX: uint16(2200 + i*20), SpawnY: 2200,
			AreaRadius: 8, DurationSeconds: 60,
			Spawns: []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}},
		}
	}
	rule.Instance.Stages[3].CompletionSpawns = []model.VolatileInstanceSpawn{{NPC: "ChamberBoss", Count: 1}}
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances[rule.Instance.ID]
	if inst == nil {
		t.Fatal("Magic Chamber de teste nao abriu")
	}
	for stage := 0; stage < 4; stage++ {
		var mob *Mob
		for id := range inst.MobIDs {
			mob = w.mobsByID[id]
			break
		}
		if mob == nil {
			t.Fatalf("sala %d nao possui mob", stage+1)
		}
		deadline := instanceCombatDeadline(inst)
		w.onItemInstanceMobKilled(mob, clock.Now())
		w.removeMobInstance(mob)
		if stage < 3 {
			clock.Advance(time.Second)
			w.tickItemInstances(clock.Now())
			continue
		}
		if !inst.BossSpawned || inst.CurrentStage != 3 || len(inst.MobIDs) != 1 ||
			!instanceCombatDeadline(inst).Equal(deadline) {
			t.Fatalf("boss nao usou o prazo da sala 4: stage=%d boss=%v mobs=%d deadline=%v/%v",
				inst.CurrentStage, inst.BossSpawned, len(inst.MobIDs), instanceCombatDeadline(inst), deadline)
		}
	}
	for id := range inst.MobIDs {
		boss := w.mobsByID[id]
		w.onItemInstanceMobKilled(boss, clock.Now())
		w.removeMobInstance(boss)
	}
	if inst.ExitAt.IsZero() {
		t.Fatal("morte do chefe nao concluiu Magic Chamber")
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
	// A rota tambem pode atravessar a sala depois do ponto de spawn. Reservar
	// apenas Segments[0] deixava esse gerador reaparecer dentro da instancia.
	routed := model.NPCGener{}
	routed.Segments[0] = model.GenerSegment{X: 2400, Y: 2400}
	routed.Segments[1] = model.GenerSegment{X: 2208, Y: 2202}
	if !w.generatorReservedForItemInstance(routed) {
		t.Fatal("segmento posterior da rota nao foi reservado")
	}
	crossing := model.NPCGener{}
	crossing.Segments[0] = model.GenerSegment{X: 2180, Y: 2200}
	crossing.Segments[1] = model.GenerSegment{X: 2220, Y: 2200}
	if !w.generatorReservedForItemInstance(crossing) {
		t.Fatal("corredor da rota que cruza a sala nao foi reservado")
	}
	outside := model.NPCGener{}
	outside.Segments[0] = model.GenerSegment{X: 2220, Y: 2211}
	outside.Segments[1] = model.GenerSegment{X: 2230, Y: 2211}
	if w.generatorReservedForItemInstance(outside) {
		t.Fatal("corredor totalmente fora da sala foi reservado")
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
	for id := range inst.MobIDs {
		if !leader.hasVisible(id) || !member.hasVisible(id) {
			t.Fatalf("mob da segunda sala %d nao foi publicado aos membros", id)
		}
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

func TestCubeQuizPromotesSurvivorWhenLeaderAnswersWrong(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	inst := &ItemInstance{
		Config: model.VolatileInstance{
			ID: "cube-leader-test", ExitX: 2100, ExitY: 2100,
			Stages: []model.VolatileInstanceStage{{
				X: 2200, Y: 2200, AreaRadius: 8,
				Quiz: &model.VolatileInstanceQuiz{
					Answer: true, TrueX: 2200, TrueY: 2200,
					FalseX: 2210, FalseY: 2210, DurationSeconds: 14,
				},
			}},
		},
		LeaderID: leader.ID, MemberIDs: []uint16{leader.ID, member.ID},
		CurrentStage: 0, QuizAt: clock.Now(), Deadline: clock.Now().Add(time.Minute),
	}
	leader.X, leader.Y = 2210, 2210
	leader.Char.X, leader.Char.Y = leader.X, leader.Y
	member.X, member.Y = 2200, 2200
	member.Char.X, member.Char.Y = member.X, member.Y
	if !w.resolveItemInstanceQuiz(inst, clock.Now()) {
		t.Fatal("resolucao do quiz falhou")
	}
	if len(inst.MemberIDs) != 1 || inst.MemberIDs[0] != member.ID ||
		inst.LeaderID != member.ID {
		t.Fatalf("sobrevivente nao virou lider: members=%v leader=%d",
			inst.MemberIDs, inst.LeaderID)
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
	if !inst.Deadline.Equal(inst.HardDeadline) {
		t.Fatalf("prazo da sala ultrapassou o prazo total: deadline=%v hard=%v",
			inst.Deadline, inst.HardDeadline)
	}
	clock.Advance(5 * time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances["water-normal-1"] != nil {
		t.Fatal("instancia permaneceu aberta depois do prazo absoluto")
	}
}

func TestInstanceDoesNotRewardAfterStageDeadline(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil {
		t.Fatal("instancia nao abriu")
	}
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	clock.Advance(120 * time.Second)
	w.onItemInstanceMobKilled(mob, clock.Now())
	if inst.RewardGranted || !inst.ExitAt.IsZero() || !inst.Deadline.Equal(clock.Now()) {
		t.Fatalf("morte apos timeout concedeu estado indevido: reward=%v exit=%v deadline=%v",
			inst.RewardGranted, inst.ExitAt, inst.Deadline)
	}
	w.tickItemInstances(clock.Now())
	if w.itemInstances["water-normal-1"] != nil {
		t.Fatal("instancia expirada permaneceu no mundo")
	}
}

func TestStagedTransitionOwnDeadlineOutlivesCombatDeadline(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.ID = "staged-deadline"
	rule.Instance.Spawns = nil
	rule.Instance.X, rule.Instance.Y = 0, 0
	rule.Instance.SpawnX, rule.Instance.SpawnY = 0, 0
	rule.Instance.AreaRadius = 0
	rule.Instance.DurationSeconds = 0
	rule.Instance.TransitionSeconds = 10
	rule.Instance.RewardItem = 0
	rule.Instance.Stages = []model.VolatileInstanceStage{
		{X: 2200, Y: 2200, SpawnX: 2202, SpawnY: 2202, AreaRadius: 8,
			DurationSeconds: 5,
			Spawns:          []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}}},
		{X: 2250, Y: 2250, SpawnX: 2252, SpawnY: 2252, AreaRadius: 8,
			DurationSeconds: 60,
			Spawns:          []model.VolatileInstanceSpawn{{NPC: "RoomMob", Count: 1}}},
	}
	w.volatiles.Items[100] = rule
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["staged-deadline"]
	if inst == nil {
		t.Fatal("instancia nao abriu")
	}
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(mob, clock.Now())
	if inst.TransitionAt.IsZero() {
		t.Fatal("morte nao agendou a transicao")
	}
	clock.Advance(11 * time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances["staged-deadline"] == nil || inst.CurrentStage != 1 {
		t.Fatalf("transicao independente nao avancou: stage=%d inst=%v",
			inst.CurrentStage, w.itemInstances["staged-deadline"])
	}
}

func TestStagedSpawnFailureCannotGrantReward(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.ID = "staged-spawn-failure"
	rule.Instance.Spawns = nil
	rule.Instance.X, rule.Instance.Y = 0, 0
	rule.Instance.SpawnX, rule.Instance.SpawnY = 0, 0
	rule.Instance.AreaRadius = 0
	rule.Instance.DurationSeconds = 0
	rule.Instance.TransitionSeconds = 1
	rule.Instance.RewardItem = 3174
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
	inst := w.itemInstances["staged-spawn-failure"]
	if inst == nil {
		t.Fatal("instancia nao abriu")
	}
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	w.onItemInstanceMobKilled(mob, clock.Now())
	w.removeMobInstance(mob)
	// Esgota a faixa de IDs somente depois que a primeira sala foi aberta.
	for id := uint32(firstMobID); id <= uint32(^uint16(0)); id++ {
		w.mobsByID[uint16(id)] = nil
	}
	clock.Advance(time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances["staged-spawn-failure"] != nil ||
		leader.Char.Inv[0].Index == 3174 {
		t.Fatalf("falha de spawn concedeu recompensa ou deixou instancia viva: inst=%v item=%d",
			w.itemInstances["staged-spawn-failure"], leader.Char.Inv[0].Index)
	}
}

func TestInstanceTemplateValidationIncludesRules(t *testing.T) {
	w, _, _, _, _ := instanceTestWorld()
	rule := instanceTestRule()
	rule.Instance.ID = "rule-only"
	rule.Instance.Spawns = []model.VolatileInstanceSpawn{{NPC: "MissingMonster", Count: 1}}
	w.volatiles.Rules = map[int]model.VolatileRule{54: rule}
	if err := w.validateItemInstanceTemplates(); err == nil {
		t.Fatal("regra de instancia com NPC ausente foi aceita")
	}
}

func TestInstanceScheduleAndEntryArea(t *testing.T) {
	cfg := &model.VolatileInstance{
		EntryAreas: []model.VolatileInstanceEntryArea{{
			MinX: 100, MinY: 200, MaxX: 101, MaxY: 201,
		}},
		Schedule: []model.VolatileInstanceWindow{{
			StartMinute: 20, DurationSeconds: 240,
		}},
	}
	now := time.Date(2026, time.January, 1, 12, 20, 30, 0, time.Local)
	end, ok := instanceScheduleEnd(cfg, now)
	if !ok || end.Sub(now) != 210*time.Second {
		t.Fatalf("janela calculada incorretamente: ok=%v end=%v", ok, end)
	}
	if !instanceTargetAllowedAt(cfg, 101, 200) {
		t.Fatal("target dentro da area foi recusado")
	}
	if instanceTargetAllowedAt(cfg, 102, 200) || instanceTargetAllowedAt(cfg, 0, 0) {
		t.Fatal("target fora/ausente foi aceito")
	}
	if _, ok := instanceScheduleEnd(cfg, now.Add(5*time.Minute)); ok {
		t.Fatal("ticket permaneceu aberto fora da janela")
	}
}

func TestInstanceScheduleCrossesHourBoundary(t *testing.T) {
	cfg := &model.VolatileInstance{
		PartyRunLimit: 1,
		Schedule: []model.VolatileInstanceWindow{{
			StartMinute: 59, DurationSeconds: 240,
		}},
	}
	now := time.Date(2026, time.January, 1, 12, 0, 30, 0, time.Local)
	end, ok := instanceScheduleEnd(cfg, now)
	if !ok || end.Sub(now) != 150*time.Second {
		t.Fatalf("janela apos virada nao foi reconhecida: ok=%v end=%v", ok, end)
	}
	if key := nightmarePartyRunKey(cfg, now); key == "" {
		t.Fatal("janela de Nightmare apos virada nao produziu chave")
	}
	closed := now.Add(150 * time.Second)
	if _, ok := instanceScheduleEnd(cfg, closed); ok {
		t.Fatal("janela permaneceu aberta apos o prazo")
	}
}
