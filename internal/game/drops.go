package game

import (
	"log"
	"math/rand"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// drops.go -- drop de loot na morte do mob, PORTADO do MobKilled.cpp do W2PP
// ("Drop comum" + "Drop Gold"). Mecanica nativa: a chance depende do SLOT em que o
// item esta no carry do monstro (tabela w.dropRates, ajustada por banda de level).
// Diferenca pedida pelo felipe: o item dropado vai DIRETO pro inventario do killer;
// o chao (spawnDrop) e so fallback de inventario cheio.

// dropRateForSlot aplica os ajustes nativos de banda de level sobre a taxa base do
// slot (MobKilled.cpp:2359+). Retorna a taxa final e se o drop e garantido.
func (w *World) dropRateForSlot(slot int, mobLevel int) (rate int, guaranteed bool) {
	rate = w.dropRates[slot]
	if slot < 60 {
		if pos := slot / 8; pos <= 2 {
			switch {
			case mobLevel < 10:
				rate = 4 * rate / 100
			case mobLevel < 20:
				rate = 5 * rate / 100
			case mobLevel < 30:
				rate = 6 * rate / 100
			case mobLevel < 40:
				rate = 7 * rate / 100
			case mobLevel < 60:
				rate = 8 * rate / 100
			default:
				rate = 99 * rate / 100
			}
		}
	} else {
		switch {
		case mobLevel < 170:
			rate = 90 * rate / 100
		case mobLevel < 200:
			rate = 60 * rate / 100
		case mobLevel < 230:
			rate = 50 * rate / 100
		case mobLevel < 255:
			rate = 43 * rate / 100
		case mobLevel < 320:
			rate = 38 * rate / 100
		default:
			rate = 50 * rate / 100
		}
	}
	// Slots especiais do nativo: 8-10 forcam taxa 4; 11 e drop GARANTIDO.
	if slot >= 8 && slot <= 10 {
		rate = 4
	}
	if slot == 11 {
		return 0, true
	}
	if rate > 32000 {
		rate = 32000
	}
	return rate, false
}

// rollMobDrops rola cada slot ocupado do carry do monstro. Item que dropa vai
// direto pro inventario do killer (SendItem confirma); inventario cheio cai no chao.
// dropBonusFor soma os bonus percentuais de drop do jogador. Hoje so a
// cidadania contribui; itens/eventos entram aqui quando existirem.
func (w *World) dropBonusFor(p *Player) int {
	if p == nil || p.Char == nil {
		return 0
	}
	return model.CitizenDropBonus(p.Char.Citizenship, w.channel, p.Char.Exp)
}

// applyDropBonus converte um bonus percentual numa taxa "1 em N".
//
// A taxa nativa e invertida: o drop acontece quando rand()%rate == 0, entao
// rate MENOR = mais comum. Um bonus de +10% precisa portanto DIVIDIR a taxa por
// 1.10 -- somar ao rate deixaria o drop mais raro, o contrario do pretendido.
func applyDropBonus(rate, bonusPercent int) int {
	if rate <= 1 || bonusPercent <= 0 {
		return rate
	}
	adjusted := rate * 100 / (100 + bonusPercent)
	if adjusted < 1 {
		adjusted = 1
	}
	return adjusted
}

func (w *World) rollMobDrops(p *Player, m *Mob) {
	lvl := int(m.Def.Extended.Level)
	for i, item := range m.Def.Carry {
		if i >= len(w.dropRates) {
			break
		}
		if item.Index == 0 {
			continue
		}
		rate, guaranteed := w.dropRateForSlot(i, lvl)
		rate = applyDropBonus(rate, w.dropBonusFor(p))
		if !guaranteed && rate > 0 && rand.Intn(rate) != 0 {
			continue // nativo: rand()%rate == 0 dropa
		}
		if slot := addToInv(p.Char, item); slot >= 0 {
			p.Session.Send(wire.SendItem(p.ID, placeInv, byte(slot), p.Char.Inv[slot]))
			log.Printf("[#%d] DROP slot=%d item=%d -> inv[%d] (%q)",
				p.Session.ID, i, item.Index, slot, m.Def.Name)
		} else {
			// Inventario cheio: cai no chao na posicao do mob (pegavel via 0x270).
			w.spawnDrop(m.X, m.Y, item)
			log.Printf("[#%d] DROP slot=%d item=%d -> CHAO (inventario cheio)",
				p.Session.ID, i, item.Index)
		}
	}
}

// rollMobGold porta o "Drop Gold" do MobKilled: chance por banda de level, valor
// pela formula nativa (cap 2000), creditado DIRETO no gold do killer (o UpdateEtc
// que o killMobState manda depois atualiza o display).
func (w *World) rollMobGold(p *Player, m *Mob) {
	coin := int(m.Def.Gold)
	if coin <= 0 {
		return
	}
	unk := 18
	switch lvl := int(m.Def.Extended.Level); {
	case lvl < 10:
		unk = 2
	case lvl < 20:
		unk = 4
	case lvl < 30:
		unk = 6
	case lvl < 50:
		unk = 9
	}
	if rand.Intn(unk+1) != 0 {
		return
	}
	amount := 4 * (rand.Intn((coin+1)/4+1) + (coin+1)/4 + coin)
	if amount > 2000 {
		amount = 2000
	}
	const goldCap = 2_000_000_000 // teto nativo de gold do char
	if p.Char.Gold > goldCap-uint32(amount) {
		p.Char.Gold = goldCap
	} else {
		p.Char.Gold += uint32(amount)
	}
	log.Printf("[#%d] DROP gold +%d (total=%d) de %q", p.Session.ID, amount, p.Char.Gold, m.Def.Name)
}
