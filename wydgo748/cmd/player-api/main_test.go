package main

import (
	"flag"
	"io"
	"os"
	"testing"
)

func setPlayerAPIArgs(t *testing.T, args ...string) {
	t.Helper()
	oldArgs, oldFlags := os.Args, flag.CommandLine
	flags := flag.NewFlagSet("player-api-test", flag.ContinueOnError)
	flags.SetOutput(io.Discard)
	flag.CommandLine = flags
	os.Args = append([]string{"player-api"}, args...)
	t.Cleanup(func() { os.Args, flag.CommandLine = oldArgs, oldFlags })
}

func TestRunRejectsUnsafeListener(t *testing.T) {
	t.Setenv("WYD_TURNSTILE_SECRET", "coverage-secret")
	setPlayerAPIArgs(t, "-addr", "0.0.0.0:8083")
	if err := run(); err == nil {
		t.Fatal("player-api accepted a public listener")
	}
}

func TestRunRejectsInvalidTurnstileConfiguration(t *testing.T) {
	t.Run("missing origin", func(t *testing.T) {
		t.Setenv("WYD_TURNSTILE_SECRET", "coverage-secret")
		setPlayerAPIArgs(t)
		if err := run(); err == nil {
			t.Fatal("player-api accepted a missing public origin")
		}
	})
	t.Run("missing secret", func(t *testing.T) {
		t.Setenv("WYD_TURNSTILE_SECRET", "")
		setPlayerAPIArgs(t, "-origin", "https://portal.example")
		if err := run(); err == nil {
			t.Fatal("player-api accepted a missing Turnstile secret")
		}
	})
}

func TestRunRequiresRegistrationDatabaseURL(t *testing.T) {
	t.Setenv("WYD_TURNSTILE_SECRET", "coverage-secret")
	t.Setenv("WYD_PLAYER_DATABASE_URL", "")
	setPlayerAPIArgs(t, "-addr", "127.0.0.1:8083", "-origin", "https://portal.example")
	err := run()
	if err == nil || err.Error() != "WYD_PLAYER_DATABASE_URL is required (registration-only role)" {
		t.Fatalf("missing database URL error=%v", err)
	}
}
