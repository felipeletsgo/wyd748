package game

import (
	"fmt"
	"strings"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/model"
	"wydgo/internal/net"
	"wydgo/internal/wire"
)

const oxEnrollmentDuration = time.Minute
const oxQuestionCadence = time.Minute

type oxMember struct {
	session *net.Session
	char    *model.Char
	uid     string
	slot    int
}

// OX membership is session/character-bound and intentionally not resumable.
// A restart is inactive; relogging never restores a paid place implicitly.
type oxRuntime struct {
	phase           string
	deadline        time.Time
	questions       []model.VolatileInstanceQuiz
	round           int
	members         map[*Player]oxMember
	admitting       map[*Player]bool
	settling        bool
	results         map[string]string
	correctOverride *bool
}

func (w *World) startOXEvent() string {
	if len(w.oxGates()) != 6 {
		return "doors_unavailable"
	}
	if w.ox != nil && len(w.ox.members) != 0 {
		return "busy"
	}
	var questions []model.VolatileInstanceQuiz
	cfg := w.volatiles.Instances["cube-native"]
	for _, stage := range instanceStages(&cfg) {
		if stage.Quiz != nil {
			questions = append(questions, *stage.Quiz)
		}
	}
	if len(questions) == 0 {
		return "questions_unavailable"
	}
	w.ox = &oxRuntime{phase: "enrollment", deadline: w.now().Add(oxEnrollmentDuration), questions: questions,
		members: make(map[*Player]oxMember), admitting: make(map[*Player]bool), results: make(map[string]string)}
	w.oxQuizMode = true
	w.setOXDoors(false)
	w.broadcast(func() []byte {
		return wire.MessagePanel("OX enrollment is open for 60 seconds. Use an Entrance Ticket in the Big Cube lobby.")
	})
	return "ok"
}

func (w *World) stopOXEvent() {
	w.oxQuizMode = false
	w.setOXDoors(true)
	if w.ox != nil {
		w.ox.phase, w.ox.deadline = "inactive", time.Time{}
		for p := range w.ox.members {
			clearOXHUD(p)
			delete(w.ox.members, p)
		}
	}
}

func (w *World) isOXMember(p *Player) bool {
	if w.ox == nil || p == nil || p.Char == nil {
		return false
	}
	m, ok := w.ox.members[p]
	return ok && p.InWorld && w.players[p.Session] == p && w.playersByID[p.ID] == p &&
		m.session == p.Session && !p.Session.IsClosed() && m.char == p.Char && m.uid == p.Char.UID && m.slot == p.CharSlot &&
		w.playerRuntimeInstanceID(p.ID) == ""
}

func (w *World) oxEntryRejection(p *Player, item model.Item) string {
	if !w.oxQuizMode || w.ox == nil {
		return "The OX event is inactive. Your Entrance Ticket was not consumed."
	}
	if w.ox.phase != "enrollment" || !w.now().Before(w.ox.deadline) {
		return "OX enrollment is closed. Your Entrance Ticket was not consumed."
	}
	if w.isOXMember(p) || w.ox.admitting[p] {
		return "You have already entered the OX event."
	}
	if w.playerRuntimeInstanceID(p.ID) != "" {
		return "Leave your current instance before entering OX."
	}
	rule, _, ok := w.volatiles.Rule(item.Index)
	if !ok || rule.Instance == nil || !instanceTargetAllowedAt(rule.Instance, p.X, p.Y) {
		return "Use the Entrance Ticket in the Big Cube entry area."
	}
	return ""
}

func (w *World) admitOXMember(p *Player) {
	w.ox.members[p] = oxMember{session: p.Session, char: p.Char, uid: p.Char.UID, slot: p.CharSlot}
	w.sendOXCountdown(p, w.ox.deadline, "OX enrollment closes")
}

func clearOXHUD(p *Player) {
	if p.Session == nil || p.Session.IsClosed() {
		return
	}
	p.Session.Send(wire.InstanceQuestion(p.ID, "", 0))
	p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, 0))
}

func (w *World) sendOXCountdown(p *Player, deadline time.Time, label string) {
	seconds := uint32(max(0, int(deadline.Sub(w.now()).Seconds()+0.999)))
	p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, seconds))
	p.Session.Send(wire.MessagePanel(fmt.Sprintf("%s in %d seconds.", label, seconds)))
}

// Only ticket-admitted players may enter the public arena. Position packets,
// walking routes and skill jumps cannot manufacture membership or switch sides
// after the authoritative answer deadline.
func (w *World) oxDestinationAllowed(p *Player, x, y uint16) bool {
	if w.playerRuntimeInstanceID(p.ID) != "" || !oxArena.contains(x, y) {
		return true
	}
	if w.ox != nil && w.ox.admitting[p] {
		return true
	}
	if !w.oxQuizMode || !w.isOXMember(p) {
		return false
	}
	if w.ox.settling || w.ox.phase == "question" && !w.now().Before(w.ox.deadline) {
		return x == p.X && y == p.Y
	}
	return true
}

