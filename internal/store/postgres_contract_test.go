package store

import (
	"context"
	"os"
	"strings"
	"testing"
	"time"

	"wydgo/internal/model"
)

func openContractPostgres(t *testing.T) *PostgresStore {
	t.Helper()
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL nao configurada")
	}
	st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 2})
	if err != nil {
		t.Fatal(err)
	}
	t.Cleanup(st.Close)
	return st
}

func uniquePostgresContractName(prefix string) string {
	return prefix + time.Now().UTC().Format("150405000000000")
}

func TestPostgresMultiAccountSaveAndNameIndexes(t *testing.T) {
	st := openContractPostgres(t)
	if err := st.Ping(context.Background()); err != nil {
		t.Fatalf("readiness PostgreSQL falhou: %v", err)
	}

	aName := uniquePostgresContractName("multia")
	bName := uniquePostgresContractName("multib")
	aChar := "Alice" + aName[len(aName)-6:]
	bChar := "Bob" + bName[len(bName)-6:]
	t.Cleanup(func() {
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=ANY($1)`,
			[]string{strings.ToLower(aName), strings.ToLower(bName)})
	})

	a := postgresTestAccount(aName, aChar, model.Item{Index: 4011})
	b := postgresTestAccount(bName, bChar, model.Item{Index: 4012})
	if err := st.CreateAccount(a); err != nil {
		t.Fatal(err)
	}
	if err := st.CreateAccount(b); err != nil {
		t.Fatal(err)
	}

	a.Chars[0].Gold = 111
	b.Chars[0].Gold = 222
	if err := st.SaveAccounts(a, b); err != nil {
		t.Fatalf("SaveAccounts atomico falhou: %v", err)
	}
	loadedA, err := st.LoadAccount(aName)
	if err != nil {
		t.Fatal(err)
	}
	loadedB, err := st.LoadAccount(bName)
	if err != nil {
		t.Fatal(err)
	}
	if loadedA.Chars[0].Gold != 111 || loadedB.Chars[0].Gold != 222 {
		t.Fatalf("save multi-conta incompleto: A=%d B=%d", loadedA.Chars[0].Gold, loadedB.Chars[0].Gold)
	}

	if exists, err := st.CharacterNameExists(strings.ToUpper(aChar)); err != nil || !exists {
		t.Fatalf("indice de nickname case-insensitive: exists=%v err=%v", exists, err)
	}
	names, err := st.CharacterNames()
	if err != nil {
		t.Fatal(err)
	}
	if _, ok := names[strings.ToLower(aChar)]; !ok {
		t.Fatalf("CharacterNames nao retornou %q", aChar)
	}

	// Autosave assincrono deve se tornar observavel depois da barreira Flush.
	a.Chars[0].Gold = 333
	if err := st.SaveAccountAsync(a); err != nil {
		t.Fatal(err)
	}
	st.Flush()
	loadedA, err = st.LoadAccount(aName)
	if err != nil {
		t.Fatal(err)
	}
	if loadedA.Chars[0].Gold != 333 {
		t.Fatalf("autosave+Flush nao ficou duravel: gold=%d", loadedA.Chars[0].Gold)
	}
}

func TestPostgresInstanceStateRoundTripAndAtomicGameState(t *testing.T) {
	st := openContractPostgres(t)
	accountName := uniquePostgresContractName("inst")
	charName := "Inst" + accountName[len(accountName)-6:]
	t.Cleanup(func() {
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(accountName))
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM instance_state WHERE singleton=true`)
	})

	acc := postgresTestAccount(accountName, charName, model.Item{Index: 4011})
	if err := st.CreateAccount(acc); err != nil {
		t.Fatal(err)
	}
	state := &model.InstanceStateSnapshot{
		Version: model.InstanceStateVersion,
		NightmarePartyRuns: map[string]int{"arcane:contract": 2},
		Instances: []model.InstanceRuntimeState{{
			RuntimeID: "contract-run", ConfigID: "nightmare-arcane", CurrentStage: 3,
		}},
	}
	if err := st.SaveInstanceState(state); err != nil {
		t.Fatal(err)
	}
	loadedState, err := st.LoadInstanceState()
	if err != nil {
		t.Fatal(err)
	}
	if loadedState.NightmarePartyRuns["arcane:contract"] != 2 || len(loadedState.Instances) != 1 ||
		loadedState.Instances[0].CurrentStage != 3 {
		t.Fatalf("round-trip de instance_state divergiu: %+v", loadedState)
	}

	acc.Chars[0].Gold = 777
	state.NightmarePartyRuns["arcane:contract"] = 3
	if err := st.SaveGameStateWithInstanceState(nil, state, acc); err != nil {
		t.Fatalf("commit conta+instancia falhou: %v", err)
	}
	loadedAcc, err := st.LoadAccount(accountName)
	if err != nil {
		t.Fatal(err)
	}
	loadedState, err = st.LoadInstanceState()
	if err != nil {
		t.Fatal(err)
	}
	if loadedAcc.Chars[0].Gold != 777 || loadedState.NightmarePartyRuns["arcane:contract"] != 3 {
		t.Fatalf("commit atomico nao materializou os dois agregados: gold=%d state=%+v",
			loadedAcc.Chars[0].Gold, loadedState.NightmarePartyRuns)
	}

	// Versao invalida deve falhar antes da transacao e nao persistir a mudanca
	// da conta que veio no mesmo pedido.
	acc.Chars[0].Gold = 999
	bad := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion + 1}
	if err := st.SaveGameStateWithInstanceState(nil, bad, acc); err == nil {
		t.Fatal("estado de instancia com versao invalida foi aceito")
	}
	loadedAcc, err = st.LoadAccount(accountName)
	if err != nil {
		t.Fatal(err)
	}
	if loadedAcc.Chars[0].Gold != 777 {
		t.Fatalf("falha de validacao vazou mutacao de conta: gold=%d", loadedAcc.Chars[0].Gold)
	}
}

