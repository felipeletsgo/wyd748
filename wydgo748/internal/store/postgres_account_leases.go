package store

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"errors"
	"expvar"
	"strings"
	"time"

	"github.com/jackc/pgx/v5"
)

// Account ownership leases (DOCS/server/server-correction-plan.md, W9 phase 1).
// Each game process registers itself with a heartbeat; each signed-in account
// has one lease naming its process, channel and session. A lease of another
// live process refuses the login; a lease whose process stopped beating is
// taken over. With one process this mirrors the in-memory claim, and it is
// what keeps login single once channels run in separate processes.

const postgresAccountLeasesSchema = `
CREATE TABLE IF NOT EXISTS game_processes (
    process      text PRIMARY KEY CHECK (length(process) BETWEEN 1 AND 80),
    started_at   timestamptz NOT NULL DEFAULT now(),
    heartbeat_at timestamptz NOT NULL DEFAULT now()
);
CREATE TABLE IF NOT EXISTS account_leases (
    account_key text PRIMARY KEY CHECK (length(account_key) BETWEEN 1 AND 64),
    process     text NOT NULL CHECK (length(process) BETWEEN 1 AND 80),
    channel     smallint NOT NULL CHECK (channel BETWEEN 0 AND 255),
    session     bigint NOT NULL,
    claimed_at  timestamptz NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS account_leases_process_idx ON account_leases(process);`

// AccountLeaseStaleAfter is how long a process may miss heartbeats before
// another process takes over its leases. Heartbeats run every third of it.
const AccountLeaseStaleAfter = 30 * time.Second

// RegisterGameProcess records this process and removes the leases of
// processes that stopped beating. A process that fenced the previous game
// instances at startup is the only one: it also removes the leases of every
// other process, so players who reconnect after a restart are not refused.
// It returns the process name.
func (s *PostgresStore) RegisterGameProcess() (string, error) {
	name, exclusive := s.gameInstance, s.gameInstance != ""
	if name == "" {
		var id [8]byte
		if _, err := rand.Read(id[:]); err != nil {
			return "", err
		}
		name = gameInstanceApplicationPrefix + hex.EncodeToString(id[:])
	}
	ctx, cancel := s.postgresContext()
	defer cancel()
	tx, err := s.pool.Begin(ctx)
	if err != nil {
		return "", err
	}
	defer func() { _ = tx.Rollback(context.Background()) }()
	if _, err := tx.Exec(ctx, `INSERT INTO game_processes(process) VALUES($1)
		ON CONFLICT(process) DO UPDATE SET heartbeat_at=now()`, name); err != nil {
		return "", err
	}
	// An exclusive process ends the processes that its fence covers: every
	// game process, or for a channel process its channel's previous process
	// and a previous whole-server process.
	if _, err := tx.Exec(ctx, `DELETE FROM game_processes WHERE process <> $2
		AND (($4 AND (NOT $5 OR process LIKE $1 OR process ~ '^wydgo-game/[0-9a-f]+$'))
			OR heartbeat_at < now() - make_interval(secs => $3))`,
		gameInstancePrefix(s.fenceChannel)+"%", name, AccountLeaseStaleAfter.Seconds(), exclusive, s.fenceChannel != 0); err != nil {
		return "", err
	}
	if _, err := tx.Exec(ctx, `DELETE FROM account_leases l WHERE NOT EXISTS (
		SELECT 1 FROM game_processes p WHERE p.process = l.process)`); err != nil {
		return "", err
	}
	if err := tx.Commit(ctx); err != nil {
		return "", err
	}
	s.leaseProcess.Store(&name)
	return name, nil
}

// HeartbeatGameProcess renews this process's liveness.
func (s *PostgresStore) HeartbeatGameProcess(process string) error {
	ctx, cancel := s.postgresContext()
	defer cancel()
	_, err := s.pool.Exec(ctx, `INSERT INTO game_processes(process) VALUES($1)
		ON CONFLICT(process) DO UPDATE SET heartbeat_at=now()`, process)
	return err
}

// ClaimAccountLease gives the account to this process and session. It
// returns the owning process when another live process holds the account,
// and ErrChannelTransferPending while a channel ticket of the account waits.
func (s *PostgresStore) ClaimAccountLease(account, process string, channel byte, session int64) (string, error) {
	key, err := postgresAccountKey(account)
	if err != nil {
		return "", err
	}
	ctx, cancel := s.postgresReadContext()
	defer cancel()
	tx, err := s.pool.BeginTx(ctx, pgx.TxOptions{IsoLevel: pgx.ReadCommitted})
	if err != nil {
		return "", err
	}
	defer func() { _ = tx.Rollback(context.Background()) }()
	var pending bool
	if err := tx.QueryRow(ctx, `SELECT EXISTS(SELECT 1 FROM channel_tickets
		WHERE account_key=$1 AND consumed_by IS NULL AND expires_at > now())`, key).Scan(&pending); err != nil {
		return "", err
	}
	if pending {
		return "", ErrChannelTransferPending
	}
	owner, err := claimAccountLeaseTx(ctx, tx, key, process, channel, session, false)
	if err != nil {
		return "", err
	}
	return owner, tx.Commit(ctx)
}

