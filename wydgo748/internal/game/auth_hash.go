package game

import (
	"expvar"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/net"
)

// Password hash admission (DOCS/server/server-correction-plan.md, W2/D9).
// auth_hash_concurrency bounds PBKDF2 work only. Account and sanction reads
// run without a hash slot, so slow storage cannot hold CPU capacity. A waiting
// attempt stops at authHashMaxWait or when its session closes.
const authHashMaxWait = 2 * time.Second

var (
	metricAuthHashWaitBuckets = expvar.NewMap("world_auth_hash_wait_buckets")
	metricAuthHashBusy        = expvar.NewInt("world_auth_hash_busy_total")
)

// authInFlightCapacity bounds authentication goroutines, which may wait on
// storage. It stays a fixed multiple of hash slots, so waiting is bounded too.
func authInFlightCapacity(hashConcurrency int) int {
	return min(max(16, 4*hashConcurrency), 256)
}

type authHashGate struct {
	slots   chan struct{}
	session *net.Session
}

// hashGate returns a nil interface without configured slots; Authenticate
// then hashes without a limit, as direct handler tests expect.
func (w *World) hashGate(s *net.Session) account.HashGate {
	if w.hashSlots == nil {
		return nil
	}
	return &authHashGate{slots: w.hashSlots, session: s}
}

func (g *authHashGate) Acquire() bool {
	started := time.Now()
	select {
	case g.slots <- struct{}{}:
		observeBucket(metricAuthHashWaitBuckets, time.Since(started))
		return true
	default:
	}
	timer := time.NewTimer(authHashMaxWait)
	defer timer.Stop()
	select {
	case g.slots <- struct{}{}:
		observeBucket(metricAuthHashWaitBuckets, time.Since(started))
		return true
	case <-timer.C:
	case <-g.session.Done():
	}
	metricAuthHashBusy.Add(1)
	return false
}

func (g *authHashGate) Release() { <-g.slots }
