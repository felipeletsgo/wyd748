package store

import (
	"context"
	"errors"
	"time"

	"github.com/jackc/pgx/v5"
)

// Durable channel tickets (DOCS/server/server-correction-plan.md, W9 phase 2).
// When channels run in separate processes, the source channel stores a
// single-use transfer ticket before the player's final write and reveals the
// token only after that write commits. The target channel consumes the ticket
// in the same transaction that claims the account lease. Only a hash of the
// token is stored. A pending ticket blocks other logins of the account until
// it is consumed or expires.

const postgresChannelTicketsSchema = `
CREATE TABLE IF NOT EXISTS channel_tickets (
    token_hash       bytea PRIMARY KEY CHECK (length(token_hash) = 32),
    account_key      text NOT NULL CHECK (length(account_key) BETWEEN 1 AND 64),
    channel          smallint NOT NULL CHECK (channel BETWEEN 1 AND 255),
    slot             smallint NOT NULL CHECK (slot BETWEEN 0 AND 3),
    expires_at       timestamptz NOT NULL,
    consumed_by      text,
    consumed_session bigint
);
CREATE UNIQUE INDEX IF NOT EXISTS channel_tickets_pending_idx
    ON channel_tickets(account_key) WHERE consumed_by IS NULL;`

var (
	// ErrChannelTransferPending refuses a second transfer or a plain login
	// while a ticket of the account waits.
	ErrChannelTransferPending = errors.New("store: a channel transfer of the account is pending")
	// ErrChannelTicketInvalid refuses an unknown, expired, consumed or
	// mismatched ticket.
	ErrChannelTicketInvalid = errors.New("store: channel ticket is not valid")
)

// IssueChannelTicket stores a ticket for the account. Expired and consumed
// tickets of the account are removed first.
func (s *PostgresStore) IssueChannelTicket(tokenHash []byte, account string, channel byte, slot int, ttl time.Duration) error {
	key, err := postgresAccountKey(account)
	if err != nil {
		return err
	}
	ctx, cancel := s.postgresContext()
	defer cancel()
	tx, err := s.pool.BeginTx(ctx, pgx.TxOptions{IsoLevel: pgx.ReadCommitted})
	if err != nil {
		return err
	}
	defer func() { _ = tx.Rollback(context.Background()) }()
	if _, err := tx.Exec(ctx, `DELETE FROM channel_tickets WHERE account_key=$1
		AND (consumed_by IS NOT NULL OR expires_at <= now())`, key); err != nil {
		return err
	}
	_, err = tx.Exec(ctx, `INSERT INTO channel_tickets(token_hash,account_key,channel,slot,expires_at)
		VALUES($1,$2,$3,$4,now() + make_interval(secs => $5))`,
		tokenHash, key, int16(channel), int16(slot), ttl.Seconds())
	if isUniqueViolation(err) {
		return ErrChannelTransferPending
	}
	if err != nil {
		return err
	}
	return tx.Commit(ctx)
}

// ChannelTicketValid reports whether the ticket can admit the account on the
// channel. It does not consume the ticket.
func (s *PostgresStore) ChannelTicketValid(tokenHash []byte, account string, channel byte) (bool, error) {
	key, err := postgresAccountKey(account)
	if err != nil {
		return false, nil
	}
	ctx, cancel := s.postgresReadContext()
	defer cancel()
	var valid bool
	err = s.pool.QueryRow(ctx, `SELECT EXISTS(SELECT 1 FROM channel_tickets WHERE token_hash=$1
		AND account_key=$2 AND channel=$3 AND consumed_by IS NULL AND expires_at > now())`,
		tokenHash, key, int16(channel)).Scan(&valid)
	return valid, err
}

// ClaimAccountLeaseWithTicket consumes the ticket and gives the account lease
// to this process and session in one transaction. It returns the character
// slot of the ticket.
func (s *PostgresStore) ClaimAccountLeaseWithTicket(account, process string, channel byte, session int64, tokenHash []byte) (int, error) {
	key, err := postgresAccountKey(account)
	if err != nil {
		return 0, err
	}
	ctx, cancel := s.postgresReadContext()
	defer cancel()
	tx, err := s.pool.BeginTx(ctx, pgx.TxOptions{IsoLevel: pgx.ReadCommitted})
	if err != nil {
		return 0, err
	}
	defer func() { _ = tx.Rollback(context.Background()) }()
	var slot int16
	err = tx.QueryRow(ctx, `UPDATE channel_tickets SET consumed_by=$4, consumed_session=$5
		WHERE token_hash=$1 AND account_key=$2 AND channel=$3 AND consumed_by IS NULL AND expires_at > now()
		RETURNING slot`, tokenHash, key, int16(channel), process, session).Scan(&slot)
	if errors.Is(err, pgx.ErrNoRows) {
		return 0, ErrChannelTicketInvalid
	}
	if err != nil {
		return 0, err
	}
	if _, err := claimAccountLeaseTx(ctx, tx, key, process, channel, session, true); err != nil {
		return 0, err
	}
	return int(slot), tx.Commit(ctx)
}

// CancelChannelTicket removes an unconsumed ticket that the source could not
// deliver.
func (s *PostgresStore) CancelChannelTicket(tokenHash []byte) error {
	ctx, cancel := s.postgresContext()
	defer cancel()
	_, err := s.pool.Exec(ctx, `DELETE FROM channel_tickets WHERE token_hash=$1 AND consumed_by IS NULL`, tokenHash)
	return err
}
