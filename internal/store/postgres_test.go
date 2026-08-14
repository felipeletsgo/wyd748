package store

import (
	"context"
	"errors"
	"os"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"

	"wydgo/internal/model"
)

func postgresTestAccount(accountName, characterName string, item model.Item) *model.Account {
	return &model.Account{
		Name: accountName, PasswordHash: "hash",
		Chars: []model.Char{{
			Name: characterName,
			Extended: &model.ExtendedScore{
				Version: model.ExtendedScoreVersion,
			},
			Inv: [64]model.Item{item},
		}},
	}
}

func TestSnapshotAccountMaterializesUIDBeforeMarshal(t *testing.T) {
	acc := postgresTestAccount("felipe", "Felipe", model.Item{Index: 4011})
	snapshot, err := snapshotAccount(acc)
	if err != nil {
		t.Fatal(err)
	}
	if len(snapshot.items) != 1 || len(snapshot.items[0].uid) != model.ItemUIDHexLength {
		t.Fatalf("item nao materializado: %+v", snapshot.items)
	}
	if acc.Chars[0].Inv[0].UID != snapshot.items[0].uid {
		t.Fatal("UID nao retornou ao estado autoritativo em memoria")
	}
	if !strings.Contains(string(snapshot.payload), snapshot.items[0].uid) {
		t.Fatal("payload foi criado antes do UID")
	}
}

func TestSnapshotAccountsRejectsDuplicatedUIDInBatch(t *testing.T) {
	uid, err := model.NewItemUID()
	if err != nil {
		t.Fatal(err)
	}
	a := postgresTestAccount("a", "Alice", model.Item{Index: 4011, UID: uid})
	b := postgresTestAccount("b", "Bob", model.Item{Index: 4011, UID: uid})
	if _, err := snapshotAccounts(a, b); err == nil {
		t.Fatal("UID repetido entre contas foi aceito")
	}
}

func TestPostgresSchemaContainsAuthoritativeItemConstraints(t *testing.T) {
	for _, contract := range []string{
		"uid            uuid PRIMARY KEY",
		"UNIQUE (account_key, location)",
		"name_key      text PRIMARY KEY",
		"REFERENCES accounts(name_key) ON DELETE CASCADE",
		"character_uid uuid PRIMARY KEY",
		"REFERENCES characters(character_uid) ON DELETE CASCADE",
		"INSERT INTO schema_migrations(version) VALUES (2)",
		"INSERT INTO schema_migrations(version) VALUES (4)",
		"activated_unix bigint NOT NULL DEFAULT 0",
		"expires_unix   bigint NOT NULL DEFAULT 0",
		"item_instances_timed_window_check",
	} {
		if !strings.Contains(postgresSchema, contract) {
			t.Fatalf("schema perdeu contrato %q", contract)
		}
	}
}

func TestPostgresAsyncQueueCoalescesWithoutBlocking(t *testing.T) {
	st := &PostgresStore{
		writeQueue:    make(chan postgresWriteJob, 1),
		overflowAcc:   make(map[string]postgresWriteJob),
		overflowState: make(map[string]postgresWriteJob),
		overflowRuns:  make(map[string]postgresWriteJob),
	}
	st.writeQueue <- postgresWriteJob{account: &accountSnapshot{key: "ocupada"}}
	for i := 0; i < 100; i++ {
		if err := st.enqueue(postgresWriteJob{
			account: &accountSnapshot{key: "felipe", display: "felipe"},
		}); err != nil {
			t.Fatal(err)
		}
	}
	queued, pending, coalesced := st.PostgresAsyncStats()
	if queued != 1 || pending != 1 || coalesced != 99 {
		t.Fatalf("fila nao coalesceu: queued=%d pending=%d coalesced=%d",
			queued, pending, coalesced)
	}
	job, ok := st.takeOverflow()
	if !ok || job.account == nil || job.account.key != "felipe" {
		t.Fatalf("snapshot coalescido incorreto: %+v", job)
	}
}

