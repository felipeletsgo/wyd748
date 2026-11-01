package store

import (
	"context"
	"testing"

	"wydgo/internal/model"
)

func storedGold(t *testing.T, st *PostgresStore, name string) uint32 {
	t.Helper()
	acc, err := st.LoadAccount(name)
	if err != nil {
		t.Fatal(err)
	}
	return acc.Chars[0].Gold
}

// W7/P2: an unchanged player-state batch skips its transaction, but never
// after another write of the same account and never more than 19 times.
func TestUnchangedPlayerStateBatchSkipsOnlyKnownDurableContent(t *testing.T) {
	st, accounts := criticalAccountsPostgresFixture(t)
	owner := accounts[0]
	uid := owner.Chars[0].UID
	if uid == "" {
		t.Fatal("fixture character has no UID")
	}
	state := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 3}}
	skipped := func(save func() error) bool {
		t.Helper()
		before := metricPostgresUnchangedSkipped.Value()
		if err := save(); err != nil {
			t.Fatal(err)
		}
		return metricPostgresUnchangedSkipped.Value() > before
	}
	batch := func(gold uint32) func() error {
		return func() error {
			owner.Chars[0].Gold = gold
			return st.SavePlayerStateBatch(nil, uid, state, owner)
		}
	}
	if skipped(batch(10)) {
		t.Fatal("the first write was skipped")
	}
	if !skipped(batch(10)) {
		t.Fatal("an identical batch was written again")
	}
	if skipped(batch(11)) || storedGold(t, st, owner.Name) != 11 {
		t.Fatal("a changed batch was skipped")
	}
	// Another write changes the durable row; the next batch must be written
	// even though it equals the last batch mark.
	owner.Chars[0].Gold = 99
	if err := st.SaveCriticalAccounts(owner); err != nil {
		t.Fatal(err)
	}
	if skipped(batch(11)) || storedGold(t, st, owner.Name) != 11 {
		t.Fatal("a batch after another committed write was skipped")
	}
	for range unchangedMaxSkips {
		if !skipped(batch(11)) {
			t.Fatal("an unchanged batch was written before the skip bound")
		}
	}
	if skipped(batch(11)) {
		t.Fatal("the skip bound did not force a write")
	}
	var payload []byte
	key, _ := postgresUUID(uid)
	if err := st.pool.QueryRow(context.Background(), `SELECT payload FROM character_states WHERE character_uid=$1`, key).Scan(&payload); err != nil || len(payload) == 0 {
		t.Fatalf("sidecar not durable: %v", err)
	}
}
