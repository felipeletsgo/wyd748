package game

import (
	"testing"

	"wydgo/internal/model"
)

func switchStoneAccount() *model.Account {
	acc := &model.Account{Name: "stones", Chars: make([]model.Char, 4)}
	acc.Chars[0] = model.Char{Name: "OldNick", UID: "a"}
	acc.Chars[1] = model.Char{Name: "NewNick", UID: "b"}
	acc.Chars[2] = model.Char{Name: "Other", UID: "c"}
	acc.Chars[0].Equip[13] = model.Item{Index: masterSwitchStone, UID: "m"}
	acc.Chars[1].Equip[13] = model.Item{Index: slaveSwitchStone, UID: "s"}
	return acc
}

func TestExchangeStonesSwapNamesAndVanish(t *testing.T) {
	acc := switchStoneAccount()
	master, slave, reason, ok := planSwitchStones(acc)
	if !ok || master != 0 || slave != 1 || reason != "" {
		t.Fatalf("plan=%d,%d %q %t", master, slave, reason, ok)
	}
	applySwitchStones(acc, master, slave)
	if acc.Chars[0].Name != "NewNick" || acc.Chars[1].Name != "OldNick" || acc.Chars[2].Name != "Other" ||
		acc.Chars[0].Equip[13].Index != 0 || acc.Chars[1].Equip[13].Index != 0 ||
		acc.Chars[0].UID != "a" || acc.Chars[1].UID != "b" {
		t.Fatalf("swap result: %+v / %+v", acc.Chars[0].Name, acc.Chars[1].Name)
	}
}

func TestExchangeStonesRequireBothStonesAndNoGuild(t *testing.T) {
	single := switchStoneAccount()
	single.Chars[1].Equip[13] = model.Item{}
	if _, _, reason, ok := planSwitchStones(single); ok || reason != "" {
		t.Fatalf("one stone must wait silently: %q %t", reason, ok)
	}
	twoMasters := switchStoneAccount()
	twoMasters.Chars[2].Equip[13] = model.Item{Index: masterSwitchStone}
	if _, _, reason, ok := planSwitchStones(twoMasters); ok || reason == "" {
		t.Fatal("two Greater Exchange Stones were accepted")
	}
	otherSlot := switchStoneAccount()
	otherSlot.Chars[1].Equip[13] = model.Item{}
	otherSlot.Chars[1].Equip[12] = model.Item{Index: slaveSwitchStone}
	if _, _, _, ok := planSwitchStones(otherSlot); ok {
		t.Fatal("a stone outside Equip[13] was accepted")
	}
	guild := switchStoneAccount()
	guild.Chars[1].GuildID = 5
	if _, _, reason, ok := planSwitchStones(guild); ok || reason == "" {
		t.Fatal("a guild member was renamed")
	}
}
