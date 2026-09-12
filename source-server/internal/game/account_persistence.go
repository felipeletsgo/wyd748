package game

import "wydgo/internal/model"

// accountPersistenceSnapshot projects the live account into the durable
// representation. Session position is deliberately not persisted: every
// character re-enters at the fixed spawn, while Player.X/Y and Char.X/Y remain
// coherent for the whole lifetime of the active session.
//
// The copy is deep for every mutable reference currently present in Account.
// Persistence adapters must never mutate the actor-owned aggregate merely to
// fit the on-disk/database representation.
func accountPersistenceSnapshot(account *model.Account) *model.Account {
	if account == nil {
		return nil
	}
	snapshot := *account
	snapshot.Chars = make([]model.Char, len(account.Chars))
	for i := range account.Chars {
		snapshot.Chars[i] = cloneCharacterState(&account.Chars[i])
	}
	snapshot.CelestialCapsules = make([]model.CelestialCapsule, len(account.CelestialCapsules))
	for i := range account.CelestialCapsules {
		snapshot.CelestialCapsules[i] = account.CelestialCapsules[i]
		snapshot.CelestialCapsules[i].Character = cloneCharacterState(
			&account.CelestialCapsules[i].Character)
	}
	pinAccountEntryPositions(&snapshot)
	return &snapshot
}

func accountPersistenceSnapshots(accounts ...*model.Account) []*model.Account {
	snapshots := make([]*model.Account, 0, len(accounts))
	for _, account := range accounts {
		if account != nil {
			snapshots = append(snapshots, accountPersistenceSnapshot(account))
		}
	}
	return snapshots
}
