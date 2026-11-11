package store

import (
	"context"
	"strings"
	"testing"
	"time"
)

func TestPruneAccountLoginsRejectsReadOnlyAndInvalidLimits(t *testing.T) {
	for _, test := range []struct {
		name     string
		store    *PostgresStore
		limit    int
		wantText string
	}{
		{name: "read only", store: &PostgresStore{readOnly: true}, limit: 1, wantText: "read-only"},
		{name: "zero limit", store: &PostgresStore{}, limit: 0, wantText: "invalid login prune limit"},
		{name: "limit too large", store: &PostgresStore{}, limit: 10001, wantText: "invalid login prune limit"},
	} {
		t.Run(test.name, func(t *testing.T) {
			if _, err := test.store.PruneAccountLogins(context.Background(), time.Now(), test.limit); err == nil || !strings.Contains(err.Error(), test.wantText) {
				t.Fatalf("prune error=%v, want %q", err, test.wantText)
			}
		})
	}
}
