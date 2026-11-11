package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type legacyInstanceRewardStore struct {
	store.Store
	accountSaves  int
	instanceSaves int
	err           error
}

func (s *legacyInstanceRewardStore) LoadInstanceState() (*model.InstanceStateSnapshot, error) {
	return &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}, nil
}

func (s *legacyInstanceRewardStore) SaveAccount(*model.Account) error {
	s.accountSaves++
	return s.err
}

func (s *legacyInstanceRewardStore) SaveInstanceState(*model.InstanceStateSnapshot) error {
	s.instanceSaves++
	return s.err
}

func instanceRewardCoverageFixture(t *testing.T) (*World, *Player, *ItemInstance, *guildFlowStore, *fakeClock) {
	t.Helper()
	w, leader, _, st, clock := instanceTestWorld()
	w.onUseItem(leader.Session, useItemPacket(0, 0))
	inst := w.itemInstances["water-normal-1"]
	if inst == nil {
		t.Fatal("Water fixture did not create its instance")
	}
	return w, leader, inst, st, clock
}

func TestGrantItemInstanceRewardRejectsMissingStateAndSkipsDuplicates(t *testing.T) {
	w, _, inst, _, clock := instanceRewardCoverageFixture(t)
	now := clock.Now()

	if _, ok := w.grantItemInstanceReward(nil, now); ok {
		t.Fatal("nil instance received a reward")
	}

	inst.RewardGranted = true
	receipt, ok := w.grantItemInstanceReward(inst, now)
	if !ok || receipt != (itemInstanceRewardReceipt{}) {
		t.Fatalf("duplicate reward result=%+v, %v; want an empty successful receipt", receipt, ok)
	}

	inst.RewardGranted = false
	delete(w.playersByID, inst.LeaderID)
	if _, ok := w.grantItemInstanceReward(inst, now); ok {
		t.Fatal("reward was granted without an active leader")
	}
	if inst.RewardGranted || !inst.ExitAt.IsZero() || !inst.ExitDeadline.IsZero() {
		t.Fatal("rejected reward changed the instance completion state")
	}
}

func TestGrantItemInstanceRewardPersistsRewardlessCompletionAndRollsBackFailure(t *testing.T) {
	for _, tc := range []struct {
		name    string
		failure error
	}{
		{name: "committed"},
		{name: "save failure", failure: errors.New("instance state unavailable")},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, _, inst, st, clock := instanceRewardCoverageFixture(t)
			inst.Config.RewardItem = 0
			st.err = tc.failure
			now := clock.Now()

			receipt, ok := w.grantItemInstanceReward(inst, now)
			if receipt != (itemInstanceRewardReceipt{}) {
				t.Fatalf("rewardless completion returned a receipt: %+v", receipt)
			}
			if tc.failure != nil {
				if ok || inst.RewardGranted || !inst.ExitAt.IsZero() || !inst.ExitDeadline.IsZero() {
					t.Fatal("failed state persistence retained the reward marker or exit deadline")
				}
				return
			}
			if !ok || !inst.RewardGranted || !inst.ExitAt.Equal(now.Add(10*time.Second)) ||
				!inst.ExitDeadline.Equal(inst.ExitAt) {
				t.Fatalf("rewardless completion did not persist its exit grace: %+v", inst)
			}
		})
	}
}

func TestGrantItemInstanceRewardRollsBackAccountRewardOnPersistenceFailure(t *testing.T) {
	w, leader, inst, st, clock := instanceRewardCoverageFixture(t)
	st.err = errors.New("account and instance transaction rejected")
	before := accountStateSnapshot(leader.Account)
	now := clock.Now()

	receipt, ok := w.grantItemInstanceReward(inst, now)
	if ok || receipt != (itemInstanceRewardReceipt{}) ||
		!reflect.DeepEqual(accountStateSnapshot(leader.Account), before) || inst.RewardGranted ||
		!inst.ExitAt.IsZero() || !inst.ExitDeadline.IsZero() {
		t.Fatal("failed account reward persistence changed the character or completion state")
	}
}

