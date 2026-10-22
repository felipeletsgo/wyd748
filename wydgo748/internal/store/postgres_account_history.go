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
	"wydgo/internal/model"
)

// RecordAccountLogin stores one admitted game login. A missing account row
// is ignored: the login itself already succeeded.
func (s *PostgresStore) RecordAccountLogin(ctx context.Context, account, clientIP string, channel int) error {
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	key := strings.ToLower(strings.TrimSpace(account))
	if !control.ValidAccountKey(key) || len(clientIP) > 64 || channel < 1 || channel > 255 {
		return errors.New("store: invalid login record")
	}
	_, err := s.pool.Exec(ctx, `
		INSERT INTO account_logins(account_key, client_ip, channel)
		SELECT $1::text, $2::text, $3::smallint WHERE EXISTS (SELECT 1 FROM accounts WHERE name_key=$1::text)`,
		key, clientIP, channel)
	return err
}

// ListAccountLogins returns the newest logins of an existing account.
func (s *PostgresStore) ListAccountLogins(ctx context.Context, account string, limit int) ([]control.AccountLogin, time.Time, error) {
	if !control.ValidAccountKey(account) || limit < 1 || limit > 200 {
		return nil, time.Time{}, errors.New("store: invalid login query")
	}
	asOf, err := s.accountExistsAt(ctx, account)
	if err != nil {
		return nil, time.Time{}, err
	}
	rows, err := s.pool.Query(ctx, `SELECT created_at, client_ip, channel FROM account_logins
		WHERE account_key=$1 ORDER BY id DESC LIMIT $2`, account, limit)
	if err != nil {
		return nil, time.Time{}, err
	}
	defer rows.Close()
	out := []control.AccountLogin{}
	for rows.Next() {
		var login control.AccountLogin
		var channel int16
		if err := rows.Scan(&login.At, &login.ClientIP, &channel); err != nil {
			return nil, time.Time{}, err
		}
		login.Channel = int(channel)
		out = append(out, login)
	}
	return out, asOf, rows.Err()
}

// ListStaffAuditForAccount returns the newest staff actions naming the
// account as target, through the expression index.
func (s *PostgresStore) ListStaffAuditForAccount(ctx context.Context, account string, limit int) ([]control.AuditEntry, time.Time, error) {
	if !control.ValidAccountKey(account) || limit < 1 || limit > 100 {
		return nil, time.Time{}, errors.New("store: invalid audit query")
	}
	var asOf time.Time
	if err := s.pool.QueryRow(ctx, `SELECT now()`).Scan(&asOf); err != nil {
		return nil, time.Time{}, err
	}
	rows, err := s.pool.Query(ctx, `
		SELECT id, created_at, actor, action, result, request_id, client_ip, method, path, details
		FROM staff_audit
		WHERE COALESCE(details->>'target_account', details->>'account') = $1
		ORDER BY id DESC LIMIT $2`, account, limit)
	if err != nil {
		return nil, time.Time{}, err
	}
	defer rows.Close()
	out := []control.AuditEntry{}
	for rows.Next() {
		var e control.AuditEntry
		var raw []byte
		if err := rows.Scan(&e.ID, &e.At, &e.Actor, &e.Action, &e.Result, &e.RequestID, &e.ClientIP, &e.Method, &e.Path, &raw); err != nil {
			return nil, time.Time{}, err
		}
		if err := json.Unmarshal(raw, &e.Details); err != nil {
			return nil, time.Time{}, fmt.Errorf("store: corrupted audit details %d: %w", e.ID, err)
		}
		out = append(out, e)
	}
	return out, asOf, rows.Err()
}

