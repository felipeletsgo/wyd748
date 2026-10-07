package game

import (
	"context"
	"errors"
	"os"
	"reflect"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
	"wydgo/internal/store"
)

// Execute a real transaction before optionally losing its acknowledgement.
// The injected ledger conflict is confined to the detached proposal.
type criticalSharedPostgresStore struct {
	*store.PostgresStore
	entered chan criticalWrite
	release <-chan struct{}
	foreign model.Item
	unknown bool
}

func (s *criticalSharedPostgresStore) SaveCriticalAccounts(accounts ...*model.Account) error {
	s.entered <- criticalWrite{accounts: accountStateSnapshotsForTest(accounts)}
	<-s.release
	if s.foreign.Index != 0 {
		accounts[0].Chars[0].Inv[63] = s.foreign
	}
	err := s.PostgresStore.SaveCriticalAccounts(accounts...)
	if s.unknown && (err == nil || store.IsWriteRejected(err)) {
		return errors.New("transaction acknowledgement unavailable")
	}
	return err
}

func TestCriticalSharedPostgresSettlement(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	for _, operation := range []string{"pickup", "drop", "kill"} {
		for _, outcome := range []string{"commit", "rejection", "unknown-after-commit", "unknown-after-rejection"} {
			t.Run(operation+"/"+outcome, func(t *testing.T) {
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
				var w *World
				var p *Player
				var mob *Mob
				var g *GroundItem
				if operation == "kill" {
					w, p, mob, _ = killRewardTestWorld(t, nil)
				} else {
					w, p, _ = handlerTestWorld(t)
					if operation == "drop" {
						p.Char.Inv[0] = model.Item{Index: 412}
						w.items[412] = model.ItemDef{Index: 412}
					} else {
						uid, err := model.NewItemUID()
						if err != nil {
							t.Fatal(err)
						}
						g = &GroundItem{ID: 10001, Item: model.Item{Index: 412, UID: uid}, X: p.X, Y: p.Y}
						w.registerGroundItem(g)
					}
				}
				suffix := time.Now().UTC().Format("150405.000000000")
				p.Account.Name = "sharedpg" + suffix
				delete(w.playersByCharacterUID, p.Char.UID)
				p.Char.UID = ""
				foreign := &model.Account{Name: "foreignpg" + suffix, PasswordHash: "hash", Chars: []model.Char{{
					Name: "ForeignPG", Score: &model.Score{Version: model.ScoreVersion}, Inv: [64]model.Item{{Index: 4011}},
				}}}
				t.Cleanup(func() {
					_, err := conn.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=ANY($1)`,
						[]string{strings.ToLower(p.Account.Name), strings.ToLower(foreign.Name)})
					if err != nil {
						t.Errorf("fixture cleanup: %v", err)
					}
				})
				for _, account := range []*model.Account{p.Account, foreign} {
					if err := st.CreateAccount(account); err != nil {
						t.Fatal(err)
					}
				}
				w.indexPlayerCharacter(p)
				claimCriticalTestPlayers(t, w, p)
				w.store = st
				gate, release := criticalGate(t)
				reject := strings.HasSuffix(outcome, "rejection")
				unknown := strings.HasPrefix(outcome, "unknown-")
				backend := &criticalSharedPostgresStore{PostgresStore: st, entered: make(chan criticalWrite, 1), release: gate, unknown: unknown}
				if reject {
					backend.foreign = foreign.Chars[0].Inv[0]
				}
				queue := newCriticalTestQueue(t, backend, 1, 2)
				t.Cleanup(release)
				w.criticalWrites = newCriticalPersistenceOwner(queue)
				w.criticalAccountContainment = true
				before := accountStateSnapshot(p.Account)
				switch operation {
				case "pickup":
					w.onGetItem(p.Session, criticalPickupRequest(g.ID, 0))
				case "drop":
					w.onDropItem(p.Session, criticalDropRequest(placeInv, 0))
					g = pendingCriticalDrop(t, w)
				case "kill":
					w.killMobState(p, mob, 500, 37)
				}
				write := awaitCriticalWrite(t, backend.entered)
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("held transaction changed RAM or published before acknowledgement")
				}
				release()
				result := awaitCriticalResult(t, queue)
				if unknown && (result.err == nil || store.IsWriteRejected(result.err)) ||
					!unknown && (reject != store.IsWriteRejected(result.err) || !reject && result.err != nil) {
					t.Fatalf("unexpected PostgreSQL result: %v", result.err)
				}
				want := write.accounts[0]
				if reject {
					want = before
				}
				loaded, err := st.LoadAccount(p.Account.Name)
				if err != nil || loaded == nil {
					t.Fatalf("durable reload failed: %v", err)
				}
				gotChar, wantChar := loaded.Chars[p.CharSlot], want.Chars[p.CharSlot]
				if gotChar.Inv != wantChar.Inv || gotChar.Gold != wantChar.Gold || gotChar.Exp != wantChar.Exp || loaded.Cargo != want.Cargo {
					t.Fatal("durable transaction outcome differs from commit or certified rollback")
				}
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !w.criticalPlayerReserved(p) || p.Session.QueuedPacketsForTest() != 0 {
					t.Fatal("database outcome bypassed actor acknowledgement")
				}
				w.applyCriticalCompletion(result)
				if unknown {
					w.handle(command{s: p.Session})
					w.tick()
					if !w.shuttingDown || w.shutdownErr == nil || !queue.stopping || len(queue.pending) != 1 ||
						!w.criticalAccountReserved(p.Account.Name) || len(w.criticalQuarantine) != 0 || !p.PersistencePoisoned ||
						!reflect.DeepEqual(accountStateSnapshot(p.Account), before) || p.Session.QueuedPacketsForTest() != 0 {
						t.Fatal("lost acknowledgement released ownership, changed RAM or published success")
					}
					if g != nil && (w.groundItems[g.ID] != g || !g.criticalReserved()) || mob != nil && (w.mobsByID[mob.ID] != mob || !mob.Dead) {
						t.Fatal("uncertain shared object was discarded or rolled back")
					}
					return
				}
				if w.shuttingDown || p.PersistencePoisoned || w.criticalPlayerReserved(p) || len(queue.pending) != 0 ||
					!reflect.DeepEqual(accountStateSnapshot(p.Account), want) {
					t.Fatal("known database outcome did not settle the live account")
				}
				if operation == "pickup" && (reject && (w.groundItems[g.ID] != g || g.criticalReserved()) || !reject && w.groundItems[g.ID] != nil) ||
					operation == "drop" && (reject && w.groundItems[g.ID] != nil || !reject && (w.groundItems[g.ID] != g || g.criticalReserved())) ||
					operation == "kill" && (reject && (mob.Dead || mob.HP != 37) || !reject && w.mobsByID[mob.ID] == mob) {
					t.Fatal("known outcome did not settle its shared object")
				}
			})
		}
	}
}
