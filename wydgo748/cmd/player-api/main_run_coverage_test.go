package main

import (
	"flag"
	"io"
	"os"
	"strings"
	"testing"
)

func runPlayerAPITest(t *testing.T, origin string, secret string, extra ...string) error {
	return runPlayerAPITestWithDatabase(t, origin, secret, "", extra...)
}

func runPlayerAPITestWithDatabase(t *testing.T, origin string, secret, databaseURL string, extra ...string) error {
	t.Helper()
	oldFlags, oldArgs := flag.CommandLine, os.Args
	flags := flag.NewFlagSet("player-api-run-test", flag.ContinueOnError)
	flags.SetOutput(io.Discard)
	flag.CommandLine = flags
	os.Args = append([]string{"player-api", "-addr", "127.0.0.1:8083", "-origin", origin}, extra...)
	t.Setenv("WYD_TURNSTILE_SECRET", secret)
	t.Setenv("WYD_PLAYER_DATABASE_URL", databaseURL)
	t.Cleanup(func() { flag.CommandLine, os.Args = oldFlags, oldArgs })
	return run()
}

func TestRunRejectsTurnstileAndDatabaseConfiguration(t *testing.T) {
	t.Run("public listener is rejected before other inputs", func(t *testing.T) {
		err := runPlayerAPITest(t, "https://player.example.test", strings.Repeat("s", 32),
			"-addr", "0.0.0.0:8083")
		if err == nil || !strings.Contains(err.Error(), "loopback") {
			t.Fatalf("public listener error=%v", err)
		}
	})
	t.Run("origin is required", func(t *testing.T) {
		err := runPlayerAPITest(t, "", "")
		if err == nil || !strings.Contains(err.Error(), "exact HTTPS") {
			t.Fatalf("invalid Turnstile origin error=%v", err)
		}
	})
	t.Run("secret is required", func(t *testing.T) {
		err := runPlayerAPITest(t, "https://player.example.test", "")
		if err == nil || !strings.Contains(err.Error(), "WYD_TURNSTILE_SECRET is required") {
			t.Fatalf("missing Turnstile secret error=%v", err)
		}
	})
	t.Run("registration-only database is required", func(t *testing.T) {
		err := runPlayerAPITest(t, "https://player.example.test", strings.Repeat("s", 32))
		if err == nil || !strings.Contains(err.Error(), "WYD_PLAYER_DATABASE_URL is required") {
			t.Fatalf("missing player database error=%v", err)
		}
	})
	t.Run("invalid database configuration is reported", func(t *testing.T) {
		err := runPlayerAPITestWithDatabase(t, "https://player.example.test", strings.Repeat("s", 32), "invalid://database")
		if err == nil || !strings.Contains(err.Error(), "invalid database configuration") {
			t.Fatalf("invalid player database error=%v", err)
		}
	})
}