func TestPostgresReadOnlyStoreRejectsEveryWriteEntryPoint(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL nao configurada")
	}
	st, err := NewPostgresReadOnlyStore(context.Background(), PostgresConfig{URL: url, MaxConns: 1})
	if err != nil {
		t.Fatal(err)
	}
	defer st.Close()
	if err := st.Ping(context.Background()); err != nil {
		t.Fatal(err)
	}

	acc := postgresTestAccount(uniquePostgresContractName("readonly"), "ReadOnly", model.Item{})
	state := &model.InstanceStateSnapshot{Version: model.InstanceStateVersion}
	writes := []struct {
		name string
		fn   func() error
	}{
		{"SaveAccount", func() error { return st.SaveAccount(acc) }},
		{"SaveAccountAsync", func() error { return st.SaveAccountAsync(acc) }},
		{"SaveAccounts", func() error { return st.SaveAccounts(acc) }},
		{"CreateAccount", func() error { return st.CreateAccount(acc) }},
		{"SaveInstanceState", func() error { return st.SaveInstanceState(state) }},
		{"SaveGameStateWithInstanceState", func() error { return st.SaveGameStateWithInstanceState(nil, state, acc) }},
	}
	for _, tc := range writes {
		if err := tc.fn(); err == nil || !strings.Contains(err.Error(), "somente leitura") {
			t.Fatalf("%s nao foi bloqueado pelo modo read-only: %v", tc.name, err)
		}
	}
}

