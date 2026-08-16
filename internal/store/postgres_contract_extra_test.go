package store

import (
	"context"
	"errors"
	"strings"
	"testing"
	"time"

	"github.com/jackc/pgx/v5/pgconn"

	"wydgo/internal/model"
)

func TestPostgresOperationTimeoutAndConstructorDefaults(t *testing.T) {
	if got := operationTimeout(PostgresConfig{}); got != postgresOperationTimeout {
		t.Fatalf("timeout default=%v, esperado=%v", got, postgresOperationTimeout)
	}
	custom := 123 * time.Millisecond
	if got := operationTimeout(PostgresConfig{OperationTimeout: custom}); got != custom {
		t.Fatalf("timeout custom=%v, esperado=%v", got, custom)
	}
	st := newPostgresStore(nil, PostgresConfig{GuildsTxtPath: "Guilds.txt", OperationTimeout: custom})
	if st.operationTimeout != custom || st.guildsTxtPath != "Guilds.txt" || cap(st.writeQueue) != postgresWriteQueueSize ||
		st.overflowAcc == nil || st.overflowState == nil || st.overflowRuns == nil {
		t.Fatalf("newPostgresStore nao materializou defaults: %+v", st)
	}
}

func TestPostgresPingAndFlushFailSafelyWithoutLivePool(t *testing.T) {
	var nilStore *PostgresStore
	if err := nilStore.Ping(context.Background()); err == nil {
		t.Fatal("Ping nil foi aceito")
	}
	if err := (&PostgresStore{}).Ping(context.Background()); err == nil {
		t.Fatal("Ping sem pool foi aceito")
	}

	readOnly := &PostgresStore{readOnly: true}
	readOnly.flushLocked()
	readOnly.Flush()
	closed := &PostgresStore{closed: true}
	closed.Flush()
	withoutQueue := &PostgresStore{}
	withoutQueue.flushLocked()
}

func TestPostgresUUIDCoversEmptyInvalidNormalizedAndValid(t *testing.T) {
	if _, err := postgresUUID(""); err == nil {
		t.Fatal("UID vazio foi aceito")
	}
	if _, err := postgresUUID("bad"); err == nil {
		t.Fatal("UID invalido foi aceito")
	}
	upper := "AAAAAAAAAAAA4AAA8AAAAAAAAAAAAAAA"
	got, err := postgresUUID(upper)
	if err != nil || !got.Valid {
		t.Fatalf("UUID normalizavel recusado: %+v err=%v", got, err)
	}
	if got.Bytes[0] != 0xaa || got.Bytes[6]&0xf0 != 0x40 || got.Bytes[8]&0xc0 != 0x80 {
		t.Fatalf("UUID bytes inesperados: %x", got.Bytes)
	}
}

func TestPostgresErrorClassificationCoversRetryUniqueAndOrdinaryErrors(t *testing.T) {
	for _, code := range []string{"40001", "40P01"} {
		if !isRetryablePostgres(&pgconn.PgError{Code: code}) {
			t.Fatalf("erro retryable %s nao reconhecido", code)
		}
	}
	for _, err := range []error{errors.New("plain"), &pgconn.PgError{Code: "23505"}, &pgconn.PgError{Code: "22000"}} {
		if isRetryablePostgres(err) {
			t.Fatalf("erro nao retryable classificado como retry: %v", err)
		}
	}
	if !isUniqueViolation(&pgconn.PgError{Code: "23505"}) {
		t.Fatal("unique violation nao reconhecida")
	}
	if isUniqueViolation(errors.New("plain")) || isUniqueViolation(&pgconn.PgError{Code: "40001"}) {
		t.Fatal("erro comum/retryable classificado como unique")
	}
}

