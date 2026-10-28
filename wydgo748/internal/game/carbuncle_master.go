package game

import (
	"log"
	"strings"

	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const (
	// O WYD 7.48 aceita somente Mortal com nivel interno abaixo de 116 (nivel 116
	// exibido) e chama SetAffect com potencia 600. Duracao, tipo e valor continuam
	// vindo do SkillData.csv autoritativo.
	carbMasterLevelLimit = uint32(116)
	carbMasterPower      = 600
)

var carbMasterSkills = [...]int{41, 43, 44, 45}

// isCarbMasterNPC reconhece somente a familia convertida do WYD 7.48. Os JSONs
// antigos possuem levels/merchants inconsistentes, portanto o nome canonico e a
// identidade estavel desta funcao; os dados foram normalizados separadamente.
func isCarbMasterNPC(def *model.NPCDef) bool {
	if def == nil || def.Tipo != model.TipoNPC {
		return false
	}
	name := strings.ToLower(strings.ReplaceAll(strings.TrimSpace(def.InteractionName()), " ", "_"))
	const prefix = "carb_mstr"
	if len(name) != len(prefix)+1 || !strings.HasPrefix(name, prefix) {
		return false
	}
	return name[len(prefix)] >= '1' && name[len(prefix)] <= '6'
}

// applyCarbMasterBuffs applies every buff or restores the snapshot. Native
// SetAffect resolves indices 41/43/44/45 through SkillData: Speed, Magic
// Shield, Magic Weapon and Athena's Touch.
func (w *World) applyCarbMasterBuffs(ch *model.Char) bool {
	if ch == nil {
		return false
	}
	snapshot := cloneCharacterState(ch)
	now := w.now()
	for _, skillID := range carbMasterSkills {
		skill, exists := w.skills[skillID]
		affectType, value, valid := skillAffect(skill)
		if !exists || !valid || skill.AffectTime <= 0 ||
			!setAffectAt(ch, affectType, value, carbMasterPower, skill.AffectTime, now) {
			*ch = snapshot
			return false
		}
	}
	return true
}

func (w *World) handleCarbMasterNPC(s *net.Session, p *Player, m *Mob) bool {
	if m == nil || !isCarbMasterNPC(m.Def) {
		return false
	}
	if s == nil || p == nil || p.Char == nil || !p.InWorld {
		return true
	}
	if strings.TrimSpace(p.Char.Evolution) != "" {
		s.Send(wire.MessagePanel("Only Mortal characters can receive this blessing."))
		return true
	}
	if playerLevel(p.Char) >= carbMasterLevelLimit {
		s.Send(wire.MessagePanel("Your level is too high for the beginner blessing."))
		return true
	}

	images := criticalPreimages(p)
	if !w.applyCarbMasterBuffs(p.Char) {
		s.Send(wire.MessagePanel("The beginner blessing could not be applied."))
		return true
	}
	w.recalcPlayer(p.Char)
	w.commitPlayerMutation(p, images, "beginner blessing", "Save failed. The blessing was not applied.", true, func() {
		w.publishPlayerAffects(p)
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		if !s.IsClosed() {
			s.Send(wire.MessagePanel("The beginner blessing has been applied."))
		}
		log.Printf("[#%d] %s applied beginner buffs to %q", s.ID, m.Def.Name, p.Char.Name)
	})
	return true
}
