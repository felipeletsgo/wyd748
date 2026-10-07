package game

import (
	"log"
	"maps"
	"reflect"

	"wydgo/internal/model"
)

// commitInstanceMutation persists a legacy instance transition whose runtime
// and account effects were already applied live. Without a coordinator it
// saves inline: failure runs rollback and returns false; success publishes.
//
// With the coordinator, the mutated accounts and the complete instance
// aggregate form one detached write. The caller's rollback restores public RAM
// before admission yields; observers and gameplay readers see only that state.
// A verified commit installs the detached proposal under the fences, then
// publishes after acknowledgement. Certified rejection preserves the restored
// state and runs rejected; admission failure returns false with RAM restored.
// Unknown outcomes neither install nor publish and retain global containment.
func (w *World) commitInstanceMutation(operation string, accounts []*model.Account,
	rollback, publish, rejected func(), retainedChecks ...func() error) bool {
	if w.criticalWrites == nil {
		if err := w.saveAccountsAndInstanceState(accounts...); err != nil {
			rollback()
			log.Printf("%s: persistence failed: %v", operation, err)
			return false
		}
		publish()
		return true
	}
	restored := false
	fail := func(err error) bool {
		if !restored {
			rollback()
		}
		log.Printf("%s: admission rejected: %v", operation, err)
		return false
	}
	if w.criticalInstanceReserved() {
		return fail(errCriticalPersistenceBusy)
	}
	if len(accounts) > 0 {
		if err := w.validateInstanceStateOwnership(); err != nil {
			return fail(err)
		}
	}
	participants := make([]*Player, 0, len(accounts))
	proposals := make([]*model.Account, 0, len(accounts))
	positions := make([][2]uint16, 0, len(accounts))
	for _, account := range accounts {
		var owner *Player
		for _, p := range w.players {
			if p != nil && p.Account == account {
				owner = p
				break
			}
		}
		if owner == nil {
			return fail(errCriticalPersistenceIdentity)
		}
		participants = append(participants, owner)
		proposals = append(proposals, accountStateSnapshot(account))
		positions = append(positions, [2]uint16{owner.X, owner.Y})
	}
	snapshot := w.instanceStateSnapshot()
	plan := &criticalInstanceDeparture{at: w.now(), proposed: cloneCriticalInstanceMembership(w),
		originals: maps.Clone(w.itemInstances)}
	plan.proposed.rebuildPlayerInstanceIndex()
	// Preparation is synchronous on the actor. Do not leave provisional
	// membership, avatar fields or positions visible during database I/O.
	rollback()
	restored = true
	w.rebuildPlayerInstanceIndex()
	plan.before = cloneCriticalInstanceMembership(w)
	plan.generation = w.criticalInstanceGeneration
	before := make([]*model.Account, len(participants))
	oldPositions := make([][2]uint16, len(participants))
	for i, p := range participants {
		before[i] = accountStateSnapshot(p.Account)
		oldPositions[i] = [2]uint16{p.X, p.Y}
	}
	check := func() error {
		if w.shutdownErr != nil {
			return errCriticalPersistenceStopping
		}
		for i, p := range participants {
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before[i]) || [2]uint16{p.X, p.Y} != oldPositions[i] {
				return errCriticalPersistenceIdentity
			}
		}
		for _, retainedCheck := range retainedChecks {
			if err := retainedCheck(); err != nil {
				return err
			}
		}
		return plan.check(w)
	}
	write := criticalWrite{accounts: proposals, instances: snapshot}
	apply := func(result criticalPersistenceResult) error {
		if result.err != nil {
			return result.err
		}
		if err := check(); err != nil {
			return err
		}
		for i, p := range participants {
			installAccountInPlace(p.Account, proposals[i])
			p.X, p.Y = positions[i][0], positions[i][1]
		}
		plan.install(w)
		return nil
	}
	afterAck := func(result criticalPersistenceResult) error {
		if result.err != nil {
			return result.err
		}
		publish()
		return nil
	}
	reject := func(criticalPersistenceResult) error {
		if err := check(); err != nil {
			return err
		}
		if rejected != nil {
			rejected()
		}
		log.Printf("%s: certified persistence rejection", operation)
		return nil
	}
	var err error
	if len(participants) == 0 {
		if w.shuttingDown {
			return fail(errCriticalPersistenceStopping)
		}
		_, err = w.criticalWrites.submitWithRejection(write, apply, afterAck, reject)
	} else {
		_, err = w.submitCriticalWriteWithRejection(write, participants, false, apply, afterAck, reject)
	}
	if err != nil {
		return fail(err)
	}
	return true
}
