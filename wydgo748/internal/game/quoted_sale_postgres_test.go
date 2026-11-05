package game

import (
	"context"
	"os"
	"strconv"
	"testing"
	"time"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

func TestQuotedSalePostgresCommitAndRejection(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
	for _, rejected := range []bool{false, true} {
		t.Run(map[bool]string{false: "commit", true: "ledger-rejection"}[rejected], func(t *testing.T) {
			st, err := store.NewPostgresStore(context.Background(), store.PostgresConfig{URL: url, MaxConns: 2})
			if err != nil {
				t.Fatal(err)
			}
			t.Cleanup(st.Close)
			w, p, fallback := saleIngressWorld(t)
			shop := w.mobsByID[p.ShopNPC]
			configureCriticalShop(w, p, shop, "sale")
			p.Account.Name = "quotepg" + time.Now().UTC().Format("150405.000000000")
			nameSuffix := strconv.FormatInt(time.Now().UnixNano(), 36)
			p.Char.Name = "Q" + nameSuffix
			delete(w.playersByCharacterUID, p.Char.UID)
			p.Char.UID = ""
			p.Char.Inv[model.PlayerCarrySlots] = model.Item{}
			for i := range p.Char.Inv {
				p.Char.Inv[i].UID = ""
			}
			if err := st.CreateAccount(p.Account); err != nil {
				t.Fatal(err)
			}
			foreign := &model.Account{Name: "foreign" + p.Account.Name, PasswordHash: "hash", Chars: []model.Char{{
				Name: "F" + nameSuffix, Score: &model.Score{Version: model.ScoreVersion}, Inv: [64]model.Item{{Index: 4011}},
			}}}
			if err := st.CreateAccount(foreign); err != nil {
				t.Fatal(err)
			}
			w.indexPlayerCharacter(p)
			if !w.claimAccountSession(p.Session, p.Account.Name) {
				t.Fatal("quote account admission failed")
			}
			gate, release := criticalGate(t)
			backend := &criticalHeldPostgresStore{PostgresStore: st, entered: make(chan criticalWrite, 2), release: gate}
			if rejected {
				backend.foreign = foreign.Chars[0].Inv[0]
			}
			queue := newCriticalTestQueue(t, backend, 1, 2)
			t.Cleanup(release)
			w.criticalWrites = newCriticalPersistenceOwner(queue)
			before := cloneCharacterState(p.Char)
			packet := quotedSalePacket(p, 5, 250, p.Char.Inv[5])
			w.handle(command{s: p.Session, pkt: packet})
			awaitCriticalWrite(t, backend.entered)
			w.onQuotedSellItem(p.Session, packet)
			if p.Char.Inv != before.Inv || p.Char.Gold != before.Gold || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("held quote changed or published state")
			}
			release()
			result := awaitCriticalResult(t, queue)
			if rejected != store.IsWriteRejected(result.err) || (!rejected && result.err != nil) {
				t.Fatalf("unexpected PostgreSQL quote outcome: %v", result.err)
			}
			wantInv, wantGold := before.Inv, before.Gold
			if !rejected {
				wantInv[5], wantGold = model.Item{}, before.Gold+250
			}
			loaded, err := st.LoadAccount(p.Account.Name)
			if err != nil || loaded == nil || loaded.Chars[0].Inv != wantInv || loaded.Chars[0].Gold != wantGold {
				t.Fatalf("durable quote item or balance differs: %v", err)
			}
			if p.Char.Inv != before.Inv || p.Char.Gold != before.Gold || p.Session.QueuedPacketsForTest() != 0 {
				t.Fatal("database completion bypassed actor acknowledgement")
			}
			w.applyCriticalCompletion(result)
			if p.Char.Inv != wantInv || p.Char.Gold != wantGold || w.criticalPlayerReserved(p) || fallback.saves != 0 {
				t.Fatal("quote acknowledgement did not settle its exact durable result")
			}
			if !rejected {
				assertCriticalShopPublication(t, p, "sale", shop.ID)
				w.onQuotedSellItem(p.Session, packet)
				if len(queue.pending) != 0 || len(backend.entered) != 0 || p.Char.Gold != wantGold {
					t.Fatal("durable quote replay credited twice")
				}
			}
		})
	}
}
