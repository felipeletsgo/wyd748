package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func configureWaterSecondRoom(w *World) {
	second := instanceTestRule()
	second.Instance.ID = "water-normal-2"
	second.Instance.ChainStart = false
	second.Instance.X, second.Instance.Y = 2250, 2250
	second.Instance.SpawnX, second.Instance.SpawnY = 2252, 2252
	w.volatiles.Items[3174] = second
	w.volatiles.ItemCodes[3174] = 21
	w.items[3174] = model.ItemDef{Index: 3174}
}

func equipActiveSilverForWaterTest(t *testing.T, w *World, p *Player) {
	t.Helper()
	w.items[3914] = fairyTestDef(3914, 7)
	p.Char.Equip[fairySlot] = model.Item{Index: 3914}
}

func killOnlyInstanceMob(t *testing.T, w *World, inst *ItemInstance, now time.Time) {
	t.Helper()
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
		break
	}
	if mob == nil {
		t.Fatal("instancia sem mob para concluir")
	}
	w.onItemInstanceMobKilled(mob, now)
}

func TestWaterSilverAutoAdvanceUsesGrantedScroll(t *testing.T) {
	w, leader, member, _, clock := instanceTestWorld()
	configureWaterSecondRoom(w)
	equipActiveSilverForWaterTest(t, w, leader)

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	first := w.itemInstances["water-normal-1"]
	if first == nil {
		t.Fatal("primeira sala nao abriu")
	}
	killOnlyInstanceMob(t, w, first, clock.Now())

	second := w.itemInstances["water-normal-2"]
	if second == nil || w.itemInstanceForPlayer(leader.ID) != second ||
		w.itemInstanceForPlayer(member.ID) != second {
		t.Fatalf("Silver Angel nao abriu a sala seguinte: second=%+v", second)
	}
	if leader.Char.Inv[0].Index != 0 {
		t.Fatalf("scroll recem concedido nao foi consumido: %+v", leader.Char.Inv[0])
	}
	if itemInstanceHasMember(first, leader.ID) || itemInstanceHasMember(first, member.ID) {
		t.Fatal("sala anterior manteve membros depois do auto-avanco")
	}
}

func TestWaterAutoAdvanceRequiresSilverOnLeaderAtReward(t *testing.T) {
	tests := []struct {
		name  string
		setup func(*testing.T, *World, *Player, *Player)
	}{
		{name: "no fairy"},
		{name: "silver only in inventory", setup: func(t *testing.T, w *World, leader, _ *Player) {
			w.items[3914] = fairyTestDef(3914, 7)
			leader.Char.Inv[6] = model.Item{Index: 3914}
		}},
		{name: "silver on non leader", setup: func(t *testing.T, w *World, _, member *Player) {
			equipActiveSilverForWaterTest(t, w, member)
		}},
	}
	for _, tc := range tests {
		t.Run(tc.name, func(t *testing.T) {
			w, leader, member, _, clock := instanceTestWorld()
			configureWaterSecondRoom(w)
			if tc.setup != nil {
				tc.setup(t, w, leader, member)
			}
			w.onUseItem(leader.Session, useItemPacket(0, 0))
			first := w.itemInstances["water-normal-1"]
			killOnlyInstanceMob(t, w, first, clock.Now())
			if w.itemInstances["water-normal-2"] != nil {
				t.Fatal("auto-avanco ocorreu sem Silver Angel ativa no lider")
			}
			if leader.Char.Inv[0].Index != 3174 || leader.Char.Inv[0].UID == "" {
				t.Fatalf("reward deveria permanecer duravel no carry: %+v", leader.Char.Inv[0])
			}
		})
	}
}

func TestWaterSilverDoesNotRetroactivelyAdvance(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	configureWaterSecondRoom(w)
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	first := w.itemInstances["water-normal-1"]
	killOnlyInstanceMob(t, w, first, clock.Now())
	if w.itemInstances["water-normal-2"] != nil {
		t.Fatal("sala seguinte abriu sem fada")
	}
	equipActiveSilverForWaterTest(t, w, leader)
	clock.Advance(time.Second)
	w.tickItemInstances(clock.Now())
	if w.itemInstances["water-normal-2"] != nil || leader.Char.Inv[0].Index != 3174 {
		t.Fatal("equipar a fada depois do reward acionou auto-avanco retroativo")
	}
}

func TestWaterSilverConsumesOnlyNewRewardUID(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	configureWaterSecondRoom(w)
	equipActiveSilverForWaterTest(t, w, leader)
	older, err := materializeItem(model.Item{Index: 3174})
	if err != nil {
		t.Fatal(err)
	}
	leader.Char.Inv[5] = older

	w.onUseItem(leader.Session, useItemPacket(0, 0))
	first := w.itemInstances["water-normal-1"]
	killOnlyInstanceMob(t, w, first, clock.Now())
	if w.itemInstances["water-normal-2"] == nil {
		t.Fatal("Silver Angel nao abriu sala seguinte")
	}
	if leader.Char.Inv[5].UID != older.UID || leader.Char.Inv[5].Index != 3174 {
		t.Fatalf("scroll antigo do mesmo indice foi consumido: before=%+v after=%+v",
			older, leader.Char.Inv[5])
	}
}

