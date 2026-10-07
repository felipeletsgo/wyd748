package store

import (
	"context"
	"fmt"
	"reflect"
	"sync"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
)

func TestPostgresCriticalGameStateRejectsInvalidAdmission(t *testing.T) {
	uid := "11111111111141118111111111111111"
	owner := &model.Account{Name: "owner", Chars: []model.Char{{UID: uid}}}
	guilds := &model.GuildRegistry{Version: model.GuildRegistryVersion}
	instances := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
	state := &model.CharState{SpecialCoins: map[string]uint32{"fame": 1}}
	for _, tc := range []struct {
		name      string
		guilds    *model.GuildRegistry
		instances *model.InstanceStateSnapshot
		uid       string
		state     *model.CharState
		accounts  []*model.Account
		readOnly  bool
	}{
		{name: "empty"},
		{name: "invalid-guild-version", guilds: &model.GuildRegistry{Version: -1}},
		{name: "invalid-instance-version", instances: &model.InstanceStateSnapshot{Version: -1}},
		{name: "nil-participant", guilds: guilds, accounts: []*model.Account{nil}},
		{name: "missing-owner", instances: instances, uid: uid},
		{name: "missing-uid", guilds: guilds, state: state, accounts: []*model.Account{owner}},
		{name: "invalid-uid", guilds: guilds, uid: "invalid", accounts: []*model.Account{owner}},
		{name: "foreign-sidecar", instances: instances, uid: "22222222222242228222222222222222", state: state, accounts: []*model.Account{owner}},
		{name: "peer-owned-delete", guilds: guilds, uid: uid, accounts: []*model.Account{{Name: "other"}, owner}},
		{name: "read-only", guilds: guilds, readOnly: true},
	} {
		t.Run(tc.name, func(t *testing.T) {
			st := &PostgresStore{readOnly: tc.readOnly}
			if err := st.SaveCriticalGameState(tc.guilds, tc.instances, tc.uid, tc.state, tc.accounts...); !IsWriteRejected(err) || st.writeSeq.Load() != 0 {
				t.Fatalf("invalid combined aggregate reached write admission or lost certification: %v", err)
			}
		})
	}
}

// Global singleton fixtures must not overlap tests in another package: use -p 1.
func preserveCriticalGameStateSingletons(t *testing.T, st *PostgresStore) {
	for _, table := range []string{"guild_state", "instance_state"} {
		var payload []byte
		var version int64
		err := st.pool.QueryRow(context.Background(), "SELECT payload,version FROM "+table+" WHERE singleton=true").Scan(&payload, &version)
		if err != nil && err != pgx.ErrNoRows {
			t.Fatal(err)
		}
		t.Cleanup(func() {
			var err error
			if payload == nil {
				_, err = st.pool.Exec(context.Background(), "DELETE FROM "+table+" WHERE singleton=true")
			} else {
				_, err = st.pool.Exec(context.Background(), "UPDATE "+table+" SET payload=$1,version=$2 WHERE singleton=true", payload, version)
			}
			if err != nil {
				t.Errorf("restore %s singleton: %v", table, err)
			}
		})
	}
}

