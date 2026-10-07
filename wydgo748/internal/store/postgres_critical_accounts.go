package store

import (
	"context"
	"errors"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
)

// SaveCriticalAccounts is the certified account-only adapter for the World
// coordinator. Unlike legacy SaveAccount/SaveAccounts, it distinguishes a
// definite transaction rejection from an unknown outcome. It shares their
// ledger transaction and autosave fences; it never falls back to separate saves.
func (s *PostgresStore) SaveCriticalAccounts(accounts ...*model.Account) (err error) {
	writeStarted := false
	defer func() {
		if err != nil && !writeStarted {
			err = rejectWrite(err)
		}
	}()
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	if len(accounts) == 0 {
		return errors.New("store: critical account write requires participants")
	}
	for _, account := range accounts {
		if account == nil {
			return errors.New("store: critical account write has a nil participant")
		}
	}
	seq := s.writeSeq.Add(1)
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	unlock := s.accountFences.lock(accountModelKeys(accounts...)...)
	defer unlock()
	snapshots, err := snapshotAccounts(accounts...)
	if err != nil {
		return err
	}
	s.fenceAccountSnapshots(seq, snapshots)
	writeStarted = true
	return s.withCertifiedSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		return persistAccountSnapshots(ctx, tx, snapshots, false)
	})
}
