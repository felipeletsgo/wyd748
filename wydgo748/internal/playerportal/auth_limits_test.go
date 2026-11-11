package playerportal

import (
	"context"
	"errors"
	"os"
	"testing"

	"wydgo/internal/authlimit"
)

func TestDatabaseExposesPostgresAuthenticationLimiter(t *testing.T) {
	limiter := (&Database{}).AuthenticationLimiter()
	if _, ok := limiter.(authlimit.Postgres); !ok {
		t.Fatalf("limiter type=%T", limiter)
	}
}

func TestDatabaseCharacterDetailRejectsInvalidSlotsBeforeQuery(t *testing.T) {
	db := &Database{}
	for _, slot := range []int{-1, 4} {
		if _, err := db.CharacterDetail(context.Background(), "account", slot); !errors.Is(err, os.ErrNotExist) {
			t.Fatalf("slot %d error=%v", slot, err)
		}
	}
}