// PruneAccountLogins deletes up to limit login rows created before cutoff.
func (s *PostgresStore) PruneAccountLogins(ctx context.Context, cutoff time.Time, limit int) (int64, error) {
	if s.readOnly {
		return 0, errors.New("store: PostgreSQL is read-only")
	}
	if limit < 1 || limit > 10000 {
		return 0, errors.New("store: invalid login prune limit")
	}
	tag, err := s.pool.Exec(ctx, `
		DELETE FROM account_logins WHERE id IN (
			SELECT id FROM account_logins WHERE created_at < $1::timestamptz ORDER BY id LIMIT $2)`, cutoff, limit)
	if err != nil {
		return 0, err
	}
	return tag.RowsAffected(), nil
}

func (s *PostgresStore) accountExistsAt(ctx context.Context, account string) (time.Time, error) {
	var asOf time.Time
	var exists bool
	if err := s.pool.QueryRow(ctx, `SELECT now(), EXISTS (SELECT 1 FROM accounts WHERE name_key=$1)`, account).
		Scan(&asOf, &exists); err != nil {
		return time.Time{}, err
	}
	if !exists {
		return time.Time{}, fmt.Errorf("%w: account %q", errAccountMissing, account)
	}
	return asOf, nil
}

// Occupied cells only; reserved carry cells beyond the UI range are excluded.
// Named equipment keys are the established persistence contract.
const characterStorageSQL = `
SELECT picked.character->>'uid', picked.character->>'name',
    COALESCE((picked.character->>'gold')::bigint, 0), COALESCE((picked.payload->>'cargoGold')::bigint, 0),
    COALESCE((SELECT jsonb_agg(jsonb_build_object('slot', e.slot, 'itemId', (picked.character->'equip'->e.key->>'index')::integer,
        'effects', COALESCE(picked.character->'equip'->e.key->'eff', '[0,0,0,0,0,0]'::jsonb)) ORDER BY e.slot)
      FROM (VALUES ('rosto',0),('elmo',1),('armadura',2),('calca',3),('luva',4),('bota',5),
        ('arma',6),('escudo',7),('acessorio1',8),('acessorio2',9),('acessorio3',10),
        ('acessorio4',11),('acessorio5',12),('acessorio6',13),('montaria',14),('capa',15)) AS e(key, slot)
      WHERE COALESCE((picked.character->'equip'->e.key->>'index')::integer, 0) > 0), '[]'::jsonb),
    COALESCE((SELECT jsonb_agg(jsonb_build_object('slot', ordinal-1, 'itemId', (item->>'index')::integer,
        'effects', COALESCE(item->'eff', '[0,0,0,0,0,0]'::jsonb)) ORDER BY ordinal)
      FROM jsonb_array_elements(picked.character->'inv') WITH ORDINALITY AS cells(item, ordinal)
      WHERE ordinal <= $4::integer AND COALESCE((item->>'index')::integer, 0) > 0), '[]'::jsonb),
    COALESCE((SELECT jsonb_agg(jsonb_build_object('slot', ordinal-1, 'itemId', (item->>'index')::integer,
        'effects', COALESCE(item->'eff', '[0,0,0,0,0,0]'::jsonb)) ORDER BY ordinal)
      FROM jsonb_array_elements(picked.payload->'cargo') WITH ORDINALITY AS cells(item, ordinal)
      WHERE ordinal <= $5::integer AND COALESCE((item->>'index')::integer, 0) > 0), '[]'::jsonb),
    picked.updated_at
FROM (
    SELECT a.payload, a.updated_at, a.payload->'chars'->c.slot AS character
    FROM characters c JOIN accounts a ON a.name_key = c.account_key
    WHERE c.account_key=$1 AND c.character_uid=$2
) AS picked
WHERE picked.character->>'uid'=$3 AND COALESCE(picked.character->>'name', '') <> ''
LIMIT 1`

