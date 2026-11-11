package main

import (
	"errors"
	"net"
	"os"
	"path/filepath"
	"runtime"
	"strings"
	"testing"
)

func TestRunInitializesPortalBeforeListenerBindFailure(t *testing.T) {
	databaseURL := os.Getenv("WYD_TEST_POSTGRES_URL")
	if databaseURL == "" {
		t.Skip("requires the isolated PostgreSQL test database")
	}
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()
	_, sourceFile, _, ok := runtime.Caller(0)
	if !ok {
		t.Fatal("cannot locate player-api test source")
	}
	dataDir := filepath.Join(filepath.Dir(sourceFile), "..", "..", "data")

	err = runPlayerAPITestWithDatabase(t, "https://player.example.test", strings.Repeat("s", 32),
		databaseURL, "-addr", listener.Addr().String(), "-data-dir", dataDir)
	var listenErr *net.OpError
	if !errors.As(err, &listenErr) || listenErr.Op != "listen" {
		t.Fatalf("startup should reach the occupied HTTP listener; error=%v", err)
	}
}
