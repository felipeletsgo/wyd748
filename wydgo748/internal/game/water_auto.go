package game

import (
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

// The reward receipt exists only between the two durable Water chain
// boundaries. The item UID is never sent on the wire.
type itemInstanceRewardReceipt struct {
	Slot      byte
	ItemIndex uint16
	ItemUID   string
}

func (r itemInstanceRewardReceipt) valid() bool {
	return r.ItemIndex != 0 && strings.TrimSpace(r.ItemUID) != "" &&
		int(r.Slot) < model.PlayerCarrySlots
}

// Manual clicks and Silver Angel auto-advance share this Water entry point.
// Only manual use originates from 0x373. Auto-advance requires the exact UID
// of the newly granted item; an empty expectedUID denotes manual use.
func (w *World) useWaterScroll(s *net.Session, p *Player, item *model.Item,
	slot byte, rule model.VolatileRule, req useItemRequest, expectedUID string) bool {
	if w == nil || s == nil || p == nil || p.Char == nil || item == nil ||
		rule.Instance == nil || !isDurablePrivateWaterConfig(*rule.Instance) ||
		int(slot) >= model.PlayerCarrySlots {
		return false
	}
	if expectedUID != "" && item.UID != expectedUID {
		return false
	}
	if w.criticalWrites != nil {
		return w.admitCriticalInstanceTicket(p, slot, rule, expectedUID)
	}
	before := w.itemInstanceForPlayer(p.ID)
	w.useInstanceTicket(s, p, item, slot, rule, req)
	after := w.itemInstanceForPlayer(p.ID)
	return after != nil && after != before
}

// Auto-advance runs once after the next scroll grant is durably committed.
// It never scans inventory, fabricates a packet or retries from a ticker.
func (w *World) tryWaterAutoAdvance(inst *ItemInstance,
	receipt itemInstanceRewardReceipt, now time.Time) {
	if w == nil || inst == nil || !receipt.valid() ||
		!isDurablePrivateWaterInstance(inst) ||
		inst.Config.RewardItem != receipt.ItemIndex ||
		expectedChainItem(&inst.Config) != receipt.ItemIndex ||
		!itemInstanceInExitGraceAt(inst, now) {
		return
	}
	leader := w.playersByID[inst.LeaderID]
	if leader == nil || !leader.InWorld || leader.Char == nil ||
		leader.Account == nil || leader.Session == nil || playerCurHP(leader.Char) == 0 ||
		!w.hasActiveSilverFairy(leader.Char) {
		return
	}
	item := &leader.Char.Inv[receipt.Slot]
	if item.Index != receipt.ItemIndex || item.UID != receipt.ItemUID {
		return
	}
	rule, _, registered := w.volatiles.Rule(item.Index)
	if !registered || rule.Action != "instance_ticket" || !rule.Consume ||
		rule.Instance == nil || !isDurablePrivateWaterConfig(*rule.Instance) {
		return
	}
	// This is a domain request, not a fabricated 0x373 frame. The critical
	// path retains the trade until settlement rather than closing it early.
	if w.criticalWrites == nil {
		w.cancelTrade(leader, "Water auto-advance")
	}
	req := useItemRequest{srcType: placeInv, srcPos: uint32(receipt.Slot)}
	w.useWaterScroll(leader.Session, leader, item, receipt.Slot, rule, req, receipt.ItemUID)
}
