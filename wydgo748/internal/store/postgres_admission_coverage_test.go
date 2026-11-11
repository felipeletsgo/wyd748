package store

import (
	"context"
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestPostgresConstructorsRejectMissingAndMalformedURLs(t *testing.T) {
	for _, tc := range []struct {
		name string
		call func(PostgresConfig) (*PostgresStore, error)
	}{
		{name: "authoritative", call: func(cfg PostgresConfig) (*PostgresStore, error) {
			return NewPostgresStore(context.Background(), cfg)
		}},
		{name: "read only", call: func(cfg PostgresConfig) (*PostgresStore, error) {
			return NewPostgresReadOnlyStore(context.Background(), cfg)
		}},
	} {
		t.Run(tc.name, func(t *testing.T) {
			store, err := tc.call(PostgresConfig{})
			if err == nil || store != nil || !strings.Contains(err.Error(), "database URL is missing") {
				t.Fatalf("missing URL result: store=%v err=%v", store, err)
			}
			store, err = tc.call(PostgresConfig{URL: "not-a-postgres-url"})
			if err == nil || store != nil || !strings.Contains(err.Error(), "parse database URL") {
				t.Fatalf("malformed URL result: store=%v err=%v", store, err)
			}
		})
	}

	store, err := NewPostgresStore(context.Background(), PostgresConfig{FenceGameInstance: true, FenceChannel: 2})
	if err == nil || store != nil || !strings.Contains(err.Error(), "database URL is missing") {
		t.Fatalf("fenced startup did not reach URL validation: store=%v err=%v", store, err)
	}
}

func TestPostgresSavePlayerStateRejectsBeforeOpeningTransaction(t *testing.T) {
	uid, err := model.NewCharacterUID()
	if err != nil {
		t.Fatal(err)
	}
	account := postgresTestAccount("owner", "Owner", model.Item{})
	for _, tc := range []struct {
		name  string
		store *PostgresStore
		guild *model.GuildRegistry
		acc   *model.Account
		uid   string
		want  string
	}{
		{name: "read only", store: &PostgresStore{readOnly: true}, acc: account, uid: uid, want: "read-only"},
		{name: "invalid character UID", store: &PostgresStore{}, acc: account, uid: "invalid", want: "character UID"},
		{name: "closed store", store: &PostgresStore{closed: true}, acc: account, uid: uid, want: "closed"},
		{name: "missing account", store: &PostgresStore{}, uid: uid, want: "missing account"},
		{name: "invalid guild registry", store: &PostgresStore{}, guild: &model.GuildRegistry{}, acc: account, uid: uid, want: "invalid guilds"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			err := tc.store.SavePlayerState(tc.guild, tc.acc, tc.uid, nil)
			if err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("SavePlayerState error=%v, want substring %q", err, tc.want)
			}
		})
	}
}
