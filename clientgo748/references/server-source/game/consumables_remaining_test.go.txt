package game

import (
	"errors"
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