// Private instances use their own admission and geometry, not the public OX.
func (w *World) oxStepAllowed(p *Player, x, y uint16) bool {
	return w.playerRuntimeInstanceID(p.ID) != "" || !oxArena.contains(x, y) || w.oxQuizMode && w.isOXMember(p)
}

// A crowded lobby must never move an eliminated player back into the arena.
// Reservations participate in collision checks until the exit is committed.
func (w *World) findOXExit(p *Player) (uint16, uint16, bool) {
	for distance := 0; distance <= 32; distance++ {
		for dy := -distance; dy <= distance; dy++ {
			for dx := -distance; dx <= distance; dx++ {
				if absInt(dx) != distance && absInt(dy) != distance {
					continue
				}
				x, y := uint16(oxExitX+dx), uint16(oxExitY+dy)
				if !oxArena.contains(x, y) && w.terrain.HeightCompatible(oxExitX, oxExitY, x, y) &&
					!w.positionOccupiedExcept(x, y, nil, p) {
					return x, y, true
				}
			}
		}
	}
	return 0, 0, false
}

func (w *World) guardOXArena() {
	if w.ox != nil && !w.ox.settling {
		for p := range w.ox.members {
			if !w.isOXMember(p) || !oxArena.contains(p.X, p.Y) {
				delete(w.ox.members, p)
				clearOXHUD(p)
			}
		}
	}
	for _, p := range w.oxPlayers(oxArena) {
		if w.criticalPlayerReserved(p) || w.oxQuizMode && w.isOXMember(p) {
			continue
		}
		x, y, ok := w.findOXExit(p)
		if !ok {
			continue
		}
		w.moveOutOfArenaWithAck(p, x, y, func() {
			clearOXHUD(p)
			p.Session.Send(wire.MessagePanel("Enter OX with an Entrance Ticket during enrollment."))
		})
	}
}

func (w *World) tickOXEvent(now time.Time) {
	if w.shuttingDown {
		return
	}
	w.guardOXArena()
	o := w.ox
	if !w.oxQuizMode || o == nil || o.settling || len(o.admitting) != 0 {
		return
	}
	if o.phase != "enrollment" && len(o.members) == 0 {
		w.stopOXEvent()
		return
	}
	if now.Before(o.deadline) {
		return
	}
	switch o.phase {
	case "enrollment", "waiting":
		if len(o.members) == 0 || o.round >= len(o.questions) {
			w.stopOXEvent()
			return
		}
		w.setOXDoors(false)
		o.correctOverride = nil
		o.phase, o.deadline = "question", now.Add(instanceQuestionDuration)
		for p := range o.members {
			p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, 10))
			p.Session.Send(wire.InstanceQuestion(p.ID, strings.ReplaceAll(o.questions[o.round].Question, "_", " "), 10000))
		}
	case "question":
		correct := o.questions[o.round].Answer
		if o.correctOverride != nil {
			correct = *o.correctOverride
		}
		w.settleOXRound(correct)
	}
}

func (w *World) resolveOXCommand(c control.OXEventCommand, losingO bool) (string, int) {
	o := w.ox
	if o == nil {
		return "unchanged", 0
	}
	if action, ok := o.results[c.OperationID]; ok {
		if action != c.Action {
			return "invalid_command", 0
		}
		return "unchanged", 0
	}
	if !w.oxQuizMode || o.phase != "question" || !w.now().Before(o.deadline) {
		return "unchanged", 0
	}
	// An operator may select the losing side, but never shorten the ten-second
	// response window. Evaluation and payment still happen at its deadline.
	correct := !losingO
	o.correctOverride = &correct
	o.results[c.OperationID] = c.Action
	return "ok", 0
}

