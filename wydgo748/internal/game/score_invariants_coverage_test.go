package game

import (
	"strings"
	"testing"

	"wydgo/internal/model"
)

func TestEnsureScoreEnforcesRequiredRepresentation(t *testing.T) {
	ensureScore(nil)
	ensureScore(&model.Char{Score: &model.Score{Version: model.ScoreVersion}})

	for _, tc := range []struct {
		name string
		char *model.Char
		want string
	}{
		{name: "missing score", char: &model.Char{}, want: "has no score"},
		{name: "invalid version", char: &model.Char{Score: &model.Score{Version: model.ScoreVersion + 1}}, want: "invalid score version"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			defer func() {
				value := recover()
				if value == nil || !strings.Contains(value.(string), tc.want) {
					t.Fatalf("panic=%v, want %q", value, tc.want)
				}
			}()
			ensureScore(tc.char)
		})
	}
}
