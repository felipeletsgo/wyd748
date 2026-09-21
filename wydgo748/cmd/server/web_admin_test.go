package main

import (
	"context"
	"errors"
	"io"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"strings"
	"sync/atomic"
	"testing"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/data"
)

type adminSourceFunc func(context.Context, control.Query) (control.Overview, error)

func (f adminSourceFunc) Overview(ctx context.Context, q control.Query) (control.Overview, error) {
	return f(ctx, q)
}

func TestLocalAdminControlReturnsFrontendContract(t *testing.T) {
	asOf := time.Now().UTC().Add(-time.Second)
	adapter := localAdminControl{started: time.Now().Add(-5 * time.Second), source: adminSourceFunc(func(context.Context, control.Query) (control.Overview, error) {
		// World supplies gameplay only; process metadata belongs to the adapter.
		return control.Overview{Version: 1, AsOf: asOf, State: "running", Players: []control.Player{}, Online: 0}, nil
	})}
	got, err := adapter.Overview(context.Background(), "operator", control.Query{})
	if err != nil {
		t.Fatal(err)
	}
	if got.Version != 1 || got.Diagnostics.Version != 1 || got.Diagnostics.AsOf.IsZero() || got.Players == nil || got.State != "running" {
		t.Fatalf("frontend rejects snapshot: %+v", got)
	}
	if !got.AsOf.Equal(asOf) || got.Diagnostics.AsOf.Before(asOf) {
		t.Fatal("game and process timestamps must retain independent meanings")
	}
	if got.UptimeSeconds < 5 || got.UptimeSeconds > 60 {
		t.Fatalf("invalid uptime: %d", got.UptimeSeconds)
	}
}

func TestLocalAdminControlPreservesContextQueryAndError(t *testing.T) {
	ctx, cancel := context.WithCancel(context.Background())
	cancel()
	query := control.Query{Search: "player", Offset: 50}
	adapter := localAdminControl{source: adminSourceFunc(func(got context.Context, q control.Query) (control.Overview, error) {
		if got != ctx || q != query {
			t.Fatal("adapter changed query/context")
		}
		return control.Overview{Version: 1, Online: 2}, got.Err()
	})}
	result, err := adapter.Overview(ctx, "operator", query)
	if result.Online != 2 || !errors.Is(err, context.Canceled) {
		t.Fatal("adapter lost result/error")
	}
}

func TestAdminDatabaseURLPrecedence(t *testing.T) {
	cfg := data.DefaultServerConfig()
	cfg.DatabaseURLEnv = "WYD_TEST_LOCAL_ADMIN_DB"
	t.Setenv(cfg.DatabaseURLEnv, "environment")
	t.Setenv("WYD_WEB_DATABASE_URL", "dedicated")
	cfg.DatabaseURL = "configured"
	if adminDatabaseURL(cfg) != "dedicated" {
		t.Fatal("dedicated URL ignored")
	}
	t.Setenv("WYD_WEB_DATABASE_URL", "")
	if adminDatabaseURL(cfg) != "configured" {
		t.Fatal("configured URL ignored")
	}
	cfg.DatabaseURL = ""
	if adminDatabaseURL(cfg) != "environment" {
		t.Fatal("environment fallback ignored")
	}
}

func adminTestConfig(t *testing.T) data.ServerConfig {
	t.Helper()
	cfg := data.DefaultServerConfig()
	cfg.WebAdminEnabled = true
	cfg.AdminAccessPIN = "001234"
	cfg.WebAdminStaticPath = t.TempDir()
	cfg.WebAdminStaffPath = filepath.Join(t.TempDir(), "staff.json")
	if err := os.WriteFile(cfg.WebAdminStaffPath, []byte(`{"operator":{"capabilities":["server.status"]}}`), 0600); err != nil {
		t.Fatal(err)
	}
	if err := os.MkdirAll(filepath.Join(cfg.WebAdminStaticPath, "admin"), 0700); err != nil {
		t.Fatal(err)
	}
	if err := os.WriteFile(filepath.Join(cfg.WebAdminStaticPath, "admin", "index.html"), []byte("admin fixture"), 0600); err != nil {
		t.Fatal(err)
	}
	return cfg
}

