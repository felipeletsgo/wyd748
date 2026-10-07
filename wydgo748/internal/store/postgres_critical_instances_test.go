package store

import (
	"context"
	"testing"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
)

func TestPostgresCriticalInstancesRejectsInvalidAdmission(t *testing.T) {
	valid := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
	for _, tc := range []struct {
		state    *model.InstanceStateSnapshot
		accounts []*model.Account
		readOnly bool
	}{
		{nil, nil, false},
		{&model.InstanceStateSnapshot{Version: -1}, nil, false},
		{valid, []*model.Account{nil}, false},
		{valid, nil, true},
	} {
		st := &PostgresStore{readOnly: tc.readOnly}
		if err := st.SaveCriticalInstanceState(tc.state, tc.accounts...); !IsWriteRejected(err) || st.writeSeq.Load() != 0 {
			t.Fatalf("invalid aggregate reached admission or lost rejection certification: %v", err)
		}
	}
}

// Tests sharing the fixture's singleton aggregate must run sequentially across
// packages (go test -p 1). Restore its previous payload after this focused gate.
func TestPostgresCriticalInstancesAtomicCommitRepetitionAndRollback(t *testing.T) {
	st, accounts := criticalAccountsPostgresFixture(t)
	ctx := context.Background()
	var previous []byte
	if err := st.pool.QueryRow(ctx, `SELECT payload FROM instance_state WHERE singleton=true`).Scan(&previous); err != nil && err != pgx.ErrNoRows {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		if previous == nil {
			_, _ = st.pool.Exec(ctx, `DELETE FROM instance_state WHERE singleton=true`)
		} else {
			_, _ = st.pool.Exec(ctx, `UPDATE instance_state SET payload=$1 WHERE singleton=true`, previous)
		}
	})
	owner, peer, foreign := accounts[0], accounts[1], accounts[2]
	state := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, GambleJackpot: 100, GamblePool: 200,
		NightmarePartyRuns: map[string]int{"window": 2}}
	if err := st.SaveCriticalInstanceState(state); err != nil {
		t.Fatal(err)
	}
	state.GambleJackpot, state.GamblePool = 101, 230
	owner.Chars[0].Gold, peer.CargoGold = 81, 32
	for range 2 {
		if err := st.SaveCriticalInstanceState(state, owner, peer); err != nil {
			t.Fatal(err)
		}
	}
	assertCommitted := func() {
		t.Helper()
		loadedState, err := st.LoadInstanceState()
		if err != nil || loadedState == nil || loadedState.GambleJackpot != 101 || loadedState.GamblePool != 230 || loadedState.NightmarePartyRuns["window"] != 2 {
			t.Fatalf("instance aggregate was lost or partially overwritten: %+v error=%v", loadedState, err)
		}
		loadedOwner, err := st.LoadAccount(owner.Name)
		if err != nil || loadedOwner == nil || loadedOwner.Chars[0].Gold != 81 {
			t.Fatalf("owner account was lost or partially overwritten: error=%v", err)
		}
		loadedPeer, err := st.LoadAccount(peer.Name)
		if err != nil || loadedPeer == nil || loadedPeer.CargoGold != 32 || loadedPeer.Chars[0].Inv[1].Index != 0 {
			t.Fatalf("peer account was lost or partially overwritten: error=%v", err)
		}
	}
	assertCommitted()
	owner.Chars[0].Gold, peer.CargoGold = 99, 55
	peer.Chars[0].Inv[1] = foreign.Chars[0].Inv[0]
	state.GambleJackpot, state.GamblePool = 999, 888
	if err := st.SaveCriticalInstanceState(state, owner, peer); !IsWriteRejected(err) {
		t.Fatalf("foreign-ledger conflict did not certify full atomic rollback: %v", err)
	}
	assertCommitted()
}
