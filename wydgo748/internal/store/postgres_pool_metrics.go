package store

import (
	"expvar"
	"sync/atomic"

	"github.com/jackc/pgx/v5/pgxpool"
)

// Pool diagnostics (DOCS/server/server-correction-plan.md, W0). Only the
// game store pool is published; staff and template read pools stay separate.
// Counters are cumulative since that pool opened; gauges are current values.
var primaryPool atomic.Pointer[pgxpool.Pool]

var poolStatReaders = map[string]func(*pgxpool.Stat) int64{
	"postgres_pool_acquired_conns":           func(s *pgxpool.Stat) int64 { return int64(s.AcquiredConns()) },
	"postgres_pool_idle_conns":               func(s *pgxpool.Stat) int64 { return int64(s.IdleConns()) },
	"postgres_pool_constructing_conns":       func(s *pgxpool.Stat) int64 { return int64(s.ConstructingConns()) },
	"postgres_pool_total_conns":              func(s *pgxpool.Stat) int64 { return int64(s.TotalConns()) },
	"postgres_pool_max_conns":                func(s *pgxpool.Stat) int64 { return int64(s.MaxConns()) },
	"postgres_pool_acquire_total":            func(s *pgxpool.Stat) int64 { return s.AcquireCount() },
	"postgres_pool_acquire_micros_total":     func(s *pgxpool.Stat) int64 { return s.AcquireDuration().Microseconds() },
	"postgres_pool_empty_acquire_total":      func(s *pgxpool.Stat) int64 { return s.EmptyAcquireCount() },
	"postgres_pool_empty_wait_micros_total":  func(s *pgxpool.Stat) int64 { return s.EmptyAcquireWaitTime().Microseconds() },
	"postgres_pool_canceled_acquire_total":   func(s *pgxpool.Stat) int64 { return s.CanceledAcquireCount() },
	"postgres_pool_new_conns_total":          func(s *pgxpool.Stat) int64 { return s.NewConnsCount() },
	"postgres_pool_lifetime_destroyed_total": func(s *pgxpool.Stat) int64 { return s.MaxLifetimeDestroyCount() },
}

func init() {
	for name, read := range poolStatReaders {
		expvar.Publish(name, expvar.Func(func() any {
			pool := primaryPool.Load()
			if pool == nil {
				return int64(0)
			}
			return read(pool.Stat())
		}))
	}
}

func publishPrimaryPool(pool *pgxpool.Pool) { primaryPool.Store(pool) }

func unpublishPrimaryPool(pool *pgxpool.Pool) { primaryPool.CompareAndSwap(pool, nil) }
