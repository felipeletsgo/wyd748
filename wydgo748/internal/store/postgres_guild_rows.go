package store

import (
	"context"
	"encoding/json"
	"errors"
	"fmt"
	"reflect"
	"slices"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgconn"

	"wydgo/internal/model"
)

// Guild participant rows (DOCS/server/server-correction-plan.md, W8). Each
// guild is one row; the registry row holds the war state of every channel,
// the Fame reset day and the format versions. A write commits only the rows
// it declares. The former singleton row stays current through an export and
// feeds the public guild view and an older binary.

const postgresGuildRowsSchema = `
CREATE TABLE IF NOT EXISTS guild_rows (
    guild_id    integer PRIMARY KEY CHECK (guild_id BETWEEN 1 AND 65535),
    payload     jsonb NOT NULL,
    version     bigint NOT NULL DEFAULT 1,
    updated_at  timestamptz NOT NULL DEFAULT now()
);
CREATE TABLE IF NOT EXISTS guild_registry_state (
    singleton   boolean PRIMARY KEY DEFAULT true CHECK (singleton),
    payload     jsonb NOT NULL,
    version     bigint NOT NULL DEFAULT 1,
    updated_at  timestamptz NOT NULL DEFAULT now()
);`

// GuildParticipant names one durable guild row; GuildID 0 is the registry.
// Version is the row version that the writer last read (W9 phase 3): 0
// skips the check, -1 expects no row, and a positive value must match.
type GuildParticipant struct {
	GuildID uint16
	Version int64
}

// ErrGuildRowConflict rejects a versioned guild write whose row another
// process changed after this process read it.
var ErrGuildRowConflict = errors.New("store: guild row changed in another process")

func guildRegistryPayload(registry *model.GuildRegistry) ([]byte, error) {
	header := *registry
	// Keep the original null-or-empty form of the list; rows hold its members.
	if header.Guilds != nil {
		header.Guilds = []model.Guild{}
	}
	return json.Marshal(header)
}

