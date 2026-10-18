package game

import (
	"fmt"
	"strings"

	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const balanceNoticeBytes = 95

// MODERNIZACAO_COMPATIVEL: read-only self-service commands reuse top notice 0x101.
// The World owns these live, acknowledged account/character balances. Do not
// reload storage over live state or include pending donate credits as spendable.
func (w *World) sendBalanceNotice(s *net.Session, p *Player, arg, command string) {
	if s == nil || p == nil || p.Account == nil || p.Char == nil || !p.InWorld {
		return
	}
	if !w.allowChat(p, command, w.now()) {
		return
	}
	if strings.TrimSpace(arg) != "" {
		usage := "Usage: /balance or /saldo (your own balances only)."
		if command == "medals" {
			usage = "Usage: /medals or /medalhas (your own medals only)."
		}
		s.Send(wire.MessagePanel(usage))
		return
	}
	message := balanceMessage(p)
	if command == "medals" {
		message = medalsMessage(p)
	}
	// Exactly one private top notice, with no chat copy, pages or delayed sends.
	s.Send(wire.MessagePanel(message))
}

func balanceMessage(p *Player) string {
	honor := "N/A"
	if value, exists := p.SpecialCoins["honor"]; exists {
		honor = fmt.Sprint(value)
	}
	// Only configured currencies/entry allowances belong here: SpecialCoins
	// also contains access-expiry timestamps, not additional spendable money.
	// Donate is account-wide; the remaining values belong to this character.
	// Legacy character donate, gold, CP and medals are deliberately absent.
	// Full uint32 values fit in 83 ASCII bytes without clipping or pagination.
	return fmt.Sprintf("Donate:%d Fame:%d Honor:%s Pass:%d Kefra:%d",
		p.Account.Donate, counterBalance(p, fameCounter), honor,
		counterBalance(p, "event_pass"), counterBalance(p, kefraTicketCounter))
}

func medalsMessage(p *Player) string {
	// Show all three counters, including zero; medals are not account Donate.
	return fmt.Sprintf("Medals - Gold:%d Red:%d Blue:%d",
		counterBalance(p, "game_medal_gold"), counterBalance(p, "game_medal_red"),
		counterBalance(p, "game_medal_blue"))
}