func TestPostgresBarrierDrainsOnlyOlderOverflowFirst(t *testing.T) {
	st := &PostgresStore{
		writeQueue:    make(chan postgresWriteJob, 1),
		overflowAcc:   make(map[string]postgresWriteJob),
		overflowState: make(map[string]postgresWriteJob),
		overflowRuns:  make(map[string]postgresWriteJob),
	}
	// Ocupa a fila para obrigar o snapshot anterior a cair no overflow.
	if err := st.enqueue(postgresWriteJob{run: func() error { return nil }, label: "ocupado"}); err != nil {
		t.Fatal(err)
	}
	if err := st.enqueue(postgresWriteJob{
		account: &accountSnapshot{key: "felipe", display: "Felipe"},
	}); err != nil {
		t.Fatal(err)
	}
	barrier := st.stampWriteJob(postgresWriteJob{done: make(chan struct{})})
	older, ok := st.takeOverflowBefore(barrier.seq)
	if !ok || older.account == nil || older.account.key != "felipe" {
		t.Fatalf("barreira ultrapassaria overflow anterior: %+v ok=%t", older, ok)
	}
	if _, ok := st.takeOverflowBefore(barrier.seq); ok {
		t.Fatal("overflow anterior deveria ter sido drenado uma unica vez")
	}
}

func TestPostgresOlderOverflowCannotOverwriteNewerQueuedSnapshot(t *testing.T) {
	st := &PostgresStore{
		writeQueue:    make(chan postgresWriteJob, 1),
		overflowAcc:   make(map[string]postgresWriteJob),
		overflowState: make(map[string]postgresWriteJob),
		overflowRuns:  make(map[string]postgresWriteJob),
	}
	if err := st.enqueue(postgresWriteJob{run: func() error { return nil }, label: "ocupado"}); err != nil {
		t.Fatal(err)
	}
	if err := st.enqueue(postgresWriteJob{
		account: &accountSnapshot{key: "felipe", display: "antigo", payload: []byte("A")},
	}); err != nil {
		t.Fatal(err)
	}
	<-st.writeQueue // abre espaco para o snapshot novo entrar na fila FIFO.
	if err := st.enqueue(postgresWriteJob{
		account: &accountSnapshot{key: "felipe", display: "novo", payload: []byte("B")},
	}); err != nil {
		t.Fatal(err)
	}
	old, ok := st.takeOverflow()
	if !ok || old.account == nil {
		t.Fatal("snapshot antigo nao entrou no overflow")
	}
	newer := <-st.writeQueue
	if st.isLatestAccountJob(old) {
		t.Fatal("snapshot antigo continuou elegivel depois de um enqueue mais novo")
	}
	if !st.isLatestAccountJob(newer) {
		t.Fatal("snapshot mais novo nao foi reconhecido como autoritativo")
	}
}

// Este teste e opt-in porque o repositorio nao sobe um banco oculto durante
// `go test`. No CI/VPS, WYD_TEST_POSTGRES_URL habilita a prova com PostgreSQL
// real, incluindo rollback da tentativa de duplicar um UID global.
func TestPostgresStoreIntegration(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL nao configurada")
	}
	st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 2})
	if err != nil {
		t.Fatal(err)
	}
	defer st.Close()

	suffix := time.Now().UTC().Format("150405.000000000")
	accountA, accountB := "pgtesta"+suffix, "pgtestb"+suffix
	charA, charB := "PgA"+suffix, "PgB"+suffix
	defer func() {
		_, _ = st.pool.Exec(context.Background(),
			`DELETE FROM accounts WHERE name_key = ANY($1)`,
			[]string{strings.ToLower(accountA), strings.ToLower(accountB)})
	}()

	a := postgresTestAccount(accountA, charA, model.Item{
		Index: 4011, UID: "55555555555545558555555555555555",
		ActivatedUnix: 1_700_000_000, ExpiresUnix: 1_702_592_000,
	})
	if err := st.CreateAccount(a); err != nil {
		t.Fatal(err)
	}
	loaded, err := st.LoadAccount(strings.ToUpper(accountA))
	if err != nil {
		t.Fatal(err)
	}
	if loaded.Chars[0].Inv[0].UID == "" {
		t.Fatal("UID nao persistiu")
	}
	if loaded.Chars[0].Inv[0].ActivatedUnix != 1_700_000_000 ||
		loaded.Chars[0].Inv[0].ExpiresUnix != 1_702_592_000 {
		t.Fatalf("UID perdeu prazo no payload: %+v", loaded.Chars[0].Inv[0])
	}
	var activatedUnix, expiresUnix int64
	if err := st.pool.QueryRow(context.Background(), `
		SELECT activated_unix,expires_unix FROM item_instances
		WHERE uid=$1`, currentUID(loaded.Chars[0].Inv[0].UID)).Scan(&activatedUnix, &expiresUnix); err != nil {
		t.Fatal(err)
	}
	if activatedUnix != 1_700_000_000 || expiresUnix != 1_702_592_000 {
		t.Fatalf("ledger perdeu prazo: activated=%d expires=%d", activatedUnix, expiresUnix)
	}

	b := postgresTestAccount(accountB, charB, model.Item{
		Index: 4011, UID: loaded.Chars[0].Inv[0].UID,
	})
	if err := st.CreateAccount(b); err == nil {
		t.Fatal("PostgreSQL aceitou UID global duplicado")
	}
	if exists, err := st.AccountNameExists(accountB); err != nil || exists {
		t.Fatalf("transacao parcial apos UID duplicado: exists=%t err=%v", exists, err)
	}
}