func TestPostgresCriticalGameStateAtomicFormsAndRepetition(t *testing.T) {
	for _, form := range []string{"guild-only", "guild-accounts", "guild-sidecar", "guild-instance", "instance-sidecar", "all", "delete"} {
		t.Run(form, func(t *testing.T) {
			st, accounts := criticalAccountsPostgresFixture(t)
			preserveCriticalGameStateSingletons(t, st)
			owner, peer := accounts[0], accounts[1]
			var guilds *model.GuildRegistry
			var instances *model.InstanceStateSnapshot
			var state *model.CharState
			var uid string
			var participants []*model.Account
			if form != "instance-sidecar" {
				guilds = &model.GuildRegistry{Version: model.GuildRegistryVersion}
			}
			if form == "guild-instance" || form == "instance-sidecar" || form == "all" {
				instances = &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, GamblePool: 123}
			}
			if form != "guild-only" && form != "guild-instance" {
				participants = []*model.Account{owner, peer}
				owner.Chars[0].Gold, peer.CargoGold = 81, 32
			}
			if form == "guild-sidecar" || form == "instance-sidecar" || form == "all" || form == "delete" {
				uid = owner.Chars[0].UID
				state = &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 2}}
				if form == "delete" {
					if err := st.SavePlayerStateBatch(nil, uid, state, owner, peer); err != nil {
						t.Fatal(err)
					}
					state = nil
				}
			}
			for range 2 {
				if err := st.SaveCriticalGameState(guilds, instances, uid, state, participants...); err != nil {
					t.Fatal(err)
				}
			}
			if guilds != nil {
				loaded, err := st.LoadGuilds()
				if err != nil || !reflect.DeepEqual(loaded, guilds) {
					t.Fatalf("guild-only or combined guild commit was lost: %+v error=%v", loaded, err)
				}
			}
			if instances != nil {
				loaded, err := st.LoadInstanceState()
				if err != nil || loaded == nil || loaded.GamblePool != 123 {
					t.Fatalf("instance commit was lost: %+v error=%v", loaded, err)
				}
			}
			if uid != "" {
				loaded, err := st.LoadCharState(uid)
				if err != nil || !reflect.DeepEqual(loaded, state) {
					t.Fatalf("sidecar commit or delete was lost: %+v error=%v", loaded, err)
				}
			}
			for i, account := range participants {
				loaded, err := st.LoadAccount(account.Name)
				if err != nil || loaded == nil || i == 0 && loaded.Chars[0].Gold != 81 || i == 1 && loaded.CargoGold != 32 {
					t.Fatalf("participant %d commit was lost: error=%v", i, err)
				}
			}
		})
	}
}

func TestPostgresCriticalGameStateLateRejectionRollsBackEveryAggregate(t *testing.T) {
	st, accounts := criticalAccountsPostgresFixture(t)
	preserveCriticalGameStateSingletons(t, st)
	owner, peer, foreign := accounts[0], accounts[1], accounts[2]
	guilds := &model.GuildRegistry{Version: model.GuildRegistryVersion, Guilds: []model.Guild{{
		ID: 1, Name: "Critical", Fame: 2, Members: []model.GuildMember{{Character: owner.Chars[0].Name, Account: owner.Name, Rank: model.GuildRankLeader}},
	}}}
	instances := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, GamblePool: 123}
	state := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 2}}
	uid := owner.Chars[0].UID
	owner.Chars[0].Gold, peer.CargoGold = 81, 32
	if err := st.SaveCriticalGameState(guilds, instances, uid, state, owner, peer); err != nil {
		t.Fatal(err)
	}
	assertCommitted := func() {
		t.Helper()
		loadedOwner, err := st.LoadAccount(owner.Name)
		if err != nil || loadedOwner == nil || loadedOwner.Chars[0].Gold != 81 {
			t.Fatalf("owner account partially committed: %v", err)
		}
		loadedPeer, err := st.LoadAccount(peer.Name)
		if err != nil || loadedPeer == nil || loadedPeer.CargoGold != 32 || loadedPeer.Chars[0].Inv[1].Index != 0 {
			t.Fatalf("peer account partially committed: %v", err)
		}
		loadedState, err := st.LoadCharState(uid)
		if err != nil || loadedState == nil || loadedState.SpecialCoins["fame"] != 2 {
			t.Fatalf("sidecar partially committed: %+v error=%v", loadedState, err)
		}
		loadedGuilds, err := st.LoadGuilds()
		if err != nil || loadedGuilds == nil || len(loadedGuilds.Guilds) != 1 || loadedGuilds.Guilds[0].Fame != 2 {
			t.Fatalf("guild aggregate partially committed: %+v error=%v", loadedGuilds, err)
		}
		loadedInstances, err := st.LoadInstanceState()
		if err != nil || loadedInstances == nil || loadedInstances.GamblePool != 123 {
			t.Fatalf("instance aggregate partially committed: %+v error=%v", loadedInstances, err)
		}
	}
	assertCommitted()
	// Force a SQL rejection in the last aggregate write, after accounts, ledger,
	// sidecar and guild have all been written inside the same transaction.
	constraint := "critical_game_state_test_failure"
	if _, err := st.pool.Exec(context.Background(), `ALTER TABLE instance_state ADD CONSTRAINT `+constraint+
		` CHECK ((payload->>'gamblePool') IS DISTINCT FROM '987654321') NOT VALID`); err != nil {
		t.Fatal(err)
	}
	t.Cleanup(func() {
		if _, err := st.pool.Exec(context.Background(), "ALTER TABLE instance_state DROP CONSTRAINT "+constraint); err != nil {
			t.Errorf("remove isolated rejection constraint: %v", err)
		}
	})
	owner.Chars[0].Gold, peer.CargoGold = 99, 55
	guilds.Guilds[0].Fame = 9
	instances.GamblePool = 987654321
	state.SpecialCoins["fame"] = 9
	for _, rejection := range []string{"last-aggregate", "delete-sidecar", "foreign-ledger"} {
		t.Run(rejection, func(t *testing.T) {
			proposedState := state
			if rejection == "delete-sidecar" {
				proposedState = nil
			} else if rejection == "foreign-ledger" {
				peer.Chars[0].Inv[1] = foreign.Chars[0].Inv[0]
			}
			if err := st.SaveCriticalGameState(guilds, instances, uid, proposedState, owner, peer); !IsWriteRejected(err) {
				t.Fatal(fmt.Errorf("%s did not certify full rollback: %w", rejection, err))
			}
			assertCommitted()
		})
	}
}