// writeGuildRows commits the declared participants of registry. Nil keys
// write the registry and every guild and delete guilds absent from it.
func writeGuildRows(ctx context.Context, tx pgx.Tx, registry *model.GuildRegistry, keys []GuildParticipant) error {
	if registry == nil {
		return errors.New("store: guild rows need a registry")
	}
	if keys == nil {
		keys = append(keys, GuildParticipant{})
		present := make([]int32, 0, len(registry.Guilds))
		for _, guild := range registry.Guilds {
			keys = append(keys, GuildParticipant{GuildID: guild.ID})
			present = append(present, int32(guild.ID))
		}
		if _, err := tx.Exec(ctx, `DELETE FROM guild_rows WHERE NOT (guild_id = ANY($1::integer[]))`, present); err != nil {
			return err
		}
	}
	keys = slices.Clone(keys)
	slices.SortFunc(keys, func(a, b GuildParticipant) int { return int(a.GuildID) - int(b.GuildID) })
	keys = slices.CompactFunc(keys, func(a, b GuildParticipant) bool { return a.GuildID == b.GuildID })
	// changed fails the write when a versioned statement found another
	// process's row version.
	changed := func(tag pgconn.CommandTag, err error, key GuildParticipant) error {
		if err != nil {
			return err
		}
		if key.Version != 0 && tag.RowsAffected() == 0 {
			return fmt.Errorf("%w: guild %d", ErrGuildRowConflict, key.GuildID)
		}
		return nil
	}
	for _, key := range keys {
		if key.GuildID == 0 {
			payload, err := guildRegistryPayload(registry)
			if err != nil {
				return err
			}
			if key.Version > 0 {
				tag, err := tx.Exec(ctx, `UPDATE guild_registry_state SET payload=$1,version=version+1,updated_at=now()
					WHERE singleton AND version=$2`, payload, key.Version)
				if err := changed(tag, err, key); err != nil {
					return err
				}
				continue
			}
			if _, err := tx.Exec(ctx, `INSERT INTO guild_registry_state(singleton,payload) VALUES(true,$1)
				ON CONFLICT(singleton) DO UPDATE SET payload=EXCLUDED.payload,
					version=guild_registry_state.version+1,updated_at=now()`, payload); err != nil {
				return err
			}
			continue
		}
		index := slices.IndexFunc(registry.Guilds, func(g model.Guild) bool { return g.ID == key.GuildID })
		if index < 0 {
			switch {
			case key.Version > 0:
				tag, err := tx.Exec(ctx, `DELETE FROM guild_rows WHERE guild_id=$1 AND version=$2`, int32(key.GuildID), key.Version)
				if err := changed(tag, err, key); err != nil {
					return err
				}
			case key.Version < 0:
				// The guild is absent on both sides.
				var exists bool
				if err := tx.QueryRow(ctx, `SELECT EXISTS(SELECT 1 FROM guild_rows WHERE guild_id=$1)`,
					int32(key.GuildID)).Scan(&exists); err != nil {
					return err
				}
				if exists {
					return fmt.Errorf("%w: guild %d", ErrGuildRowConflict, key.GuildID)
				}
			default:
				if _, err := tx.Exec(ctx, `DELETE FROM guild_rows WHERE guild_id=$1`, int32(key.GuildID)); err != nil {
					return err
				}
			}
			continue
		}
		payload, err := json.Marshal(registry.Guilds[index])
		if err != nil {
			return err
		}
		switch {
		case key.Version > 0:
			tag, err := tx.Exec(ctx, `UPDATE guild_rows SET payload=$2,version=version+1,updated_at=now()
				WHERE guild_id=$1 AND version=$3`, int32(key.GuildID), payload, key.Version)
			if err := changed(tag, err, key); err != nil {
				return err
			}
		case key.Version < 0:
			tag, err := tx.Exec(ctx, `INSERT INTO guild_rows(guild_id,payload) VALUES($1,$2)
				ON CONFLICT(guild_id) DO NOTHING`, int32(key.GuildID), payload)
			if err := changed(tag, err, key); err != nil {
				return err
			}
		default:
			if _, err := tx.Exec(ctx, `INSERT INTO guild_rows(guild_id,payload) VALUES($1,$2)
				ON CONFLICT(guild_id) DO UPDATE SET payload=EXCLUDED.payload,
					version=guild_rows.version+1,updated_at=now()`, int32(key.GuildID), payload); err != nil {
				return err
			}
		}
	}
	return nil
}

// loadGuildRows rebuilds the registry. ok is false when no registry row
// exists, so callers can fall back to an empty registry.
func loadGuildRows(ctx context.Context, q instanceRowQuerier) (*model.GuildRegistry, bool, error) {
	rows, err := q.Query(ctx, `SELECT payload FROM guild_registry_state WHERE singleton=true`)
	if err != nil {
		return nil, false, err
	}
	var header []byte
	for rows.Next() {
		if err := rows.Scan(&header); err != nil {
			rows.Close()
			return nil, false, err
		}
	}
	rows.Close()
	if err := rows.Err(); err != nil {
		return nil, false, err
	}
	if header == nil {
		return nil, false, nil
	}
	var registry model.GuildRegistry
	if err := json.Unmarshal(header, &registry); err != nil {
		return nil, false, fmt.Errorf("store: corrupt guild registry: %w", err)
	}
	rows, err = q.Query(ctx, `SELECT guild_id,payload FROM guild_rows ORDER BY guild_id`)
	if err != nil {
		return nil, false, err
	}
	defer rows.Close()
	for rows.Next() {
		var id int32
		var payload []byte
		if err := rows.Scan(&id, &payload); err != nil {
			return nil, false, err
		}
		var guild model.Guild
		if err := json.Unmarshal(payload, &guild); err != nil {
			return nil, false, fmt.Errorf("store: corrupt guild %d: %w", id, err)
		}
		if int32(guild.ID) != id {
			return nil, false, fmt.Errorf("store: guild row %d holds guild %d", id, guild.ID)
		}
		registry.Guilds = append(registry.Guilds, guild)
	}
	if err := rows.Err(); err != nil {
		return nil, false, err
	}
	return &registry, true, nil
}

