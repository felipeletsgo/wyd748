package game

import (
	"errors"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/wire"
)

type hellGateExperienceChange struct {
	player         *Player
	account        *model.Account
	levels         int
	cytheraChanged bool
}

func hellGateRewardMembers(w *World, inst *ItemInstance) []*Player {
	if w == nil || inst == nil {
		return nil
	}
	space := itemInstanceGameplaySpace(inst)
	if space == "" {
		return nil
	}
	members := make([]*Player, 0, len(inst.MemberIDs))
	seen := make(map[uint16]struct{}, len(inst.MemberIDs))
	for _, memberID := range inst.MemberIDs {
		if _, alreadySeen := seen[memberID]; alreadySeen {
			continue
		}
		seen[memberID] = struct{}{}
		member := w.playersByID[memberID]
		if member == nil || !member.InWorld || member.Char == nil || member.Account == nil ||
			member.Session == nil || member.Session.IsClosed() ||
			!itemInstanceHasMember(inst, member.ID) ||
			w.playerRuntimeInstanceID(member.ID) != space ||
			!bindCriticalPlayer(member).current(w) {
			continue
		}
		members = append(members, member)
	}
	return members
}

func prepareHellGateExperience(w *World, member *Player, reward uint32) (hellGateExperienceChange, error) {
	if w == nil || member == nil || member.Account == nil || member.Char == nil {
		return hellGateExperienceChange{}, errCriticalPersistenceIdentity
	}
	slot := int(member.CharSlot)
	if slot < 0 || slot >= len(member.Account.Chars) || member.Char != &member.Account.Chars[slot] {
		return hellGateExperienceChange{}, errCriticalPersistenceIdentity
	}
	if reward == 0 {
		return hellGateExperienceChange{}, errors.New("hell gate reward EXP must be positive")
	}
	proposal := accountStateSnapshot(member.Account)
	target := &proposal.Chars[slot]
	oldHP, oldMP := playerCurHP(target), playerCurMP(target)
	levels, _ := grantExp(target, reward)
	cytheraChanged := levels > 0 && updateCelestialCythera(target)
	w.recalcPlayer(target)
	// Completion grants EXP, not a resurrection or a mana refill.
	setPlayerCurHP(target, minU32(oldHP, playerMaxHP(target)))
	setPlayerCurMP(target, minU32(oldMP, playerMaxMP(target)))
	return hellGateExperienceChange{player: member, account: proposal, levels: levels, cytheraChanged: cytheraChanged}, nil
}

func publishHellGateExperience(w *World, changes []hellGateExperienceChange) {
	for _, change := range changes {
		p := change.player
		if p == nil || !p.InWorld || p.Session == nil || p.Session.IsClosed() || p.Char == nil {
			continue
		}
		w.syncPlayerVitals(p)
		w.updatePartyMember(p)
		if change.levels > 0 {
			p.Session.Send(playerScorePacket(p))
			w.syncPlayerChaos(p)
		}
		if change.cytheraChanged {
			p.Session.Send(wire.SendItem(p.ID, placeEquip, 1, p.Char.Equip[1]))
			w.refreshAppearance(p)
		}
		p.Session.Send(wire.UpdateEtc(p.ID, *p.Char))
	}
}

