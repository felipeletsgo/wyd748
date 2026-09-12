package game

import (
	"encoding/binary"
	"fmt"
	"log"
	"math"

	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	gambleMinBet     = uint32(1_000)
	gambleMaxBet     = uint32(100_000)
	gambleJackpotMax = uint32(2_000_000_000)
	gamblePoolMax    = uint64(4_000_000_000)
	gambleReelLength = 22
	gambleSilver100M = uint16(4010)
	gambleSilver1B   = uint16(4011)
)

var gambleMultipliers = [...]uint32{
	0, 2, 2, 2, 2, 5, 14, 50, 10, 20, 30, 50, 100, 150, 300, 500, 1000, 1500, 80,
}

var gambleReels = [3][gambleReelLength]byte{
	{1, 6, 2, 3, 1, 4, 1, 8, 2, 5, 1, 9, 1, 6, 1, 4, 3, 0, 2, 5, 1, 7},
	{0, 1, 3, 5, 0, 3, 2, 4, 0, 1, 3, 6, 0, 3, 9, 2, 0, 3, 7, 3, 0, 8},
	{3, 2, 0, 4, 2, 6, 1, 0, 2, 0, 1, 8, 2, 3, 0, 5, 2, 0, 7, 0, 2, 9},
}

type gambleOutcome struct {
	stops  [3]byte
	result [5]byte
	prize  uint32
}

func classifyGambleLine(line [3]byte) byte {
	if line == [3]byte{4, 5, 6} {
		return 7
	}
	if line[0] >= 7 && line[1] >= 7 && line[2] >= 7 &&
		(line[0] != line[1] || line[1] != line[2]) {
		return 6
	}
	if line[0] == line[1] {
		if line[1] == line[2] {
			return line[0] + 8
		}
		if line[0] == 0 {
			return 5
		}
		if line[0] == 5 || line[0] == 6 {
			return line[0] - 2
		}
	} else if line[0] == 0 {
		return 1
	}
	return 0
}

func gambleResults(stops [3]byte) [5]byte {
	line := func(a, b, c int) byte {
		return classifyGambleLine([3]byte{
			gambleReels[0][(int(stops[0])+a)%gambleReelLength],
			gambleReels[1][(int(stops[1])+b)%gambleReelLength],
			gambleReels[2][(int(stops[2])+c)%gambleReelLength],
		})
	}
	return [5]byte{
		line(0, 0, 0), line(1, 1, 1), line(2, 2, 2),
		line(0, 1, 2), line(2, 1, 0),
	}
}

func gamblePrize(bet uint32, result [5]byte) uint64 {
	value := func(symbol byte) uint64 {
		if int(symbol) >= len(gambleMultipliers) {
			return 0
		}
		return uint64(bet) * uint64(gambleMultipliers[symbol])
	}
	return value(result[1]) + value(result[0])/2 + value(result[2])/2 +
		value(result[3])/2 + value(result[4])/2
}

func (w *World) randomGambleStops() [3]byte {
	return [3]byte{byte(w.intn(gambleReelLength)), byte(w.intn(gambleReelLength)), byte(w.intn(gambleReelLength))}
}

func (w *World) noPrizeGambleStops() [3]byte {
	for attempt := 0; attempt < 15; attempt++ {
		stops := w.randomGambleStops()
		if gamblePrize(gambleMinBet, gambleResults(stops)) == 0 {
			return stops
		}
	}
	// O gerador de referencia tambem limita as tentativas. A busca determinista
	// fecha o fallback sem introduzir uma 16a chamada imprevisivel ao RNG.
	for a := 0; a < gambleReelLength; a++ {
		for b := 0; b < gambleReelLength; b++ {
			for c := 0; c < gambleReelLength; c++ {
				stops := [3]byte{byte(a), byte(b), byte(c)}
				if gamblePrize(gambleMinBet, gambleResults(stops)) == 0 {
					return stops
				}
			}
		}
	}
	return [3]byte{}
}

func (w *World) rollGamble(kind uint32, bet uint32) gambleOutcome {
	noPrizeChance := 50
	if kind >= 1 {
		noPrizeChance = 60
	}
	var stops [3]byte
	if w.intn(100) < noPrizeChance {
		stops = w.noPrizeGambleStops()
	} else {
		stops = w.randomGambleStops()
	}
	result := gambleResults(stops)
	prize := gamblePrize(bet, result)
	if result[1] == 14 {
		prize += uint64(w.gambleJackpot)
	}
	// O campo Prize do contrato 7.48 e int32. Saturar antes da persistencia
	// evita pagar um valor que a apresentacao transformaria em negativo.
	if prize > math.MaxInt32 {
		prize = math.MaxInt32
	}
	return gambleOutcome{stops: stops, result: result, prize: uint32(prize)}
}