func TestWaterSilverFullInventoryKeepsGroundFallback(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	configureWaterSecondRoom(w)
	equipActiveSilverForWaterTest(t, w, leader)
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	first := w.itemInstances["water-normal-1"]
	for slot := 0; slot < model.PlayerCarrySlots; slot++ {
		leader.Char.Inv[slot] = model.Item{Index: 500}
	}
	killOnlyInstanceMob(t, w, first, clock.Now())
	if w.itemInstances["water-normal-2"] != nil {
		t.Fatal("reward no chao nao pode acionar auto-avanco")
	}
	if len(w.groundItems) == 0 {
		t.Fatal("inventario cheio nao gerou fallback de reward no chao")
	}
}

func TestWaterSilverGrantPersistenceFailureDoesNotAdvance(t *testing.T) {
	w, leader, _, st, clock := instanceTestWorld()
	configureWaterSecondRoom(w)
	equipActiveSilverForWaterTest(t, w, leader)
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	first := w.itemInstances["water-normal-1"]
	st.err = errors.New("reward commit failed")
	killOnlyInstanceMob(t, w, first, clock.Now())
	if first.RewardGranted || w.itemInstances["water-normal-2"] != nil || leader.Char.Inv[0].Index != 0 {
		t.Fatalf("falha do grant deixou estado parcial: reward=%v next=%v inv=%+v",
			first.RewardGranted, w.itemInstances["water-normal-2"], leader.Char.Inv[0])
	}
}

type failWaterTransitionStore struct {
	guildFlowStore
	calls  int
	failOn int
	failed bool
}

func (s *failWaterTransitionStore) SaveGameStateWithInstanceState(g *model.GuildRegistry,
	snapshot *model.InstanceStateSnapshot, accounts ...*model.Account) error {
	s.calls++
	if s.calls == s.failOn && !s.failed {
		s.failed = true
		return errors.New("next room commit failed")
	}
	return s.guildFlowStore.SaveGameStateWithInstanceState(g, snapshot, accounts...)
}

func TestWaterSilverTransitionFailureLeavesExactGrantedScroll(t *testing.T) {
	w, leader, _, baseStore, clock := instanceTestWorld()
	configureWaterSecondRoom(w)
	equipActiveSilverForWaterTest(t, w, leader)
	older, err := materializeItem(model.Item{Index: 3174})
	if err != nil {
		t.Fatal(err)
	}
	leader.Char.Inv[5] = older

	// Open room 1 with the normal store. From this point on the first
	// transaction is the durable reward and the second is auto-advance.
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	first := w.itemInstances["water-normal-1"]
	if first == nil {
		t.Fatal("primeira sala nao abriu")
	}
	failing := &failWaterTransitionStore{guildFlowStore: *baseStore, failOn: 2}
	w.store = failing
	killOnlyInstanceMob(t, w, first, clock.Now())

	if !failing.failed {
		t.Fatal("segunda fronteira de persistencia nao foi exercitada")
	}
	if w.itemInstances["water-normal-2"] != nil {
		t.Fatal("sala seguinte foi publicada apesar da falha de persistencia")
	}
	granted := leader.Char.Inv[0]
	if granted.Index != 3174 || granted.UID == "" || granted.UID == older.UID {
		t.Fatalf("scroll recem concedido nao sobreviveu ao rollback: granted=%+v older=%+v",
			granted, older)
	}
	if leader.Char.Inv[5].UID != older.UID {
		t.Fatal("rollback alterou o scroll antigo do mesmo indice")
	}
}

func TestWaterSilverFinalRewardZeroDoesNotConsumeExistingScroll(t *testing.T) {
	w, leader, _, _, clock := instanceTestWorld()
	equipActiveSilverForWaterTest(t, w, leader)
	existing, err := materializeItem(model.Item{Index: 3173})
	if err != nil {
		t.Fatal(err)
	}
	leader.Char.Inv[5] = existing
	inst := &ItemInstance{
		Config: model.VolatileInstance{
			ID: "water-normal-boss", Mode: "private_chain", RewardItem: 0,
			ChainNextItem: 3173, AllowChainDuringExitGrace: true,
		},
		RuntimeID: "water-normal-boss", LeaderID: leader.ID, MemberIDs: []uint16{leader.ID},
	}
	w.itemInstances[inst.RuntimeID] = inst
	w.indexInstanceMembers(inst)
	w.completeItemInstance(inst, clock.Now())
	if leader.Char.Inv[5].UID != existing.UID || leader.Char.Inv[5].Index != 3173 {
		t.Fatal("reward zero consumiu scroll preexistente")
	}
}
