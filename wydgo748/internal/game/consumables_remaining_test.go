package game

import (
	"errors"
	"reflect"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestLootBoxMaterializesRewardAndRollsBack(t *testing.T) {
	rule := model.VolatileRule{
		Action: "loot_box", Consume: true, RewardItems: []uint16{200},
	}
	w, p, st := useItemWorld(rule)
	w.items[200] = model.ItemDef{Index: 200, Name: "Reward"}

	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0].Index != 200 || p.Char.Inv[0].UID == "" || st.saves != 1 {
		t.Fatalf("caixa: item=%+v saves=%d", p.Char.Inv[0], st.saves)
	}

	p.Char.Inv[0] = model.Item{Index: 100}
	st.err = errors.New("disk")
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0].Index != 100 {
		t.Fatal("falha de persistencia consumiu a caixa")
	}
}

func TestLootBoxKeepsSourceWhenNoRewardIsConfigured(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{Action: "loot_box", Consume: true})
	before := p.Char.Inv[0]

	w.onUseItem(p.Session, useItemPacket(0, 0))

	if p.Char.Inv[0] != before || st.saves != 0 {
		t.Fatalf("empty reward list consumed or persisted the source: item=%+v saves=%d", p.Char.Inv[0], st.saves)
	}
}

func TestLootBoxRestoresSourceWhenInventoryIsFull(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{
		Action: "loot_box", Consume: true, RewardItems: []uint16{200},
	})
	w.items[200] = model.ItemDef{Index: 200, Name: "Reward"}
	p.Char.Inv[0].Eff[0], p.Char.Inv[0].Eff[1] = effectAmount, 2
	for slot := 1; slot < model.PlayerCarrySlots; slot++ {
		p.Char.Inv[slot] = model.Item{Index: 300}
	}
	before := p.Char.Inv[0]

	w.onUseItem(p.Session, useItemPacket(0, 0))

	if p.Char.Inv[0] != before || st.saves != 0 {
		t.Fatalf("full inventory did not restore the source stack: item=%+v saves=%d", p.Char.Inv[0], st.saves)
	}
}

func TestLootBoxPlacesRewardInEarlierFreeSlot(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{
		Action: "loot_box", Consume: true, RewardItems: []uint16{200},
	})
	w.items[200] = model.ItemDef{Index: 200, Name: "Reward"}
	p.Char.Inv[1] = p.Char.Inv[0]
	p.Char.Inv[0] = model.Item{}

	w.onUseItem(p.Session, useItemPacket(1, 1))

	if p.Char.Inv[0].Index != 200 || p.Char.Inv[0].UID == "" || p.Char.Inv[1].Index != 0 || st.saves != 1 {
		t.Fatalf("reward was not committed to the earlier free slot: inv0=%+v inv1=%+v saves=%d",
			p.Char.Inv[0], p.Char.Inv[1], st.saves)
	}
}

func TestLootBoxInitializesFreshMountReward(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{
		Action: "loot_box", Consume: true, RewardItems: []uint16{model.MountAdultBase},
	})
	w.items[model.MountAdultBase] = model.ItemDef{Index: model.MountAdultBase, Name: "Pig"}
	w.rng = fixedRNG{value: 4}

	w.onUseItem(p.Session, useItemPacket(0, 0))

	reward := p.Char.Inv[0]
	if reward.Index != model.MountAdultBase || reward.UID == "" || reward.MountHP() != model.MountMaxHP ||
		reward.MountLongev() != 14 || reward.MountFood() != model.MountMaxFood || st.saves != 1 {
		t.Fatalf("loot-box mount was not initialized: reward=%+v saves=%d", reward, st.saves)
	}
}

