package game

import (
	"crypto/rand"
	"fmt"
	"log"
	"strings"
	"time"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

type quizParticipant struct {
	player    *Player
	character *model.Char
	account   string
	token     [16]byte
	correct   uint16
	answered  bool
}

func (w *World) closeQuizRound() {
	e := w.quiz
	if e == nil {
		return
	}
	for session, p := range e.participants {
		if current := w.players[session]; current != nil && current == p.player && current.Char == p.character {
			session.Send(wire.QuizClose(current.ID, p.token))
		}
	}
	e.participants = nil
	e.claimed = nil
	e.status.RoundClosesAt = nil
}
func (w *World) endQuiz(reason string) {
	if w.quiz == nil || !w.quiz.status.Active {
		return
	}
	w.closeQuizRound()
	s := &w.quiz.status
	s.Active = false
	s.EndedBy = reason
	s.NextRoundAt = nil
	log.Printf("[event] quiz id=%q ended=%q rounds=%d rewarded=%d failures=%d", s.EventID, reason, s.Rounds, s.Rewarded, s.DeliveryFailures)
}
func (w *World) tickQuiz(now time.Time) {
	e := w.quiz
	if e == nil || !e.status.Active {
		return
	}
	if !now.Before(*e.status.EndsAt) {
		w.endQuiz("time")
		return
	}
	if e.status.RoundClosesAt != nil && !now.Before(*e.status.RoundClosesAt) {
		w.closeQuizRound()
	}
	if e.status.NextRoundAt == nil || now.Before(*e.status.NextRoundAt) {
		return
	}
	// Skip catch-up bursts after a stalled tick.
	w.closeQuizRound()
	next := now.Add(time.Minute)
	deadline := now.Add(10 * time.Second)
	if deadline.After(*e.status.EndsAt) {
		e.status.NextRoundAt = nil
		return
	}
	e.status.NextRoundAt = &next
	e.status.RoundClosesAt = &deadline
	e.status.Rounds++
	e.participants = make(map[*net.Session]*quizParticipant)
	e.claimed = make(map[string]bool)
	question, answer := w.quizQuestion()
	for session, p := range w.players {
		if p == nil || p.Session != session || session.IsClosed() || !p.InWorld || p.Char == nil || p.Account == nil || p.PersistencePoisoned {
			continue
		}
		token := [16]byte{}
		if _, err := rand.Read(token[:]); err != nil {
			continue
		}
		alternatives := []int32{}
		for _, delta := range []int32{-7, -3, -1, 1, 3, 7} {
			if answer+delta > 0 {
				alternatives = append(alternatives, answer+delta)
			}
		}
		for i := len(alternatives) - 1; i > 0; i-- {
			j := w.intn(i + 1)
			alternatives[i], alternatives[j] = alternatives[j], alternatives[i]
		}
		choices := [4]int32{answer, alternatives[0], alternatives[1], alternatives[2]}
		for i := 3; i > 0; i-- {
			j := w.intn(i + 1)
			choices[i], choices[j] = choices[j], choices[i]
		}
		var correct uint16
		for i, a := range choices {
			if a == answer {
				correct = uint16(i)
			}
		}
		e.participants[session] = &quizParticipant{player: p, character: p.Char, account: strings.ToLower(p.Account.Name), token: token, correct: correct}
		session.Send(wire.QuizChallenge(p.ID, token, question, choices))
	}
	log.Printf("[event] quiz id=%q round=%d participants=%d", e.status.EventID, e.status.Rounds, len(e.participants))
}

func (w *World) quizQuestion() (string, int32) {
	factor := 2 + w.intn(8)
	if w.intn(2) == 0 {
		operand := 12 + w.intn(88)
		if operand%10 == 0 {
			operand++
		}
		return fmt.Sprintf("%d x %d = ?", operand, factor), int32(operand * factor)
	}
	minQuotient := (12 + factor - 1) / factor
	if minQuotient < 3 {
		minQuotient = 3
	}
	quotient := minQuotient + w.intn(99/factor-minQuotient+1)
	return fmt.Sprintf("%d / %d = ?", quotient*factor, factor), int32(quotient)
}

func (w *World) onQuizAnswer(session *net.Session, pkt []byte) {
	response, err := wire.ParseQuizAnswer(pkt)
	if err != nil {
		w.recordSecurityViolation(session, wire.OpQuizAnswer, "quiz: malformed answer")
		return
	}
	e := w.quiz
	if e == nil || !e.status.Active || e.status.RoundClosesAt == nil || !w.now().Before(*e.status.RoundClosesAt) || !w.now().Before(*e.status.EndsAt) {
		return
	}
	participant := e.participants[session]
	p := w.players[session]
	if participant == nil || p == nil || p != participant.player || p.Char != participant.character || !p.InWorld || p.Account == nil || p.PersistencePoisoned || session.IsClosed() || p.ID != response.ID ||
		strings.ToLower(p.Account.Name) != participant.account || participant.answered || e.claimed[participant.account] || participant.token != response.Token {
		return
	}
	// Consume even a wrong answer before saving: no slot brute force or retry
	// after an ambiguous DB commit can award again in this round.
	participant.answered = true
	e.claimed[participant.account] = true
	session.Send(wire.QuizClose(p.ID, participant.token))
	if response.Choice != participant.correct {
		session.Send(wire.MessagePanel("Quiz: resposta incorreta."))
		return
	}
	e.status.Correct++
	before := cloneCharacterState(p.Char)
	drops := make([]plannedDrop, 0, e.status.RewardQuantity)
	for i := 0; i < e.status.RewardQuantity; i++ {
		item, err := materializeItem(model.Item{Index: e.status.ItemID})
		if err != nil {
			*p.Char = before
			e.status.DeliveryFailures++
			session.Send(wire.MessagePanel("Quiz: falha ao preparar recompensa. Contate a equipe."))
			return
		}
		pos := addToInv(p.Char, item)
		if pos < 0 {
			*p.Char = before
			e.status.DeliveryFailures++
			session.Send(wire.MessagePanel("Quiz: correto, mas sem espaco para a recompensa. Libere espaco antes da proxima pergunta."))
			return
		}
		drops = append(drops, plannedDrop{player: p, inventoryPos: pos, item: item, source: "quiz", sourceSlot: -1})
	}
	if err := w.saveAccountsAtomic(p.Account); err != nil {
		*p.Char = before
		e.status.DeliveryFailures++
		w.poisonAccountsAfterPersistenceFailure([]*model.Account{p.Account}, "quiz reward", err)
		log.Printf("[event] quiz reward failed event=%q round=%d account=%q error=%v", e.status.EventID, e.status.Rounds, participant.account, err)
		return
	}
	w.publishPlannedDrops(drops)
	e.status.Rewarded++
	session.Send(wire.MessagePanel("Quiz: resposta correta! Recompensa entregue."))
	log.Printf("[event] quiz reward event=%q round=%d account=%q item=%d quantity=%d", e.status.EventID, e.status.Rounds, participant.account, e.status.ItemID, e.status.RewardQuantity)
}
