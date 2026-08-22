package game

import (
	"fmt"
	"strings"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

// isBigCubeConfig keeps the special O/X flow explicit.  Big Cube has no mob
// wave: the question is opened as soon as the ticket is committed and the
// answer is determined only by the player's authoritative position.
func isBigCubeConfig(cfg *model.VolatileInstance) bool {
	return cfg != nil && strings.EqualFold(strings.TrimSpace(cfg.StateMachine), "big_cube")
}

func isBigCubeInstance(inst *ItemInstance) bool {
	return inst != nil && isBigCubeConfig(&inst.Config)
}

func (w *World) startBigCubeQuestion(inst *ItemInstance, now time.Time) bool {
	if !isBigCubeInstance(inst) || !inst.QuizAt.IsZero() || !inst.TransitionAt.IsZero() ||
		inst.ExitAt != (time.Time{}) {
		return false
	}
	stages := instanceStages(&inst.Config)
	if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
		return false
	}
	quiz := stages[inst.CurrentStage].Quiz
	if quiz == nil || quiz.DurationSeconds <= 0 || len(inst.MemberIDs) == 0 {
		return false
	}
	deadline := now.Add(time.Duration(quiz.DurationSeconds) * time.Second)
	inst.State = "big_cube_quiz"
	inst.QuizAt = deadline
	inst.QuizDeadline = deadline
	for _, id := range inst.MemberIDs {
		if member := w.playersByID[id]; member != nil && member.InWorld {
			member.Session.Send(wire.StandardParm(
				wire.OpInstanceTime, instanceSignalID, uint32(quiz.DurationSeconds)))
			member.Session.Send(wire.MessagePanel(strings.ReplaceAll(quiz.Question, "_", " ")))
		}
	}
	w.markInstanceStateDirty()
	return true
}

// resolveBigCubeQuestion is deliberately separate from the regular Cube
// resolver.  Big Cube has no combat wave and can finish on its last question;
// reusing the linear Cube resolver would schedule a nonexistent next room and
// eventually expire the instance without running its normal completion path.
func (w *World) resolveBigCubeQuestion(inst *ItemInstance, now time.Time) bool {
	if !isBigCubeInstance(inst) {
		return false
	}
	stages := instanceStages(&inst.Config)
	if inst.CurrentStage < 0 || inst.CurrentStage >= len(stages) {
		return false
	}
	quiz := stages[inst.CurrentStage].Quiz
	if quiz == nil {
		return false
	}
	type result struct {
		player *Player
		char   model.Char
		x, y   uint16
		ok     bool
		levels int
		exp    uint32
	}
	results := make([]result, 0, len(inst.MemberIDs))
	accounts := make([]*model.Account, 0, len(inst.MemberIDs))
	seenAccounts := make(map[*model.Account]struct{}, len(inst.MemberIDs))
	for _, id := range inst.MemberIDs {
		p := w.playersByID[id]
		if p == nil || !p.InWorld || p.Char == nil || p.Account == nil {
			continue
		}
		current := result{player: p, char: cloneCharacterState(p.Char), x: p.X, y: p.Y}
		current.ok = instanceQuizCorrect(p, quiz)
		if current.ok && quiz.RewardExp > 0 {
			oldHP, oldMP := playerCurHP(p.Char), playerCurMP(p.Char)
			current.levels, current.exp = grantExp(p.Char, quiz.RewardExp)
			if current.levels > 0 {
				updateCelestialCythera(p.Char)
			}
			w.recalcPlayer(p.Char)
			setPlayerCurHP(p.Char, minU32(oldHP, playerMaxHP(p.Char)))
			setPlayerCurMP(p.Char, minU32(oldMP, playerMaxMP(p.Char)))
		} else if !current.ok {
			p.X, p.Y = w.findFreePlayerPosition(inst.Config.ExitX, inst.Config.ExitY, 6, p)
			p.Char.X, p.Char.Y = p.X, p.Y
		}
		results = append(results, current)
		if _, seen := seenAccounts[p.Account]; !seen {
			seenAccounts[p.Account] = struct{}{}
			accounts = append(accounts, p.Account)
		}
	}
	if len(results) == 0 {
		inst.MemberIDs = nil
		inst.QuizAt = time.Time{}
		inst.QuizDeadline = time.Time{}
		setInstanceCombatDeadline(inst, now)
		w.markInstanceStateDirty()
		return true
	}

	oldInstance := *inst
	survivors := make([]uint16, 0, len(results))
	for _, current := range results {
		if current.ok {
			survivors = append(survivors, current.player.ID)
		}
	}
	inst.MemberIDs = survivors
	ensureItemInstanceLeader(inst)
	inst.QuizAt = time.Time{}
	inst.QuizDeadline = time.Time{}
	inst.TransitionAt = time.Time{}
	inst.TransitionDeadline = time.Time{}
	lastQuestion := inst.CurrentStage+1 >= len(stages)
	if len(survivors) == 0 {
		setInstanceCombatDeadline(inst, now)
	} else if lastQuestion {
		// completeItemInstance uses HardDeadline as the absolute safety limit;
		// leaving CombatDeadline empty allows the final answer to award normally.
		inst.State = "big_cube_complete"
	} else {
		delay := inst.Config.TransitionSeconds
		if delay < 0 {
			delay = 0
		}
		inst.TransitionAt = now.Add(time.Duration(delay) * time.Second)
		inst.TransitionDeadline = inst.TransitionAt
	}
	if err := w.saveAccountsAndInstanceState(accounts...); err != nil {
		*inst = oldInstance
		for _, current := range results {
			*current.player.Char = current.char
			current.player.X, current.player.Y = current.x, current.y
		}
		return false
	}

	for _, current := range results {
		p := current.player
		if !current.ok {
			w.refreshPlayerVisibility(p)
			w.sendToPlayerView(p, func() []byte { return wire.ActionStop(p.ID, p.X, p.Y) })
			p.Session.Send(wire.StandardParm(wire.OpInstanceTime, instanceSignalID, 0))
			p.Session.Send(wire.StandardParm(wire.OpInstanceMobs, instanceSignalID, 0))
			p.Session.Send(wire.MessagePanel("Wrong answer. You left Big Cube."))
			continue
		}
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		if current.levels > 0 {
			p.Session.Send(playerScorePacket(p))
		}
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
		p.Session.Send(wire.MessagePanel(fmt.Sprintf("Correct! +%d EXP.", current.exp)))
	}
	if len(survivors) == 0 {
		return true
	}
	if lastQuestion {
		w.completeItemInstance(inst, now)
		// Big Cube ends immediately after the answer; unlike Water/Cube, there
		// is no next ticket to consume during an exit grace window.
		if inst.Config.RewardItem == 0 && !inst.ExitAt.IsZero() {
			inst.ExitAt = now
			inst.ExitDeadline = now
		}
		return !inst.ExitAt.IsZero()
	}
	return true
}