func gamblePayoutItems(total uint64) (uint32, []uint16) {
	if total < uint64(maxCharacterGold) {
		return uint32(total), nil
	}
	items := make([]uint16, 0, 6)
	for total >= 1_000_000_000 {
		items = append(items, gambleSilver1B)
		total -= 1_000_000_000
	}
	for total >= 100_000_000 {
		items = append(items, gambleSilver100M)
		total -= 100_000_000
	}
	return uint32(total), items
}

func materializeGamblePayout(ch *model.Char, indexes []uint16) ([]int, error) {
	if ch == nil {
		return nil, fmt.Errorf("nil character")
	}
	free := make([]int, 0, len(indexes))
	for slot := 0; slot < model.PlayerCarrySlots && len(free) < len(indexes); slot++ {
		if ch.Inv[slot].Index == 0 {
			free = append(free, slot)
		}
	}
	if len(free) != len(indexes) {
		return nil, fmt.Errorf("need %d free inventory slots", len(indexes))
	}
	for i, index := range indexes {
		item, err := materializeItem(model.Item{Index: index})
		if err != nil {
			for _, slot := range free[:i] {
				ch.Inv[slot] = model.Item{}
			}
			return nil, err
		}
		ch.Inv[free[i]] = item
	}
	return free, nil
}

// onDoJackpotBet implementa a fronteira 0x2BE/0x1BF preservando a UI nativa,
// mas torna aposta, premio e pools uma unica transacao autoritativa do World.
func (w *World) onDoJackpotBet(s *gameNet.Session, pkt []byte) {
	p := w.players[s]
	if p == nil || p.Char == nil || !p.InWorld || playerCurHP(p.Char) == 0 || len(pkt) != 20 {
		return
	}
	kind := binary.LittleEndian.Uint32(pkt[12:16])
	bet := binary.LittleEndian.Uint32(pkt[16:20])
	if (kind != 1 && kind != 2) || bet < gambleMinBet || bet > gambleMaxBet || bet%gambleMinBet != 0 {
		s.Send(wire.MessagePanel("Invalid Gamble bet."))
		return
	}
	if p.Char.Gold < bet {
		s.Send(wire.MessagePanel("You do not have enough gold."))
		return
	}
	if _, ok := w.store.(instanceRuntimeTransactionStore); !ok {
		log.Printf("[#%d] Gamble indisponivel: store sem transacao conta+estado", s.ID)
		s.Send(wire.MessagePanel("Gamble is temporarily unavailable."))
		return
	}

	oldGold, oldInv := p.Char.Gold, p.Char.Inv
	oldJackpot, oldPool := w.gambleJackpot, w.gamblePool
	if p.Trade != nil {
		w.cancelTrade(p, "aposta Gamble")
	}
	w.gambleJackpot += bet / 1000
	if w.gambleJackpot > gambleJackpotMax {
		w.gambleJackpot = gambleJackpotMax
	}
	poolAdd := uint64((bet / 100) * 3)
	if w.gamblePool >= gamblePoolMax || poolAdd >= gamblePoolMax-w.gamblePool {
		w.gamblePool = gamblePoolMax
	} else {
		w.gamblePool += poolAdd
	}
	outcome := w.rollGamble(kind, bet)
	if outcome.result[1] == 14 {
		w.gambleJackpot = 0
	}
	poolDebit := uint64(outcome.prize / 1000)
	if poolDebit > w.gamblePool {
		w.gamblePool = 0
	} else {
		w.gamblePool -= poolDebit
	}

	total := uint64(p.Char.Gold-bet) + uint64(outcome.prize)
	newGold, payoutIndexes := gamblePayoutItems(total)
	p.Char.Gold = newGold
	if _, err := materializeGamblePayout(p.Char, payoutIndexes); err != nil {
		p.Char.Gold, p.Char.Inv = oldGold, oldInv
		w.gambleJackpot, w.gamblePool = oldJackpot, oldPool
		s.Send(wire.MessagePanel("Free more inventory slots before playing Gamble."))
		return
	}

	if err := w.saveAccountsAndInstanceState(p.Account); err != nil {
		p.Char.Gold, p.Char.Inv = oldGold, oldInv
		w.gambleJackpot, w.gamblePool = oldJackpot, oldPool
		log.Printf("[#%d] ERRO ao persistir Gamble: %v", s.ID, err)
		s.Send(wire.MessagePanel("The Gamble bet could not be completed."))
		return
	}

	s.Send(wire.ResultGamble(p.ID, outcome.result, outcome.stops, int32(outcome.prize), w.gambleJackpot))
	s.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
	s.Send(wire.UpdateEtc(p.ID, *p.Char))
	log.Printf("[#%d] Gamble tipo=%d aposta=%d premio=%d jackpot=%d", s.ID, kind, bet, outcome.prize, w.gambleJackpot)
}