// equalGuildRegistries compares persisted forms with guilds in ID order.
func equalGuildRegistries(a, b *model.GuildRegistry) bool {
	normal := func(r *model.GuildRegistry) any {
		copy := *r
		copy.Guilds = slices.Clone(r.Guilds)
		slices.SortFunc(copy.Guilds, func(x, y model.Guild) int { return int(x.ID) - int(y.ID) })
		if copy.Guilds == nil {
			copy.Guilds = []model.Guild{}
		}
		payload, err := json.Marshal(copy)
		if err != nil {
			return err
		}
		var out any
		if err := json.Unmarshal(payload, &out); err != nil {
			return err
		}
		return out
	}
	return reflect.DeepEqual(normal(a), normal(b))
}

// migrateGuildRows splits the singleton registry into participant rows and
// requires the rebuilt registry to equal the original.
func migrateGuildRows(ctx context.Context, tx pgx.Tx) error {
	if _, err := tx.Exec(ctx, postgresGuildRowsSchema); err != nil {
		return err
	}
	var existing int
	if err := tx.QueryRow(ctx, `SELECT (SELECT count(*) FROM guild_rows) +
		(SELECT count(*) FROM guild_registry_state)`).Scan(&existing); err != nil {
		return err
	}
	var payload []byte
	err := tx.QueryRow(ctx, `SELECT payload FROM guild_state WHERE singleton=true`).Scan(&payload)
	if errors.Is(err, pgx.ErrNoRows) {
		return nil
	}
	if err != nil {
		return err
	}
	if existing > 0 {
		return errors.New("store: guild rows exist before the v15 migration; refusing to merge")
	}
	var original model.GuildRegistry
	if err := json.Unmarshal(payload, &original); err != nil {
		return fmt.Errorf("store: corrupt guild state before migration: %w", err)
	}
	if err := writeGuildRows(ctx, tx, &original, nil); err != nil {
		return err
	}
	rebuilt, _, err := loadGuildRows(ctx, tx)
	if err != nil {
		return err
	}
	if rebuilt == nil || !equalGuildRegistries(&original, rebuilt) {
		return errors.New("store: guild rows do not rebuild the singleton registry; migration refused")
	}
	return nil
}

// scheduleGuildStateExport keeps the singleton guild_state row current for
// the public view and for rollback. The newest request wins.
func (s *PostgresStore) scheduleGuildStateExport() {
	generation := s.guildStateExportGen.Add(1)
	_ = s.enqueue(postgresWriteJob{
		label: "export guild rollback state",
		run: func() error {
			if s.guildStateExportGen.Load() != generation {
				return nil
			}
			return s.exportGuildSingleton()
		},
	})
}

func (s *PostgresStore) exportGuildSingleton() error {
	ctx, cancel := s.postgresContext()
	defer cancel()
	tx, err := s.pool.BeginTx(ctx, pgx.TxOptions{IsoLevel: pgx.RepeatableRead})
	if err != nil {
		return err
	}
	defer func() { _ = tx.Rollback(context.Background()) }()
	registry, ok, err := loadGuildRows(ctx, tx)
	if err != nil || !ok {
		return err
	}
	payload, err := json.Marshal(registry)
	if err != nil {
		return err
	}
	if _, err := tx.Exec(ctx, `INSERT INTO guild_state(singleton,payload) VALUES(true,$1)
		ON CONFLICT(singleton) DO UPDATE SET payload=EXCLUDED.payload,
			version=guild_state.version+1,updated_at=now()`, payload); err != nil {
		return err
	}
	return tx.Commit(ctx)
}
