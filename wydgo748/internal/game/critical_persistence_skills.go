package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// queueSkillLearning receives an actor-prepared account. Capture the committed
// fields by value: neither worker snapshots nor live account/character pointers
// are installed on completion. Recalculation was part of the detached proposal.
func (w *World) queueSkillLearning(p *Player, proposed *model.Account, globalSkill int, skill model.SkillDef) error {
	ch := &proposed.Chars[p.CharSlot]
	gold, learned, shortcuts := ch.Gold, ch.LearnedSkill, ch.ShortSkill
	score, runtime := *ch.Score, *ch.RuntimeScore
	attackSpeed := ch.RuntimeAttackSpeedPercent
	_, err := w.submitCriticalWrite(criticalWrite{accounts: []*model.Account{proposed}}, []*Player{p},
		func(criticalPersistenceResult) error {
			p.Char.Gold, p.Char.LearnedSkill, p.Char.ShortSkill = gold, learned, shortcuts
			*p.Char.Score = score // Keep the retained live score identity.
			committedRuntime := runtime
			p.Char.RuntimeScore = &committedRuntime
			p.Char.RuntimeAttackSpeedPercent = attackSpeed
			if !p.Session.IsClosed() {
				p.Session.Send(playerScorePacket(p))
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				p.Session.Send(wire.SetShortSkill(p.ID, p.Char.ShortSkill))
			}
			log.Printf("[#%d] learned skill=%d %q cost=%d remaining points=%d", p.Session.ID,
				globalSkill, skill.Name, skill.SkillPoint, playerSkillPoints(p.Char))
			return nil
		})
	return err
}