func TestSnapshotAccountRejectsNilBrokenCharacterAndItemIdentity(t *testing.T) {
	if _, err := snapshotAccount(nil); err == nil {
		t.Fatal("snapshot nil foi aceito")
	}

	invalidChar := postgresTestAccount("invalidchar", "Hero", model.Item{})
	invalidChar.Chars[0].UID = "bad"
	if _, err := snapshotAccount(invalidChar); err == nil {
		t.Fatal("UID de personagem invalido foi aceito")
	}

	dupChar := postgresTestAccount("dupchar", "HeroOne", model.Item{})
	dupChar.Chars = append(dupChar.Chars, model.Char{
		Name: "HeroTwo", UID: "11111111111141118111111111111111",
		Extended: &model.ExtendedScore{Version: model.ExtendedScoreVersion},
	})
	dupChar.Chars[0].UID = "11111111111141118111111111111111"
	if _, err := snapshotAccount(dupChar); err == nil || !strings.Contains(err.Error(), "repetido") {
		t.Fatalf("UID de personagem duplicado nao foi recusado: %v", err)
	}

	emptyMetadata := postgresTestAccount("emptymeta", "Hero", model.Item{})
	emptyMetadata.Chars[0].Inv[0].UID = "22222222222242228222222222222222"
	if _, err := snapshotAccount(emptyMetadata); err == nil || !strings.Contains(err.Error(), "vazio possui") {
		t.Fatalf("slot vazio com metadata nao foi recusado: %v", err)
	}

	invalidItem := postgresTestAccount("invaliditem", "Hero", model.Item{Index: 4011, UID: "bad"})
	if _, err := snapshotAccount(invalidItem); err == nil {
		t.Fatal("UID de item invalido foi aceito")
	}
}

func TestSnapshotAccountMaterializesCharacterAndArchOriginUUIDs(t *testing.T) {
	acc := postgresTestAccount("archacc", "ArchHero", model.Item{Index: 4011})
	acc.Chars[0].Evolution = "arch"
	acc.Chars[0].ArchMortalUID = "AAAAAAAAAAAA4AAA8AAAAAAAAAAAAAAA"
	snapshot, err := snapshotAccount(acc)
	if err != nil {
		t.Fatal(err)
	}
	if len(snapshot.characters) != 1 || snapshot.characters[0].uid == "" || snapshot.characters[0].mortalUID == nil ||
		!snapshot.characters[0].mortalUID.Valid || snapshot.key != "archacc" || snapshot.display != "archacc" {
		t.Fatalf("snapshot de personagem incompleto: %+v", snapshot)
	}
	if acc.Chars[0].UID == "" {
		t.Fatal("UID de personagem gerado nao retornou ao agregado autoritativo")
	}
}

func TestSnapshotAccountsRejectsDuplicateAccountKeysAndAcceptsDistinctBatch(t *testing.T) {
	a := postgresTestAccount("Same", "HeroA", model.Item{})
	b := postgresTestAccount("same", "HeroB", model.Item{})
	if _, err := snapshotAccounts(a, b); err == nil || !strings.Contains(err.Error(), "duplicada") {
		t.Fatalf("conta duplicada case-insensitive foi aceita: %v", err)
	}

	c := postgresTestAccount("distinct-a", "HeroC", model.Item{})
	d := postgresTestAccount("distinct-b", "HeroD", model.Item{})
	got, err := snapshotAccounts(c, d)
	if err != nil || len(got) != 2 || got[0].key != "distinct-a" || got[1].key != "distinct-b" {
		t.Fatalf("batch distinto invalido: %+v err=%v", got, err)
	}
}

func TestPostgresContextUsesConfiguredAndFallbackTimeouts(t *testing.T) {
	st := &PostgresStore{operationTimeout: 50 * time.Millisecond}
	ctx, cancel := st.postgresContext()
	defer cancel()
	deadline, ok := ctx.Deadline()
	if !ok || time.Until(deadline) <= 0 || time.Until(deadline) > 100*time.Millisecond {
		t.Fatalf("deadline custom invalido: %v ok=%v", deadline, ok)
	}

	fallback := &PostgresStore{}
	ctx2, cancel2 := fallback.postgresContext()
	defer cancel2()
	deadline2, ok := ctx2.Deadline()
	if !ok || time.Until(deadline2) <= 0 || time.Until(deadline2) > postgresOperationTimeout+time.Second {
		t.Fatalf("deadline fallback invalido: %v ok=%v", deadline2, ok)
	}
}

func TestRetryDerivedNoopWithoutRunnableJob(t *testing.T) {
	st := &PostgresStore{}
	st.retryDerived(postgresWriteJob{})
}
