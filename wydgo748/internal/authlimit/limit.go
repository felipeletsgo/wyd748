// Package authlimit bounds authentication work across processes and restarts.
package authlimit

import (
	"context"
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"net/netip"
	"sort"
	"strings"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

type Limit struct {
	Key     string
	Window  time.Duration
	Maximum int
}
type Limiter interface {
	Admit(context.Context, []Limit) (time.Duration, error)
}

// Policy hashes identifiers at rest and groups IPv6 addresses by /64.
// Short in-memory burst limits remain a separate first-line CPU protection.
func Policy(scope, peer, account string) []Limit {
	ip, err := netip.ParseAddr(peer)
	network := "unknown"
	if err == nil {
		ip = ip.Unmap()
		network = ip.String()
		if ip.Is6() {
			network = netip.PrefixFrom(ip, 64).Masked().String()
		}
	}
	makeLimit := func(kind, value string, window time.Duration, maximum int) Limit {
		hash := sha256.Sum256([]byte(scope + "\x00" + kind + "\x00" + value))
		return Limit{Key: scope + ":" + kind + ":" + hex.EncodeToString(hash[:]) + ":" + window.String(), Window: window, Maximum: maximum}
	}
	if scope == "registration" {
		return []Limit{makeLimit("0global", "all", time.Minute, 30), makeLimit("0global", "all", time.Hour, 300), makeLimit("0global", "all", 24*time.Hour, 1000), makeLimit("1network", network, time.Hour, 5), makeLimit("1network", network, 24*time.Hour, 10)}
	}
	return []Limit{makeLimit("0global", "all", time.Hour, 600), makeLimit("1network", network, time.Hour, 100), makeLimit("2account", strings.ToLower(account), time.Hour, 40)}
}

type Postgres struct{ Pool *pgxpool.Pool }

// Admit reserves all quotas atomically using database time. Rejection rolls
// every reservation back, so exhausted accounts cannot burn the shared budget.
func (p Postgres) Admit(ctx context.Context, limits []Limit) (time.Duration, error) {
	if ctx == nil || p.Pool == nil || len(limits) == 0 || len(limits) > 8 {
		return 0, errors.New("authentication limiter unavailable")
	}
	ordered := append([]Limit(nil), limits...)
	sort.Slice(ordered, func(i, j int) bool { return ordered[i].Key < ordered[j].Key })
	for i, limit := range ordered {
		if limit.Key == "" || len(limit.Key) > 160 || limit.Window < time.Second || limit.Window > 24*time.Hour || limit.Window%time.Second != 0 || limit.Maximum < 1 || (i > 0 && ordered[i-1].Key == limit.Key) {
			return 0, errors.New("invalid authentication quota")
		}
	}
	ctx, cancel := context.WithTimeout(ctx, time.Second)
	defer cancel()
	tx, err := p.Pool.Begin(ctx)
	if err != nil {
		return 0, errors.New("authentication limiter unavailable")
	}
	defer tx.Rollback(ctx)
	for _, limit := range ordered {
		var expires time.Time
		err = tx.QueryRow(ctx, `INSERT INTO web_auth_limits(key, attempts, expires_at)
   VALUES ($1, 1, to_timestamp((floor(extract(epoch FROM transaction_timestamp()) / $2::bigint) + 1) * $2::bigint))
   ON CONFLICT (key) DO UPDATE SET
   attempts = CASE WHEN web_auth_limits.expires_at <= transaction_timestamp() THEN 1 ELSE web_auth_limits.attempts + 1 END,
   expires_at = CASE WHEN web_auth_limits.expires_at <= transaction_timestamp() THEN EXCLUDED.expires_at ELSE web_auth_limits.expires_at END
   WHERE web_auth_limits.expires_at <= transaction_timestamp() OR web_auth_limits.attempts < $3
   RETURNING expires_at`, limit.Key, int64(limit.Window/time.Second), limit.Maximum).Scan(&expires)
		if errors.Is(err, pgx.ErrNoRows) {
			var wait float64
			if err = tx.QueryRow(ctx, `SELECT greatest(1, extract(epoch FROM expires_at - transaction_timestamp())) FROM web_auth_limits WHERE key = $1`, limit.Key).Scan(&wait); err != nil {
				return 0, errors.New("authentication limiter unavailable")
			}
			return time.Duration(wait * float64(time.Second)), nil
		}
		if err != nil {
			return 0, errors.New("authentication limiter unavailable")
		}
	}
	// Cleanup is bounded and skips rows locked by concurrent requests.
	if _, err = tx.Exec(ctx, `DELETE FROM web_auth_limits WHERE key IN (SELECT key FROM web_auth_limits WHERE expires_at <= transaction_timestamp() ORDER BY expires_at LIMIT 64 FOR UPDATE SKIP LOCKED)`); err != nil {
		return 0, errors.New("authentication limiter unavailable")
	}
	if err = tx.Commit(ctx); err != nil {
		return 0, errors.New("authentication limiter unavailable")
	}
	return 0, nil
}
