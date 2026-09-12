package game

import (
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
)

// itemInstanceRewardReceipt existe somente entre as duas fronteiras
// duraveis do encadeamento Water. O UID nunca vai ao wire.
type itemInstanceRewardReceipt struct {
	Slot      byte
	ItemIndex uint16
	ItemUID   string
}

func (r itemInstanceRewardReceipt) valid() bool {
	return r.ItemIndex != 0 && strings.TrimSpace(r.ItemUID) != "" &&
		int(r.Slot) < model.PlayerCarrySlots
}

// useWaterScroll e a unica entrada de dominio para pergaminhos Water.
// O clique do client e o auto-avanco da Silver Angel convergem aqui;
// apenas o primeiro nasce de um pacote 0x373. expectedUID vazio e uso
// manual, enquanto o caminho automatico exige a instancia exata recem
// concedida pelo servidor.
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
	before := w.itemInstanceForPlayer(p.ID)
	w.useInstanceTicket(s, p, item, slot, rule, req)
	after := w.itemInstanceForPlayer(p.ID)
	return after != nil && after != before
}

// tryWaterAutoAdvance roda uma unica vez, imediatamente depois que o
// grant do proximo scroll foi persistido. Nao varre inventario, nao
// cria pacote sintetico e nao tenta novamente em ticker.
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
	// Mesma fronteira comportamental do clique manual. O request aqui e
	// apenas um valor de dominio; nenhum frame 0x373 e fabricado.
	w.cancelTrade(leader, "Water auto-advance")
	req := useItemRequest{srcType: placeInv, srcPos: uint32(receipt.Slot)}
	w.useWaterScroll(leader.Session, leader, item, receipt.Slot, rule, req, receipt.ItemUID)
}
