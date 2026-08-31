package game

import (
	"fmt"
	"testing"

	"wydgo/internal/wire"
)

// TestWorldDispatchRoutesEveryGameplayOpcode e um contrato do roteador, nao um
// substituto para os testes dos handlers. Cada opcode conhecido atravessa
// framing, fase e switch usando um mundo isolado; qualquer panic, rota removida
// ou tamanho autoritativo divergente quebra indicando o opcode exato.
func TestWorldDispatchRoutesEveryGameplayOpcode(t *testing.T) {
	tests := []struct {
		opcode uint16
		size   int
	}{
		{wire.OpCharacterLogout, 12},
		{wire.OpSwapItem, 20},
		{wire.OpDeposit, 16},
		{wire.OpWithdraw, 16},
		{wire.OpUseItem, 36},
		{wire.OpUsePremiumFirework, premiumFireworkUsePacketSize},
		{wire.OpUseNPC, 20},
		{wire.OpReqShopList, 16},
		{wire.OpBuyItem, 24},
		{wire.OpBuyToto, 36},
		{wire.OpSellItem, 20},
		{wire.OpApplyBonus, applyBonusPacketSize},
		{wire.OpPartyRequest, 44},
		{wire.OpPartyAccept, 32},
		{wire.OpPartyRemove, 16},
		{wire.OpTrade, 156},
		{wire.OpCloseTrade, 12},
		{wire.OpAutoTrade, 196},
		{wire.OpReqTradeList, 16},
		{wire.OpReqBuyAutoTrade, 36},
		{wire.OpDropItem, 32},
		{wire.OpGetItem, 28},
		{wire.OpDeleteItem, deleteItemPacketSize},
		{wire.OpSplitItem, splitItemPacketSize},
		{wire.OpUpdateItem, 20},
		{wire.OpMessageChat, 140},
		{wire.OpMessageWhisper, 128},
		{wire.OpSetShortSkill, 32},
		{wire.OpChangeCity, 16},
		{wire.OpReqTeleport, 16},
		{wire.OpPKMode, 16},
		{wire.OpGuildDeprivate, 16},
		{wire.OpInviteGuild, 20},
		{wire.OpRebuy, repurchasePacketSize},
		{wire.OpGuildAlly, 20},
		{wire.OpGuildWar, 20},
		{wire.OpChallenge, 16},
		{wire.OpChallengeConfirm, 20},
		{wire.OpMoveStop, 36},
		{wire.OpRestart, 12},
		{wire.OpPing, 12},
		{wire.OpUpdateScore, wire.HeaderSize},
		{wire.OpSysQuit, 16},
		{wire.OpAction, 52},
		{wire.OpActionStop, 52},
		{wire.OpIllusion, 52},
		{wire.OpREQMobByID, 16},
		{wire.OpMotion, 20},
		{wire.OpClientUnknown2BC, 108},
		{wire.OpAttackOne, 48},
		{wire.OpAttackTwo, 52},
		{wire.OpAttackMulti, 96},
		{wire.OpReqRanking, 20},
		{wire.OpCombineTiny, combinePacketSize},
		{wire.OpCombineLindy, combinePacketSize},
		{wire.OpCombineCompositor, combinePacketSize},
		{wire.OpCombineAgatha, combinePacketSize},
		{wire.OpCombineAylin, combinePacketSize},
		{wire.OpCombineEhre, combinePacketSize},
		{wire.OpCombineOdin, combinePacketSize},
	}
	for _, tc := range tests {
		tc := tc
		t.Run(fmt.Sprintf("opcode_0x%X", tc.opcode), func(t *testing.T) {
			if tc.opcode != 0xFFFF && tc.opcode != wire.OpRebuy {
				if size, exact := exactInboundPacketSize(tc.opcode); !exact || size != tc.size {
					t.Fatalf("handler sem framing exato: size=%d exact=%v, esperado=%d",
						size, exact, tc.size)
				}
			}
			w, p, _ := handlerTestWorld(t)
			w.handle(command{s: p.Session, pkt: inboundPacket(tc.opcode, tc.size)})
		})
	}
}

func TestUnknownWorldOpcodeRejectedBeforeDispatch(t *testing.T) {
	w, p, _ := handlerTestWorld(t)
	w.handle(command{s: p.Session, pkt: inboundPacket(0xFFFF, wire.HeaderSize)})
	state := w.security[p.Session]
	if state == nil || state.violations != 1 {
		t.Fatalf("opcode desconhecido nao foi registrado na borda: %#v", state)
	}
	if got := commandLabel(command{pkt: inboundPacket(0xFFFF, wire.HeaderSize)}); got != "unknown" {
		t.Fatalf("label de opcode desconhecido=%q", got)
	}
}
