package store

import (
	"context"
	"errors"
	"fmt"
	"time"

	"github.com/jackc/pgx/v5"
)

type postgresCriticalBegin func(context.Context, pgx.TxOptions) (pgx.Tx, error)

// withCertifiedSerializableTx is used by migrated critical adapters. A callback
// must use only its supplied transaction and must never commit it itself. Unlike
// an ordinary error return, a successful rollback or a known server-side abort
// proves rejection. A transport error during COMMIT remains unknown, even when
// a subsequent rollback succeeds; retrying could duplicate committed effects.
func (s *PostgresStore) withCertifiedSerializableTx(run func(context.Context, pgx.Tx) error) error {
	started := time.Now()
	defer observePostgresTransaction(started)
	ctx, cancel := s.postgresContext()
	defer cancel()
	return certifiedSerializableTx(ctx, s.pool.BeginTx, run)
}

func certifiedSerializableTx(ctx context.Context, begin postgresCriticalBegin,
	run func(context.Context, pgx.Tx) error) error {
	var last error
	for attempt := 0; attempt < 3; attempt++ {
		if err := ctx.Err(); err != nil {
			return rejectWrite(fmt.Errorf("store: critical transaction deadline: %w", err))
		}
		tx, err := begin(ctx, pgx.TxOptions{IsoLevel: pgx.Serializable})
		if err != nil {
			return rejectWrite(err) // No transaction callback has run.
		}
		err = certifiedTransactionAttempt(ctx, tx, run)
		if err == nil || !IsWriteRejected(err) || !isRetryablePostgres(err) {
			return err
		}
		last = err
		if attempt == 2 {
			break
		}
		metricPostgresTxRetries.Add(1)
		timer := time.NewTimer(time.Duration(attempt+1) * 10 * time.Millisecond)
		select {
		case <-timer.C:
		case <-ctx.Done():
			timer.Stop()
			return rejectWrite(fmt.Errorf("store: critical transaction deadline: %w", ctx.Err()))
		}
	}
	return rejectWrite(fmt.Errorf("store: critical transaction retry limit: %w", last))
}

func certifiedTransactionAttempt(ctx context.Context, tx pgx.Tx,
	run func(context.Context, pgx.Tx) error) error {
	// Also release the transaction on callback panic. The caller still treats
	// the panic as unknown rather than certifying its outcome.
	defer func() { _ = tx.Rollback(ctx) }()
	if err := run(ctx, tx); err != nil {
		if rollbackErr := tx.Rollback(ctx); rollbackErr != nil {
			return indeterminateWrite(fmt.Errorf("store: rollback outcome unknown: %w", errors.Join(err, rollbackErr)))
		}
		return rejectWrite(err)
	}
	err := tx.Commit(ctx)
	if err == nil {
		return nil
	}
	if errors.Is(err, pgx.ErrTxCommitRollback) || isRetryablePostgres(err) {
		return rejectWrite(err) // pgx/server explicitly reports an aborted commit.
	}
	return indeterminateWrite(fmt.Errorf("store: commit outcome unknown: %w", err))
}