func TestPostgresCharacterUIDStateCascadeAndIncrementalItems(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL nao configurada")
	}
	st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 2})
	if err != nil {
		t.Fatal(err)
	}
	defer st.Close()

	suffix := time.Now().UTC().Format("150405.000000000")
	accountName := "pgchar" + suffix
	otherName := "pgother" + suffix
	defer func() {
		_, _ = st.pool.Exec(context.Background(),
			`DELETE FROM accounts WHERE name_key=ANY($1)`,
			[]string{strings.ToLower(accountName), strings.ToLower(otherName)})
	}()
	acc := postgresTestAccount(accountName, "Twin", model.Item{Index: 4011})
	acc.Chars = append(acc.Chars, model.Char{
		Name: "Twin",
		Extended: &model.ExtendedScore{
			Version: model.ExtendedScoreVersion,
		},
	})
	if err := st.CreateAccount(acc); err != nil {
		t.Fatal(err)
	}
	firstUID, secondUID := acc.Chars[0].UID, acc.Chars[1].UID
	if firstUID == "" || secondUID == "" || firstUID == secondUID {
		t.Fatalf("identidades invalidas: %q %q", firstUID, secondUID)
	}
	if err := st.SaveCharState(firstUID, &model.CharState{
		Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 10},
	}); err != nil {
		t.Fatal(err)
	}
	if err := st.SaveCharState(secondUID, &model.CharState{
		Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 20},
	}); err != nil {
		t.Fatal(err)
	}
	first, err := st.LoadCharState(firstUID)
	if err != nil || first.SpecialCoins["fame"] != 10 {
		t.Fatalf("charstate 1 cruzou identidade: state=%v err=%v", first, err)
	}
	second, err := st.LoadCharState(secondUID)
	if err != nil || second.SpecialCoins["fame"] != 20 {
		t.Fatalf("charstate 2 cruzou identidade: state=%v err=%v", second, err)
	}

	var xminBefore string
	if err := st.pool.QueryRow(context.Background(), `
		SELECT xmin::text FROM item_instances
		WHERE account_key=$1 AND uid=$2`,
		strings.ToLower(accountName), currentUID(acc.Chars[0].Inv[0].UID)).
		Scan(&xminBefore); err != nil {
		t.Fatal(err)
	}
	acc.Chars[0].Exp++
	if err := st.SaveAccount(acc); err != nil {
		t.Fatal(err)
	}
	var xminAfter string
	if err := st.pool.QueryRow(context.Background(), `
		SELECT xmin::text FROM item_instances
		WHERE account_key=$1 AND uid=$2`,
		strings.ToLower(accountName), currentUID(acc.Chars[0].Inv[0].UID)).
		Scan(&xminAfter); err != nil {
		t.Fatal(err)
	}
	if xminAfter != xminBefore {
		t.Fatalf("item inalterado foi regravado: xmin %s -> %s", xminBefore, xminAfter)
	}

	acc.Chars[0].Exp = 77
	if err := st.SavePlayerState(nil, acc, firstUID, &model.CharState{
		Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 30},
	}); err != nil {
		t.Fatal(err)
	}
	loadedAccount, err := st.LoadAccount(accountName)
	if err != nil || loadedAccount.Chars[0].Exp != 77 {
		t.Fatalf("conta nao entrou no commit atomico: acc=%v err=%v", loadedAccount, err)
	}
	stateAfterAtomic, err := st.LoadCharState(firstUID)
	if err != nil || stateAfterAtomic.SpecialCoins["fame"] != 30 {
		t.Fatalf("charstate nao entrou no commit atomico: state=%v err=%v",
			stateAfterAtomic, err)
	}

	other := postgresTestAccount(otherName, "Other", model.Item{Index: 4011})
	if err := st.CreateAccount(other); err != nil {
		t.Fatal(err)
	}
	acc.Chars[0].Exp = 99
	acc.Chars[0].Inv[0].UID = other.Chars[0].Inv[0].UID
	if err := st.SavePlayerState(nil, acc, firstUID, &model.CharState{
		Version: model.CharStateVersion, SpecialCoins: map[string]uint32{"fame": 99},
	}); err == nil {
		t.Fatal("conflito de UID deveria abortar o commit inteiro")
	}
	loadedAccount, err = st.LoadAccount(accountName)
	if err != nil {
		t.Fatal(err)
	}
	if loadedAccount.Chars[0].Exp != 77 {
		t.Fatalf("rollback nao restaurou conta persistida: exp=%d err=%v",
			loadedAccount.Chars[0].Exp, err)
	}
	stateAfterAtomic, err = st.LoadCharState(firstUID)
	if err != nil || stateAfterAtomic.SpecialCoins["fame"] != 30 {
		t.Fatalf("rollback nao restaurou charstate: state=%v err=%v",
			stateAfterAtomic, err)
	}
	// Restaura o snapshot autoritativo em memoria antes de testar a exclusao.
	acc.Chars[0].Inv[0].UID = loadedAccount.Chars[0].Inv[0].UID
	acc.Chars[0].Exp = loadedAccount.Chars[0].Exp

	acc.Chars[0] = model.Char{}
	if err := st.SaveAccount(acc); err != nil {
		t.Fatal(err)
	}
	if state, err := st.LoadCharState(firstUID); err != nil || state != nil {
		t.Fatalf("charstate excluido sobreviveu ao cascade: state=%v err=%v", state, err)
	}
	if state, err := st.LoadCharState(secondUID); err != nil || state == nil ||
		state.SpecialCoins["fame"] != 20 {
		t.Fatalf("cascade apagou o gemeo errado: state=%v err=%v", state, err)
	}
}