func TestLootAndMountItemsRespectCriticalAccountFence(t *testing.T) {
	t.Run("loot box", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{
			Action: "loot_box", Consume: true, RewardItems: []uint16{200},
		})
		w.items[200] = model.ItemDef{Index: 200, Name: "Reward"}
		fenceConsumableAccount(w, p)
		before := p.Char.Inv[0]
		packets := p.Session.QueuedPacketsForTest()

		w.onUseItem(p.Session, useItemPacket(0, 0))
		w.useLootBox(p.Session, p, &p.Char.Inv[0], 0, model.VolatileRule{
			Action: "loot_box", Consume: true, RewardItems: []uint16{200},
		})

		if p.Char.Inv[0] != before || st.saves != 0 || p.Session.QueuedPacketsForTest() != packets {
			t.Fatalf("fenced loot-box use changed state or published packets: item=%+v saves=%d",
				p.Char.Inv[0], st.saves)
		}
	})

	t.Run("mount revive", func(t *testing.T) {
		w, p, st := useItemWorld(model.VolatileRule{Action: "mount_revive", Consume: true})
		mount := mountWithState(0, 20)
		p.Char.Equip[mountSlot] = mount
		p.Char.Gold = 1_000
		w.items[mount.Index] = model.ItemDef{Index: mount.Index, Price: 100}
		fenceConsumableAccount(w, p)
		before := p.Char.Inv[0]
		packets := p.Session.QueuedPacketsForTest()

		w.onUseItem(p.Session, useItemPacket(0, 0))
		w.useMountRevive(p.Session, p, &p.Char.Inv[0], 0, model.VolatileRule{
			Action: "mount_revive", Consume: true,
		})

		if p.Char.Inv[0] != before || p.Char.Equip[mountSlot] != mount || p.Char.Gold != 1_000 ||
			st.saves != 0 || p.Session.QueuedPacketsForTest() != packets {
			t.Fatalf("fenced mount revival changed state or published packets: mount=%+v item=%+v gold=%d saves=%d",
				p.Char.Equip[mountSlot], p.Char.Inv[0], p.Char.Gold, st.saves)
		}
	})
}

func TestLootBoxUsesCriticalPersistenceAdmission(t *testing.T) {
	w, p, _, fallback, base, queue, release := criticalTreasuryFixture(t, 100)
	rule, _ := configureCriticalConsumableSettlement(w, p, "loot_box")
	before := accountStateSnapshot(p.Account)

	w.useLootBox(p.Session, p, &p.Char.Inv[0], 0, rule)
	proposal := awaitCriticalWrite(t, base.entered)
	prepared := proposal.accounts[0].Chars[p.CharSlot]
	if prepared.Inv[0].Index != 901 || prepared.Inv[0].UID == "" ||
		!reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
		t.Fatal("critical loot-box admission did not retain an uninstalled reward proposal")
	}
	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Inv[0].Index != 901 || p.Char.Inv[0].UID == "" || len(queue.pending) != 0 ||
		w.criticalPlayerReserved(p) || fallback.saves != 0 || fallback.gameSaves != 0 {
		t.Fatalf("critical loot-box commit failed: item=%+v pending=%d saves=%d gameSaves=%d",
			p.Char.Inv[0], len(queue.pending), fallback.saves, fallback.gameSaves)
	}
}

func TestMountReviveUsesCriticalPersistenceAdmission(t *testing.T) {
	w, p, _, fallback, base, queue, release := criticalTreasuryFixture(t, 100)
	rule, _ := configureCriticalConsumableSettlement(w, p, "mount_revive")
	before := accountStateSnapshot(p.Account)

	w.useMountRevive(p.Session, p, &p.Char.Inv[0], 0, rule)
	proposal := awaitCriticalWrite(t, base.entered)
	prepared := proposal.accounts[0].Chars[p.CharSlot]
	if prepared.Inv[0].Index != 0 || prepared.Equip[mountSlot].MountHP() <= 0 || prepared.Gold != p.Char.Gold-100 ||
		!reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
		t.Fatal("critical mount-revival admission did not retain an uninstalled recovery proposal")
	}

	release()
	awaitReadyCriticalResults(t, queue, 1)
	w.processCriticalCompletions(1)
	if p.Char.Inv[0].Index != 0 || p.Char.Equip[mountSlot].MountHP() <= 0 || p.Char.Gold != 900 ||
		len(queue.pending) != 0 || w.criticalPlayerReserved(p) || fallback.saves != 0 || fallback.gameSaves != 0 {
		t.Fatalf("critical mount-revival commit failed: item=%+v mount=%+v gold=%d pending=%d",
			p.Char.Inv[0], p.Char.Equip[mountSlot], p.Char.Gold, len(queue.pending))
	}
}

