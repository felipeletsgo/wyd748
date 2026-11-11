package store

import (
	"context"
	"fmt"
	"os"
	"testing"
	"time"

	"github.com/jackc/pgx/v5"
	"github.com/jackc/pgx/v5/pgxpool"
)

func TestPostgresChannelMessagesPublishReadCleanupAndListen(t *testing.T) {
	url := os.Getenv("WYD_TEST_POSTGRES_URL")
	if url == "" {
		t.Skip("WYD_TEST_POSTGRES_URL requires an isolated test database")
	}
	ctx, cancel := context.WithTimeout(context.Background(), 20*time.Second)
	defer cancel()
	adminConfig, err := pgx.ParseConfig(url)
	if err != nil {
		t.Fatal("parse test database configuration")
	}
	admin, err := pgx.ConnectConfig(ctx, adminConfig)
	if err != nil {
		t.Fatal("connect to test database")
	}
	schema := fmt.Sprintf("channel_messages_%x", time.Now().UnixNano())
	identifier := pgx.Identifier{schema}.Sanitize()
	if _, err := admin.Exec(ctx, "CREATE SCHEMA "+identifier); err != nil {
		_ = admin.Close(context.Background())
		t.Fatal("create isolated channel-message schema")
	}
	var pool *pgxpool.Pool
	defer func() {
		if pool != nil {
			pool.Close()
		}
		_, _ = admin.Exec(context.Background(), "DROP SCHEMA "+identifier+" CASCADE")
		_ = admin.Close(context.Background())
	}()
	poolConfig, err := pgxpool.ParseConfig(url)
	if err != nil {
		t.Fatal("parse test pool configuration")
	}
	poolConfig.ConnConfig.RuntimeParams["search_path"] = schema
	poolConfig.ConnConfig.RuntimeParams["application_name"] = schema + "_setup"
	poolConfig.MaxConns = 3
	pool, err = pgxpool.NewWithConfig(ctx, poolConfig)
	if err != nil {
		t.Fatal("create isolated channel-message pool")
	}
	if _, err := pool.Exec(ctx, `CREATE TABLE channel_messages (
		id bigserial PRIMARY KEY,
		source text NOT NULL CHECK (length(source) BETWEEN 1 AND 80),
		target smallint NOT NULL DEFAULT 0 CHECK (target BETWEEN 0 AND 255),
		kind text NOT NULL CHECK (length(kind) BETWEEN 1 AND 40),
		payload jsonb NOT NULL,
		created_at timestamptz NOT NULL DEFAULT now())`); err != nil {
		t.Fatal("create isolated channel-message table")
	}
	if _, err := pool.Exec(ctx, "CREATE INDEX channel_messages_created_idx ON channel_messages(created_at)"); err != nil {
		t.Fatal("create isolated channel-message index")
	}
	pool.Close()
	poolConfig.ConnConfig.RuntimeParams["application_name"] = schema
	pool, err = pgxpool.NewWithConfig(ctx, poolConfig)
	if err != nil {
		t.Fatal("create listener pool")
	}
	st := newPostgresStore(pool, PostgresConfig{OperationTimeout: 3 * time.Second, ReadTimeout: 3 * time.Second})
	if err := st.PublishChannelMessages(nil); err != nil {
		t.Fatalf("publish empty batch: %v", err)
	}
	start, err := st.ChannelMessageStart()
	if err != nil || start.IsZero() {
		t.Fatalf("database message start=%v err=%v", start, err)
	}
	var baseline int64
	if err := pool.QueryRow(ctx, "SELECT COALESCE(MAX(id), 0) FROM channel_messages").Scan(&baseline); err != nil {
		t.Fatal(err)
	}
	self, remote, old := schema+"_self", schema+"_remote", schema+"_old"
	messages := []ChannelMessage{
		{Source: self, Target: 0, Kind: "self", Payload: []byte(`{"id":1}`)},
		{Source: remote, Target: 0, Kind: "broadcast", Payload: []byte(`{"id":2}`)},
		{Source: remote, Target: 1, Kind: "channel-one", Payload: []byte(`{"id":3}`)},
		{Source: remote, Target: 2, Kind: "channel-two", Payload: []byte(`{"id":4}`)},
		{Source: old, Target: 1, Kind: "old", Payload: []byte(`{"id":5}`)},
	}

	listenerCtx, stopListener := context.WithCancel(ctx)
	wake := make(chan struct{}, 4)
	listenerDone := make(chan struct{})
	go func() {
		defer close(listenerDone)
		st.ListenChannelMessages(listenerCtx, func() { wake <- struct{}{} })
	}()
	readyDeadline := time.Now().Add(5 * time.Second)
	for {
		var ready bool
		if err := admin.QueryRow(ctx, `SELECT EXISTS (
			SELECT 1 FROM pg_stat_activity WHERE application_name=$1 AND state='idle'
			AND wait_event_type='Client' AND wait_event='ClientRead' AND query=$2)`, schema,
			"LISTEN "+pgx.Identifier{channelMessagesChannel}.Sanitize()).Scan(&ready); err != nil {
			stopListener()
			t.Fatal("wait for message listener")
		}
		if ready {
			break
		}
		if time.Now().After(readyDeadline) {
			stopListener()
			t.Fatal("message listener did not subscribe")
		}
		time.Sleep(20 * time.Millisecond)
	}
	if err := st.PublishChannelMessages(messages); err != nil {
		stopListener()
		t.Fatalf("publish channel messages: %v", err)
	}
	if err := st.PublishChannelMessages([]ChannelMessage{{
		Source: schema + "_invalid", Target: 1, Kind: "bad-payload", Payload: []byte("not-json"),
	}}); err == nil {
		stopListener()
		t.Fatal("invalid JSON payload was published")
	}
	select {
	case <-wake:
	case <-time.After(3 * time.Second):
		stopListener()
		t.Fatal("listener missed the PostgreSQL notification")
	}
	stopListener()
	select {
	case <-listenerDone:
	case <-time.After(3 * time.Second):
		t.Fatal("listener did not stop after context cancellation")
	}

	var oldID int64
	if err := pool.QueryRow(ctx, "SELECT id FROM channel_messages WHERE source=$1", old).Scan(&oldID); err != nil {
		t.Fatal(err)
	}
	if _, err := pool.Exec(ctx, "UPDATE channel_messages SET created_at=now()-interval '2 hours' WHERE id=$1", oldID); err != nil {
		t.Fatal(err)
	}
	selected, err := st.ReadChannelMessages(self, 1, baseline, time.Now().Add(-time.Hour), 10)
	if err != nil || len(selected) != 2 {
		t.Fatalf("read channel messages=%+v err=%v, want broadcast and channel-one", selected, err)
	}
	if selected[0].Kind != "broadcast" || selected[1].Kind != "channel-one" || selected[0].Target != 0 || selected[1].Target != 1 {
		t.Fatalf("read channel order/filter = %+v", selected)
	}
	empty, err := st.ReadChannelMessages(self, 3, baseline, time.Now().Add(-time.Hour), 10)
	if err != nil || len(empty) != 1 || empty[0].Target != 0 {
		t.Fatalf("read channel broadcast=%+v err=%v, want only the broadcast", empty, err)
	}
	late, err := st.ReadChannelMessages(self, 1, oldID, time.Now().Add(-time.Hour), 1)
	if err != nil || len(late) != 1 || late[0].Kind != "broadcast" {
		t.Fatalf("late channel-message read=%+v err=%v", late, err)
	}
	if err := st.CleanupChannelMessages(); err != nil {
		t.Fatalf("clean old channel messages: %v", err)
	}
	var remaining int
	if err := pool.QueryRow(ctx, "SELECT count(*) FROM channel_messages WHERE id=$1", oldID).Scan(&remaining); err != nil || remaining != 0 {
		t.Fatalf("old channel message remaining=%d err=%v", remaining, err)
	}
}

func TestListenChannelMessagesWakesAfterAcquireFailure(t *testing.T) {
	poolConfig, err := pgxpool.ParseConfig("postgres://unused:unused@127.0.0.1:1/unused")
	if err != nil {
		t.Fatal(err)
	}
	pool, err := pgxpool.NewWithConfig(context.Background(), poolConfig)
	if err != nil {
		t.Fatal(err)
	}
	pool.Close()
	ctx, cancel := context.WithCancel(context.Background())
	wake := make(chan struct{}, 1)
	done := make(chan struct{})
	go func() {
		defer close(done)
		(&PostgresStore{pool: pool}).ListenChannelMessages(ctx, func() {
			wake <- struct{}{}
			cancel()
		})
	}()
	select {
	case <-wake:
	case <-time.After(2 * time.Second):
		cancel()
		t.Fatal("listener did not wake after a failed pool acquisition")
	}
	select {
	case <-done:
	case <-time.After(2 * time.Second):
		t.Fatal("listener did not stop after the wake callback canceled its context")
	}
}
