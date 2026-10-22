package store

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"strings"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/control"
)

// PermanentSanction is the effective end of a sanction without expiry.
var PermanentSanction = time.Date(9999, 12, 31, 0, 0, 0, 0, time.UTC)

// SanctionState is the live effect of an account's active sanctions. A zero
// time means none; PermanentSanction means no expiry.
type SanctionState struct {
	BanUntil  time.Time
	MuteUntil time.Time
}

const sanctionColumns = `id, account_key, kind, reason, actor, duration_minutes, created_at, expires_at,
	revoked_at, revoked_by, revoke_reason,
	revoked_at IS NULL AND (expires_at IS NULL OR expires_at > now())`

func scanSanction(row pgx.Row) (control.Sanction, error) {
	var out control.Sanction
	err := row.Scan(&out.ID, &out.Account, &out.Kind, &out.Reason, &out.Actor, &out.DurationMinutes,
		&out.CreatedAt, &out.ExpiresAt, &out.RevokedAt, &out.RevokedBy, &out.RevokeReason, &out.Active)
	return out, err
}

// AddSanction records a sanction. A repeated request ID returns the existing
// row with duplicate=true; a different command under the same ID conflicts.
func (s *PostgresStore) AddSanction(ctx context.Context, actor string, cmd control.SanctionCommand) (control.Sanction, bool, error) {
	if s.readOnly {
		return control.Sanction{}, false, errors.New("store: PostgreSQL is read-only")
	}
	if cmd.Validate() != nil || actor == "" {
		return control.Sanction{}, false, errors.New("store: invalid sanction")
	}
	reason := strings.TrimSpace(cmd.Reason)
	out, err := scanSanction(s.pool.QueryRow(ctx, `
		INSERT INTO account_sanctions(account_key, kind, reason, actor, duration_minutes, request_id, expires_at)
		SELECT $1::text, $2::text, $3::text, $4::text, $5::integer, $6::text,
			CASE WHEN $5::integer = 0 THEN NULL ELSE now() + make_interval(mins => $5::integer) END
		WHERE EXISTS (SELECT 1 FROM accounts WHERE name_key=$1::text)
		ON CONFLICT (request_id) DO NOTHING
		RETURNING `+sanctionColumns,
		cmd.Account, cmd.Kind, reason, actor, cmd.DurationMinutes, cmd.RequestID))
	if err == nil {
		return out, false, nil
	}
	if !errors.Is(err, pgx.ErrNoRows) {
		return control.Sanction{}, false, err
	}
	existing, err := scanSanction(s.pool.QueryRow(ctx,
		`SELECT `+sanctionColumns+` FROM account_sanctions WHERE request_id=$1`, cmd.RequestID))
	if errors.Is(err, pgx.ErrNoRows) {
		return control.Sanction{}, false, fmt.Errorf("%w: account %q", errAccountMissing, cmd.Account)
	}
	if err != nil {
		return control.Sanction{}, false, err
	}
	if existing.Account != cmd.Account || existing.Kind != cmd.Kind || existing.Reason != reason ||
		existing.Actor != actor || existing.DurationMinutes != cmd.DurationMinutes {
		return control.Sanction{}, false, control.ErrSanctionConflict
	}
	return existing, true, nil
}

// RevokeSanction stamps one unrevoked sanction of the account. changed=false
// means it was already revoked by an earlier request.
func (s *PostgresStore) RevokeSanction(ctx context.Context, id int64, account, kind, actor, reason string) (control.Sanction, bool, error) {
	if s.readOnly {
		return control.Sanction{}, false, errors.New("store: PostgreSQL is read-only")
	}
	if id < 1 || !control.ValidAccountKey(account) || (kind != control.SanctionBan && kind != control.SanctionMute) ||
		actor == "" || strings.TrimSpace(reason) == "" {
		return control.Sanction{}, false, errors.New("store: invalid revocation")
	}
	out, err := scanSanction(s.pool.QueryRow(ctx, `
		UPDATE account_sanctions SET revoked_at=now(), revoked_by=$3, revoke_reason=$4
		WHERE id=$1 AND account_key=$2 AND kind=$5 AND revoked_at IS NULL
		RETURNING `+sanctionColumns, id, account, actor, strings.TrimSpace(reason), kind))
	if err == nil {
		return out, true, nil
	}
	if !errors.Is(err, pgx.ErrNoRows) {
		return control.Sanction{}, false, err
	}
	out, err = scanSanction(s.pool.QueryRow(ctx,
		`SELECT `+sanctionColumns+` FROM account_sanctions WHERE id=$1 AND account_key=$2 AND kind=$3`, id, account, kind))
	if errors.Is(err, pgx.ErrNoRows) {
		return control.Sanction{}, false, control.ErrUnknownSanction
	}
	return out, false, err
}