func TestPostgresCriticalGameStateFencesQueuedAutosaves(t *testing.T) {
	for _, deletion := range []bool{false, true} {
		t.Run(fmt.Sprintf("delete=%t", deletion), func(t *testing.T) {
			st, accounts := criticalAccountsPostgresFixture(t)
			preserveCriticalGameStateSingletons(t, st)
			owner, peer := accounts[0], accounts[1]
			uid := owner.Chars[0].UID
			started, release := make(chan struct{}), make(chan struct{})
			var once sync.Once
			doRelease := func() { once.Do(func() { close(release) }) }
			t.Cleanup(doRelease)
			if err := st.enqueue(postgresWriteJob{label: "test: hold combined-state autosaves", run: func() error {
				close(started)
				<-release
				return nil
			}}); err != nil {
				t.Fatal(err)
			}
			select {
			case <-started:
			case <-time.After(2 * time.Second):
				t.Fatal("autosave worker did not reach the held job")
			}
			for _, account := range []*model.Account{owner, peer} {
				account.CargoGold = 111
				if err := st.SaveAccountAsync(account); err != nil {
					t.Fatal(err)
				}
				account.CargoGold = 222
			}
			oldState := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 1}}
			if err := st.SaveCharStateAsync(uid, oldState); err != nil {
				t.Fatal(err)
			}
			state := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 2}}
			if deletion {
				state = nil
			}
			guilds := &model.GuildRegistry{Version: model.GuildRegistryVersion}
			instances := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion, GamblePool: 123}
			done := make(chan error, 1)
			go func() { done <- st.SaveCriticalGameState(guilds, instances, uid, state, owner, peer) }()
			select {
			case err := <-done:
				if err != nil {
					t.Fatal(err)
				}
			case <-time.After(2 * time.Second):
				doRelease()
				t.Fatal("combined-state transaction waited for an unrelated async-worker barrier")
			}
			doRelease()
			st.Flush()
			for _, account := range []*model.Account{owner, peer} {
				loaded, err := st.LoadAccount(account.Name)
				if err != nil || loaded == nil || loaded.CargoGold != 222 {
					t.Fatalf("old autosave overwrote a combined-state participant: account=%s error=%v", account.Name, err)
				}
			}
			loaded, err := st.LoadCharState(uid)
			if err != nil || !reflect.DeepEqual(loaded, state) {
				t.Fatalf("old sidecar autosave overwrote or resurrected combined state: %+v error=%v", loaded, err)
			}
		})
	}
}