// The complete round is one persisted account mutation, with loser exit
// positions retained in the actor proposal (durable login positions remain
// neutral). No reward, removal or next round is published before commit.
func (w *World) settleOXRound(correctO bool) bool {
	o := w.ox
	if o == nil || o.settling || w.criticalInstanceReserved() {
		return false
	}
	for p := range o.members {
		if w.criticalPlayerReserved(p) {
			return false
		}
	}
	w.setOXDoors(true)
	type outcome struct {
		p               *Player
		before          model.Char
		x, y            uint16
		won             bool
		levels          int
		exp             uint32
		corpseObservers []*Player
	}
	var outcomes []outcome
	var accounts []*model.Account
	seen := make(map[*model.Account]bool)
	// Keep detached loser destinations unique and unavailable to competing
	// movement/teleports until acknowledgement (or certified rejection).
	tiles := make(map[criticalTeleportTile]*Player)
	if w.criticalTeleportTiles == nil {
		w.criticalTeleportTiles = make(map[criticalTeleportTile]*Player)
	}
	releaseTiles := func() {
		for tile, owner := range tiles {
			if w.criticalTeleportTiles[tile] == owner {
				delete(w.criticalTeleportTiles, tile)
			}
		}
	}
	correct := oxSideX
	if correctO {
		correct = oxSideO
	}
	for p := range o.members {
		if !w.isOXMember(p) || !oxArena.contains(p.X, p.Y) {
			continue
		}
		w.advancePlayerMovement(p, minTime(w.now(), o.deadline))
		w.publishPlayerStop(p)
		r := outcome{p: p, before: cloneCharacterState(p.Char), x: p.X, y: p.Y, won: correct.contains(p.X, p.Y) && playerCurHP(p.Char) > 0}
		if playerCurHP(p.Char) == 0 {
			for _, observer := range w.nearbyWorldPlayers(p.X, p.Y, viewHalfX) {
				if observer != p && observer.Session != nil && observer.hasVisible(p.ID) {
					r.corpseObservers = append(r.corpseObservers, observer)
				}
			}
		}
		if r.won {
			hp, mp := playerCurHP(p.Char), playerCurMP(p.Char)
			r.levels, r.exp = grantExp(p.Char, o.questions[o.round].RewardExp)
			if r.levels > 0 {
				updateCelestialCythera(p.Char)
			}
			w.recalcPlayer(p.Char)
			setPlayerCurHP(p.Char, minU32(hp, playerMaxHP(p.Char)))
			setPlayerCurMP(p.Char, minU32(mp, playerMaxMP(p.Char)))
		} else {
			x, y, ok := w.findOXExit(p)
			if !ok {
				// No destination means no partial round or partial payment.
				for _, previous := range outcomes {
					*previous.p.Char = previous.before
					previous.p.X, previous.p.Y = previous.x, previous.y
				}
				releaseTiles()
				return false
			}
			tile := criticalTile(x, y, "")
			tiles[tile], w.criticalTeleportTiles[tile] = p, p
			p.X, p.Y = x, y
			p.Char.X, p.Char.Y = p.X, p.Y
			if playerCurHP(p.Char) == 0 {
				setPlayerCurHP(p.Char, 1)
			}
		}
		outcomes = append(outcomes, r)
		if !seen[p.Account] {
			accounts = append(accounts, p.Account)
			seen[p.Account] = true
		}
	}
	if len(outcomes) == 0 {
		w.stopOXEvent()
		return true
	}
	o.settling = true
	restore := func() {
		for _, r := range outcomes {
			*r.p.Char = r.before
			r.p.X, r.p.Y = r.x, r.y
		}
	}
	rejected := func() {
		o.settling = false
		releaseTiles()
	}
	accepted := w.commitInstanceMutation("OX round", accounts, restore, func() {
		o.settling = false
		releaseTiles()
		for _, r := range outcomes {
			p := r.p
			p.Session.Send(wire.InstanceQuestion(p.ID, "", 0))
			if !r.won {
				delete(o.members, p)
				p.DeadAt = time.Time{}
				for _, observer := range r.corpseObservers {
					if w.players[observer.Session] == observer && observer.InWorld && observer.hasVisible(p.ID) {
						observer.Session.Send(wire.RemoveMob(p.ID, 3))
						observer.hide(p.ID)
					}
				}
				w.publishPlayerTeleport(p)
				w.syncPlayerVitals(p)
				clearOXHUD(p)
				p.Session.Send(wire.MessagePanel("Wrong answer. You left OX without a reward."))
			} else {
				w.syncPlayerVitals(p)
				w.updatePartyMember(p)
				if r.levels > 0 {
					p.Session.Send(playerScorePacket(p))
					w.syncPlayerChaos(p)
				}
				p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
				p.Session.Send(wire.MessagePanel(fmt.Sprintf("Correct! +%d EXP.", r.exp)))
			}
		}
		o.round++
		if len(o.members) == 0 || o.round >= len(o.questions) {
			w.stopOXEvent()
			return
		}
		// Normally 10 + 50 seconds is exactly one minute per question. Slow
		// persistence extends the cycle instead of shortening the next countdown.
		o.phase, o.deadline = "waiting", w.now().Add(oxQuestionCadence-instanceQuestionDuration)
		w.setOXDoors(false)
		for p := range o.members {
			w.sendOXCountdown(p, o.deadline, "Next OX question")
		}
	}, rejected, func() error {
		for tile, owner := range tiles {
			x, y := tile.xy()
			if w.criticalTeleportTiles[tile] != owner || w.positionOccupiedExcept(x, y, nil, owner) {
				return errCriticalPersistenceIdentity
			}
		}
		return nil
	})
	if !accepted {
		rejected()
	}
	return accepted
}

func minTime(a, b time.Time) time.Time {
	if a.Before(b) {
		return a
	}
	return b
}