// ListSanctions returns the newest sanctions of an existing account.
func (s *PostgresStore) ListSanctions(ctx context.Context, account string, limit int) ([]control.Sanction, time.Time, error) {
	if !control.ValidAccountKey(account) || limit < 1 || limit > 200 {
		return nil, time.Time{}, errors.New("store: invalid sanction query")
	}
	var asOf time.Time
	var exists bool
	if err := s.pool.QueryRow(ctx, `SELECT now(), EXISTS (SELECT 1 FROM accounts WHERE name_key=$1)`, account).
		Scan(&asOf, &exists); err != nil {
		return nil, time.Time{}, err
	}
	if !exists {
		return nil, time.Time{}, fmt.Errorf("%w: account %q", errAccountMissing, account)
	}
	rows, err := s.pool.Query(ctx, `SELECT `+sanctionColumns+` FROM account_sanctions
		WHERE account_key=$1 ORDER BY id DESC LIMIT $2`, account, limit)
	if err != nil {
		return nil, time.Time{}, err
	}
	defer rows.Close()
	out := []control.Sanction{}
	for rows.Next() {
		sanction, err := scanSanction(rows)
		if err != nil {
			return nil, time.Time{}, err
		}
		out = append(out, sanction)
	}
	return out, asOf, rows.Err()
}

// ActiveSanctionState returns the latest end of each active sanction kind.
func (s *PostgresStore) ActiveSanctionState(ctx context.Context, account string) (SanctionState, error) {
	key := strings.ToLower(strings.TrimSpace(account))
	rows, err := s.pool.Query(ctx, `
		SELECT kind, MAX(COALESCE(expires_at, $2::timestamptz))
		FROM account_sanctions
		WHERE account_key=$1 AND revoked_at IS NULL AND (expires_at IS NULL OR expires_at > now())
		GROUP BY kind`, key, PermanentSanction)
	if err != nil {
		return SanctionState{}, err
	}
	defer rows.Close()
	var state SanctionState
	for rows.Next() {
		var kind string
		var until time.Time
		if err := rows.Scan(&kind, &until); err != nil {
			return SanctionState{}, err
		}
		switch kind {
		case control.SanctionBan:
			state.BanUntil = until.UTC()
		case control.SanctionMute:
			state.MuteUntil = until.UTC()
		}
	}
	return state, rows.Err()
}

// AppendStaffAudit persists one staff panel event.
func (s *PostgresStore) AppendStaffAudit(ctx context.Context, entry control.AuditEntry) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	if entry.Action == "" || entry.Result == "" {
		return errors.New("store: invalid audit entry")
	}
	details := entry.Details
	if details == nil {
		details = map[string]any{}
	}
	raw, err := json.Marshal(details)
	if err != nil {
		return err
	}
	_, err = s.pool.Exec(ctx, `
		INSERT INTO staff_audit(actor, action, result, request_id, client_ip, method, path, details)
		VALUES ($1, $2, $3, $4, $5, $6, $7, $8)`,
		entry.Actor, entry.Action, entry.Result, entry.RequestID, entry.ClientIP, entry.Method, entry.Path, raw)
	return err
}

// PruneStaffAudit deletes up to limit audit rows created before cutoff,
// oldest first. Ids grow with time, so the primary key bounds the scan.
func (s *PostgresStore) PruneStaffAudit(ctx context.Context, cutoff time.Time, limit int) (int64, error) {
	if s.readOnly {
		return 0, errors.New("store: PostgreSQL is read-only")
	}
	if limit < 1 || limit > 10000 {
		return 0, errors.New("store: invalid audit prune limit")
	}
	tag, err := s.pool.Exec(ctx, `
		DELETE FROM staff_audit WHERE id IN (
			SELECT id FROM staff_audit WHERE created_at < $1::timestamptz ORDER BY id LIMIT $2)`, cutoff, limit)
	if err != nil {
		return 0, err
	}
	return tag.RowsAffected(), nil
}

// ListStaffAudit pages the audit history newest first by keyset on id.
func (s *PostgresStore) ListStaffAudit(ctx context.Context, q control.AuditQuery) (control.AuditPage, error) {
	if q.Limit < 1 || q.Limit > 100 || q.Before < 0 {
		return control.AuditPage{}, errors.New("store: invalid audit query")
	}
	page := control.AuditPage{Version: 1, Entries: []control.AuditEntry{}}
	if err := s.pool.QueryRow(ctx, `SELECT now()`).Scan(&page.AsOf); err != nil {
		return control.AuditPage{}, err
	}
	rows, err := s.pool.Query(ctx, `
		SELECT id, created_at, actor, action, result, request_id, client_ip, method, path, details
		FROM staff_audit
		WHERE ($1::text = '' OR actor = $1::text) AND ($2::text = '' OR action = $2::text)
			AND ($3::bigint = 0 OR id < $3::bigint)
		ORDER BY id DESC LIMIT $4`, q.Actor, q.Action, q.Before, q.Limit+1)
	if err != nil {
		return control.AuditPage{}, err
	}
	defer rows.Close()
	for rows.Next() {
		var e control.AuditEntry
		var raw []byte
		if err := rows.Scan(&e.ID, &e.At, &e.Actor, &e.Action, &e.Result, &e.RequestID, &e.ClientIP,
			&e.Method, &e.Path, &raw); err != nil {
			return control.AuditPage{}, err
		}
		if err := json.Unmarshal(raw, &e.Details); err != nil {
			return control.AuditPage{}, fmt.Errorf("store: corrupted audit details %d: %w", e.ID, err)
		}
		page.Entries = append(page.Entries, e)
	}
	if err := rows.Err(); err != nil {
		return control.AuditPage{}, err
	}
	if len(page.Entries) > q.Limit {
		page.Entries = page.Entries[:q.Limit]
		page.NextBefore = page.Entries[q.Limit-1].ID
	}
	return page, nil
}
