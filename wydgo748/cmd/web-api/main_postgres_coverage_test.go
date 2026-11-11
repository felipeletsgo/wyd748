package main

import (
	"net"
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func TestRunInitializesWebAdminBeforeListenerBindFailure(t *testing.T) {
	databaseURL := os.Getenv("WYD_TEST_POSTGRES_URL")
	if databaseURL == "" {
		t.Skip("requires the isolated PostgreSQL test database")
	}

	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()

	configPath, staffPath := writeWebAPITestInputs(t, "admin_access_pin=001234\n", `{"operator":{"capabilities":["server.status"]}}`)
	staticPath := filepath.Join(t.TempDir(), "dist")
	adminPath := filepath.Join(staticPath, "admin")
	if err := os.MkdirAll(adminPath, 0o700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(adminPath, "index.html"), []byte("<!doctype html><title>admin</title>"), 0o600); err != nil {
		t.Fatal(err)
	}

	t.Setenv("WYD_WEB_DATABASE_URL", databaseURL)
	t.Setenv("WYD_WEB_AUDIT_LOG", "")
	err = runWebAPITest(t, configPath, staffPath,
		"-addr", listener.Addr().String(),
		"-origin", "http://127.0.0.1:8082",
		"-control-url", "http://127.0.0.1:8081",
		"-static", staticPath,
	)
	if err == nil || !strings.Contains(err.Error(), "web listener failed") {
		t.Fatalf("startup should reach the occupied HTTP listener; error=%v", err)
	}
}