func TestMountReviveCapsRestoredHPAtTypeMaximum(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{Action: "mount_revive", Consume: true})
	mount := mountWithState(0, 20)
	p.Char.Equip[mountSlot] = mount
	p.Char.Gold = 1_000
	w.items[mount.Index] = model.ItemDef{Index: mount.Index, Price: 100}
	w.mounts = model.MountCatalog{Types: map[int]model.MountStats{
		model.MountType(mount.Index): {MaxHP: 10_000},
	}}
	w.rng = fixedRNG{value: 0}

	w.onUseItem(p.Session, useItemPacket(0, 0))

	got := p.Char.Equip[mountSlot]
	if got.MountHP() != 10_000 || got.MountFood() < 5 || p.Char.Gold != 900 || st.saves != 1 {
		t.Fatalf("capped revival: mount=%+v gold=%d saves=%d", got, p.Char.Gold, st.saves)
	}
}

func TestMountReviveRejectsInvalidTargetAndInsufficientGold(t *testing.T) {
	tests := []struct {
		name  string
		mount model.Item
		def   bool
		gold  uint32
	}{
		{name: "no mount"},
		{name: "living mount", mount: mountWithState(10_000, 20)},
		{name: "dead mount without longevity", mount: mountWithState(0, 0)},
		{name: "missing mount definition", mount: mountWithState(0, 20), gold: 1_000},
		{name: "insufficient gold", mount: mountWithState(0, 20), def: true, gold: 99},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			w, p, st := useItemWorld(model.VolatileRule{Action: "mount_revive", Consume: true})
			if tt.mount.Index != 0 {
				p.Char.Equip[mountSlot] = tt.mount
				if tt.def {
					w.items[tt.mount.Index] = model.ItemDef{Index: tt.mount.Index, Price: 100}
				}
			}
			p.Char.Gold = tt.gold
			beforeMount := p.Char.Equip[mountSlot]
			beforeItem := p.Char.Inv[0]

			w.onUseItem(p.Session, useItemPacket(0, 0))

			if p.Char.Equip[mountSlot] != beforeMount || p.Char.Inv[0] != beforeItem ||
				p.Char.Gold != tt.gold || st.saves != 0 {
				t.Fatalf("rejected revival changed state: mount=%+v item=%+v gold=%d saves=%d",
					p.Char.Equip[mountSlot], p.Char.Inv[0], p.Char.Gold, st.saves)
			}
		})
	}
}

func TestMountReviveRemovesMountWhenLastLifePointIsSpent(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{Action: "mount_revive", Consume: true})
	mount := mountWithState(0, 1)
	p.Char.Equip[mountSlot] = mount
	p.Char.Gold = 1_000
	w.items[mount.Index] = model.ItemDef{Index: mount.Index, Price: 100}
	w.rng = fixedRNG{value: 2}

	w.onUseItem(p.Session, useItemPacket(0, 0))

	if p.Char.Equip[mountSlot] != (model.Item{}) || p.Char.Inv[0].Index != 0 || p.Char.Gold != 900 || st.saves != 1 {
		t.Fatalf("last-life revival outcome: mount=%+v item=%+v gold=%d saves=%d",
			p.Char.Equip[mountSlot], p.Char.Inv[0], p.Char.Gold, st.saves)
	}
}

func TestMountReviveRestoresStateWhenSaveFails(t *testing.T) {
	w, p, st := useItemWorld(model.VolatileRule{Action: "mount_revive", Consume: true})
	mount := mountWithState(0, 20)
	p.Char.Equip[mountSlot] = mount
	p.Char.Gold = 1_000
	w.items[mount.Index] = model.ItemDef{Index: mount.Index, Price: 100}
	w.rng = fixedRNG{value: 0}
	st.err = errors.New("database unavailable")

	w.onUseItem(p.Session, useItemPacket(0, 0))

	if p.Char.Equip[mountSlot] != mount || p.Char.Inv[0].Index != 100 || p.Char.Gold != 1_000 || st.saves != 1 {
		t.Fatalf("failed save did not restore mount, item and gold: mount=%+v item=%+v gold=%d saves=%d",
			p.Char.Equip[mountSlot], p.Char.Inv[0], p.Char.Gold, st.saves)
	}
}

func mountWithState(hp, longevity int) model.Item {
	mount := model.Item{Index: model.MountAdultBase}
	mount.SetMountHP(hp)
	mount.SetMountLongev(longevity)
	return mount
}