func TestStartWebAdminDisabledAndInvalid(t *testing.T) {
	stop, err := startWebAdmin(data.DefaultServerConfig(), nil, nil)
	if err != nil {
		t.Fatal(err)
	}
	stop()
	for _, address := range []string{"0.0.0.0:8082", "[::]:8082", "localhost:8082", "127.0.0.1:0", "127.0.0.1:65536"} {
		cfg := data.DefaultServerConfig()
		cfg.WebAdminEnabled = true
		cfg.WebAdminAddress = address
		if _, err := startWebAdmin(cfg, nil, nil); err == nil {
			t.Fatalf("accepted %s", address)
		}
	}
	cfg := adminTestConfig(t)
	cfg.WebAdminStaticPath = t.TempDir()
	if _, err := startWebAdmin(cfg, nil, nil); err == nil || !strings.Contains(err.Error(), "build do painel ausente") {
		t.Fatalf("missing assets: %v", err)
	}
	cfg.AdminAccessPIN = ""
	if _, err := startWebAdmin(cfg, nil, nil); err == nil {
		t.Fatal("empty PIN accepted")
	}
}

func TestStartWebAdminOccupiedPortDoesNotStopOwner(t *testing.T) {
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()
	cfg := adminTestConfig(t)
	cfg.WebAdminAddress = listener.Addr().String()
	if _, err := startWebAdmin(cfg, nil, nil); err == nil || !strings.Contains(err.Error(), "porta do painel indisponivel") {
		t.Fatalf("occupied port: %v", err)
	}
	conn, err := net.DialTimeout("tcp", cfg.WebAdminAddress, time.Second)
	if err != nil {
		t.Fatalf("existing listener was affected: %v", err)
	}
	conn.Close()
}

func TestServeWebAdminLifecycle(t *testing.T) {
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	address := listener.Addr().String()
	var closed atomic.Int32
	stop := serveWebAdmin(listener, http.HandlerFunc(func(w http.ResponseWriter, r *http.Request) { w.WriteHeader(204) }), func() { closed.Add(1) })
	t.Cleanup(stop)
	client := &http.Client{Timeout: 2 * time.Second}
	response, err := client.Get("http://" + address)
	if err != nil {
		t.Fatal(err)
	}
	response.Body.Close()
	if response.StatusCode != 204 {
		t.Fatal(response.Status)
	}
	stop()
	stop()
	if closed.Load() != 1 {
		t.Fatal("cleanup must run exactly once")
	}
	rebound, err := net.Listen("tcp", address)
	if err != nil {
		t.Fatalf("listener leaked: %v", err)
	}
	rebound.Close()
}

func TestStartWebAdminDatabaseFailureReleasesPort(t *testing.T) {
	t.Setenv("WYD_WEB_DATABASE_URL", "postgres://user:DO_NOT_LOG@127.0.0.1:invalid/db")
	cfg := adminTestConfig(t)
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	cfg.WebAdminAddress = listener.Addr().String()
	listener.Close()
	_, err = startWebAdmin(cfg, nil, nil)
	if err == nil || !strings.Contains(err.Error(), "banco do painel indisponivel") || strings.Contains(err.Error(), "DO_NOT_LOG") {
		t.Fatalf("database error not sanitized: %v", err)
	}
	rebound, err := net.Listen("tcp", cfg.WebAdminAddress)
	if err != nil {
		t.Fatalf("failed startup leaked listener: %v", err)
	}
	rebound.Close()
}

func TestServeWebAdminImmediateStop(t *testing.T) {
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	address := listener.Addr().String()
	stop := serveWebAdmin(listener, http.NotFoundHandler(), func() {})
	stop()
	rebound, err := net.Listen("tcp", address)
	if err != nil {
		t.Fatalf("immediate stop leaked listener: %v", err)
	}
	rebound.Close()
}

// Explicit opt-in: real database, read-only pool, no migrations or fixture writes.
func TestStartWebAdminPostgres(t *testing.T) {
	url := os.Getenv("WYD_WEB_TEST_DATABASE_URL")
	if url == "" {
		t.Skip("requires WYD_WEB_TEST_DATABASE_URL; read-only integration")
	}
	t.Setenv("WYD_WEB_DATABASE_URL", url)
	cfg := adminTestConfig(t)
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	cfg.WebAdminAddress = listener.Addr().String()
	listener.Close()
	source := adminSourceFunc(func(context.Context, control.Query) (control.Overview, error) {
		return control.Overview{Version: 1}, nil
	})
	stop, err := startWebAdmin(cfg, source, nil)
	if err != nil {
		t.Fatal(err)
	}
	defer stop()
	client := &http.Client{Timeout: 3 * time.Second}
	for path, want := range map[string]int{"/admin/": 200, "/healthz": 200, "/readyz": 200, "/api/v1/auth/session": 200, "/api/v1/staff/overview": 401} {
		response, err := client.Get("http://" + cfg.WebAdminAddress + path)
		if err != nil {
			t.Fatal(err)
		}
		io.Copy(io.Discard, response.Body)
		response.Body.Close()
		if response.StatusCode != want {
			t.Fatalf("%s: got %d, want %d", path, response.StatusCode, want)
		}
	}
}