func TestPostgresGuildExportFailureDoesNotUndoCommittedState(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL nao configurada")
	}
	blocker := t.TempDir() + `\arquivo`
	if err := os.WriteFile(blocker, []byte("nao e diretorio"), 0o600); err != nil {
		t.Fatal(err)
	}
	st, err := NewPostgresStore(context.Background(), PostgresConfig{
		URL: url, MaxConns: 2, GuildsTxtPath: blocker + `\Guilds.txt`,
	})
	if err != nil {
		t.Fatal(err)
	}
	var previousPayload []byte
	var previousVersion int64
	var previousUpdatedAt time.Time
	hadPrevious := true
	err = st.pool.QueryRow(context.Background(), `
		SELECT payload,version,updated_at FROM guild_state WHERE singleton=true`).
		Scan(&previousPayload, &previousVersion, &previousUpdatedAt)
	if errors.Is(err, pgx.ErrNoRows) {
		hadPrevious = false
	} else if err != nil {
		st.Close()
		t.Fatal(err)
	}
	t.Cleanup(func() {
		if hadPrevious {
			_, _ = st.pool.Exec(context.Background(), `
				INSERT INTO guild_state(singleton,payload,version,updated_at)
				VALUES(true,$1,$2,$3)
				ON CONFLICT(singleton) DO UPDATE SET
					payload=EXCLUDED.payload,
					version=EXCLUDED.version,
					updated_at=EXCLUDED.updated_at`,
				previousPayload, previousVersion, previousUpdatedAt)
		} else {
			_, _ = st.pool.Exec(context.Background(), `DELETE FROM guild_state`)
		}
		st.Close()
	})
	registry := sampleGuild()
	if err := st.SaveGameState(registry); err != nil {
		t.Fatalf("artefato derivado nao pode falsear o commit: %v", err)
	}
	st.Flush()
	loaded, err := st.LoadGuilds()
	if err != nil {
		t.Fatal(err)
	}
	if len(loaded.Guilds) != len(registry.Guilds) {
		t.Fatalf("guild autoritativa nao foi confirmada: got=%d want=%d",
			len(loaded.Guilds), len(registry.Guilds))
	}
}
