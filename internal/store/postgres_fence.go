package store

import (
	"errors"
	"fmt"
	"sort"
	"strings"
	"sync"

	"wydgo/internal/model"
)

type postgresFenceEntry struct {
	mu   sync.Mutex
	refs int
}

// postgresFenceSet serializa somente agregados que compartilham a mesma
// identidade e descarta a entrada quando o ultimo usuario libera a fence.
type postgresFenceSet struct {
	mu      sync.Mutex
	entries map[string]*postgresFenceEntry
}

func (f *postgresFenceSet) lock(keys ...string) func() {
	ordered := append([]string(nil), keys...)
	sort.Strings(ordered)
	unique := ordered[:0]
	for _, key := range ordered {
		if key == "" || (len(unique) != 0 && unique[len(unique)-1] == key) {
			continue
		}
		unique = append(unique, key)
	}
	if len(unique) == 0 {
		return func() {}
	}

	f.mu.Lock()
	if f.entries == nil {
		f.entries = make(map[string]*postgresFenceEntry)
	}
	entries := make([]*postgresFenceEntry, len(unique))
	for i, key := range unique {
		entry := f.entries[key]
		if entry == nil {
			entry = &postgresFenceEntry{}
			f.entries[key] = entry
		}
		entry.refs++
		entries[i] = entry
	}
	f.mu.Unlock()

	for _, entry := range entries {
		entry.mu.Lock()
	}

	return func() {
		for i := len(entries) - 1; i >= 0; i-- {
			entries[i].mu.Unlock()
		}
		f.mu.Lock()
		for i, key := range unique {
			entry := entries[i]
			entry.refs--
			if entry.refs == 0 {
				delete(f.entries, key)
			}
		}
		f.mu.Unlock()
	}
}

func (s *PostgresStore) beginPostgresWrite() (func(), error) {
	s.mu.RLock()
	if s.closed {
		s.mu.RUnlock()
		return nil, errors.New("store: PostgreSQL fechado")
	}
	if s.readOnly {
		s.mu.RUnlock()
		return nil, errors.New("store: PostgreSQL somente leitura")
	}
	return s.mu.RUnlock, nil
}

func accountModelKeys(accounts ...*model.Account) []string {
	keys := make([]string, 0, len(accounts))
	for _, account := range accounts {
		if account == nil || account.Name == "" {
			continue
		}
		keys = append(keys, strings.ToLower(account.Name))
	}
	return keys
}

func accountSnapshotKeys(snapshots []*accountSnapshot) []string {
	keys := make([]string, 0, len(snapshots))
	for _, snapshot := range snapshots {
		if snapshot != nil && snapshot.key != "" {
			keys = append(keys, snapshot.key)
		}
	}
	return keys
}

func (s *PostgresStore) fenceAccountSnapshots(seq uint64, snapshots []*accountSnapshot) {
	if seq == 0 || len(snapshots) == 0 {
		return
	}
	s.orderMu.Lock()
	defer s.orderMu.Unlock()
	if s.latestAccountSeq == nil {
		s.latestAccountSeq = make(map[string]uint64)
	}
	for _, key := range accountSnapshotKeys(snapshots) {
		if seq > s.latestAccountSeq[key] {
			s.latestAccountSeq[key] = seq
		}
	}
}

func (s *PostgresStore) fenceCharStates(seq uint64, uids ...string) {
	if seq == 0 || len(uids) == 0 {
		return
	}
	s.orderMu.Lock()
	defer s.orderMu.Unlock()
	if s.latestStateSeq == nil {
		s.latestStateSeq = make(map[string]uint64)
	}
	for _, uid := range uids {
		if uid != "" && seq > s.latestStateSeq[uid] {
			s.latestStateSeq[uid] = seq
		}
	}
}

func postgresStateFenceKey(uid string) (string, error) {
	normalized, err := model.NormalizeCharacterUID(uid)
	if err != nil {
		return "", fmt.Errorf("store: charstate: %w", err)
	}
	if normalized == "" {
		return "", errors.New("store: charstate sem UID de personagem")
	}
	return normalized, nil
}
