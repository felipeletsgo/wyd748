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

func newGameInstanceName() (string, error) {
	var id [8]byte
	if _, err := rand.Read(id[:]); err != nil {
		return "", err
	}
	return gameInstanceApplicationPrefix + hex.EncodeToString(id[:]), nil
}

// fencePreviousGameInstances terminates other game instance backends of this
// database and waits for them to end. It returns the number terminated.
func fencePreviousGameInstances(ctx context.Context, pool *pgxpool.Pool, self string) (int, error) {
	ctx, cancel := context.WithTimeout(ctx, instanceFenceTimeout)
	defer cancel()
	var terminated int
	if err := pool.QueryRow(ctx, `
		SELECT count(*) FILTER (WHERE pg_terminate_backend(pid))
		FROM pg_stat_activity
		WHERE datname = current_database() AND application_name LIKE $1 AND application_name <> $2`,
		gameInstanceApplicationPrefix+"%", self).Scan(&terminated); err != nil {
		return 0, fmt.Errorf("store: terminate previous game instance backends: %w", err)
	}
	for {
		var remaining int
		if err := pool.QueryRow(ctx, `
			SELECT count(*) FROM pg_stat_activity
			WHERE datname = current_database() AND application_name LIKE $1 AND application_name <> $2`,
			gameInstanceApplicationPrefix+"%", self).Scan(&remaining); err != nil {
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
