package store

import (
	"context"
	"log"
	"time"

	"github.com/jackc/pgx/v5"
)

// Cross-process channel messages (DOCS/server/server-correction-plan.md, W9 phase 4).
// A game process stores each message for other channels in channel_messages
// and sends a notification as a wake-up hint. Every process reads the recent
// messages for its channel, skips the IDs it already delivered, and removes
// messages older than ten minutes. Delivery is at most once per process.

const postgresChannelMessagesSchema = `
CREATE TABLE IF NOT EXISTS channel_messages (
    id         bigserial PRIMARY KEY,
    source     text NOT NULL CHECK (length(source) BETWEEN 1 AND 80),
    target     smallint NOT NULL DEFAULT 0 CHECK (target BETWEEN 0 AND 255),
    kind       text NOT NULL CHECK (length(kind) BETWEEN 1 AND 40),
    payload    jsonb NOT NULL,
    created_at timestamptz NOT NULL DEFAULT now()
);
CREATE INDEX IF NOT EXISTS channel_messages_created_idx ON channel_messages(created_at);`

const channelMessagesChannel = "wyd_channel_messages"

// channelMessageLag bounds how late a message may become visible after a
// newer one and still be delivered.
const channelMessageLag = 10 * time.Second

// ChannelMessage is one stored cross-process message. Target 0 addresses
// every channel.
type ChannelMessage struct {
	ID      int64
	Source  string
	Target  byte
	Kind    string
	Payload []byte
}

// PublishChannelMessages stores the messages in one transaction and wakes
// the other processes when it commits.
func (s *PostgresStore) PublishChannelMessages(messages []ChannelMessage) error {
	if len(messages) == 0 {
		return nil
	}
	sources := make([]string, len(messages))
	targets := make([]int16, len(messages))
	kinds := make([]string, len(messages))
	payloads := make([]string, len(messages))
	for i, message := range messages {
		sources[i], targets[i], kinds[i], payloads[i] = message.Source, int16(message.Target), message.Kind, string(message.Payload)
	}
	ctx, cancel := s.postgresContext()
	defer cancel()
	tx, err := s.pool.Begin(ctx)
	if err != nil {
		return err
	}
	defer func() { _ = tx.Rollback(context.Background()) }()
	if _, err := tx.Exec(ctx, `INSERT INTO channel_messages(source,target,kind,payload)
		SELECT * FROM unnest($1::text[], $2::smallint[], $3::text[], $4::jsonb[])`,
		sources, targets, kinds, payloads); err != nil {
		return err
	}
	if _, err := tx.Exec(ctx, `SELECT pg_notify($1, '')`, channelMessagesChannel); err != nil {
		return err
	}
	return tx.Commit(ctx)
}

// ChannelMessageStart returns the database time from which a new reader
// delivers messages.
func (s *PostgresStore) ChannelMessageStart() (time.Time, error) {
	ctx, cancel := s.postgresReadContext()
	defer cancel()
	var now time.Time
	err := s.pool.QueryRow(ctx, `SELECT now()`).Scan(&now)
	return now, err
}

// ReadChannelMessages returns the messages of other processes for channel
// that are newer than lastID, or recent enough to have become visible late.
// It never returns messages created before since.
func (s *PostgresStore) ReadChannelMessages(self string, channel byte, lastID int64, since time.Time, limit int) ([]ChannelMessage, error) {
	ctx, cancel := s.postgresReadContext()
	defer cancel()
	rows, err := s.pool.Query(ctx, `SELECT id, source, target, kind, payload FROM channel_messages
		WHERE created_at > $4 AND (target = 0 OR target = $2) AND source <> $1
		AND (id > $3 OR created_at > now() - make_interval(secs => $5))
		ORDER BY id LIMIT $6`, self, int16(channel), lastID, since, channelMessageLag.Seconds(), limit)
	if err != nil {
		return nil, err
	}
	defer rows.Close()
	var messages []ChannelMessage
	for rows.Next() {
		var message ChannelMessage
		var target int16
		if err := rows.Scan(&message.ID, &message.Source, &target, &message.Kind, &message.Payload); err != nil {
			return nil, err
		}
		message.Target = byte(target)
		messages = append(messages, message)
	}
	return messages, rows.Err()
}

// CleanupChannelMessages removes messages older than ten minutes.
func (s *PostgresStore) CleanupChannelMessages() error {
	ctx, cancel := s.postgresContext()
	defer cancel()
	_, err := s.pool.Exec(ctx, `DELETE FROM channel_messages WHERE created_at < now() - interval '10 minutes'`)
	return err
}

// ListenChannelMessages calls wake for each message notification and after
// each reconnect until ctx ends.
func (s *PostgresStore) ListenChannelMessages(ctx context.Context, wake func()) {
	for ctx.Err() == nil {
		err := s.listen(ctx, channelMessagesChannel, func(string) { wake() })
		if ctx.Err() != nil {
			return
		}
		log.Printf("channel message notifications: %v; reconnecting", err)
		wake() // notifications sent meanwhile are lost
		select {
		case <-ctx.Done():
		case <-time.After(2 * time.Second):
		}
	}
}

// listen holds one connection outside the pool and reports each payload of
// the notification channel until an error or ctx ends.
func (s *PostgresStore) listen(ctx context.Context, channel string, payload func(string)) error {
	pooled, err := s.pool.Acquire(ctx)
	if err != nil {
		return err
	}
	conn := pooled.Hijack()
	defer conn.Close(context.Background())
	if _, err := conn.Exec(ctx, "LISTEN "+pgx.Identifier{channel}.Sanitize()); err != nil {
		return err
	}
	for {
		notification, err := conn.WaitForNotification(ctx)
		if err != nil {
			return err
		}
		payload(notification.Payload)
	}
}
