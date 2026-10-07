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

type criticalHeldTreasuryPostgresStore struct{ *criticalHeldPostgresStore }

func (s *criticalHeldTreasuryPostgresStore) SaveCriticalGameState(guilds *model.GuildRegistry,
	instances *model.InstanceStateSnapshot, uid string, state *model.CharState, accounts ...*model.Account) error {
	s.attempts++
	s.entered <- criticalWrite{accounts: accountStateSnapshotsForTest(accounts), guilds: guilds}
	<-s.release
	if s.attempts == 1 && s.foreign.Index != 0 {
		accounts[0].Chars[0].Inv[0] = s.foreign
	}
	return s.PostgresStore.SaveCriticalGameState(guilds, instances, uid, state, accounts...)
}

// Run with -p 1: the isolated guild singleton is shared with store fixtures.
func TestCriticalCityTreasuryPostgresCommitAndRejection(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	for _, mode := range []string{"gold", "partial-cheques", "cheques-ledger-rejection"} {
		t.Run(mode, func(t *testing.T) {
			ctx := context.Background()
			st, err := store.NewPostgresStore(ctx, store.PostgresConfig{URL: url, MaxConns: 2})
			if err != nil {
				t.Fatal(err)
			}
			t.Cleanup(st.Close)
			conn, err := pgx.Connect(ctx, url)
			if err != nil {
				t.Fatal(err)
			}
			t.Cleanup(func() { _ = conn.Close(ctx) })
			var previous []byte
			var version int64
			if err := conn.QueryRow(ctx, `SELECT payload,version FROM guild_state WHERE singleton=true`).Scan(&previous, &version); err != nil && err != pgx.ErrNoRows {
				t.Fatal(err)
			}
			t.Cleanup(func() {
				var err error
				if previous == nil {
					_, err = conn.Exec(ctx, `DELETE FROM guild_state WHERE singleton=true`)
				} else {
					_, err = conn.Exec(ctx, `UPDATE guild_state SET payload=$1,version=$2 WHERE singleton=true`, previous, version)
				}
				if err != nil {
					t.Errorf("restore isolated guild aggregate: %v", err)
				}
			})
			w, p, fallback := handlerTestWorld(t)
			w.clock = newFakeClock(time.Date(2026, 9, 13, 12, 0, 0, 0, time.UTC))
			suffix := time.Now().UTC().Format("150405.000000000")
			p.Account.Name, p.Char.Name = "treasurypg"+suffix, "TreasuryPG"
			delete(w.playersByCharacterUID, p.Char.UID)
			p.Char.UID = ""
			amount := uint64(100)
			if mode != "gold" {
				amount = 3_500_000_000
			}
			collector := configureCriticalTreasury(w, p, amount)
			if mode == "partial-cheques" {
				for slot := 0; slot < model.PlayerCarrySlots-2; slot++ {
					p.Char.Inv[slot] = model.Item{Index: 400}
				}
			}
			foreign := &model.Account{Name: "treasuryforeign" + suffix, PasswordHash: "hash", Chars: []model.Char{{
				Name: "ForeignTreasury", Score: &model.Score{Version: model.ScoreVersion}, Inv: [64]model.Item{{Index: 4011}},
			}}}
			t.Cleanup(func() {
				if _, err := conn.Exec(ctx, `DELETE FROM accounts WHERE name_key=ANY($1)`,
					[]string{strings.ToLower(p.Account.Name), strings.ToLower(foreign.Name)}); err != nil {
					t.Errorf("remove isolated treasury accounts: %v", err)
				}
			})
			for _, account := range []*model.Account{p.Account, foreign} {
				if err := st.CreateAccount(account); err != nil {
					t.Fatal(err)
				}
			}
			w.indexPlayerCharacter(p)
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("PostgreSQL treasury account admission failed")
			}
			retained, before := w.criticalGuildSnapshot(), accountStateSnapshot(p.Account)
			if err := st.SaveCriticalGameState(retained, nil, "", nil, before); err != nil {
				t.Fatal(err)
			}
			gate, release := criticalGate(t)
			backend := &criticalHeldTreasuryPostgresStore{&criticalHeldPostgresStore{
				PostgresStore: st, entered: make(chan criticalWrite, 2), release: gate,
			}}
			reject := mode == "cheques-ledger-rejection"
			if reject {
				backend.foreign = foreign.Chars[0].Inv[0]
			}
			q := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(q)
			assertDurable := func(account *model.Account, guilds *model.GuildRegistry) {
				t.Helper()
				loaded, err := st.LoadAccount(p.Account.Name)
				if err != nil || loaded == nil || loaded.Chars[0].Gold != account.Chars[0].Gold || loaded.Chars[0].Inv != account.Chars[0].Inv {
					t.Fatalf("durable treasury payout differs from the atomic proposal: %v", err)
				}
				registry, err := st.LoadGuilds()
				if err != nil || !reflect.DeepEqual(registry, guilds) {
					t.Fatalf("durable guild/treasury differs from the atomic proposal: %v", err)
				}
			}
			request := criticalTreasuryRequest(p, collector)
			w.handle(command{s: p.Session, pkt: request})
			proposal := awaitCriticalWrite(t, backend.entered)
			w.handle(command{s: p.Session, pkt: request})
			assertDurable(before, retained)
			release()
			result := awaitCriticalResult(t, q)
			if reject != store.IsWriteRejected(result.err) || (!reject && result.err != nil) {
				t.Fatalf("unexpected PostgreSQL treasury outcome: %v", result.err)
			}
			if reject {
				assertDurable(before, retained)
			} else {
				assertDurable(proposal.accounts[0], proposal.guilds)
			}
			if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
				!w.criticalGuildReserved() || !w.criticalPlayerReserved(p) || p.Session.QueuedPacketsForTest() != 0 || len(backend.entered) != 0 {
				t.Fatal("database completion bypassed actor acknowledgement or replayed the payout")
			}
			discardCriticalDeferredDuplicates(w)
			w.applyCriticalCompletion(result)
			if reject {
				assertCriticalPickupPacket(t, p, wire.MessagePanel("The city treasury collection could not be completed."))
				if !reflect.DeepEqual(accountStateSnapshot(p.Account), before) || !reflect.DeepEqual(w.criticalGuildSnapshot(), retained) ||
					len(q.pending) != 0 || len(backend.entered) != 0 {
					t.Fatal("certified rejection changed retained state or automatically replayed")
				}
				w.handle(command{s: p.Session, pkt: request})
				proposal = awaitCriticalWrite(t, backend.entered)
				w.applyCriticalCompletion(awaitCriticalResult(t, q))
				assertDurable(proposal.accounts[0], proposal.guilds)
			}
			if p.Char.Gold != proposal.accounts[0].Chars[0].Gold || p.Char.Inv != proposal.accounts[0].Chars[0].Inv ||
				!reflect.DeepEqual(w.criticalGuildSnapshot(), proposal.guilds) || w.criticalGuildReserved() || len(q.pending) != 0 ||
				w.shuttingDown || fallback.saves != 0 || p.Session.QueuedPacketsForTest() != 3 {
				t.Fatal("production treasury did not settle one atomic account/guild payout")
			}
		})
	}
}
