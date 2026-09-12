package game

import (
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type plannedDrop struct {
	player       *Player
	inventoryPos int
	item         model.Item
	x, y         uint16
	instanceID   string
	source       string
	sourceSlot   int
}

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
	return model.CitizenDropBonus(p.Char.Citizenship, w.channel, p.Char.Exp) +
		w.equipmentGemBonuses(p.Char).dropPercent
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
	w.publishPlannedDrops(w.planMobDrops(p, m))
}

// planMobDrops mutates only the authoritative inventory. Ground publication
// and client packets are deferred until the enclosing kill transaction has
// committed.
func (w *World) planMobDrops(p *Player, m *Mob) []plannedDrop {
	if p == nil || p.Char == nil || m == nil || m.Def == nil {
		return nil
	}
	planned := make([]plannedDrop, 0, len(m.Def.Carry))
	lvl := int(m.Def.Score.Level)
	for i, item := range m.Def.Carry {
		if i >= len(w.dropRates) {
			break
		}
		if item.Index == 0 {
			continue
		}
		rate, guaranteed := w.dropRateForSlot(i, lvl)
		rate = applyDropBonus(rate, w.dropBonusFor(p))
		if !guaranteed && rate > 0 && w.intn(rate) != 0 {
			continue // nativo: rand()%rate == 0 dropa
		}
		if slot := addToInv(p.Char, item); slot >= 0 {
			planned = append(planned, plannedDrop{player: p, inventoryPos: slot,
				item: p.Char.Inv[slot], source: m.Def.Name, sourceSlot: i})
		} else {
			planned = append(planned, plannedDrop{player: p, inventoryPos: -1,
				item: item, x: m.X, y: m.Y, instanceID: strings.TrimSpace(m.InstanceID),
				source: m.Def.Name, sourceSlot: i})
		}
	}
	return planned
}

func (w *World) publishPlannedDrops(drops []plannedDrop) {
	for _, drop := range drops {
		if drop.player == nil || drop.player.Session == nil {
			continue
		}
		if drop.inventoryPos >= 0 {
			drop.player.Session.Send(wire.SendItem(drop.player.ID, placeInv,
				byte(drop.inventoryPos), drop.player.Char.Inv[drop.inventoryPos]))
			w.gameplayLogf("drop", "[#%d] DROP slot=%d item=%d -> inv[%d] (%q)",
				drop.player.Session.ID, drop.sourceSlot, drop.item.Index,
				drop.inventoryPos, drop.source)
			continue
		}
		w.createGroundDropForInstance(drop.x, drop.y, drop.item, true, drop.instanceID)
		w.gameplayLogf("drop", "[#%d] DROP slot=%d item=%d -> CHAO (inventario cheio)",
			drop.player.Session.ID, drop.sourceSlot, drop.item.Index)
	}
}

// rollMobGold porta o "Drop Gold" do MobKilled: chance por banda de level, valor
// pela formula nativa (cap 2000), creditado DIRETO no gold do killer (o UpdateEtc
// que o killMobState manda depois atualiza o display).
func (w *World) rollMobGold(p *Player, m *Mob) uint32 {
	coin := int(m.Def.Gold)
	if coin <= 0 {
		return 0
	}
	unk := 18
	switch lvl := int(m.Def.Score.Level); {
	case lvl < 10:
		unk = 2
	case lvl < 20:
		unk = 4
	case lvl < 30:
		unk = 6
	case lvl < 50:
		unk = 9
	}
	if w.intn(unk+1) != 0 {
		return 0
	}
	amount := 4 * (w.intn((coin+1)/4+1) + (coin+1)/4 + coin)
	if amount > 2000 {
		amount = 2000
	}
	const goldCap = 2_000_000_000 // teto nativo de gold do char
	if p.Char.Gold > goldCap-uint32(amount) {
		p.Char.Gold = goldCap
	} else {
		p.Char.Gold += uint32(amount)
	}
	return uint32(amount)
}
