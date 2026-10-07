package game

import (
	"context"
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

// Hold the actual certified adapter at its worker boundary. The optional
// foreign item injects a durable-ledger conflict into the detached proposal,
// never the actor's retained account. All transaction outcomes come from PG.
type criticalHeldPostgresStore struct {
	*store.PostgresStore
	entered  chan criticalWrite
	release  <-chan struct{}
	foreign  model.Item
	attempts int
}

func (s *criticalHeldPostgresStore) SaveCriticalAccounts(accounts ...*model.Account) error {
	s.attempts++
	s.entered <- criticalWrite{accounts: accountStateSnapshotsForTest(accounts)}
	<-s.release
	if s.attempts == 1 && s.foreign.Index != 0 {
		accounts[0].Chars[0].Inv[0] = s.foreign
	}
	return s.PostgresStore.SaveCriticalAccounts(accounts...)
}

func accountStateSnapshotsForTest(accounts []*model.Account) []*model.Account {
	snapshots := make([]*model.Account, len(accounts))
	for i, account := range accounts {
		snapshots[i] = accountStateSnapshot(account)
	}
	return snapshots
}

func TestCriticalNativeConsumablePostgresCommitAndRejection(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	for _, tc := range criticalNativeCases() {
		for _, reject := range []bool{false, true} {
			mode := "commit"
			if reject {
				mode = "ledger-rejection"
			}
			t.Run(tc.name+"/"+mode, func(t *testing.T) {
				st, err := store.NewPostgresStore(context.Background(), store.PostgresConfig{URL: url, MaxConns: 2})
				if err != nil {
					t.Fatal(err)
				}
				t.Cleanup(st.Close)
				conn, err := pgx.Connect(context.Background(), url)
				if err != nil {
					t.Fatal(err)
				}
				t.Cleanup(func() { _ = conn.Close(context.Background()) })
				w, p, fallback := handlerTestWorld(t)
				w.clock = newFakeClock(time.Unix(100, 0))
				configureCriticalNative(w, p, tc)
				suffix := time.Now().UTC().Format("150405.000000000")
				p.Account.Name, p.Char.Name = "nativepg"+suffix, "NativePG"
				delete(w.playersByCharacterUID, p.Char.UID)
				p.Char.UID = ""
				for _, item := range []*model.Item{&p.Char.Inv[62], &p.Char.Equip[13]} {
					item.UID, err = model.NewItemUID()
					if err != nil {
						t.Fatal(err)
					}
				}
				foreign := &model.Account{Name: "foreignpg" + suffix, PasswordHash: "hash", Chars: []model.Char{{
					Name: "ForeignPG", Score: &model.Score{Version: model.ScoreVersion}, Inv: [64]model.Item{{Index: 4011}},
				}}}
				t.Cleanup(func() {
					_, _ = conn.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=ANY($1)`,
						[]string{strings.ToLower(p.Account.Name), strings.ToLower(foreign.Name)})
				})
				for _, account := range []*model.Account{p.Account, foreign} {
					if err := st.CreateAccount(account); err != nil {
						t.Fatal(err)
					}
				}
				w.indexPlayerCharacter(p)
				if !w.claimAccountSession(p.Session, p.Account.Name) {
					t.Fatal("PostgreSQL fixture account admission failed")
				}
				gate, release := criticalGate(t)
				backend := &criticalHeldPostgresStore{PostgresStore: st, entered: make(chan criticalWrite, 2), release: gate}
				if reject {
					backend.foreign = foreign.Chars[0].Inv[0]
				}
				queue := newCriticalTestQueue(t, backend, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(queue)
				before := accountStateSnapshot(p.Account)
				want := criticalNativeExpected(w, before, p.CharSlot, tc)
				w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
				awaitCriticalWrite(t, backend.entered)
				w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
					t.Fatal("held production intention mutated, confirmed or replayed before commit")
				}
				release()
				result := awaitCriticalResult(t, queue)
				if reject != store.IsWriteRejected(result.err) || (!reject && result.err != nil) {
					t.Fatalf("unexpected PostgreSQL outcome: %v", result.err)
				}
				assertDurable := func(expected *model.Account) {
					t.Helper()
					loaded, err := st.LoadAccount(p.Account.Name)
					if err != nil || loaded == nil {
						t.Fatalf("durable account load failed: %v", err)
					}
					got, expectedChar := loaded.Chars[0], expected.Chars[0]
					if got.Inv != expectedChar.Inv || got.Equip != expectedChar.Equip || got.NightmareTickets != expectedChar.NightmareTickets ||
						got.LastNightmareUnix != expectedChar.LastNightmareUnix || got.Score.Mastery != expectedChar.Score.Mastery ||
						got.LearnedSkill != expectedChar.LearnedSkill || got.ShortSkill != expectedChar.ShortSkill || !reflect.DeepEqual(got.QuestsDone, expectedChar.QuestsDone) {
						t.Fatal("durable native-consumable delta was partially committed or rolled back")
					}
				}
				if reject {
					assertDurable(before)
				} else {
					assertDurable(want)
				}
				// The DB completion alone is not permission to publish or release.
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Session.QueuedPacketsForTest() != 0 || !w.criticalPlayerReserved(p) {
					t.Fatal("database completion bypassed actor acknowledgement")
				}
				discardCriticalDeferredDuplicates(w)
				w.applyCriticalCompletion(result)
				if reject {
					if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) {
						t.Fatal("certified rollback changed live state")
					}
					assertCriticalPickupPacket(t, p, wire.SendItem(p.ID, placeInv, 62, p.Char.Inv[62]))
					assertCriticalPickupPacket(t, p, wire.MessagePanel("Item use could not be saved. Please try again later."))
					w.handle(command{s: p.Session, pkt: criticalNativeRequest(p)})
					awaitCriticalWrite(t, backend.entered)
					w.applyCriticalCompletion(awaitCriticalResult(t, queue))
					assertDurable(want)
				}
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), want) || w.shuttingDown || p.PersistencePoisoned ||
					len(queue.pending) != 0 || w.criticalPlayerReserved(p) || fallback.saves != 0 {
					t.Fatal("production PostgreSQL completion lost state, retained fences or entered maintenance")
				}
				assertCriticalNativePublication(t, p, tc)
			})
		}
	}
}
