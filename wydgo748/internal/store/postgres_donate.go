package store

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"strings"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
)

// LoadPremiumCatalog returns the staff catalog; version 0 means none was saved.
func (s *PostgresStore) LoadPremiumCatalog(ctx context.Context) (PremiumCatalog, error) {
	var catalog PremiumCatalog
	var raw []byte
	var version int64
	err := s.pool.QueryRow(ctx,
		`SELECT version, entries, updated_by, updated_at FROM premium_shop WHERE singleton`).
		Scan(&version, &raw, &catalog.UpdatedBy, &catalog.UpdatedAt)
	if errors.Is(err, pgx.ErrNoRows) {
		return PremiumCatalog{}, nil
	}
	if err != nil {
		return PremiumCatalog{}, err
	}
	if version <= 0 || version > int64(^uint32(0)) {
		return PremiumCatalog{}, fmt.Errorf("store: premium catalog version %d out of range", version)
	}
	catalog.Version = uint32(version)
	if err := json.Unmarshal(raw, &catalog.Entries); err != nil {
		return PremiumCatalog{}, fmt.Errorf("store: corrupted premium catalog: %w", err)
	}
	if err := ValidatePremiumEntries(catalog.Entries); err != nil {
		return PremiumCatalog{}, fmt.Errorf("store: invalid premium catalog: %w", err)
	}
	return catalog, nil
}

// SavePremiumCatalog replaces the catalog when its current version equals
// expected (0 = none saved yet) and increments the version. A concurrent save
// returns ErrPremiumCatalogStale.
func (s *PostgresStore) SavePremiumCatalog(ctx context.Context, expected uint32, entries []PremiumCatalogEntry, actor string) (PremiumCatalog, error) {
	if s.readOnly {
		return PremiumCatalog{}, errors.New("store: PostgreSQL is read-only")
	}
	if entries == nil {
		entries = []PremiumCatalogEntry{}
	}
	if err := ValidatePremiumEntries(entries); err != nil {
		return PremiumCatalog{}, err
	}
	raw, err := json.Marshal(entries)
	if err != nil {
		return PremiumCatalog{}, err
	}
	catalog := PremiumCatalog{Entries: entries, UpdatedBy: actor}
	var version int64
	if expected == 0 {
		err = s.pool.QueryRow(ctx, `
			INSERT INTO premium_shop(singleton, version, entries, updated_by, updated_at)
			VALUES (true, 1, $1, $2, now())
			ON CONFLICT (singleton) DO NOTHING
			RETURNING version, updated_at`, raw, actor).Scan(&version, &catalog.UpdatedAt)
	} else {
		err = s.pool.QueryRow(ctx, `
			UPDATE premium_shop SET version=version+1, entries=$1, updated_by=$2, updated_at=now()
			WHERE singleton AND version=$3
			RETURNING version, updated_at`, raw, actor, int64(expected)).Scan(&version, &catalog.UpdatedAt)
	}
	if errors.Is(err, pgx.ErrNoRows) {
		return PremiumCatalog{}, ErrPremiumCatalogStale
	}
	if err != nil {
		return PremiumCatalog{}, err
	}
	if version > int64(^uint32(0)) {
		return PremiumCatalog{}, errors.New("store: premium catalog version exhausted")
	}
	catalog.Version = uint32(version)
	return catalog, nil
}

