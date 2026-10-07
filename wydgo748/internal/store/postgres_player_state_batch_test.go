package store

import (
	"context"
	"os"
	"reflect"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestPostgresPlayerStateBatchRejectsInvalidOwnerBeforeDatabase(t *testing.T) {
	uid := "11111111111141118111111111111111"
	state := &model.CharState{SpecialCoins: map[string]uint32{"fame": 1}}
	owner := &model.Account{Name: "owner", Chars: []model.Char{{UID: uid}}}
	for _, test := range []struct {
		name     string
		uid      string
		accounts []*model.Account
	}{
		{"missing-owner", uid, nil},
		{"nil-owner", uid, []*model.Account{nil}},
		{"invalid-uid", "invalid", []*model.Account{owner}},
		{"foreign-sidecar", "22222222222242228222222222222222", []*model.Account{owner}},
	} {
		t.Run(test.name, func(t *testing.T) {
			st := &PostgresStore{}
			if err := st.SavePlayerStateBatch(nil, test.uid, state, test.accounts...); !IsWriteRejected(err) {
				t.Fatalf("invalid owner did not return a certified rejection: %v", err)
			}
			if st.writeSeq.Load() != 0 {
				t.Fatal("invalid owner reached write admission")
			}
		})
	}
	if err := (&PostgresStore{readOnly: true}).SavePlayerStateBatch(nil, uid, state, owner); !IsWriteRejected(err) {
		t.Fatalf("read-only batch did not return a certified rejection: %v", err)
	}
}

// Run only against a provisioned disposable database. Ordinary local test runs
// explicitly remove this variable and report this integration gate as skipped.
func TestPostgresPlayerStateBatchCommitsAndRollsBackAtomically(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 2})
	if err != nil {
		t.Fatal(err)
	}
	defer st.Close()
	suffix := time.Now().UTC().Format("150405.000000000")
	owner := postgresTestAccount("batchowner"+suffix, "BatchOwner", model.Item{Index: 4011})
	peer := postgresTestAccount("batchpeer"+suffix, "BatchPeer", model.Item{Index: 4011})
	foreign := postgresTestAccount("batchforeign"+suffix, "BatchForeign", model.Item{Index: 4011})
	defer func() {
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=ANY($1)`,
			[]string{strings.ToLower(owner.Name), strings.ToLower(peer.Name), strings.ToLower(foreign.Name)})
	}()
	for _, account := range []*model.Account{owner, peer, foreign} {
		if err := st.CreateAccount(account); err != nil {
			t.Fatal(err)
		}
	}
	uid := owner.Chars[0].UID
	owner.Chars[0].ArchLevel370, owner.Chars[0].Gold = true, 81
	peer.CargoGold = 32
	state := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 1}}
	if err := st.SavePlayerStateBatch(nil, uid, state, owner, peer); err != nil {
		t.Fatal(err)
	}
	assertCommitted := func() {
		t.Helper()
		loadedOwner, err := st.LoadAccount(owner.Name)
		if err != nil || loadedOwner == nil || !loadedOwner.Chars[0].ArchLevel370 || loadedOwner.Chars[0].Gold != 81 {
			t.Fatalf("owner state did not commit or was partially overwritten: %v", err)
		}
		loadedPeer, err := st.LoadAccount(peer.Name)
		if err != nil || loadedPeer == nil || loadedPeer.CargoGold != 32 {
			t.Fatalf("peer state did not commit or was partially overwritten: %v", err)
		}
		loadedState, err := st.LoadCharState(uid)
		if err != nil || !reflect.DeepEqual(loadedState, state) {
			t.Fatalf("sidecar state did not commit or was partially overwritten: state=%+v error=%v", loadedState, err)
		}
	}
	assertCommitted()
	// The conflicting item belongs to a third account, so rejection must come
	// from the durable ledger rather than duplicate detection within this batch.
	owner.Chars[0].Gold, peer.CargoGold = 99, 55
	peer.Chars[0].Inv[1] = foreign.Chars[0].Inv[0]
	if err := st.SavePlayerStateBatch(nil, uid, &model.CharState{SpecialCoins: map[string]uint32{"fame": 0}}, owner, peer); !IsWriteRejected(err) {
		t.Fatalf("ledger conflict did not return a certified atomic rejection: %v", err)
	}
	assertCommitted()
	peer.Chars[0].Inv[1] = model.Item{}
	if err := st.SavePlayerStateBatch(nil, uid, nil, owner, peer); err != nil {
		t.Fatal(err)
	}
	if loaded, err := st.LoadCharState(uid); err != nil || loaded != nil {
		t.Fatalf("empty sidecar was not deleted atomically: state=%+v error=%v", loaded, err)
	}
}
