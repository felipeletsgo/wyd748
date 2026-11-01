package store

import (
	"context"
	"encoding/json"
	"fmt"
	"log"
	"strconv"
	"strings"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
)

// Guild row synchronization (DOCS/server/server-correction-plan.md, W9 phase 3).
// Each committed guild write of a registered game process sends a
// notification with the process name and the written guild IDs. Other
// processes listen and reload those rows; a notification is only a wake-up
// hint, and a versioned write rejects a stale copy.

const guildRowsChannel = "wyd_guild_rows"

// postgresGuildNamesSchema makes guild names unique in the database, with the
// same rule as model.NormalizeGuildName (names are trimmed ASCII). Two
// processes that create one name at once then cannot both commit.
const postgresGuildNamesSchema = `
ALTER TABLE guild_rows ADD COLUMN IF NOT EXISTS name_key text
    GENERATED ALWAYS AS (lower(btrim(payload->>'name'))) STORED;
CREATE UNIQUE INDEX IF NOT EXISTS guild_rows_name_key_idx ON guild_rows(name_key);`

// GuildRowState is one guild row as stored. GuildID 0 is the registry
// header; Present is false for a removed guild.
type GuildRowState struct {
	GuildID uint16
	Present bool
	Guild   model.Guild
	Header  *model.GuildRegistry
	Version int64
}

// EnableGuildRowSync makes this process announce its guild writes. Enable it
// only when other game processes share the database.
func (s *PostgresStore) EnableGuildRowSync() { s.guildSync.Store(true) }

// persistGuildRows writes the guild rows and, in a registered process with
// sync enabled, notifies the other processes when the transaction commits.
func (s *PostgresStore) persistGuildRows(ctx context.Context, tx pgx.Tx, registry *model.GuildRegistry, keys []GuildParticipant) error {
	if err := writeGuildRows(ctx, tx, registry, keys); err != nil {
		return err
	}
	process := s.leaseProcess.Load()
	if process == nil || !s.guildSync.Load() {
		return nil
	}
	ids := "*"
	if keys != nil {
		parts := make([]string, 0, len(keys))
		for _, key := range keys {
			parts = append(parts, strconv.Itoa(int(key.GuildID)))
		}
		ids = strings.Join(parts, ",")
	}
	_, err := tx.Exec(ctx, `SELECT pg_notify($1, $2)`, guildRowsChannel, *process+"|"+ids)
	return err
}

// LoadGuildRowStates reads the named rows, or every row and the header when
// ids is nil. A named guild without a row is returned as not present.
func (s *PostgresStore) LoadGuildRowStates(ids []uint16) ([]GuildRowState, error) {
	ctx, cancel := s.postgresReadContext()
	defer cancel()
	var states []GuildRowState
	header := ids == nil
	wanted := make(map[uint16]bool, len(ids))
	for _, id := range ids {
		if id == 0 {
			header = true
		} else {
			wanted[id] = true
		}
	}
	if header {
		var payload []byte
		var version int64
		err := s.pool.QueryRow(ctx, `SELECT payload, version FROM guild_registry_state WHERE singleton`).Scan(&payload, &version)
		if err != nil && err != pgx.ErrNoRows {
			return nil, err
		}
		if err == nil {
			var registry model.GuildRegistry
			if err := json.Unmarshal(payload, &registry); err != nil {
				return nil, fmt.Errorf("store: corrupt guild registry: %w", err)
			}
			states = append(states, GuildRowState{Present: true, Header: &registry, Version: version})
		}
	}
	if ids != nil && len(wanted) == 0 {
		return states, nil
	}
	list := make([]int32, 0, len(wanted))
	for id := range wanted {
		list = append(list, int32(id))
	}
	rows, err := s.pool.Query(ctx, `SELECT guild_id, payload, version FROM guild_rows
		WHERE $1::integer[] IS NULL OR guild_id = ANY($1::integer[]) ORDER BY guild_id`, nilIfAll(ids == nil, list))
	if err != nil {
		return nil, err
	}
	defer rows.Close()
	for rows.Next() {
		var id int32
		var payload []byte
		var version int64
		if err := rows.Scan(&id, &payload, &version); err != nil {
			return nil, err
		}
		var guild model.Guild
		if err := json.Unmarshal(payload, &guild); err != nil {
			return nil, fmt.Errorf("store: corrupt guild %d: %w", id, err)
		}
		delete(wanted, uint16(id))
		states = append(states, GuildRowState{GuildID: uint16(id), Present: true, Guild: guild, Version: version})
	}
	if err := rows.Err(); err != nil {
		return nil, err
	}
	for id := range wanted {
		states = append(states, GuildRowState{GuildID: id})
	}
	return states, nil
}

func nilIfAll(all bool, list []int32) []int32 {
	if all {
		return nil
	}
	return list
}

// ListenGuildRows calls handle for each guild row notification of another
// process until ctx ends. ids is nil for a whole-registry write. After a lost
// connection it reconnects and reports a whole-registry change, because
// notifications sent meanwhile are lost.
func (s *PostgresStore) ListenGuildRows(ctx context.Context, self string, handle func(ids []uint16)) {
	first := true
	for ctx.Err() == nil {
		if !first {
			handle(nil)
		}
		first = false
		if err := s.listenGuildRowsOnce(ctx, self, handle); err != nil && ctx.Err() == nil {
			log.Printf("guild row notifications: %v; reconnecting", err)
			select {
			case <-ctx.Done():
			case <-time.After(2 * time.Second):
			}
		}
	}
}

func (s *PostgresStore) listenGuildRowsOnce(ctx context.Context, self string, handle func(ids []uint16)) error {
	return s.listen(ctx, guildRowsChannel, func(payload string) {
		process, list, ok := strings.Cut(payload, "|")
		if !ok || process == self {
			return
		}
		if list == "*" {
			handle(nil)
			return
		}
		var ids []uint16
		for _, part := range strings.Split(list, ",") {
			if id, err := strconv.ParseUint(part, 10, 16); err == nil {
				ids = append(ids, uint16(id))
			}
		}
		handle(ids)
	})
}
