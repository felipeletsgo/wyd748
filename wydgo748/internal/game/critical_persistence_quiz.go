package game

import (
	"log"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// The event and round remain actor-owned until all admitted rewards settle.
// Account claims survive certified rejection; unknown outcomes retain both
// the account fence and round reservation rather than permitting a reroll.
func (w *World) admitQuizReward(e *quizState, participant *quizParticipant, p *Player) {
	proposed := accountStateSnapshot(p.Account)
	ch := &proposed.Chars[p.CharSlot]
	drops := make([]plannedDrop, 0, e.status.RewardQuantity)
	failPreparation := func(message string) {
		participant.answered, e.claimed[participant.account] = true, true
		e.status.Correct++
		e.status.DeliveryFailures++
		p.Session.Send(wire.QuizClose(p.ID, participant.token))
		p.Session.Send(wire.MessagePanel(message))
	}
	for i := 0; i < e.status.RewardQuantity; i++ {
		item, err := materializeItem(model.Item{Index: e.status.ItemID})
		if err != nil {
			failPreparation("Quiz: the reward could not be prepared. Contact staff.")
			return
		}
		pos := addToInv(ch, item)
		if pos < 0 {
			failPreparation("Quiz: correct, but your inventory is full. Make room before the next question.")
			return
		}
		drops = append(drops, plannedDrop{player: p, inventoryPos: pos, item: item, source: "quiz", sourceSlot: -1})
	}
	round, token := e.status.Rounds, participant.token
	admitted := false
	hooks := criticalAccountMutationHooks{
		check: func() error {
			if w.quiz != e || !e.status.Active || e.status.Rounds != round || e.participants[p.Session] != participant ||
				participant.token != token || participant.answered != admitted || e.claimed[participant.account] != admitted ||
				(admitted && e.criticalRewards <= 0) {
				return errCriticalPersistenceIdentity
			}
			return nil
		},
		reject: func() {
			e.criticalRewards--
			e.status.Correct++
			e.status.DeliveryFailures++
			if !p.Session.IsClosed() {
				p.Session.Send(wire.QuizClose(p.ID, token))
			}
		},
	}
	err := w.queueCriticalAccountWrite(p, proposed, criticalWrite{}, "quiz reward",
		"Quiz: the reward could not be saved. Contact staff.", func() {
			p.Char.Inv = ch.Inv
			e.criticalRewards--
			e.status.Correct++
			e.status.Rewarded++
		}, func() {
			p.Session.Send(wire.QuizClose(p.ID, token))
			w.publishPlannedDrops(drops)
			p.Session.Send(wire.MessagePanel("Quiz: correct answer! Reward delivered."))
			log.Printf("[event] quiz reward event=%q round=%d account=%q item=%d quantity=%d",
				e.status.EventID, round, participant.account, e.status.ItemID, e.status.RewardQuantity)
		}, false, hooks)
	if err != nil {
		p.Session.Send(wire.MessagePanel("Quiz: reward delivery is busy. Please try again before the deadline."))
		log.Printf("[event] quiz reward admission rejected event=%q round=%d account=%q error=%v",
			e.status.EventID, round, participant.account, err)
		return
	}
	admitted = true
	participant.answered, e.claimed[participant.account] = true, true
	e.criticalRewards++
}
