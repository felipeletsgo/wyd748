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
	// LoadAccount is already deadline-bounded by the store. Its full payload
	// stays inside this adapter; only this explicit display projection escapes.
	if err := ctx.Err(); err != nil {
		return Profile{}, err
	}
	a, err := d.reader.LoadAccount(name)
	if err != nil {
		return Profile{}, err
	}
	p := Profile{Username: a.Name, Donate: a.Donate, Characters: make([]Character, 0, 4)}
	for slot, c := range a.Chars {
		if c.Name == "" || c.Score == nil {
			continue
		}
		evolution := c.Evolution
		if evolution == "" {
			evolution = "mortal"
		}
		p.Characters = append(p.Characters, Character{Slot: slot, Name: c.Name, Class: c.Class, Level: c.Score.Level, Evolution: evolution})
	}
	return p, nil
}

const rankingSQL = `SELECT c->>'name', COALESCE((c->>'class')::integer,0), COALESCE((c->'score'->>'level')::bigint,0), COALESCE(NULLIF(c->>'evolution',''),'mortal')
FROM accounts a CROSS JOIN LATERAL jsonb_array_elements(a.payload->'chars') c
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