func fenceConsumableAccount(w *World, p *Player) {
	key := accountSessionKey(p.Account.Name)
	w.criticalWrites = &criticalPersistenceOwner{queue: &criticalPersistence{
		failedAccounts: map[string]uint64{key: 1},
	}}
}

func TestTimedAccessDoesNotConsumeDuplicateAndRollsBack(t *testing.T) {
	rule := model.VolatileRule{
		Action: "timed_access", Consume: true,
		AccessKey: "hidden_ruins", DurationSeconds: 3600,
	}
	w, p, st := useItemWorld(rule)
	clock := newFakeClock(time.Unix(2_000_000_000, 0))
	w.clock = clock

	w.onUseItem(p.Session, useItemPacket(0, 0))
	want := uint32(clock.Now().Add(time.Hour).Unix())
	if p.Char.Inv[0].Index != 0 ||
		p.SpecialCoins[accessCounterPrefix+"hidden_ruins"] != want {
		t.Fatalf("acesso nao ativado: item=%d counters=%v", p.Char.Inv[0].Index, p.SpecialCoins)
	}

	p.Char.Inv[0] = model.Item{Index: 100}
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0].Index != 100 || st.saves != 1 {
		t.Fatal("acesso duplicado consumiu/persistiu o item")
	}

	p.SpecialCoins = nil
	st.err = errors.New("disk")
	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.Char.Inv[0].Index != 100 || p.SpecialCoins != nil {
		t.Fatal("falha de persistencia deixou acesso/item alterado")
	}
}

func TestTimedAccessRejectsExpiryOutsidePersistedRange(t *testing.T) {
	tests := []struct {
		name string
		now  time.Time
	}{
		{name: "nonpositive expiry", now: time.Unix(-10, 0)},
		{name: "expiry exceeds uint32", now: time.Unix(int64(^uint32(0))-1, 0)},
	}
	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			rule := model.VolatileRule{Action: "timed_access", Consume: true, AccessKey: "boundary", DurationSeconds: 2}
			w, p, st := useItemWorld(rule)
			w.clock = newFakeClock(tt.now)
			before := p.Char.Inv[0]

			w.onUseItem(p.Session, useItemPacket(0, 0))

			if p.Char.Inv[0] != before || p.SpecialCoins != nil || st.saves != 0 {
				t.Fatalf("invalid expiry changed item or counters: item=%+v counters=%v saves=%d",
					p.Char.Inv[0], p.SpecialCoins, st.saves)
			}
		})
	}
}

func TestTimedAccessDoesNotMutateWhenCriticalAccountIsFenced(t *testing.T) {
	rule := model.VolatileRule{Action: "timed_access", Consume: true, AccessKey: "fenced", DurationSeconds: 60}
	w, p, st := useItemWorld(rule)
	fenceConsumableAccount(w, p)
	before := p.Char.Inv[0]

	w.useTimedAccess(p.Session, p, &p.Char.Inv[0], 0, rule)

	if p.Char.Inv[0] != before || p.SpecialCoins != nil || st.saves != 0 {
		t.Fatalf("fenced timed access changed item or counters: item=%+v counters=%v saves=%d",
			p.Char.Inv[0], p.SpecialCoins, st.saves)
	}
}

func TestCounterUnlockRewardsOnce(t *testing.T) {
	rule := model.VolatileRule{
		Action: "grant_counter_once", Consume: true,
		Counters: map[string]uint32{"event_pass": 1}, RewardItems: []uint16{200},
	}
	w, p, st := useItemWorld(rule)
	w.items[200] = model.ItemDef{Index: 200, Name: "Pass"}

	w.onUseItem(p.Session, useItemPacket(0, 0))
	if p.SpecialCoins["event_pass"] != 1 || p.Char.Inv[0].Index != 200 ||
		p.Char.Inv[0].UID == "" || st.saves != 1 {
		t.Fatalf("unlock incorreto: counters=%v inv=%+v saves=%d",
			p.SpecialCoins, p.Char.Inv[0], st.saves)
	}

	p.Char.Inv[1] = model.Item{Index: 100}
	w.onUseItem(p.Session, useItemPacket(1, 1))
	if p.Char.Inv[1].Index != 100 || st.saves != 1 {
		t.Fatal("unlock repetido consumiu/persistiu o segundo item")
	}
}
