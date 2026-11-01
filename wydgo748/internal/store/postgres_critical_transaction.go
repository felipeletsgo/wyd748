package store

import (
	"context"
	"errors"
	"fmt"
	"log"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgconn"
)

// postgresCleanupTimeout bounds rollback independently of the operation
// deadline. An expired operation context cannot release a transaction, and
// cleanup never sends COMMIT, so it cannot extend a publication deadline.
const postgresCleanupTimeout = time.Second

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
	return s.certifiedTxWithReconciliation(ctx, s.pool.BeginTx, run, postgresReconcileBudget)
}

// certifiedTxWithReconciliation records each attempt's top-level transaction
// ID before its callback. Only a COMMIT that may have reached the server stays
// unknown; its exact ID is then resolved before the result leaves the worker.
func (s *PostgresStore) certifiedTxWithReconciliation(ctx context.Context, begin postgresCriticalBegin,
	run func(context.Context, pgx.Tx) error, budget time.Duration) error {
	var xid string
	err := certifiedSerializableTx(ctx, begin, func(ctx context.Context, tx pgx.Tx) error {
		xid = ""
		if err := tx.QueryRow(ctx, `SELECT pg_current_xact_id()::text`).Scan(&xid); err != nil {
			return err
		}
		return run(ctx, tx)
	})
	if err == nil || IsWriteRejected(err) || xid == "" {
		return err
	}
	return s.reconcileUnknownCommit(xid, err, budget)
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
	// the panic as unknown rather than certifying its outcome. After COMMIT the
	// transaction is closed and this rollback sends nothing.
	defer func() { _ = rollbackCertifiedTx(tx) }()
	if err := run(ctx, tx); err != nil {
		// Callbacks use only tx and never commit it. COMMIT was not sent, so the
		// server cannot make this attempt durable. A failed rollback makes pgx
		// discard the connection; the server aborts the open transaction.
		if rollbackErr := rollbackCertifiedTx(tx); rollbackErr != nil {
			err = errors.Join(err, fmt.Errorf("store: rollback before commit failed; connection discarded: %w", rollbackErr))
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
	// pgconn reports a closed connection as safe to retry even when it closed
	// after COMMIT was sent (receiveMessage on an asynchronously closed
	// connection). Only the transaction's exact status can settle that case.
	if pgconn.SafeToRetry(err) && !errors.Is(err, pgconn.ErrConnClosed) {
		// pgconn certifies that COMMIT never reached the server, for example
		// after the operation deadline expired. pgx closes that connection.
		return rejectWrite(fmt.Errorf("store: commit not sent: %w", err))
	}
	return indeterminateWrite(fmt.Errorf("store: commit outcome unknown: %w", err))
}

func rollbackCertifiedTx(tx pgx.Tx) error {
	ctx, cancel := context.WithTimeout(context.Background(), postgresCleanupTimeout)
	defer cancel()
	err := tx.Rollback(ctx)
	if errors.Is(err, pgx.ErrTxClosed) {
		return nil
	}
	return err
}

// postgresReconcileBudget bounds how long a worker resolves an unknown COMMIT.
// An unresolved result keeps the existing containment and fences.
const postgresReconcileBudget = 3 * time.Second

// reconcileUnknownCommit asks PostgreSQL for the final status of the exact
// transaction. "committed" and "aborted" are final; "in progress", NULL and
// read errors stay unknown until the budget ends. The unknown cause is quoted,
// not wrapped, because a wrapped unknown error would dominate the rejection.
func (s *PostgresStore) reconcileUnknownCommit(xid string, unknown error, budget time.Duration) error {
	deadline := time.Now().Add(budget)
	delay := 50 * time.Millisecond
	for {
		status, err := s.transactionStatus(xid)
		switch {
		case err == nil && status == "committed":
			metricPostgresCommitReconciled.Add("committed", 1)
			log.Printf("store: unknown COMMIT of transaction %s reconciled as committed (%v)", xid, unknown)
			return nil
		case err == nil && status == "aborted":
			metricPostgresCommitReconciled.Add("aborted", 1)
			return rejectWrite(fmt.Errorf("store: unknown COMMIT of transaction %s reconciled as aborted: %v", xid, unknown))
		}
		if !time.Now().Add(delay).Before(deadline) {
			metricPostgresCommitReconciled.Add("unresolved", 1)
			return indeterminateWrite(&UnresolvedCommitError{XID: xid,
				Cause: fmt.Errorf("store: transaction %s unresolved (status %q, read error %v): %w", xid, status, err, unknown)})
		}
		time.Sleep(delay)
		delay = min(2*delay, 400*time.Millisecond)
	}
}

// UnresolvedCommitError names the exact transaction of an unknown COMMIT, so
// its owner can ask TransactionStatus again later. It never certifies anything.
type UnresolvedCommitError struct {
	XID   string
	Cause error
}

func (e *UnresolvedCommitError) Error() string { return e.Cause.Error() }
func (e *UnresolvedCommitError) Unwrap() error { return e.Cause }

// TransactionStatus returns PostgreSQL's status of one top-level transaction:
// "committed", "aborted", "in progress" or "" when it is unknown.
func (s *PostgresStore) TransactionStatus(xid string) (string, error) {
	return s.transactionStatus(xid)
}

func (s *PostgresStore) transactionStatus(xid string) (string, error) {
	ctx, cancel := context.WithTimeout(context.Background(), time.Second)
	defer cancel()
	var status *string
	if err := s.pool.QueryRow(ctx, `SELECT pg_xact_status($1::text::xid8)`, xid).Scan(&status); err != nil {
		return "", err
	}
	if status == nil {
		return "", nil
	}
	return *status, nil
}
