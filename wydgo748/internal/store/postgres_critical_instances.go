package store

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
)

// SaveCriticalInstanceState commits the global instance aggregate and optional
// account participants atomically. Definite rejection is certified only before
// writing or after a proven abort; a lost commit response remains unknown.
func (s *PostgresStore) SaveCriticalInstanceState(state *model.InstanceStateSnapshot, accounts ...*model.Account) (err error) {
	writeStarted := false
	defer func() {
		if err != nil && !writeStarted {
			err = rejectWrite(err)
		}
	}()
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	if state == nil {
		return errors.New("store: critical instance write requires a snapshot")
	}
	if state.Version != model.InstanceStateVersion {
		return fmt.Errorf("store: instance state version %d; expected %d", state.Version, model.InstanceStateVersion)
	}
	for _, account := range accounts {
		if account == nil {
			return errors.New("store: critical instance write has a nil participant")
		}
	}
	payload, err := json.Marshal(state)
	if err != nil {
		return err
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
		if err := persistAccountSnapshots(ctx, tx, snapshots, false); err != nil {
			return err
		}
		_, err := tx.Exec(ctx, `
			INSERT INTO instance_state(singleton,payload) VALUES(true,$1)
			ON CONFLICT(singleton) DO UPDATE SET
				payload=EXCLUDED.payload,
				version=instance_state.version+1,
				updated_at=now()`, payload)
		return err
	})
}