func (w *World) hellGateRewardProposal(inst *ItemInstance, plan *criticalInstanceDeparture, now time.Time) error {
	if w == nil || w.criticalWrites == nil || inst == nil || plan == nil || w.shuttingDown {
		return errCriticalPersistenceIdentity
	}
	if w.criticalInstanceReserved() {
		return errCriticalPersistenceBusy
	}
	if !hellGateVersion2(inst) {
		return errCriticalPersistenceIdentity
	}
	cfg, ok := hellGateInstance(inst)
	if !ok || cfg.RewardEXP == 0 {
		return errors.New("hell gate reward EXP is not configured")
	}
	id := itemInstanceGameplaySpace(inst)
	if id == "" || w.itemInstances[id] != inst || inst.RewardGranted || !inst.ExitAt.IsZero() {
		return errCriticalPersistenceIdentity
	}
	proposed := plan.proposed.itemInstances[id]
	if proposed == nil || !proposed.RewardGranted || !proposed.ExitAt.Equal(now.Add(10*time.Second)) ||
		!proposed.ExitDeadline.Equal(proposed.ExitAt) {
		return errCriticalPersistenceIdentity
	}
	members := hellGateRewardMembers(w, inst)
	changes := make([]hellGateExperienceChange, 0, len(members))
	accountChanges := make(map[*Player]*model.Account, len(members))
	seenAccounts := make(map[*model.Account]struct{}, len(members))
	for _, member := range members {
		change, err := prepareHellGateExperience(w, member, cfg.RewardEXP)
		if err != nil {
			return err
		}
		if _, duplicate := seenAccounts[member.Account]; duplicate {
			return errCriticalPersistenceIdentity
		}
		seenAccounts[member.Account] = struct{}{}
		changes = append(changes, change)
		accountChanges[member] = change.account
	}
	check := func() error {
		if w.shutdownErr != nil {
			return errCriticalPersistenceStopping
		}
		return plan.check(w)
	}
	if len(changes) == 0 {
		if err := check(); err != nil {
			return err
		}
		_, err := w.criticalWrites.submitWithRejection(criticalWrite{instances: plan.snapshot()},
			func(result criticalPersistenceResult) error {
				if result.err != nil {
					return result.err
				}
				if err := check(); err != nil {
					return err
				}
				plan.install(w)
				return nil
			}, nil, func(criticalPersistenceResult) error { return check() })
		return err
	}
	apply := func() {
		for _, change := range changes {
			installAccountInPlace(change.player.Account, change.account)
		}
	}
	publish := func() { publishHellGateExperience(w, changes) }
	return w.queueCriticalInstanceAccounts(members[0], members, accountChanges, plan, apply, publish, nil, nil)
}

// queueCriticalHellGateExperience prepares the configured EXP grant, honoring
// normal grantExp caps, and the ten-second exit grace as one detached
// account/instance transaction.
func (w *World) queueCriticalHellGateExperience(inst *ItemInstance, plan *criticalInstanceDeparture, now time.Time) error {
	return w.hellGateRewardProposal(inst, plan, now)
}

// grantHellGateExperience is the legacy synchronous path used without S2.
// Every eligible online member receives the configured reward; disconnected
// members are intentionally omitted from both the account mutation and the
// publication.
func (w *World) grantHellGateExperience(inst *ItemInstance, now time.Time) bool {
	if w == nil || w.criticalWrites != nil || inst == nil || w.shuttingDown || !hellGateVersion2(inst) {
		return false
	}
	if inst.RewardGranted {
		return true
	}
	cfg, ok := hellGateInstance(inst)
	if !ok || cfg.RewardEXP == 0 {
		return false
	}
	id := itemInstanceGameplaySpace(inst)
	if id == "" || w.itemInstances[id] != inst || !inst.ExitAt.IsZero() {
		return false
	}
	members := hellGateRewardMembers(w, inst)
	changes := make([]hellGateExperienceChange, 0, len(members))
	accounts := make([]*model.Account, 0, len(members))
	seenAccounts := make(map[*model.Account]struct{}, len(members))
	for _, member := range members {
		change, err := prepareHellGateExperience(w, member, cfg.RewardEXP)
		if err != nil {
			return false
		}
		if _, duplicate := seenAccounts[member.Account]; duplicate {
			return false
		}
		seenAccounts[member.Account] = struct{}{}
		changes = append(changes, change)
		accounts = append(accounts, member.Account)
	}
	beforeAccounts := make(map[*model.Account]*model.Account, len(accounts))
	for _, account := range accounts {
		beforeAccounts[account] = accountStateSnapshot(account)
	}
	wasGranted, beforeExit, beforeDeadline := inst.RewardGranted, inst.ExitAt, inst.ExitDeadline
	rollback := func() {
		for account, snapshot := range beforeAccounts {
			installAccountInPlace(account, snapshot)
		}
		inst.RewardGranted, inst.ExitAt, inst.ExitDeadline = wasGranted, beforeExit, beforeDeadline
	}
	for _, change := range changes {
		if !installAccountInPlace(change.player.Account, change.account) {
			rollback()
			return false
		}
	}
	inst.RewardGranted = true
	inst.ExitAt = now.Add(10 * time.Second)
	inst.ExitDeadline = inst.ExitAt
	return w.commitInstanceMutation("Hell Gate EXP reward", accounts, rollback,
		func() { publishHellGateExperience(w, changes) }, nil)
}
