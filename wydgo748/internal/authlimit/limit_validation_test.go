package authlimit_test

import (
	"context"
	"testing"
	"time"

	"github.com/jackc/pgx/v5/pgxpool"
	"wydgo/internal/authlimit"
)

func TestPolicyNormalizesIPv4AndAccountIdentifiers(t *testing.T) {
	mapped := authlimit.Policy("player_login", "::ffff:192.0.2.7", "ExampleUser")
	native := authlimit.Policy("player_login", "192.0.2.7", "exampleuser")
	if len(mapped) != 3 || len(native) != 3 {
		t.Fatalf("login policy dimensions = %d and %d, want 3", len(mapped), len(native))
	}
	for index := range mapped {
		if mapped[index].Key != native[index].Key || mapped[index].Window != time.Hour || mapped[index].Maximum < 1 {
			t.Fatalf("mapped IPv4 policy differs from native IPv4: mapped=%+v native=%+v", mapped[index], native[index])
		}
	}
	unknown := authlimit.Policy("player_login", "not-an-address", "exampleuser")
	if unknown[1].Key != authlimit.Policy("player_login", "also-invalid", "exampleuser")[1].Key {
		t.Fatal("invalid peer addresses did not use the stable unknown-network quota")
	}
	registration := authlimit.Policy("registration", "192.0.2.7", "ignored")
	if len(registration) != 5 || registration[0].Maximum != 30 || registration[1].Maximum != 300 ||
		registration[2].Maximum != 1000 || registration[3].Maximum != 5 || registration[4].Maximum != 10 {
		t.Fatalf("registration quota limits changed: %+v", registration)
	}
}

func TestAdmissionRejectsInvalidInputsBeforeOpeningStorage(t *testing.T) {
	valid := authlimit.Limit{Key: "quota", Window: time.Hour, Maximum: 1}
	longKey := valid
	longKey.Key = string(make([]byte, 161))
	zeroWindow := valid
	zeroWindow.Window = 999 * time.Millisecond
	largeWindow := valid
	largeWindow.Window = 24*time.Hour + time.Second
	fractionalWindow := valid
	fractionalWindow.Window = 1500 * time.Millisecond
	noMaximum := valid
	noMaximum.Maximum = 0

	cases := []struct {
		name   string
		ctx    context.Context
		limits []authlimit.Limit
	}{
		{name: "nil context", ctx: nil, limits: []authlimit.Limit{valid}},
		{name: "empty limits", ctx: context.Background()},
		{name: "too many limits", ctx: context.Background(), limits: make([]authlimit.Limit, 9)},
		{name: "empty key", ctx: context.Background(), limits: []authlimit.Limit{{Window: time.Hour, Maximum: 1}}},
		{name: "oversized key", ctx: context.Background(), limits: []authlimit.Limit{longKey}},
		{name: "short window", ctx: context.Background(), limits: []authlimit.Limit{zeroWindow}},
		{name: "long window", ctx: context.Background(), limits: []authlimit.Limit{largeWindow}},
		{name: "subsecond precision", ctx: context.Background(), limits: []authlimit.Limit{fractionalWindow}},
		{name: "zero maximum", ctx: context.Background(), limits: []authlimit.Limit{noMaximum}},
		{name: "duplicate keys", ctx: context.Background(), limits: []authlimit.Limit{valid, valid}},
	}
	limiter := authlimit.Postgres{Pool: &pgxpool.Pool{}}
	for _, tc := range cases {
		t.Run(tc.name, func(t *testing.T) {
			if _, err := limiter.Admit(tc.ctx, tc.limits); err == nil {
				t.Fatal("invalid quota input was admitted")
			}
		})
	}

	if _, err := (authlimit.Postgres{}).Admit(context.Background(), []authlimit.Limit{valid}); err == nil {
		t.Fatal("missing storage did not fail closed")
	}
}