// ReadCharacterStorage reads the saved equipment, carry inventory and Cargo of
// one character. Item names are added by the caller from the item catalog.
func (s *PostgresStore) ReadCharacterStorage(ctx context.Context, account, uid string) (control.CharacterStorage, error) {
	key := strings.ToLower(strings.TrimSpace(account))
	normalizedUID, err := model.NormalizeCharacterUID(uid)
	if !control.ValidAccountKey(key) || err != nil || normalizedUID == "" {
		return control.CharacterStorage{}, errors.New("store: invalid storage target")
	}
	out := control.CharacterStorage{Version: 1, Account: key}
	var gold, cargoGold int64
	var equipment, inventory, cargo []byte
	err = s.pool.QueryRow(ctx, characterStorageSQL, key, currentUID(normalizedUID), normalizedUID,
		model.PlayerCarrySlots, model.PlayerCargoSlots).
		Scan(&out.UID, &out.Name, &gold, &cargoGold, &equipment, &inventory, &cargo, &out.AsOf)
	if errors.Is(err, pgx.ErrNoRows) {
		return control.CharacterStorage{}, fmt.Errorf("%w: character %q", errAccountMissing, uid)
	}
	if err != nil {
		return control.CharacterStorage{}, err
	}
	out.Gold, out.CargoGold = uint32(max(gold, 0)), uint64(max(cargoGold, 0))
	for _, part := range []struct {
		raw    []byte
		target *[]control.StoredItem
	}{{equipment, &out.Equipment}, {inventory, &out.Inventory}, {cargo, &out.Cargo}} {
		if err := json.Unmarshal(part.raw, part.target); err != nil {
			return control.CharacterStorage{}, fmt.Errorf("store: corrupted storage projection: %w", err)
		}
	}
	return out, nil
}

// ListAccountNotes returns the newest case notes of an existing account.
func (s *PostgresStore) ListAccountNotes(ctx context.Context, account string, limit int) ([]control.AccountNote, time.Time, error) {
	if !control.ValidAccountKey(account) || limit < 1 || limit > 100 {
		return nil, time.Time{}, errors.New("store: invalid note query")
	}
	asOf, err := s.accountExistsAt(ctx, account)
	if err != nil {
		return nil, time.Time{}, err
	}
	rows, err := s.pool.Query(ctx, `SELECT id, created_at, actor, body, bug_refs FROM account_notes
		WHERE account_key=$1 ORDER BY id DESC LIMIT $2`, account, limit)
	if err != nil {
		return nil, time.Time{}, err
	}
	defer rows.Close()
	out := []control.AccountNote{}
	for rows.Next() {
		var note control.AccountNote
		if err := rows.Scan(&note.ID, &note.At, &note.Actor, &note.Body, &note.BugRefs); err != nil {
			return nil, time.Time{}, err
		}
		if note.BugRefs == nil {
			note.BugRefs = []string{}
		}
		out = append(out, note)
	}
	return out, asOf, rows.Err()
}

// AddAccountNote appends one case note to an existing account.
func (s *PostgresStore) AddAccountNote(ctx context.Context, account, actor string, cmd control.AccountNoteCommand) (control.AccountNote, error) {
	if s.readOnly {
		return control.AccountNote{}, errors.New("store: PostgreSQL is read-only")
	}
	if !control.ValidAccountKey(account) || actor == "" || cmd.Validate() != nil {
		return control.AccountNote{}, errors.New("store: invalid note")
	}
	refs := cmd.BugRefs
	if refs == nil {
		refs = []string{}
	}
	note := control.AccountNote{Actor: actor, Body: strings.TrimSpace(cmd.Body), BugRefs: refs}
	err := s.pool.QueryRow(ctx, `
		INSERT INTO account_notes(account_key, actor, body, bug_refs)
		SELECT $1::text, $2::text, $3::text, $4::text[] WHERE EXISTS (SELECT 1 FROM accounts WHERE name_key=$1::text)
		RETURNING id, created_at`, account, actor, note.Body, refs).Scan(&note.ID, &note.At)
	if errors.Is(err, pgx.ErrNoRows) {
		return control.AccountNote{}, fmt.Errorf("%w: account %q", errAccountMissing, account)
	}
	return note, err
}
