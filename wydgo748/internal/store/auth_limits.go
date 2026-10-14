package store

import "wydgo/internal/authlimit"

// AuthenticationLimiter writes security counters, never account payloads.
func (s *PostgresStore) AuthenticationLimiter() authlimit.Limiter {
	return authlimit.Postgres{Pool: s.pool}
}
