package playerportal

import "wydgo/internal/authlimit"

func (d *Database) AuthenticationLimiter() authlimit.Limiter {
	return authlimit.Postgres{Pool: d.pool}
}
