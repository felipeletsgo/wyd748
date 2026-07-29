package game

import (
	"fmt"
	"math"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// IDs internos reservados: registram as duas recompensas one-shot sem misturar
// seu estado com quests configuradas positivas de data/quests.json.
const (
	questFlagOpportunityCompound = -192
	questFlagBalanceCompound     = -193
)

func masteryResetFlag(code int) int {
	if code == 192 {
		return questFlagOpportunityCompound
	}
	return questFlagBalanceCompound
}

func matchesEvolution(ch *model.Char, required string) bool {
	if ch == nil {
		return false
	}
	actual := strings.ToLower(strings.TrimSpace(ch.Evolution))
	switch strings.ToLower(strings.TrimSpace(required)) {
	case "mortal":
		return actual == ""
	case "arch":
		return actual == "arch"
	case "celestial":
		return actual == "celestial" || actual == "subcelestial"
	default:
		return false
	}
}

func (w *World) useTerritoryPass(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule) {
	if !matchesEvolution(p.Char, rule.RequiredEvolution) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("This pass does not belong to your evolution."))
		return
	}
	oldItem := *item
	if rule.Consume {
		consumeOne(item)
	}
	// teleportPlayer persiste a conta (incluindo o consumo) antes de publicar a
	// nova posicao. Em falha ele volta a coordenada; restaure tambem o passe.
	if !w.teleportPlayer(p, rule.X, rule.Y) {
		*item = oldItem
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
}

func containsItemIndex(indices []uint16, index uint16) bool {
	for _, allowed := range indices {
		if allowed == index {
			return true
		}
	}
	return false
}

// useEquippedRefine cobre itens que refinam um slot fixo sem rolagem. A Agua
// das Fadas (3367) aceita somente Naiads/Grewpain no slot 13 e vai ate +6.
func (w *World) useEquippedRefine(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule) {
	if rule.TargetSlot < 0 || rule.TargetSlot >= len(p.Char.Equip) {
		return
	}
	target := &p.Char.Equip[rule.TargetSlot]
	level := itemSanc(*target)
	if !containsItemIndex(rule.TargetItems, target.Index) || level >= rule.RefineMax {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	oldItem, oldTarget := *item, *target
	if !setItemSanc(target, level+1) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	if rule.Consume {
		consumeOne(item)
	}
	w.recalcPlayer(p.Char)
	if err := w.saveAccount(p.Account); err != nil {
		*item, *target = oldItem, oldTarget
		w.recalcPlayer(p.Char)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	s.Send(wire.SendItem(p.ID, placeEquip, byte(rule.TargetSlot), *target))
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	s.Send(wire.UpdateScore(p.ID, *p.Char))
	w.refreshAppearance(p)
	s.Send(wire.MessagePanel("Refine successful!"))
}

func (w *World) useNightmareTicket(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule) {
	now := w.now()
	readyAt := time.Unix(p.Char.LastNightmareUnix, 0).Add(time.Duration(rule.CooldownSeconds) * time.Second)
	if p.Char.LastNightmareUnix > 0 && now.Before(readyAt) {
		remaining := readyAt.Sub(now).Round(time.Minute)
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel(fmt.Sprintf("Nightmare Letter available in %s.", remaining)))
		return
	}
	amount := uint32(rule.Amount)
	if amount > math.MaxUint32-p.Char.NightmareTickets {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		s.Send(wire.MessagePanel("Nightmare ticket limit reached."))
		return
	}

	oldItem := *item
	oldTickets, oldLast := p.Char.NightmareTickets, p.Char.LastNightmareUnix
	p.Char.NightmareTickets += amount
	p.Char.LastNightmareUnix = now.Unix()
	if rule.Consume {
		consumeOne(item)
	}
	if err := w.saveAccount(p.Account); err != nil {
		*item = oldItem
		p.Char.NightmareTickets, p.Char.LastNightmareUnix = oldTickets, oldLast
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}
	s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
	s.Send(wire.MessagePanel(fmt.Sprintf("%d Nightmare entries received.", amount)))
}

// useMasteryReset porta os Compostos de Chance/Equilibrio. O nativo devolve
// ate 50/100 pontos de cada um dos ramos 1 e 2 e apaga as skills Mortais.
// No modelo ExtendedScore, MasterPts e derivado do orçamento; reduzir Mastery
// e recalcular devolve os pontos sem manter um segundo saldo mutável.
func (w *World) useMasteryReset(s *net.Session, p *Player, item *model.Item, slot byte,
	rule model.VolatileRule, code int) {
	ch := p.Char
	if ch == nil || ch.Extended == nil || strings.TrimSpace(ch.Evolution) != "" ||
		ch.Extended.Level < rule.MinLevel || ch.Extended.Level >= rule.MaxLevelExclusive ||
		questCompleted(ch, masteryResetFlag(code)) {
		s.Send(wire.SendItem(p.ID, placeInv, slot, *item))
		return
	}

	snapshot := cloneCharacterState(ch)
	limit := uint32(rule.Amount)
	for branch := 1; branch <= 2; branch++ {
		if ch.Extended.Mastery[branch] > limit {
			ch.Extended.Mastery[branch] -= limit
		} else {
			ch.Extended.Mastery[branch] = 0
		}
	}
	ch.LearnedSkill &= 0xFF000000
	filterShortSkills(ch)
	markQuestCompleted(ch, masteryResetFlag(code))
	if rule.Consume {
		consumeOne(item)
	}
	w.recalcPlayer(ch)

	if err := w.saveAccount(p.Account); err != nil {
		*ch = snapshot
		s.Send(wire.SendItem(p.ID, placeInv, slot, ch.Inv[slot]))
		return
	}
	s.Send(wire.UpdateScore(p.ID, *ch))
	s.Send(wire.UpdateEtc(p.ID, *ch))
	s.Send(wire.SetShortSkill(p.ID, ch.ShortSkill))
	s.Send(wire.SendItem(p.ID, placeInv, slot, ch.Inv[slot]))
	s.Send(wire.MessagePanel("Skills initialized."))
}
