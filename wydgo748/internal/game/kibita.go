package game

import (
	"log"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

// The official TMSRVIN case 0x4A tests the stored level against 0x172
// (370, displayed 371 under the current zero-based level contract).
const kibitaSoulSkillLevel = 370

func mortalSoulCharacter(ch *model.Char) bool {
	return ch != nil && (ch.Evolution == "" || strings.EqualFold(ch.Evolution, "mortal"))
}

// The exchange schedule is the user's selected project policy. The official
// score routine proves the fixed affect, but its Kibita case has no Lac offer.
func kibitaLacWindow(now time.Time) bool {
	return now.Weekday() >= time.Monday && now.Weekday() <= time.Friday && now.Hour() == 21
}

func (w *World) handleKibitaSoul(s *net.Session, p *Player, m *Mob, quest *model.QuestDef) bool {
	if quest == nil || !quest.Rewards.Citizenship || m == nil || m.Def == nil ||
		model.NormalizeQuestNPC(generName(m.Def.InteractionName())) != "kibita" {
		return false
	}
	if p == nil || p.Char == nil || p.Account == nil || !p.InWorld || w.shuttingDown ||
		p.PersistencePoisoned || s.IsClosed() || w.criticalPlayerReserved(p) || playerCurHP(p.Char) == 0 {
		return true
	}
	// Preserve the configured citizenship purchase on the first interaction.
	if p.Char.Citizenship == 0 {
		return false
	}
	if p.Char.Citizenship != w.channel {
		s.Send(wire.MessagePanel(w.msg("_WG_Speak_Kibita_Citizenship_Channel")))
		return true
	}
	if !mortalSoulCharacter(p.Char) {
		s.Send(wire.MessagePanel(w.msg("_WG_Soul_Exchange_Only_Available_Mortal_Characters")))
		return true
	}
	now := w.now()
	learned := p.Char.LearnedSkill&celestialSoulBit != 0
	temporary := !learned && playerLevel(p.Char) < 369
	item := uint16(420)
	if temporary {
		if !kibitaLacWindow(now) {
			s.Send(wire.MessagePanel(w.msg("_WG_Lac_Soul_Exchange_Opens_Monday_Through")))
			return true
		}
		if activePlayerAffectAt(p.Char, 29, now) != nil {
			s.Send(wire.MessagePanel(w.msg("_WG_Soul_Effect_Already_Active")))
			return true
		}
	} else {
		if learned || playerLevel(p.Char) < kibitaSoulSkillLevel || p.Char.Class > 3 {
			s.Send(wire.MessagePanel(w.msg("_WG_Learning_Soul_Requires_Mortal_Level_Without")))
			return true
		}
		item = [...]uint16{5334, 5336, 5335, 5337}[p.Char.Class]
	}
	if countInventoryItem(p.Char, item) < 1 {
		message := "Bring one Secret Stone for your class to learn Soul."
		if temporary {
			message = "Bring one Lac scrap for the one-hour Soul effect."
		}
		s.Send(wire.MessagePanel(w.tr(message)))
		return true
	}
	images := criticalPreimages(p)
	previous := cloneCharacterState(p.Char)
	rollback := func() { *p.Char = previous }
	if temporary {
		// Check affect capacity before payment. Keep the existing expiry-based
		// sidecar and public/private affect channels; no new packet is needed.
		if !setAffectAt(p.Char, 29, 0, 0, 450, now) {
			s.Send(wire.MessagePanel(w.msg("_WG_There_No_Room_Another_Effect")))
			return true
		}
	} else {
		cape, ok := model.KingdomCapeAtTier(model.CapeTierHero,
			model.KingdomFromCape(p.Char.Equip[model.CapeSlot].Index))
		if !ok {
			return true
		}
		p.Char.Equip[model.CapeSlot] = model.Item{Index: cape}
		p.Char.LearnedSkill |= celestialSoulBit
	}
	if !consumeInventoryItem(p.Char, item, 1) {
		rollback()
		return true
	}
	publish := func() {
		w.recalcPlayer(p.Char)
		if !s.IsClosed() {
			s.Send(wire.UpdateCarry(p.ID, p.Char.Inv[:], p.Char.Gold))
			s.Send(wire.UpdateEtc(p.ID, *p.Char))
			if !temporary {
				s.Send(wire.SendItem(p.ID, placeEquip, model.CapeSlot, p.Char.Equip[model.CapeSlot]))
			}
			message := "Soul learned. You can now use Soul Limit."
			if temporary {
				message = "Soul granted for one hour: 2,000 STR, 2,000 INT and 10,000 maximum HP."
			}
			s.Send(wire.MessagePanel(w.tr(message)))
		}
		w.publishPlayerAffects(p)
		w.syncPlayerVitals(p)
		if !temporary {
			w.refreshAppearance(p)
		}
	}
	failed := func() {
		if !s.IsClosed() {
			s.Send(wire.MessagePanel(w.msg("_WG_Save_Failed_Soul_Exchange_Was_Not")))
		}
	}
	if w.criticalWrites != nil {
		if err := w.queueCriticalLiveMutation("kibita-soul", images, publish,
			criticalLiveMutationOptions{sidecar: true, reject: failed}); err != nil {
			log.Printf("[#%d] Kibita Soul admission rejected: %v", s.ID, err)
			failed()
		}
		return true
	}
	if err := w.saveAccountAndCharStateResult(p); err != nil {
		rollback()
		log.Printf("[#%d] Kibita Soul save failed: %v", s.ID, err)
		failed()
		return true
	}
	publish()
	return true
}