// AddDonateCredit inserts an inbox credit. A repeated request ID returns the
// existing row with duplicate=true and never credits twice.
func (s *PostgresStore) AddDonateCredit(ctx context.Context, credit DonateCredit) (DonateCredit, bool, error) {
	if s.readOnly {
		return DonateCredit{}, false, errors.New("store: PostgreSQL is read-only")
	}
	if err := ValidateDonateCredit(credit); err != nil {
		return DonateCredit{}, false, err
	}
	key := strings.ToLower(strings.TrimSpace(credit.Account))
	var requestID any
	if credit.RequestID != "" {
		requestID = credit.RequestID
	}
	out := credit
	out.Account = key
	err := s.pool.QueryRow(ctx, `
		INSERT INTO donate_credits(account_key, amount, reason, source, actor, request_id)
		SELECT $1, $2, $3, $4, $5, $6 WHERE EXISTS (SELECT 1 FROM accounts WHERE name_key=$1)
		ON CONFLICT (request_id) DO NOTHING
		RETURNING id, created_at`,
		key, int64(credit.Amount), strings.TrimSpace(credit.Reason), credit.Source, credit.Actor, requestID).
		Scan(&out.ID, &out.CreatedAt)
	if err == nil {
		return out, false, nil
	}
	if !errors.Is(err, pgx.ErrNoRows) {
		return DonateCredit{}, false, err
	}
	if credit.RequestID == "" {
		return DonateCredit{}, false, fmt.Errorf("%w: account %q", errAccountMissing, key)
	}
	existing, err := s.donateCreditByRequest(ctx, credit.RequestID)
	if err != nil {
		if errors.Is(err, pgx.ErrNoRows) {
			return DonateCredit{}, false, fmt.Errorf("%w: account %q", errAccountMissing, key)
		}
		return DonateCredit{}, false, err
	}
	if existing.Account != key || existing.Amount != credit.Amount {
		return DonateCredit{}, false, ErrDonateCreditConflict
	}
	return existing, true, nil
}

var errAccountMissing = errors.New("store: account does not exist")

func (s *PostgresStore) donateCreditByRequest(ctx context.Context, requestID string) (DonateCredit, error) {
	var c DonateCredit
	var amount int64
	var request *string
	err := s.pool.QueryRow(ctx, `
		SELECT id, account_key, amount, reason, source, actor, request_id, created_at, applied_at
		FROM donate_credits WHERE request_id=$1`, requestID).
		Scan(&c.ID, &c.Account, &amount, &c.Reason, &c.Source, &c.Actor, &request, &c.CreatedAt, &c.AppliedAt)
	if err != nil {
		return DonateCredit{}, err
	}
	c.Amount = uint32(amount)
	if request != nil {
		c.RequestID = *request
	}
	return c, nil
}

// PendingDonateCredits lists unapplied credits of the given accounts, oldest
// first.
func (s *PostgresStore) PendingDonateCredits(ctx context.Context, accounts []string, limit int) ([]DonateCredit, error) {
	if len(accounts) == 0 || limit < 1 {
		return nil, nil
	}
	keys := make([]string, 0, len(accounts))
	for _, account := range accounts {
		if key := strings.ToLower(strings.TrimSpace(account)); key != "" {
			keys = append(keys, key)
		}
	}
	rows, err := s.pool.Query(ctx, `
		SELECT id, account_key, amount, reason, source, actor, COALESCE(request_id, ''), created_at
		FROM donate_credits
		WHERE applied_at IS NULL AND account_key = ANY($1)
		ORDER BY id LIMIT $2`, keys, limit)
	if err != nil {
		return nil, err
	}
	defer rows.Close()
	var out []DonateCredit
	for rows.Next() {
		var c DonateCredit
		var amount int64
		if err := rows.Scan(&c.ID, &c.Account, &amount, &c.Reason, &c.Source, &c.Actor, &c.RequestID, &c.CreatedAt); err != nil {
			return nil, err
		}
		if amount <= 0 || amount > int64(model.MaxDonateBalance) {
			return nil, fmt.Errorf("store: donate credit %d has invalid amount %d", c.ID, amount)
		}
		c.Amount = uint32(amount)
		out = append(out, c)
	}
	return out, rows.Err()
}

