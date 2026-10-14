package playerportal

import (
	"context"
	"encoding/json"
	"errors"
	"strings"
	"time"

	"github.com/jackc/pgx/v5/pgconn"
	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

// Database has no schema or gameplay-write privileges. Registration is the
// only INSERT; existing accounts can never be updated by this service.
type Database struct {
	pool   *pgxpool.Pool
	reader *store.PostgresStore
}

func OpenDatabase(ctx context.Context, databaseURL string) (*Database, error) {
	cfg, err := pgxpool.ParseConfig(databaseURL)
	if err != nil {
		return nil, errors.New("portal: invalid database configuration")
	}
	cfg.MaxConns = 2
	cfg.ConnConfig.ConnectTimeout = 3 * time.Second
	cfg.ConnConfig.RuntimeParams["statement_timeout"] = "3000"
	pool, err := pgxpool.NewWithConfig(ctx, cfg)
	if err != nil {
		return nil, errors.New("portal: database unavailable")
	}
	reader, err := store.NewPostgresReadOnlyStore(ctx, store.PostgresConfig{URL: databaseURL, MaxConns: 2, OperationTimeout: 3 * time.Second})
	if err != nil {
		pool.Close()
		return nil, errors.New("portal: database unavailable")
	}
	return &Database{pool: pool, reader: reader}, nil
}
func (d *Database) Close()                         { d.reader.Close(); d.pool.Close() }
func (d *Database) Ping(ctx context.Context) error { return d.pool.Ping(ctx) }
func (d *Database) LoadAccount(name string) (*model.Account, error) {
	return d.reader.LoadAccount(name)
}
func (d *Database) AccountNameExists(name string) (bool, error) {
	return d.reader.AccountNameExists(name)
}
func registrationPayload(a *model.Account) ([]byte, error) {
	if a == nil || len(a.Chars) != 0 || a.Donate != 0 || a.CargoGold != 0 || len(a.CelestialCapsules) != 0 {
		return nil, errors.New("portal: registration requires an empty account")
	}
	for _, item := range a.Cargo {
		if item != (model.Item{}) {
			return nil, errors.New("portal: registration requires empty cargo")
		}
	}
	if err := a.Validate(); err != nil {
		return nil, err
	}
	return json.Marshal(a)
}
func (d *Database) CreateAccount(a *model.Account) error {
	payload, err := registrationPayload(a)
	if err != nil {
		return err
	}
	ctx, cancel := context.WithTimeout(context.Background(), 3*time.Second)
	defer cancel()
	_, err = d.pool.Exec(ctx, `INSERT INTO accounts(name_key,payload) VALUES($1,$2::jsonb)`, strings.ToLower(a.Name), string(payload))
	var pgerr *pgconn.PgError
	if errors.As(err, &pgerr) && pgerr.Code == "23505" {
		return store.ErrAccountExists
	}
	return err
}
func (d *Database) Profile(ctx context.Context, name string) (Profile, error) {
	ctx, cancel := context.WithTimeout(ctx, 3*time.Second)
	defer cancel()
	// One statement keeps timestamps and fields from the same persisted snapshot.
	// Credentials, cargo, inventory and internal character/item IDs never leave SQL.
	var p Profile
	var characters []byte
	err := d.pool.QueryRow(ctx, profileSQL, strings.ToLower(name)).Scan(
		&p.Username, &p.Donate, &p.CreatedAt, &p.AsOf, &characters)
	if err != nil {
		return Profile{}, err
	}
	if err := json.Unmarshal(characters, &p.Characters); err != nil {
		return Profile{}, err
	}
	return p, nil
}

const profileSQL = `SELECT payload->>'name', COALESCE((payload->>'donate')::bigint,0), created_at, updated_at,
COALESCE((SELECT jsonb_agg(jsonb_build_object(
 'slot', ordinal-1, 'name', c->>'name', 'class', COALESCE((c->>'class')::integer,0),
 'level', COALESCE((c->'score'->>'level')::bigint,0),
 'evolution', COALESCE(NULLIF(c->>'evolution',''),'mortal')) ORDER BY ordinal)
 FROM jsonb_array_elements(COALESCE(NULLIF(payload->'chars','null'::jsonb),'[]'::jsonb)) WITH ORDINALITY AS chars(c, ordinal)
 WHERE ordinal<=4 AND COALESCE(c->>'name','')<>'' AND jsonb_typeof(c->'score')='object'), '[]'::jsonb)
FROM accounts WHERE name_key=$1`

const rankingSQL = `SELECT c->>'name', COALESCE((c->>'class')::integer,0), COALESCE((c->'score'->>'level')::bigint,0), COALESCE(NULLIF(c->>'evolution',''),'mortal')
FROM accounts a CROSS JOIN LATERAL jsonb_array_elements(COALESCE(NULLIF(a.payload->'chars','null'::jsonb),'[]'::jsonb)) c
WHERE COALESCE(c->>'name','')<>'' AND jsonb_typeof(c->'score')='object'
ORDER BY CASE c->>'evolution' WHEN 'subcelestial' THEN 3 WHEN 'celestial' THEN 2 WHEN 'arch' THEN 1 ELSE 0 END DESC,
COALESCE((c->'score'->>'level')::bigint,0) DESC, COALESCE((c->>'exp')::bigint,0) DESC, lower(c->>'name') ASC, a.name_key ASC
LIMIT 100`

func (d *Database) Ranking(ctx context.Context) ([]Character, error) {
	ctx, cancel := context.WithTimeout(ctx, 3*time.Second)
	defer cancel()
	rows, err := d.pool.Query(ctx, rankingSQL)
	if err != nil {
		return nil, err
	}
	defer rows.Close()
	result := make([]Character, 0, 100)
	for rows.Next() {
		var c Character
		var class int
		var level int64
		if err := rows.Scan(&c.Name, &class, &level, &c.Evolution); err != nil {
			return nil, err
		}
		if class < 0 || class > 255 || level < 0 || level > int64(^uint32(0)) {
			return nil, errors.New("portal: invalid ranking data")
		}
		c.Class = byte(class)
		c.Level = uint32(level)
		result = append(result, c)
	}
	return result, rows.Err()
}
