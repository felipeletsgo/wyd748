package game

import (
	"errors"
	"testing"
	"time"

	"wydgo/internal/model"
)

func fairyTestDef(index uint16, days int) model.ItemDef {
	def := model.ItemDef{Index: index}
	if days > 0 {
		def.StaticEffects = []model.StaticEffect{
			{Name: "EF_WDAY", Value: days},
			{Name: "EF_HOUR", Value: 0},
			{Name: "EF_MIN", Value: 0},
		}
	}
	return def
}

func fairyTestWorld() *World {
	items := make(map[uint16]model.ItemDef)
	for _, index := range []uint16{3900, 3901, 3902, 3903, 3904, 3905, 3906, 3907, 3908} {
		days := 3
		if index >= 3903 && index <= 3905 {
			days = 5
		}
		if index >= 3906 {
			days = 7
		}
		items[index] = fairyTestDef(index, days)
	}
	items[3911] = fairyTestDef(3911, 7)
	items[3912] = fairyTestDef(3912, 15)
	items[3913] = fairyTestDef(3913, 30)
	items[3914] = fairyTestDef(3914, 7)
	items[3915] = fairyTestDef(3915, 7)
	return &World{items: items}
}

func TestFairyBonusMatrix(t *testing.T) {
	w := fairyTestWorld()
	for _, tc := range []struct {
		name      string
		index     uint16
		exp, drop int
	}{
		{name: "green", index: 3900, exp: 16},
		{name: "blue", index: 3901, drop: 32},
		{name: "red", index: 3902, exp: 8, drop: 16},
		{name: "silver", index: 3914, exp: 16, drop: 32},
		{name: "gold", index: 3915, exp: 24, drop: 48},
	} {
		t.Run(tc.name, func(t *testing.T) {
			ch := &model.Char{}
			ch.Equip[fairySlot] = model.Item{Index: tc.index}
			got := w.activeFairyBonus(ch)
			if got.expPercent != tc.exp || got.dropPercent != tc.drop {
				t.Fatalf("bonus=%+v, want exp=%d drop=%d", got, tc.exp, tc.drop)
			}
		})
	}
}

func TestFairyFirstEquipUsesUIDBoundAbsoluteDeadline(t *testing.T) {
	w := fairyTestWorld()
	now := time.Unix(1_700_000_000, 0).UTC()
	item := model.Item{Index: 3900, UID: "11111111111141118111111111111111"}
	changed, err := w.activateTimedItem(&item, fairySlot, now)
	if err != nil || !changed {
		t.Fatalf("activation changed=%t err=%v", changed, err)
	}
	if item.ActivatedUnix != now.Unix() || item.ExpiresUnix != now.Add(3*24*time.Hour).Unix() {
		t.Fatalf("unexpected deadline: %+v", item)
	}
	before := item
	changed, err = w.activateTimedItem(&item, fairySlot, now.Add(time.Hour))
	if err != nil || changed || item != before {
		t.Fatalf("second equip reset deadline: changed=%t err=%v item=%+v", changed, err, item)
	}
}

func TestFairyDeadlineContinuesUnequippedAndOffline(t *testing.T) {
	w := fairyTestWorld()
	now := time.Unix(1_700_000_000, 0).UTC()
	item := model.Item{Index: 3902, UID: "22222222222242228222222222222222"}
	if _, err := w.activateTimedItem(&item, fairySlot, now); err != nil {
		t.Fatal(err)
	}
	// The deadline belongs to the UID, not to a Player clock or an equipment
	// slot. Moving/relogging cannot pause or reset it.
	copyInInventory := item
	if copyInInventory.ExpiresUnix != now.Add(3*24*time.Hour).Unix() {
		t.Fatalf("deadline changed outside equipment: %+v", copyInInventory)
	}
	if _, err := w.activateTimedItem(&copyInInventory, fairySlot, now.Add(4*24*time.Hour)); !errors.Is(err, errTimedItemExpired) {
		t.Fatalf("expired fairy accepted after offline time: %v", err)
	}
}

func TestExpiredFairyProvidesNoBonus(t *testing.T) {
	w := fairyTestWorld()
	w.clock = newFakeClock(time.Unix(1_700_000_000, 0))
	ch := &model.Char{}
	ch.Equip[fairySlot] = model.Item{Index: 3915, ActivatedUnix: 1, ExpiresUnix: 2}
	if got := w.activeFairyBonus(ch); got != (fairyBonus{}) {
		t.Fatalf("expired fairy bonus=%+v", got)
	}
}

func TestDirectSilverUsesCatalogDuration(t *testing.T) {
	w := fairyTestWorld()
	ch := &model.Char{}
	ch.Equip[fairySlot] = model.Item{Index: 3914}
	if got := w.activeFairyBonus(ch); got != (fairyBonus{expPercent: 16, dropPercent: 32}) {
		t.Fatalf("direct silver bonus=%+v", got)
	}
	if !w.hasActiveSilverFairy(ch) {
		t.Fatal("active silver fairy must enable Water automation")
	}
}
