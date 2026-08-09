package game

import (
	"log"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

const fairySlot = 13

type fairyBonus struct {
	expPercent  int
	dropPercent int
}

// fairyBonusByIndex concentra somente a progressao funcional das fadas. A
// duracao continua data-driven pelo itemlist/ItemEffect e nunca nasce desta
// tabela.
func fairyBonusByIndex(index uint16) fairyBonus {
	switch index {
	case 3900, 3903, 3906, 3911, 3912, 3913: // Green Angel
		return fairyBonus{expPercent: 16}
	case 3901, 3904, 3907: // Blue Angel
		return fairyBonus{dropPercent: 32}
	case 3902, 3905, 3908: // Red Angel
		return fairyBonus{expPercent: 8, dropPercent: 16}
	case 3914: // Silver Angel
		return fairyBonus{expPercent: 16, dropPercent: 32}
	case 3915: // Gold Angel
		return fairyBonus{expPercent: 24, dropPercent: 48}
	default:
		return fairyBonus{}
	}
}

func isFairyIndex(index uint16) bool {
	bonus := fairyBonusByIndex(index)
	return bonus.expPercent != 0 || bonus.dropPercent != 0
}

// dynamicEffectID resolve o ID persistido exclusivamente pelo ItemEffect.h que
// foi carregado no ItemDef. O servidor nao duplica 106/107/108 em constantes.
func dynamicEffectID(def model.ItemDef, name string) (byte, bool) {
	for id, effectName := range def.DynamicEffectNames {
		if effectName == name {
			return id, true
		}
	}
	return 0, false
}

func fairyTimerEffectIDs(def model.ItemDef) (day, hour, minute byte, ok bool) {
	day, dayOK := dynamicEffectID(def, "EF_WDAY")
	hour, hourOK := dynamicEffectID(def, "EF_HOUR")
	minute, minuteOK := dynamicEffectID(def, "EF_MIN")
	return day, hour, minute, dayOK && hourOK && minuteOK
}

func dynamicEffectValue(item model.Item, effectID byte) (int, bool) {
	if effectID == 0 {
		return 0, false
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == effectID {
			return int(item.Eff[i*2+1]), true
		}
	}
	return 0, false
}

func setDynamicEffect(item *model.Item, effectID byte, value int) bool {
	if item == nil || effectID == 0 || value < 0 || value > 255 {
		return false
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == effectID {
			item.Eff[i*2+1] = byte(value)
			return true
		}
	}
	for i := 0; i < 3; i++ {
		if item.Eff[i*2] == 0 {
			item.Eff[i*2] = effectID
			item.Eff[i*2+1] = byte(value)
			return true
		}
	}
	return false
}

// fairyTimerMinutes devolve o saldo persistido da fada. Os tres efeitos sao um
// conjunto atomico: estado parcial e tratado como nao inicializado/malformado,
// nunca como uma duracao inventada.
func fairyTimerMinutes(item model.Item, def model.ItemDef) (int, bool) {
	dayID, hourID, minuteID, ok := fairyTimerEffectIDs(def)
	if !ok {
		return 0, false
	}
	day, dayOK := dynamicEffectValue(item, dayID)
	hour, hourOK := dynamicEffectValue(item, hourID)
	minute, minuteOK := dynamicEffectValue(item, minuteID)
	if !dayOK || !hourOK || !minuteOK || hour > 23 || minute > 59 {
		return 0, false
	}
	return day*24*60 + hour*60 + minute, true
}

func fairyStaticDurationMinutes(def model.ItemDef) int {
	day := staticAbility(def, "EF_WDAY")
	hour := staticAbility(def, "EF_HOUR")
	minute := staticAbility(def, "EF_MIN")
	if day < 0 || hour < 0 || minute < 0 || hour > 23 || minute > 59 {
		return 0
	}
	return day*24*60 + hour*60 + minute
}

func writeFairyTimerMinutes(item *model.Item, def model.ItemDef, total int) bool {
	if item == nil || total < 0 {
		return false
	}
	dayID, hourID, minuteID, ok := fairyTimerEffectIDs(def)
	if !ok {
		return false
	}
	day := total / (24 * 60)
	hour := total / 60 % 24
	minute := total % 60
	if day > 255 {
		return false
	}
	// Um timer de fada ocupa exatamente os tres pares dinamicos. Materializar
	// os tres de uma vez evita um item parcialmente temporal se um slot estiver
	// ocupado por estado estranho.
	original := *item
	if !setDynamicEffect(item, dayID, day) ||
		!setDynamicEffect(item, hourID, hour) ||
		!setDynamicEffect(item, minuteID, minute) {
		*item = original
		return false
	}
	return true
}

// initializeFairyTimer reproduz BASE_SetDateFairy sem hardcode de duracao. A
// primeira equipacao materializa no proprio item o saldo definido no itemlist.
// Silver/Gold transformadas preservam o saldo dinamico herdado; quando um item
// nasce sem timer, usa a duracao estatica do catalogo como qualquer outra fada.
func (w *World) initializeFairyTimer(item *model.Item) bool {
	if w == nil || item == nil || !isFairyIndex(item.Index) {
		return false
	}
	def, ok := w.items[item.Index]
	if !ok {
		return false
	}
	if _, initialized := fairyTimerMinutes(*item, def); initialized {
		return true
	}
	duration := fairyStaticDurationMinutes(def)
	if duration <= 0 {
		return false
	}
	return writeFairyTimerMinutes(item, def, duration)
}

func (w *World) activeFairyBonus(ch *model.Char) fairyBonus {
	if w == nil || ch == nil {
		return fairyBonus{}
	}
	item := ch.Equip[fairySlot]
	bonus := fairyBonusByIndex(item.Index)
	if bonus == (fairyBonus{}) {
		return fairyBonus{}
	}
	def, ok := w.items[item.Index]
	if !ok {
		return fairyBonus{}
	}
	if remaining, initialized := fairyTimerMinutes(item, def); initialized {
		if remaining <= 0 {
			return fairyBonus{}
		}
		return bonus
	}
	// Antes do primeiro tick depois de equipar, as fadas-base ainda carregam a
	// duracao apenas no itemlist. Elas ja devem conceder o bonus imediatamente.
	if fairyStaticDurationMinutes(def) > 0 {
		return bonus
	}
	return fairyBonus{}
}

func (w *World) hasActiveSilverFairy(ch *model.Char) bool {
	if ch == nil || ch.Equip[fairySlot].Index != 3914 {
		return false
	}
	bonus := w.activeFairyBonus(ch)
	return bonus.expPercent == 16 && bonus.dropPercent == 32
}

// tickEquippedFairy usa a cadencia de minuto ja existente em NextMountTick.
// tickPlayerAffects roda antes de tickPlayerMounts, portanto quando o deadline
// vence a fada consome exatamente um minuto e, logo depois, o subsistema de
// montaria agenda o proximo. No primeiro tick do login NextMountTick e zero:
// inicializamos o item, mas NAO debitamos tempo offline.
//
// Se a fada for desequipada este metodo nao toca no item que foi para Carry;
// consequentemente o contador fica congelado ate ela voltar ao slot 13.
func (w *World) tickEquippedFairy(p *Player, now time.Time) {
	if w == nil || p == nil || p.Char == nil || !p.InWorld {
		return
	}
	item := &p.Char.Equip[fairySlot]
	if !isFairyIndex(item.Index) {
		return
	}
	def, ok := w.items[item.Index]
	if !ok {
		return
	}
	if _, initialized := fairyTimerMinutes(*item, def); !initialized {
		w.initializeFairyTimer(item)
		return
	}
	if p.NextMountTick.IsZero() || now.Before(p.NextMountTick) {
		return
	}
	remaining, ok := fairyTimerMinutes(*item, def)
	if !ok {
		return
	}
	if remaining > 0 {
		remaining--
	}
	if remaining > 0 {
		writeFairyTimerMinutes(item, def, remaining)
		return
	}

	// Expiracao remove o item autoritativamente. Persistimos antes de publicar;
	// se o store falhar, restauramos o snapshot e tentamos de novo no proximo
	// minuto, em vez de confirmar visualmente uma remocao que o banco rejeitou.
	old := *item
	*item = model.Item{}
	if p.Account != nil {
		if err := w.saveAccount(p.Account); err != nil {
			*item = old
			log.Printf("[#%d] ERRO ao expirar fada %d: %v", p.ID, old.Index, err)
			return
		}
	}
	w.recalcPlayer(p.Char)
	if p.Session != nil {
		p.Session.Send(wire.SendItem(p.ID, placeEquip, fairySlot, *item))
		p.Session.Send(wire.UpdateScore(p.ID, *p.Char))
		p.Session.Send(wire.MessagePanel("Your fairy has expired."))
	}
	w.refreshAppearance(p)
}
