package store

import (
	"context"
	"crypto/rand"
	"encoding/hex"
	"fmt"
	"log"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
)

// Instance fencing (DOCS/server/server-correction-plan.md, W4). A crashed or
// hung game process can leave backends that still hold an open transaction or
// are finishing a COMMIT. Before the new process reads any account, it ends
// those backends and waits until PostgreSQL has removed them. A transaction
// that never committed is then aborted; one that committed is visible. Only
// the authoritative game server enables this; tools and panels never do.

const (
	gameInstanceApplicationPrefix = "wydgo-game/"
	instanceFenceTimeout          = 10 * time.Second
)

// newGameInstanceName names a game process. A process of one channel (W9
// phase 5) carries the channel, so its fence ends only that channel's
// previous process.
func newGameInstanceName(channel byte) (string, error) {
	var id [8]byte
	if _, err := rand.Read(id[:]); err != nil {
		return "", err
	}
	return gameInstancePrefix(channel) + hex.EncodeToString(id[:]), nil
}

func gameInstancePrefix(channel byte) string {
	if channel == 0 {
		return gameInstanceApplicationPrefix
	}
	return fmt.Sprintf("%sch%d/", gameInstanceApplicationPrefix, channel)
}

// gameInstanceScope is the SQL condition on application_name ($1 prefix
// pattern, $3 channel process) that one process fences: every game process
// for the whole server, or for a channel process its channel's processes and
// a previous whole-server process.
const gameInstanceScope = `(application_name LIKE $1 OR ($3 AND application_name ~ '^wydgo-game/[0-9a-f]+$'))`

// fencePreviousGameInstances terminates other game instance backends of this
// database and waits for them to end. It returns the number terminated.
func fencePreviousGameInstances(ctx context.Context, pool *pgxpool.Pool, self string, channel byte) (int, error) {
	ctx, cancel := context.WithTimeout(ctx, instanceFenceTimeout)
	defer cancel()
	var terminated int
	if err := pool.QueryRow(ctx, `
		SELECT count(*) FILTER (WHERE pg_terminate_backend(pid))
		FROM pg_stat_activity
		WHERE datname = current_database() AND application_name <> $2 AND `+gameInstanceScope,
		gameInstancePrefix(channel)+"%", self, channel != 0).Scan(&terminated); err != nil {
		return 0, fmt.Errorf("store: terminate previous game instance backends: %w", err)
	}
	for {
		var remaining int
		if err := pool.QueryRow(ctx, `
			SELECT count(*) FROM pg_stat_activity
			WHERE datname = current_database() AND application_name <> $2 AND `+gameInstanceScope,
			gameInstancePrefix(channel)+"%", self, channel != 0).Scan(&remaining); err != nil {
			return terminated, fmt.Errorf("store: wait for previous game instance backends: %w", err)
		}
		if remaining == 0 {
			if terminated > 0 {
				log.Printf("store: ended %d backend(s) of a previous game instance before admission", terminated)
			}
			return terminated, nil
		}
		select {
		case <-ctx.Done():
			return terminated, fmt.Errorf("store: %d previous game instance backend(s) still running: %w", remaining, ctx.Err())
		case <-time.After(100 * time.Millisecond):
		}
	}
}