// claimAccountLeaseTx claims the lease in tx. With force, a live owner is
// replaced too: a consumed channel ticket proves that the source gave up the
// account after its final write.
func claimAccountLeaseTx(ctx context.Context, tx pgx.Tx, key, process string, channel byte, session int64, force bool) (string, error) {
	var owner string
	var alive bool
	err := tx.QueryRow(ctx, `SELECT l.process, COALESCE(p.heartbeat_at >= now() - make_interval(secs => $2), false)
		FROM account_leases l LEFT JOIN game_processes p ON p.process = l.process
		WHERE l.account_key=$1 FOR UPDATE OF l`, key, AccountLeaseStaleAfter.Seconds()).Scan(&owner, &alive)
	switch {
	case errors.Is(err, pgx.ErrNoRows):
		if _, err := tx.Exec(ctx, `INSERT INTO account_leases(account_key,process,channel,session) VALUES($1,$2,$3,$4)
			ON CONFLICT(account_key) DO NOTHING`, key, process, int16(channel), session); err != nil {
			return "", err
		}
		// A concurrent claim may have inserted first; read the winner.
		if err := tx.QueryRow(ctx, `SELECT process FROM account_leases WHERE account_key=$1`, key).Scan(&owner); err != nil {
			return "", err
		}
		if owner == process {
			return "", nil
		}
		if !force {
			return owner, nil
		}
		return "", errors.New("store: account lease changed during a channel transfer")
	case err != nil:
		return "", err
	case owner != process && alive && !force:
		return owner, nil
	}
	// Ours, a stopped process, or a consumed transfer: take it for this session.
	if _, err := tx.Exec(ctx, `UPDATE account_leases SET process=$2,channel=$3,session=$4,claimed_at=now()
		WHERE account_key=$1`, key, process, int16(channel), session); err != nil {
		return "", err
	}
	return "", nil
}

// ReleaseAccountLease ends one session's lease. A later session's claim of
// the same account is kept.
func (s *PostgresStore) ReleaseAccountLease(account, process string, session int64) error {
	key, err := postgresAccountKey(account)
	if err != nil {
		return err
	}
	ctx, cancel := s.postgresContext()
	defer cancel()
	_, err = s.pool.Exec(ctx, `DELETE FROM account_leases WHERE account_key=$1 AND process=$2 AND session=$3`,
		key, process, session)
	return err
}

// ReconcileAccountLeases removes this process's leases for accounts it no
// longer holds. Leases younger than the stale bound are kept, so a claim
// made after the held list was taken survives.
func (s *PostgresStore) ReconcileAccountLeases(process string, held []string) error {
	keys := make([]string, 0, len(held))
	for _, account := range held {
		if key, err := postgresAccountKey(account); err == nil {
			keys = append(keys, key)
		}
	}
	ctx, cancel := s.postgresContext()
	defer cancel()
	_, err := s.pool.Exec(ctx, `DELETE FROM account_leases WHERE process=$1 AND NOT (account_key = ANY($2::text[]))
		AND claimed_at < now() - make_interval(secs => $3)`, process, keys, AccountLeaseStaleAfter.Seconds())
	return err
}

// ReleaseProcessLeases ends every lease of this process at shutdown.
func (s *PostgresStore) ReleaseProcessLeases(process string) error {
	ctx, cancel := s.postgresContext()
	defer cancel()
	if _, err := s.pool.Exec(ctx, `DELETE FROM account_leases WHERE process=$1`, process); err != nil {
		return err
	}
	_, err := s.pool.Exec(ctx, `DELETE FROM game_processes WHERE process=$1`, process)
	return err
}

// ErrAccountLeaseFenced rejects an account write of a process that does not
// own the account: another live process holds its lease.
var ErrAccountLeaseFenced = errors.New("store: another game process owns the account")

var metricAccountLeaseFenced = expvar.NewInt("postgres_account_lease_fenced_total")

// persistAccountSnapshots writes account rows after the lease fence. In a
// registered game process the write locks the accounts' lease rows and is
// rejected when another live process owns one. The shared lock keeps a
// takeover waiting until this transaction ends, so the new owner reads the
// committed state. Without registration (tools, tests) there is no fence.
func (s *PostgresStore) persistAccountSnapshots(ctx context.Context, tx pgx.Tx,
	snapshots []*accountSnapshot, createOnly bool) error {
	if process := s.leaseProcess.Load(); process != nil && len(snapshots) > 0 {
		keys := make([]string, len(snapshots))
		for i, snapshot := range snapshots {
			keys[i] = snapshot.key
		}
		rows, err := tx.Query(ctx, `SELECT l.process, COALESCE(p.heartbeat_at >= now() - make_interval(secs => $2), false)
			FROM account_leases l LEFT JOIN game_processes p ON p.process = l.process
			WHERE l.account_key = ANY($1::text[]) ORDER BY l.account_key FOR SHARE OF l`,
			keys, AccountLeaseStaleAfter.Seconds())
		if err != nil {
			return err
		}
		fenced := false
		for rows.Next() {
			var owner string
			var alive bool
			if err := rows.Scan(&owner, &alive); err != nil {
				rows.Close()
				return err
			}
			fenced = fenced || (owner != *process && alive)
		}
		rows.Close()
		if err := rows.Err(); err != nil {
			return err
		}
		if fenced {
			metricAccountLeaseFenced.Add(1)
			return ErrAccountLeaseFenced
		}
	}
	return writeAccountSnapshots(ctx, tx, snapshots, createOnly)
}

func postgresAccountKey(account string) (string, error) {
	key := strings.ToLower(strings.TrimSpace(account))
	if key == "" {
		return "", errors.New("store: account name is empty")
	}
	return key, nil
}