func TestGrantItemInstanceRewardUsesAndRollsBackGroundFallback(t *testing.T) {
	for _, tc := range []struct {
		name    string
		failure error
	}{
		{name: "committed"},
		{name: "save failure", failure: errors.New("ground reward transaction rejected")},
	} {
		t.Run(tc.name, func(t *testing.T) {
			w, leader, inst, st, clock := instanceRewardCoverageFixture(t)
			for slot := range leader.Char.Inv[:model.PlayerCarrySlots] {
				leader.Char.Inv[slot] = model.Item{Index: uint16(500 + slot)}
			}
			st.err = tc.failure
			now := clock.Now()

			receipt, ok := w.grantItemInstanceReward(inst, now)
			if tc.failure != nil {
				if ok || receipt != (itemInstanceRewardReceipt{}) || len(w.groundItems) != 0 ||
					inst.RewardGranted || !inst.ExitAt.IsZero() || !inst.ExitDeadline.IsZero() {
					t.Fatal("failed ground reward persistence left a visible drop or completion state")
				}
				return
			}
			if !ok || receipt != (itemInstanceRewardReceipt{}) || !inst.RewardGranted ||
				!inst.ExitAt.Equal(now.Add(10*time.Second)) || len(w.groundItems) != 1 {
				t.Fatalf("full-inventory reward did not use ground fallback: ok=%v receipt=%+v instance=%+v drops=%d",
					ok, receipt, inst, len(w.groundItems))
			}
			for _, drop := range w.groundItems {
				if drop.Item.Index != inst.Config.RewardItem || drop.Item.UID == "" ||
					drop.InstanceID != inst.RuntimeID || drop.OwnerUID != leader.Char.UID {
					t.Fatalf("ground reward has wrong identity or ownership: %+v", drop)
				}
			}
		})
	}
}

func TestGrantItemInstanceRewardRejectsUnavailableGroundCell(t *testing.T) {
	w, leader, inst, st, clock := instanceRewardCoverageFixture(t)
	for slot := range leader.Char.Inv[:model.PlayerCarrySlots] {
		leader.Char.Inv[slot] = model.Item{Index: uint16(500 + slot)}
	}
	for y := int(leader.Y) - 1; y <= int(leader.Y)+1; y++ {
		for x := int(leader.X) - 1; x <= int(leader.X)+1; x++ {
			id := uint16(40000 + (y-int(leader.Y)+1)*3 + x - int(leader.X) + 1)
			w.registerGroundItem(&GroundItem{ID: id, X: uint16(x), Y: uint16(y), InstanceID: inst.RuntimeID})
		}
	}
	before := accountStateSnapshot(leader.Account)
	beforeDrops := len(w.groundItems)

	if _, ok := w.grantItemInstanceReward(inst, clock.Now()); ok {
		t.Fatal("reward was granted without an available ground cell")
	}
	if !reflect.DeepEqual(accountStateSnapshot(leader.Account), before) || len(w.groundItems) != beforeDrops ||
		inst.RewardGranted || !inst.ExitAt.IsZero() || !inst.ExitDeadline.IsZero() || st.saves != 0 {
		t.Fatal("unavailable ground cell changed the account, drops, persistence, or completion state")
	}
}

func TestGrantItemInstanceRewardUsesLegacyPersistenceOnlyForNonWaterInstances(t *testing.T) {
	t.Run("ordinary reward uses account save", func(t *testing.T) {
		w, leader, inst, st, clock := instanceRewardCoverageFixture(t)
		legacy := &legacyInstanceRewardStore{Store: st}
		w.store = legacy
		inst.Config.ID = "ordinary-instance"

		receipt, ok := w.grantItemInstanceReward(inst, clock.Now())
		if !ok || receipt.ItemIndex != inst.Config.RewardItem || receipt.ItemUID == "" ||
			leader.Char.Inv[receipt.Slot].UID != receipt.ItemUID || legacy.accountSaves != 1 ||
			legacy.instanceSaves != 0 {
			t.Fatalf("legacy account reward result=%+v ok=%v saves=%d/%d", receipt, ok,
				legacy.accountSaves, legacy.instanceSaves)
		}
	})

	t.Run("water reward refuses split persistence", func(t *testing.T) {
		w, leader, inst, st, clock := instanceRewardCoverageFixture(t)
		legacy := &legacyInstanceRewardStore{Store: st}
		w.store = legacy
		before := accountStateSnapshot(leader.Account)

		if _, ok := w.grantItemInstanceReward(inst, clock.Now()); ok ||
			!reflect.DeepEqual(accountStateSnapshot(leader.Account), before) || inst.RewardGranted ||
			!inst.ExitAt.IsZero() || legacy.accountSaves != 0 {
			t.Fatal("legacy store split the durable Water reward from its instance state")
		}
	})

	t.Run("rewardless completion saves instance state", func(t *testing.T) {
		w, _, inst, st, clock := instanceRewardCoverageFixture(t)
		legacy := &legacyInstanceRewardStore{Store: st}
		w.store = legacy
		inst.Config.RewardItem = 0

		if _, ok := w.grantItemInstanceReward(inst, clock.Now()); !ok ||
			!inst.RewardGranted || legacy.accountSaves != 0 || legacy.instanceSaves != 1 {
			t.Fatalf("legacy rewardless completion ok=%v stateSaves=%d accountSaves=%d",
				inst.RewardGranted, legacy.instanceSaves, legacy.accountSaves)
		}
	})
}
