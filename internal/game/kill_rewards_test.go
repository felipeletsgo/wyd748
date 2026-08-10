package game

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
)

type killAuditStore struct {
	err          error
	saves        int
	session      *gameNet.Session
	queuedAtSave int
	saved        []*model.Account
}

func (s *killAuditStore) LoadAccount(string) (*model.Account, error) {
	return nil, errors.New("not used")
}
func (s *killAuditStore) SaveAccount(account *model.Account) error {
	return s.SaveAccounts(account)
}
func (s *killAuditStore) SaveAccounts(accounts ...*model.Account) error {
	s.saves++
	s.queuedAtSave = s.session.QueuedPacketsForTest()
	s.saved = accounts
	return s.err
}
func (s *killAuditStore) CharacterNameExists(string) (bool, error) { return false, nil }

func killRewardTestWorld(t *testing.T, storeErr error) (*World, *Player, *Mob, *killAuditStore) {
	t.Helper()
	p, session := networkedTestPlayer(1, "Killer", 2100, 2100)
	w := worldWithNetworkedPlayers(p)
	w.rng = fixedRNG{value: 0}
	w.gameplay = model.DefaultGameplayConfig()
	w.items = map[uint16]model.ItemDef{500: {Index: 500, Name: "Reward"}}
	w.dropRates[11] = 1
	st := &killAuditStore{err: storeErr, session: session}
	w.store = st
	mob := &Mob{ID: 1400, X: 2101, Y: 2100, HP: 0, GenerIndex: -1,
		Def: &model.NPCDef{Name: "RewardMob", Tipo: model.TipoMonstro,
			ExpReward: 1000, Gold: 100,
			Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion,
				Level: 20, MaxHP: 100, CurHP: 100, MaxMP: 10, CurMP: 10},
			Carry: make([]model.Item, 12)}}
	mob.Def.Carry[11] = model.Item{Index: 500}
	w.mobs = append(w.mobs, mob)
	w.registerMobSpatial(mob)
	p.show(mob.ID)
	return w, p, mob, st
}

func TestMobKillPersistenceFailureRestoresAllRewardsBeforePublishing(t *testing.T) {
	w, p, mob, st := killRewardTestWorld(t, errors.New("database unavailable"))
	p.Char.Equip[mountSlot] = model.Item{Index: model.MountBabyBase}
	p.Char.Equip[mountSlot].SetMountHP(100)
	beforeChar := cloneCharacterState(p.Char)
	beforePackets := p.Session.QueuedPacketsForTest()

	w.killMobState(p, mob, 500, 37)

	if st.saves != 1 || st.queuedAtSave != beforePackets {
		t.Fatalf("reward published before commit: saves=%d queuedAtSave=%d before=%d",
			st.saves, st.queuedAtSave, beforePackets)
	}
	if p.Char.Exp != beforeChar.Exp || p.Char.Gold != beforeChar.Gold ||
		p.Char.Inv != beforeChar.Inv || p.Char.Equip != beforeChar.Equip {
		t.Fatalf("failed kill was not rolled back: before=%+v after=%+v", beforeChar, *p.Char)
	}
	// Nenhuma morte/recompensa e confirmada. Um unico SetMobHpMp corrige nos
	// observadores o hit letal que o client ja animou antes do commit falhar.
	if got := p.Session.QueuedPacketsForTest(); got != beforePackets+1 {
		t.Fatalf("failed reward emitted unexpected packets: got=%d want=%d", got, beforePackets+1)
	}
	if !p.PersistencePoisoned || !p.Session.IsClosed() {
		t.Fatal("account was not isolated after a durable commit failure")
	}
	if mob.Dead || mob.HP != 37 || w.mobsByID[mob.ID] != mob {
		t.Fatalf("mob was not restored after reward rollback: dead=%v hp=%d indexed=%v",
			mob.Dead, mob.HP, w.mobsByID[mob.ID] == mob)
	}
}

func TestBossDropIsPersistedInsideKillRewardCommit(t *testing.T) {
	w, p, mob, st := killRewardTestWorld(t, nil)
	mob.Def.Carry = nil
	state := &bossSpawnState{mobID: mob.ID, config: model.BossConfig{
		ID:    "transactional-boss",
		Drops: []model.BossDrop{{Item: 500, ChancePercent: 100, Amount: 1}},
	}}
	w.bossSpawns = []*bossSpawnState{state}
	beforePackets := p.Session.QueuedPacketsForTest()

	w.killMobState(p, mob, 500, 100)

	if st.saves != 1 || st.queuedAtSave != beforePackets {
		t.Fatalf("boss reward published before save: saves=%d queued=%d", st.saves, st.queuedAtSave)
	}
	if len(st.saved) != 1 || len(st.saved[0].Chars) == 0 || st.saved[0].Chars[0].Inv[0].Index != 500 {
		t.Fatalf("boss drop absent from committed account snapshot: %+v", st.saved)
	}
	if p.Char.Inv[0].Index != 500 || state.mobID != 0 {
		t.Fatalf("boss kill did not finalize: item=%d mobID=%d", p.Char.Inv[0].Index, state.mobID)
	}
	if p.Session.QueuedPacketsForTest() <= beforePackets+2 {
		t.Fatal("committed boss reward was not published")
	}
}

func TestInstanceKillPersistenceFailureDoesNotAdvanceRoom(t *testing.T) {
	w, leader, _, st, _ := instanceTestWorld()
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil || inst.Remaining != 1 {
		t.Fatalf("instancia inicial invalida: %+v", inst)
	}
	var mob *Mob
	for id := range inst.MobIDs {
		mob = w.mobsByID[id]
	}
	if mob == nil {
		t.Fatal("mob da instancia ausente")
	}
	st.err = errors.New("database unavailable")
	mob.HP = 0
	w.killMobState(leader, mob, 100, 100)

	if inst.Remaining != 1 || inst.RewardGranted || !inst.ExitAt.IsZero() {
		t.Fatalf("falha de DB avancou a instancia: remaining=%d reward=%v exit=%v",
			inst.Remaining, inst.RewardGranted, inst.ExitAt)
	}
	if mob.Dead || mob.HP != 100 || w.mobsByID[mob.ID] != mob {
		t.Fatalf("mob da instancia nao foi restaurado: dead=%v hp=%d", mob.Dead, mob.HP)
	}
}
