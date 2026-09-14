// web-api is the local-first staff console. It never migrates or writes game data.
package main

import (
	"context"
	"errors"
	"flag"
	"fmt"
	"io"
	"log/slog"
	"net/http"
	"net/netip"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/data"
	"wydgo/internal/store"
	"wydgo/internal/webadmin"
)

func main() {
	if err := run(); err != nil {
		slog.Error("web-api stopped", "reason", err.Error())
		os.Exit(1)
	}
}
func run() error {
	address := flag.String("addr", "127.0.0.1:8082", "literal loopback listener")
	origin := flag.String("origin", "", "exact browser origin, without trailing slash")
	dev := flag.Bool("dev-http", false, "allow HTTP on literal loopback for local development only")
	configPath := flag.String("config", "data/server.txt", "server configuration containing admin_access_pin")
	staffPath := flag.String("staff-file", "", "private staff JSON outside repository and static directory")
	controlURL := flag.String("control-url", "http://127.0.0.1:8081", "private game Control API")
	staticPath := flag.String("static", "web/portal/dist", "compiled Astro assets directory")
	trustedProxyRaw := flag.String("trusted-proxies", os.Getenv("WYD_WEB_TRUSTED_PROXIES"), "comma-separated trusted proxy CIDRs allowed to supply X-Forwarded-For")
	auditPath := flag.String("audit-file", os.Getenv("WYD_WEB_AUDIT_LOG"), "optional append-only JSON audit log path; stderr is always retained")
	flag.Parse()
	if err := control.LoopbackAddress(*address); err != nil {
		return err
	}
	if *origin == "" || *staffPath == "" {
		return errors.New("origin and staff-file are required")
	}
	serverConfig, err := data.LoadServerConfig(*configPath)
	if err != nil {
		return err
	}
	if serverConfig.AdminAccessPIN == "" {
		return errors.New("admin_access_pin is required in server config")
	}
	if err := data.ValidateAdminAccessPIN(serverConfig.AdminAccessPIN); err != nil {
		return fmt.Errorf("admin_access_pin: %w", err)
	}
	trustedProxies, err := parseTrustedProxies(*trustedProxyRaw)
	if err != nil {
		return err
	}
	staff := func() (map[string]webadmin.Staff, error) { return webadmin.ReadStaff(*staffPath) }
	if _, err := staff(); err != nil {
		return err
	}
	client, err := control.NewClient(*controlURL, os.Getenv("WYD_CONTROL_TOKEN"))
	if err != nil {
		return err
	}
	databaseURL := os.Getenv("WYD_WEB_DATABASE_URL")
	if databaseURL == "" {
		return errors.New("WYD_WEB_DATABASE_URL is required (dedicated read-only PostgreSQL role)")
	}
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	db, err := store.NewPostgresReadOnlyStore(ctx, store.PostgresConfig{URL: databaseURL, MaxConns: 2, OperationTimeout: 3 * time.Second})
	cancel()
	if err != nil {
		return errors.New("read-only account database unavailable")
	}
	defer db.Close()
	auditWriter := io.Writer(os.Stderr)
	var auditFile *os.File
	if strings.TrimSpace(*auditPath) != "" {
		auditFile, err = os.OpenFile(*auditPath, os.O_CREATE|os.O_APPEND|os.O_WRONLY, 0o600)
		if err != nil {
			return fmt.Errorf("open audit file: %w", err)
		}
		defer auditFile.Close()
		auditWriter = io.MultiWriter(os.Stderr, auditFile)
	}
	audit := slog.New(slog.NewJSONHandler(auditWriter, nil))
	handler, err := webadmin.New(webadmin.Config{Origin: *origin, DevHTTP: *dev, AdminAccessPIN: serverConfig.AdminAccessPIN, Staff: staff, Accounts: db, Control: client, Persistent: webadmin.NewAccountPersistentReader(db), Ready: db.Ping, Audit: audit, TrustedProxies: trustedProxies})
	if err != nil {
		return err
	}
	site, err := handler.Site(*staticPath)
	if err != nil {
		return err
	}
	server := &http.Server{Addr: *address, Handler: site, ReadHeaderTimeout: 3 * time.Second, ReadTimeout: 5 * time.Second, WriteTimeout: 10 * time.Second, IdleTimeout: 30 * time.Second, MaxHeaderBytes: 8192}
	signalCtx, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()
	stopped := make(chan struct{})
	defer close(stopped)
	go func() {
		select {
		case <-signalCtx.Done():
			ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
			defer cancel()
			_ = server.Shutdown(ctx)
		case <-stopped:
		}
	}()
	audit.Info("web_admin_started", "address", *address, "mode", "local-first read-only", "devHTTP", *dev)
	if err := server.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
		return errors.New("web listener failed")
	}
	return nil
}

func parseTrustedProxies(raw string) ([]netip.Prefix, error) {
	raw = strings.TrimSpace(raw)
	if raw == "" {
		return nil, nil
	}
	parts := strings.Split(raw, ",")
	prefixes := make([]netip.Prefix, 0, len(parts))
	for _, part := range parts {
		value := strings.TrimSpace(part)
		prefix, err := netip.ParsePrefix(value)
		if err != nil {
			return nil, fmt.Errorf("invalid trusted proxy CIDR %q: %w", value, err)
		}
		prefixes = append(prefixes, prefix.Masked())
	}
	return prefixes, nil
}
