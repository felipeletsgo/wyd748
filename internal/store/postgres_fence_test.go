package store

import (
	"context"
	"os"
	"strconv"
	"strings"
	"sync"
	"testing"
	"time"

	"wydgo/internal/model"
)

func TestPostgresFenceSetReclaimsKeys(t *testing.T) {
	var fences postgresFenceSet
	unlock := fences.lock("bravo", "alpha", "alpha", "")
	fences.mu.Lock()
	if len(fences.entries) != 2 {
		fences.mu.Unlock()
		t.Fatalf("fences materializados=%d, esperado=2", len(fences.entries))
	}
	fences.mu.Unlock()
	unlock()
	fences.mu.Lock()
	remaining := len(fences.entries)
	fences.mu.Unlock()
	if remaining != 0 {
		t.Fatalf("fences vazaram depois do unlock: %d", remaining)
	}
}

func TestPostgresCriticalFenceInvalidatesDelayedAsyncJobs(t *testing.T) {
	st := &PostgresStore{}
	account := &accountSnapshot{key: "felipe", display: "Felipe"}
	oldAccountSeq := st.writeSeq.Add(1)
	criticalSeq := st.writeSeq.Add(1)
	st.fenceAccountSnapshots(criticalSeq, []*accountSnapshot{account})
	oldAccount := st.stampWriteJob(postgresWriteJob{seq: oldAccountSeq, account: account})
	if st.isLatestAccountJob(oldAccount) {
		t.Fatal("autosave de conta iniciado antes do fence recuperou autoridade ao enfileirar tarde")
	}

	uid := "11111111111141118111111111111111"
	oldStateSeq := st.writeSeq.Add(1)
	criticalStateSeq := st.writeSeq.Add(1)
	st.fenceCharStates(criticalStateSeq, uid)
	oldState := st.stampWriteJob(postgresWriteJob{
		seq:       oldStateSeq,
		charState: &postgresCharStateSnapshot{uid: uid},
	})
	if st.isLatestCharStateJob(oldState) {
		t.Fatal("autosave de charstate iniciado antes do fence recuperou autoridade ao enfileirar tarde")
	}

	newer := st.stampWriteJob(postgresWriteJob{
		seq:     st.writeSeq.Add(1),
		account: account,
	})
	if !st.isLatestAccountJob(newer) {
		t.Fatal("autosave realmente posterior ao fence foi bloqueado")
	}
}

func TestPostgresPlayerStateFenceSkipsQueuedAutosaves(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL nao configurada")
	}
	st, err := NewPostgresStore(context.Background(), PostgresConfig{URL: url, MaxConns: 3})
	if err != nil {
		t.Fatal(err)
	}

	suffix := strconv.FormatInt(time.Now().UnixNano(), 36)
	if len(suffix) > 8 {
		suffix = suffix[len(suffix)-8:]
	}
	accountName := "fenc" + suffix
	charName := "Fen" + suffix
	account := postgresTestAccount(accountName, charName, model.Item{Index: 4011})
	if err := st.CreateAccount(account); err != nil {
		st.Close()
		t.Fatal(err)
	}
	uid := account.Chars[0].UID

	var releaseOnce sync.Once
	started := make(chan struct{})
	release := make(chan struct{})
	doRelease := func() { releaseOnce.Do(func() { close(release) }) }
	defer func() {
		doRelease()
		_, _ = st.pool.Exec(context.Background(),
			`DELETE FROM accounts WHERE name_key=$1`, strings.ToLower(accountName))
		st.Close()
	}()

	if err := st.enqueue(postgresWriteJob{
		label: "test: hold async worker",
		run: func() error {
			close(started)
			<-release
			return nil
		},
	}); err != nil {
		t.Fatal(err)
	}
	select {
	case <-started:
	case <-time.After(2 * time.Second):
		t.Fatal("worker PostgreSQL nao iniciou o bloqueio de teste")
	}

	account.Chars[0].Gold = 111
	if err := st.SaveAccountAsync(account); err != nil {
		t.Fatal(err)
	}
	if err := st.SaveCharStateAsync(uid, &model.CharState{
		Version:      model.CharStateVersion,
		SpecialCoins: map[string]uint32{"fame": 1},
	}); err != nil {
		t.Fatal(err)
	}

	account.Chars[0].Gold = 222
	done := make(chan error, 1)
	go func() {
		done <- st.SavePlayerState(nil, account, uid, &model.CharState{
			Version:      model.CharStateVersion,
			SpecialCoins: map[string]uint32{"fame": 2},
		})
	}()
	select {
	case err := <-done:
		if err != nil {
			t.Fatalf("SavePlayerState falhou com worker ocupado por outro job: %v", err)
		}
	case <-time.After(2 * time.Second):
		doRelease()
		t.Fatal("save critico esperou a barreira global do worker assinc")
	}

	doRelease()
	st.Flush()
	loaded, err := st.LoadAccount(accountName)
	if err != nil {
		t.Fatal(err)
	}
	if loaded.Chars[0].Gold != 222 {
		t.Fatalf("autosave antigo sobrescreveu conta critica: gold=%d", loaded.Chars[0].Gold)
	}
	loadedState, err := st.LoadCharState(uid)
	if err != nil {
		t.Fatal(err)
	}
	if loadedState == nil || loadedState.SpecialCoins["fame"] != 2 {
		t.Fatalf("autosave antigo sobrescreveu charstate critico: %+v", loadedState)
	}
}
