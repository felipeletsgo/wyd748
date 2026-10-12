package game

import (
	"context"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"wydgo/internal/model"
	gameNet "wydgo/internal/net"
	"wydgo/internal/store"
)

func TestCriticalCommittedDisconnectPostgresRecoversWithFreshLogin(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL is not configured for an isolated database")
	}
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
	world, held, _ := handlerTestWorld(t)
	name := "recoverpg" + time.Now().UTC().Format("150405.000000000")
	world.unindexPlayerCharacter(held)
	held.Account.Name, held.Char.Name = name, name+"-hero"
	for slot := 1; slot < 4; slot++ {
		extra := accountStateSnapshot(held.Account).Chars[0]
		extra.Name += string(rune('a' + slot))
		held.Account.Chars = append(held.Account.Chars, extra)
	}
	for slot := range held.Account.Chars {
		held.Account.Chars[slot].UID, err = model.NewCharacterUID()
		if err != nil {
			t.Fatal(err)
		}
	}
	held.Char = &held.Account.Chars[0]
	t.Cleanup(func() {
		if _, err := conn.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(name)); err != nil {
			t.Errorf("fixture cleanup: %v", err)
		}
	})
	if err := st.CreateAccount(held.Account); err != nil {
		t.Fatal(err)
	}
	held.Account, err = st.LoadAccount(name)
	if err != nil {
		t.Fatal(err)
	}
	held.Char = &held.Account.Chars[0]
	world.indexPlayerCharacter(held)
	claimCriticalTestPlayers(t, world, held)
	world.store = st
	queue := newCriticalTestQueue(t, st, 1, 2)
	world.criticalWrites = newCriticalPersistenceOwner(queue)
	world.criticalAccountContainment = true
	authSnapshot := accountStateSnapshot(held.Account)
	held.Char.Gold += 12345
	held.SpecialCoins = map[string]uint32{"water": 7}
	gold, uid := held.Char.Gold, held.Char.UID
	world.onDisconnect(held.Session)
	// Reproduce the final-save producer's exact identity failure, after the
	// write is admitted, without mocking the transaction or login reload.
	world.criticalDisconnects[held.Session] = &criticalDisconnect{player: held}
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if world.players[held.Session] != nil || world.shuttingDown || len(queue.pending) != 0 ||
		len(world.criticalQuarantine) != 0 || !held.PersistencePoisoned {
		t.Fatal("confirmed final PostgreSQL save did not discard the failed session")
	}
	loaded, err := st.LoadAccount(name)
	if err != nil || loaded == nil || len(loaded.Chars) != 4 || loaded.Chars[0].Gold != gold {
		t.Fatalf("final account state was lost or overwritten during recovery: %v", err)
	}
	state, err := st.LoadCharState(uid)
	if err != nil || state == nil || state.SpecialCoins["water"] != 7 {
		t.Fatalf("atomic final-save sidecar was not durable: %v", err)
	}
	fresh := newLoginTestSession(99, 128)
	world.authPending = make(map[*gameNet.Session]bool)
	world.authPending[fresh] = true
	world.onLoginResult(fresh, &loginResult{accountName: name, account: authSnapshot})
	player := world.players[fresh]
	if player == nil || player.Account == held.Account || player.Account.Chars[0].Gold != gold ||
		len(player.Account.Chars) != 4 || player.PersistencePoisoned || world.accountSessions[strings.ToLower(name)] != fresh {
		t.Fatal("relogin reused poisoned RAM or the stale authentication snapshot")
	}
	world.onDisconnect(held.Session)
	world.onEnterWorld(fresh, enterWorldPacket(0))
	if !player.InWorld || player.Char == nil || player.Char.UID != uid || player.Char.Gold != gold || player.SpecialCoins["water"] != 7 {
		t.Fatal("recovered account could not enter the world with committed account/sidecar state")
	}
	world.onDisconnect(fresh)
	awaitReadyCriticalResults(t, queue, 1)
	world.processCriticalCompletions(1)
	if world.players[fresh] != nil || len(queue.pending) != 0 || world.shuttingDown {
		t.Fatal("recovered account became stuck on its next normal disconnect")
	}
}