// SaveDonateAccount commits the account, marks the applied credits and writes
// the ledger in one certified serializable transaction. A credit that is not
// pending for this account rolls everything back (ErrDonateCreditApplied).
func (s *PostgresStore) SaveDonateAccount(account *model.Account, credits []int64, ledger []DonateLedgerEntry) (err error) {
	writeStarted := false
	defer func() {
		if err != nil && !writeStarted {
			err = rejectWrite(err)
		}
	}()
	if s.readOnly {
		return errors.New("store: PostgreSQL is read-only")
	}
	if account == nil {
		return errors.New("store: donate write requires an account")
	}
	if err := ValidateDonateLedger(account, ledger); err != nil {
		return err
	}
	seq := s.writeSeq.Add(1)
	release, err := s.beginPostgresWrite()
	if err != nil {
		return err
	}
	defer release()
	unlock := s.accountFences.lock(accountModelKeys(account)...)
	defer unlock()
	snapshots, err := snapshotAccounts(account)
	if err != nil {
		return err
	}
	s.fenceAccountSnapshots(seq, snapshots)
	key := snapshots[0].key
	writeStarted = true
	return s.withCertifiedSerializableTx(func(ctx context.Context, tx pgx.Tx) error {
		if err := persistAccountSnapshots(ctx, tx, snapshots, false); err != nil {
			return err
		}
		if len(credits) > 0 {
			tag, err := tx.Exec(ctx, `
				UPDATE donate_credits SET applied_at=now()
				WHERE id = ANY($1) AND account_key=$2 AND applied_at IS NULL`, credits, key)
			if err != nil {
				return err
			}
			if tag.RowsAffected() != int64(len(credits)) {
				return ErrDonateCreditApplied
			}
		}
		for _, entry := range ledger {
			var creditID any
			if entry.CreditID != 0 {
				creditID = entry.CreditID
			}
			if _, err := tx.Exec(ctx, `
				INSERT INTO donate_ledger(account_key, delta, balance_after, kind, item_index, item_uid, credit_id, character)
				VALUES ($1,$2,$3,$4,$5,$6,$7,$8)`,
				key, entry.Delta, int64(entry.BalanceAfter), entry.Kind, int(entry.ItemIndex),
				entry.ItemUID, creditID, entry.Character); err != nil {
				return err
			}
		}
		return nil
	})
}

// DonateSummary reads the balance, pending credits and latest ledger rows.
func (s *PostgresStore) DonateSummary(ctx context.Context, account string, ledgerLimit int) (DonateSummary, error) {
	key := strings.ToLower(strings.TrimSpace(account))
	if key == "" || len(key) > 12 {
		return DonateSummary{}, errors.New("store: invalid account")
	}
	if ledgerLimit < 1 || ledgerLimit > 100 {
		ledgerLimit = 20
	}
	summary := DonateSummary{Account: key, Pending: []DonateCredit{}, Ledger: []DonateLedgerEntry{}}
	var balance int64
	err := s.pool.QueryRow(ctx,
		`SELECT COALESCE((payload->>'donate')::bigint, 0) FROM accounts WHERE name_key=$1`, key).Scan(&balance)
	if errors.Is(err, pgx.ErrNoRows) {
		return DonateSummary{}, fmt.Errorf("%w: account %q", errAccountMissing, key)
	}
	if err != nil {
		return DonateSummary{}, err
	}
	if balance < 0 || balance > int64(model.MaxDonateBalance) {
		return DonateSummary{}, fmt.Errorf("store: account %q has invalid donate balance", key)
	}
	summary.Balance = uint32(balance)
	pending, err := s.PendingDonateCredits(ctx, []string{key}, 50)
	if err != nil {
		return DonateSummary{}, err
	}
	if pending != nil {
		summary.Pending = pending
	}
	rows, err := s.pool.Query(ctx, `
		SELECT id, delta, balance_after, kind, item_index, item_uid, COALESCE(credit_id, 0), character, created_at
		FROM donate_ledger WHERE account_key=$1 ORDER BY id DESC LIMIT $2`, key, ledgerLimit)
	if err != nil {
		return DonateSummary{}, err
	}
	defer rows.Close()
	for rows.Next() {
		entry := DonateLedgerEntry{Account: key}
		var balanceAfter int64
		var itemIndex int32
		if err := rows.Scan(&entry.ID, &entry.Delta, &balanceAfter, &entry.Kind, &itemIndex,
			&entry.ItemUID, &entry.CreditID, &entry.Character, &entry.CreatedAt); err != nil {
			return DonateSummary{}, err
		}
		entry.BalanceAfter, entry.ItemIndex = uint32(balanceAfter), uint16(itemIndex)
		summary.Ledger = append(summary.Ledger, entry)
	}
	return summary, rows.Err()
}

// IsAccountMissing reports a donate operation on an unknown account.
func IsAccountMissing(err error) bool { return errors.Is(err, errAccountMissing) }
