package store

import (
	"context"
	"os"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestPostgresBatchWritesAndPlayerDirectoryReads(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated test database")
	}
	ctx := context.Background()
	st, err := NewPostgresStore(ctx, PostgresConfig{URL: url, MaxConns: 2})
	if err != nil {
		t.Fatal("open PostgreSQL test store")
	}
	t.Cleanup(st.Close)

	uid, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	accountAName := "cov" + uid[:8]
	accountBName := "covb" + uid[4:11]
	characterAName := "Coverage" + uid[:6]
	characterBName := "Batch" + uid[8:14]
	accounts := []*model.Account{
		postgresTestAccount(accountAName, characterAName, model.Item{}),
		postgresTestAccount(accountBName, characterBName, model.Item{}),
	}
	t.Cleanup(func() {
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key = ANY($1)`,
			[]string{strings.ToLower(accountAName), strings.ToLower(accountBName)})
	})
	for _, account := range accounts {
		if err := st.CreateAccount(account); err != nil {
			t.Fatalf("create %q: %v", account.Name, err)
		}
	}
	accounts[0].Chars[0].Gold = 321
	accounts[1].Chars[0].Gold = 654
	if err := st.SaveAccounts(accounts...); err != nil {
		t.Fatalf("batch save: %v", err)
	}

	exists, err := st.CharacterNameExists(strings.ToLower(characterAName))
	if err != nil || !exists {
		t.Fatalf("character name exists=%t err=%v", exists, err)
	}
	names, err := st.CharacterNames()
	if err != nil {
		t.Fatal(err)
	}
	for _, name := range []string{strings.ToLower(characterAName), strings.ToLower(characterBName)} {
		if _, ok := names[name]; !ok {
			t.Errorf("character name %q absent from PostgreSQL index", name)
		}
	}

	uidA := accounts[0].Chars[0].UID
	player, err := st.ReadPersistentPlayer(ctx, strings.ToUpper(accountAName), uidA)
	if err != nil {
		t.Fatal(err)
	}
	if player.Account != accountAName || player.Name != characterAName || player.Gold != 321 ||
		player.UID != uidA || player.Level != accounts[0].Chars[0].Score.Level {
		t.Fatalf("persistent player projection=%+v", player)
	}
	state := &model.CharState{SpecialCoins: map[string]uint32{"coverage": 17}}
	if err := st.SaveCharStateAsync(uidA, state); err != nil {
		t.Fatalf("async charstate save: %v", err)
	}
	st.Flush()
	loadedState, err := st.LoadCharState(uidA)
	if err != nil || loadedState == nil || loadedState.SpecialCoins["coverage"] != 17 {
		t.Fatalf("async charstate=%+v err=%v", loadedState, err)
	}
	if err := st.SaveCharStateAsync(uidA, nil); err != nil {
		t.Fatalf("async charstate removal: %v", err)
	}
	st.Flush()
	if loadedState, err = st.LoadCharState(uidA); err != nil || loadedState != nil {
		t.Fatalf("removed charstate=%+v err=%v", loadedState, err)
	}
}