func TestPostgresLoadInstanceStateRejectsCorruptPayload(t *testing.T) {
	st := openContractPostgres(t)
	t.Cleanup(func() { _, _ = st.pool.Exec(context.Background(), `DELETE FROM instance_state WHERE singleton=true`) })
	_, err := st.pool.Exec(context.Background(), `
		INSERT INTO instance_state(singleton,payload) VALUES(true,$1)
		ON CONFLICT(singleton) DO UPDATE SET payload=EXCLUDED.payload`, []byte(`{"version":1,"unexpected":true}`))
	if err != nil {
		t.Fatal(err)
	}
	if _, err := st.LoadInstanceState(); err == nil {
		t.Fatal("instance_state com campo desconhecido foi aceito")
	}

	_, err = st.pool.Exec(context.Background(), `UPDATE instance_state SET payload=$1 WHERE singleton=true`, []byte(`{"version":999}`))
	if err != nil {
		t.Fatal(err)
	}
	if _, err := st.LoadInstanceState(); err == nil {
		t.Fatal("instance_state com versao futura foi aceito")
	}
}

func TestPostgresPingRejectsClosedOrNilStore(t *testing.T) {
	var nilStore *PostgresStore
	if err := nilStore.Ping(context.Background()); err == nil {
		t.Fatal("Ping em store nil deveria falhar")
	}
	st := &PostgresStore{}
	if err := st.Ping(context.Background()); err == nil {
		t.Fatal("Ping em store sem pool deveria falhar")
	}
}

func TestPostgresResetLoadtestAccountsDeletesOnlyExactBotPrefixPattern(t *testing.T) {
	st := openContractPostgres(t)
	accounts := []*model.Account{
		postgresTestAccount("botx0001", "LoadCovA", model.Item{}),
		postgresTestAccount("botx0002", "LoadCovB", model.Item{}),
		postgresTestAccount("boty0001", "KeepCov", model.Item{}),
	}
	keys := []string{"botx0001", "botx0002", "boty0001"}
	_, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=ANY($1)`, keys)
	t.Cleanup(func() { _, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=ANY($1)`, keys) })
	for _, acc := range accounts {
		if err := st.CreateAccount(acc); err != nil {
			t.Fatal(err)
		}
	}
	deleted, err := st.ResetLoadtestAccounts(nil, " BOTX ")
	if err != nil || deleted != 2 {
		t.Fatalf("ResetLoadtestAccounts deleted=%d err=%v", deleted, err)
	}
	if exists, err := st.AccountNameExists("botx0001"); err != nil || exists {
		t.Fatalf("botx0001 ainda existe: exists=%v err=%v", exists, err)
	}
	if exists, err := st.AccountNameExists("boty0001"); err != nil || !exists {
		t.Fatalf("conta fora do prefixo foi removida: exists=%v err=%v", exists, err)
	}
	for _, bad := range []string{"", "bo", "bot1", "player", "bot-too-long"} {
		if _, err := st.ResetLoadtestAccounts(context.Background(), bad); err == nil {
			t.Fatalf("prefixo inseguro %q foi aceito", bad)
		}
	}
}

func TestPostgresAsyncCharStateRoundTripAndDelete(t *testing.T) {
	st := openContractPostgres(t)
	accountName := uniquePostgresContractName("charstate")
	acc := postgresTestAccount(accountName, "StateHero", model.Item{})
	t.Cleanup(func() {
		_, _ = st.pool.Exec(context.Background(), `DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(accountName))
	})
	if err := st.CreateAccount(acc); err != nil {
		t.Fatal(err)
	}
	uid := acc.Chars[0].UID
	state := &model.CharState{Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 77}}
	if err := st.SaveCharStateAsync(uid, state); err != nil {
		t.Fatal(err)
	}
	st.Flush()
	loaded, err := st.LoadCharState(uid)
	if err != nil {
		t.Fatal(err)
	}
	if loaded == nil || loaded.SpecialCoins["fame"] != 77 {
		t.Fatalf("charstate async nao persistiu: %+v", loaded)
	}
	if err := st.SaveCharStateAsync(uid, nil); err != nil {
		t.Fatal(err)
	}
	st.Flush()
	loaded, err = st.LoadCharState(uid)
	if err != nil || loaded != nil {
		t.Fatalf("delete async do charstate falhou: state=%+v err=%v", loaded, err)
	}
}
