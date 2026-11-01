package store

import (
	"expvar"
	"fmt"
	"hash/fnv"
	"sync"
)

// Unchanged periodic writes (DOCS/server/server-correction-plan.md, W7/P2). A
// player-state batch equal to what this process last committed for the same
// accounts and sidecar skips its transaction. The worker already encodes the
// snapshot, so the game actor pays nothing. A mark is set only after a
// confirmed COMMIT and is removed when any other write of that account or
// sidecar starts. A skipped account is still written after unchangedMaxSkips
// skips. Writes by other processes are not seen; the game stays authoritative
// for online accounts and overwrites them on the next change, as before.

const (
	unchangedMaxSkips = 19
	unchangedMaxMarks = 8192
)

var metricPostgresUnchangedSkipped = expvar.NewInt("postgres_unchanged_writes_skipped_total")

type unchangedMark struct {
	hash  uint64
	skips uint8
}

type unchangedMarks struct {
	mu       sync.Mutex
	accounts map[string]unchangedMark
	states   map[string]unchangedMark
}

func accountSnapshotMark(snapshot *accountSnapshot) uint64 {
	hash := fnv.New64a()
	fmt.Fprintf(hash, "%s\x00%s\x00", snapshot.key, snapshot.display)
	_, _ = hash.Write(snapshot.payload)
	for _, c := range snapshot.characters {
		mortal := ""
		if c.mortalUID != nil {
			mortal = fmt.Sprintf("%x", c.mortalUID.Bytes)
		}
		fmt.Fprintf(hash, "\x00c%s|%d|%s|%s|%s", c.uid, c.slot, c.nameKey, c.evolution, mortal)
	}
	for _, item := range snapshot.items {
		fmt.Fprintf(hash, "\x00i%s|%s|%d|%x|%d|%d", item.uid, item.location, item.index, item.effects,
			item.activatedUnix, item.expiresUnix)
	}
	return hash.Sum64()
}

func stateMark(key string, payload []byte) uint64 {
	hash := fnv.New64a()
	fmt.Fprintf(hash, "%s\x00%t\x00", key, payload == nil)
	_, _ = hash.Write(payload)
	return hash.Sum64()
}

// forgetUnchanged removes the marks of accounts and sidecars a write touches.
func (s *PostgresStore) forgetUnchanged(accountKeys, stateKeys []string) {
	s.unchanged.mu.Lock()
	defer s.unchanged.mu.Unlock()
	for _, key := range accountKeys {
		delete(s.unchanged.accounts, key)
	}
	for _, key := range stateKeys {
		delete(s.unchanged.states, key)
	}
}

// unchangedBatch reports whether every account and the sidecar equal their
// last committed marks, and counts the skip. The caller holds their fences.
func (s *PostgresStore) unchangedBatch(snapshots []*accountSnapshot, stateKey string, statePayload []byte) bool {
	s.unchanged.mu.Lock()
	defer s.unchanged.mu.Unlock()
	state, ok := s.unchanged.states[stateKey]
	if !ok || state.hash != stateMark(stateKey, statePayload) || state.skips >= unchangedMaxSkips {
		return false
	}
	for _, snapshot := range snapshots {
		mark, ok := s.unchanged.accounts[snapshot.key]
		if !ok || mark.hash != accountSnapshotMark(snapshot) || mark.skips >= unchangedMaxSkips {
			return false
		}
	}
	state.skips++
	s.unchanged.states[stateKey] = state
	for _, snapshot := range snapshots {
		mark := s.unchanged.accounts[snapshot.key]
		mark.skips++
		s.unchanged.accounts[snapshot.key] = mark
	}
	return true
}

// rememberUnchanged records what a confirmed COMMIT wrote.
func (s *PostgresStore) rememberUnchanged(snapshots []*accountSnapshot, stateKey string, statePayload []byte) {
	s.unchanged.mu.Lock()
	defer s.unchanged.mu.Unlock()
	if s.unchanged.accounts == nil || len(s.unchanged.accounts) > unchangedMaxMarks {
		s.unchanged.accounts = make(map[string]unchangedMark)
		s.unchanged.states = make(map[string]unchangedMark)
	}
	for _, snapshot := range snapshots {
		s.unchanged.accounts[snapshot.key] = unchangedMark{hash: accountSnapshotMark(snapshot)}
	}
	s.unchanged.states[stateKey] = unchangedMark{hash: stateMark(stateKey, statePayload)}
}
