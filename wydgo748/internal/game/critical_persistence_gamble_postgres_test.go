package game

import (
	"bytes"
	"context"
	"encoding/json"
	"os"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
	"wydgo/internal/store"
	"wydgo/internal/wire"
)

type criticalHeldGamblePostgresStore struct{ *criticalHeldPostgresStore }

func (s *criticalHeldGamblePostgresStore) SaveCriticalInstanceState(state *model.InstanceStateSnapshot, accounts ...*model.Account) error {
	s.attempts++
	s.entered <- criticalWrite{accounts: accountStateSnapshotsForTest(accounts), instances: state}
	<-s.release
	if s.attempts == 1 && s.foreign.Index != 0 {
		accounts[0].Chars[0].Inv[0] = s.foreign
	}
	return s.PostgresStore.SaveCriticalInstanceState(state, accounts...)
}

// Like the store aggregate gate, this fixture must run with go test -p 1:
// instance_state is a singleton shared by packages in the isolated database.
func TestCriticalGamblePostgresCommitAndRejection(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	for _, mode := range []string{"loss", "silver", "silver-ledger-rejection"} {
		t.Run(mode, func(t *testing.T) {
			st, err := store.NewPostgresStore(context.Background(), store.PostgresConfig{URL: url, MaxConns: 2})
			if err != nil {
				t.Fatal(err)
			}
			t.Cleanup(st.Close)
			ctx := context.Background()
			conn, err := pgx.Connect(ctx, url)
			if err != nil {
				t.Fatal(err)
			}
			t.Cleanup(func() { _ = conn.Close(ctx) })
			var previous []byte
			if err := conn.QueryRow(ctx, `SELECT payload FROM instance_state WHERE singleton=true`).Scan(&previous); err != nil && err != pgx.ErrNoRows {
				t.Fatal(err)
			}
			t.Cleanup(func() {
				var err error
				if previous == nil {
					_, err = conn.Exec(ctx, `DELETE FROM instance_state WHERE singleton=true`)
				} else {
					_, err = conn.Exec(ctx, `UPDATE instance_state SET payload=$1 WHERE singleton=true`, previous)
				}
				if err != nil {
					t.Errorf("restore isolated instance aggregate: %v", err)
				}
			})
			w, p, fallback := handlerTestWorld(t)
			w.clock = newFakeClock(time.Unix(100, 0))
			stops := findGambleStops(t, func(r [5]byte) bool {
				if mode == "loss" {
					return gamblePrize(1000, r) == 0
				}
				return r[1] == 14
			})
			rng := configureCriticalGamble(w, p, stops)
			if mode != "loss" {
				p.Char.Gold, w.gambleJackpot, w.gamblePool = maxCharacterGold, gambleJackpotMax, gamblePoolMax
			}
			suffix := time.Now().UTC().Format("150405.000000000")
			p.Account.Name, p.Char.Name = "gamblepg"+suffix, "GamblePG"
			delete(w.playersByCharacterUID, p.Char.UID)
			p.Char.UID = ""
			foreign := &model.Account{Name: "gambleforeign" + suffix, PasswordHash: "hash", Chars: []model.Char{{
				Name: "ForeignPG", Score: &model.Score{Version: model.ScoreVersion}, Inv: [64]model.Item{{Index: 4011}},
			}}}
			t.Cleanup(func() {
				if _, err := conn.Exec(ctx, `DELETE FROM accounts WHERE name_key=ANY($1)`,
					[]string{strings.ToLower(p.Account.Name), strings.ToLower(foreign.Name)}); err != nil {
					t.Errorf("remove isolated Gamble accounts: %v", err)
				}
			})
			for _, account := range []*model.Account{p.Account, foreign} {
				if err := st.CreateAccount(account); err != nil {
					t.Fatal(err)
				}
			}
			w.indexPlayerCharacter(p)
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("PostgreSQL Gamble account admission failed")
			}
			retainedState := w.instanceStateSnapshot()
			if err := st.SaveCriticalInstanceState(retainedState); err != nil {
				t.Fatal(err)
			}
			gate, release := criticalGate(t)
			backend := &criticalHeldGamblePostgresStore{&criticalHeldPostgresStore{
				PostgresStore: st, entered: make(chan criticalWrite, 2), release: gate,
			}}
			reject := mode == "silver-ledger-rejection"
			if reject {
				backend.foreign = foreign.Chars[0].Inv[0]
			}
			q := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(q)
			before := accountStateSnapshot(p.Account)
			assertDurable := func(account *model.Account, state *model.InstanceStateSnapshot) {
				t.Helper()
				loaded, err := st.LoadAccount(p.Account.Name)
				if err != nil || loaded == nil || loaded.Chars[0].Gold != account.Chars[0].Gold || loaded.Chars[0].Inv != account.Chars[0].Inv {
					t.Fatalf("durable Gamble gold/items were partially committed or rolled back: %v", err)
				}
				pools, err := st.LoadInstanceState()
				if err != nil || pools == nil {
					t.Fatalf("durable Gamble pools were partially committed or rolled back: %v", err)
				}
				gotJSON, err := json.Marshal(pools)
				if err != nil {
					t.Fatal(err)
				}
				wantJSON, err := json.Marshal(state)
				if err != nil || !bytes.Equal(gotJSON, wantJSON) {
					t.Fatalf("durable Gamble aggregate differs from its JSON contract: got=%s want=%s error=%v", gotJSON, wantJSON, err)
				}
			}
			w.handle(command{s: p.Session, pkt: criticalGambleRequest(p, 1000)})
			proposal := awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session, pkt: criticalGambleRequest(p, 1000)})
			assertDurable(before, retainedState)
			release()
			result := awaitCriticalResult(t, q)
			if reject != store.IsWriteRejected(result.err) || (!reject && result.err != nil) {
				t.Fatalf("unexpected PostgreSQL Gamble outcome: %v", result.err)
			}
			if reject {
				assertDurable(before, retainedState)
			} else {
				assertDurable(proposal.accounts[0], proposal.instances)
			}
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Session.QueuedPacketsForTest() != 0 ||
				!w.criticalInstanceReserved() || !w.criticalPlayerReserved(p) || rng.next != 4 || len(backend.entered) != 0 {
				t.Fatal("database completion bypassed Gamble actor acknowledgement or rerolled")
			}
			discardCriticalDeferredDuplicates(w)
			w.applyCriticalCompletion(result)
			if reject {
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.instanceStateSnapshot(), retainedState) {
					t.Fatal("certified Gamble rejection changed retained economic state")
				}
				assertCriticalPickupPacket(t, p, wire.MessagePanel("The Gamble bet could not be completed."))
				if len(q.pending) != 0 || len(backend.entered) != 0 || rng.next != 4 {
					t.Fatal("certified Gamble rejection automatically replayed")
				}
				w.handle(command{s: p.Session, pkt: criticalGambleRequest(p, 1000)})
				proposal = awaitCriticalWrite(t, backend.entered)
				w.applyCriticalCompletion(awaitCriticalResult(t, q))
				assertDurable(proposal.accounts[0], proposal.instances)
			}
			if p.Char.Gold != proposal.accounts[0].Chars[0].Gold || p.Char.Inv != proposal.accounts[0].Chars[0].Inv ||
				w.gambleJackpot != proposal.instances.GambleJackpot || w.gamblePool != proposal.instances.GamblePool ||
				len(q.pending) != 0 || w.criticalInstanceReserved() || w.shuttingDown || fallback.saves != 0 ||
				p.Session.QueuedPacketsForTest() != 3 {
				t.Fatal("production Gamble did not settle one atomic account/pool proposal")
			}
		})
	}
}
