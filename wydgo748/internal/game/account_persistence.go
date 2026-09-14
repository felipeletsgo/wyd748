package game

import "wydgo/internal/model"

// accountStateSnapshot makes a deep copy of the live account without changing
// session-derived fields. It is used by cross-aggregate transactions that
// must restore the exact in-memory state after a persistence failure.
func accountStateSnapshot(account *model.Account) *model.Account {
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
	return &snapshot
}

// restoreAccountState restores a prior live-account snapshot without aliasing
// its mutable character slices into the active session.
func restoreAccountState(account, snapshot *model.Account) {
	if account == nil || snapshot == nil {
		return
	}
	*account = *snapshot
	account.Chars = make([]model.Char, len(snapshot.Chars))
	for i := range snapshot.Chars {
		account.Chars[i] = cloneCharacterState(&snapshot.Chars[i])
	}
	account.CelestialCapsules = make([]model.CelestialCapsule, len(snapshot.CelestialCapsules))
	for i := range snapshot.CelestialCapsules {
		account.CelestialCapsules[i] = snapshot.CelestialCapsules[i]
		account.CelestialCapsules[i].Character = cloneCharacterState(
			&snapshot.CelestialCapsules[i].Character)
	}
}

// accountPersistenceSnapshot projects the live account into the durable
// representation. Session position is deliberately not persisted: Char.X/Y is
// normalized to the neutral legacy entry point, while the actual login/recall
// city is selected from the hometown bits in Score.Merchant. Player.X/Y and
// Char.X/Y remain coherent for the whole lifetime of the active session.
//
// The copy is deep for every mutable reference currently present in Account.
// Persistence adapters must never mutate the actor-owned aggregate merely to
// fit the on-disk/database representation.
func accountPersistenceSnapshot(account *model.Account) *model.Account {
	snapshot := accountStateSnapshot(account)
	if snapshot == nil {
		return nil
	}
	pinAccountEntryPositions(snapshot)
	return snapshot
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
