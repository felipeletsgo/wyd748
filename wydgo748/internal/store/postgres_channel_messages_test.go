package store

import (
	"context"
	"testing"
)

// W9 phase 4 channel messages.
func TestChannelMessagesReachOtherProcessesOfTheirChannel(t *testing.T) {
	st, conn := instanceRowsStore(t)
	ctx := context.Background()
	cleanup := func() { _, _ = conn.Exec(ctx, `DELETE FROM channel_messages WHERE source LIKE 'w9-test/%'`) }
	cleanup()
	t.Cleanup(cleanup)
	since, err := st.ChannelMessageStart()
	if err != nil {
		t.Fatal(err)
	}
	if err := st.PublishChannelMessages([]ChannelMessage{
		{Source: "w9-test/a", Kind: "chat.global", Payload: []byte(`{"message":"to all"}`)},
		{Source: "w9-test/a", Target: 2, Kind: "chat.whisper", Payload: []byte(`{"message":"to two"}`)},
		{Source: "w9-test/a", Target: 3, Kind: "chat.whisper", Payload: []byte(`{"message":"to three"}`)},
		{Source: "w9-test/b", Kind: "chat.global", Payload: []byte(`{"message":"from b"}`)},
	}); err != nil {
		t.Fatal(err)
	}
	messages, err := st.ReadChannelMessages("w9-test/b", 2, 0, since, 100)
	if err != nil {
		t.Fatal(err)
	}
	var kinds []string
	for _, message := range messages {
		if message.Source != "w9-test/a" || message.Target != 0 && message.Target != 2 {
			t.Fatalf("message for another reader: %+v", message)
		}
		kinds = append(kinds, message.Kind)
	}
	if len(kinds) != 2 || kinds[0] != "chat.global" || kinds[1] != "chat.whisper" {
		t.Fatalf("channel 2 read %v", kinds)
	}
	// A later read past the last ID still returns recent messages, so a late
	// commit is not lost; the reader skips IDs it delivered.
	again, err := st.ReadChannelMessages("w9-test/b", 2, messages[1].ID, since, 100)
	if err != nil || len(again) != 2 {
		t.Fatalf("recent window read %d err=%v", len(again), err)
	}
	// A reader that started later does not receive older messages.
	later, err := st.ChannelMessageStart()
	if err != nil {
		t.Fatal(err)
	}
	if old, err := st.ReadChannelMessages("w9-test/b", 2, 0, later, 100); err != nil || len(old) != 0 {
		t.Fatalf("new reader received %d old message(s) err=%v", len(old), err)
	}
	if _, err := conn.Exec(ctx, `UPDATE channel_messages SET created_at = now() - interval '11 minutes'
		WHERE source LIKE 'w9-test/%'`); err != nil {
		t.Fatal(err)
	}
	if err := st.CleanupChannelMessages(); err != nil {
		t.Fatal(err)
	}
	var left int
	if err := conn.QueryRow(ctx, `SELECT count(*) FROM channel_messages WHERE source LIKE 'w9-test/%'`).Scan(&left); err != nil || left != 0 {
		t.Fatalf("cleanup left %d err=%v", left, err)
	}
}
