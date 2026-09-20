package main

import (
	"context"
	"errors"
	"fmt"
	"log"
	"log/slog"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"sync"
	"time"

	"wydgo/internal/control"
	"wydgo/internal/data"
	"wydgo/internal/store"
	"wydgo/internal/webadmin"
)

// MODERNIZACAO_COMPATIVEL: same read-only World queue, without a local HTTP hop.
// Authentication, capabilities and actor auditing remain in the web handler.
type localAdminControl struct {
	source  control.Source
	started time.Time
}

func (c localAdminControl) Overview(ctx context.Context, _ string, q control.Query) (control.Overview, error) {
	result, err := c.source.Overview(ctx, q)
	if err != nil {
		return result, err
	}
	return control.WithProcessDiagnostics(result, c.started), nil
}

func adminDatabaseURL(cfg data.ServerConfig) string {
	if url := os.Getenv("WYD_WEB_DATABASE_URL"); url != "" {
		return url
	}
	// Local-only convenience: a separate read-only store, not the game's writer.
	return configuredDatabaseURL(cfg)
}

// Failure disables only the panel. Never stop or replace the game process.
func startWebAdmin(cfg data.ServerConfig, source control.Source) (func(), error) {
	if !cfg.WebAdminEnabled {
		return func() {}, nil
	}
	if err := control.LoopbackAddress(cfg.WebAdminAddress); err != nil {
		return nil, fmt.Errorf("web_admin_address: %w", err)
	}
	if err := data.ValidateAdminAccessPIN(cfg.AdminAccessPIN); err != nil {
		return nil, fmt.Errorf("admin_access_pin: %w", err)
	}
	if cfg.DatabaseDriver != "postgres" {
		return nil, errors.New("painel integrado requer database_driver=postgres")
	}
	staff := func() (map[string]webadmin.Staff, error) { return webadmin.ReadStaff(cfg.WebAdminStaffPath) }
	if _, err := staff(); err != nil {
		return nil, fmt.Errorf("web_admin_staff: %w", err)
	}
	if info, err := os.Stat(filepath.Join(cfg.WebAdminStaticPath, "admin", "index.html")); err != nil || !info.Mode().IsRegular() {
		return nil, errors.New("build do painel ausente; execute tools/web-admin/Start-WYDAdmin.ps1 antes de abrir tm.exe")
	}
	listener, err := net.Listen("tcp", cfg.WebAdminAddress)
	if err != nil {
		return nil, fmt.Errorf("porta do painel indisponivel: %w", err)
	}
	started := false
	defer func() {
		if !started {
			_ = listener.Close()
		}
	}()
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	db, err := store.NewPostgresReadOnlyStore(ctx, store.PostgresConfig{
		URL: adminDatabaseURL(cfg), MaxConns: 2, OperationTimeout: 3 * time.Second,
	})
	cancel()
	if err != nil {
		// Do not print driver errors: they may contain database credentials.
		return nil, errors.New("banco do painel indisponivel; verifique WYD_WEB_DATABASE_URL ou a configuracao PostgreSQL do servidor")
	}
	defer func() {
		if !started {
			db.Close()
		}
	}()
	origin := "http://" + cfg.WebAdminAddress
	events, _ := source.(control.GlobalDropSource)
	quiz, _ := source.(control.QuizSource)
	bosses, _ := source.(control.BossesSource)
	kick, _ := source.(control.KickSource)
	teleport, _ := source.(control.TeleportSource)
	handler, err := webadmin.New(webadmin.Config{
		Origin: origin, DevHTTP: true, AdminAccessPIN: cfg.AdminAccessPIN,
		Staff: staff, Accounts: db, Control: localAdminControl{source: source, started: time.Now()},
		Persistent: webadmin.NewAccountPersistentReader(db), Ready: db.Ping,
		GlobalDrop: events,
		Quiz:       quiz,
		Bosses:     bosses,
		Kick:       kick,
		Teleport:   teleport,
		Audit:      slog.New(slog.NewJSONHandler(os.Stderr, nil)),
	})
	if err != nil {
		return nil, err
	}
	site, err := handler.Site(cfg.WebAdminStaticPath)
	if err != nil {
		return nil, err
	}
	stop := serveWebAdmin(listener, site, db.Close)
	started = true
	log.Printf("Painel administrativo: %s/admin/ (local; senha da conta + PIN; operacoes conforme staff.json)", origin)
	return stop, nil
}

func serveWebAdmin(listener net.Listener, handler http.Handler, cleanup func()) func() {
	server := &http.Server{Handler: handler, ReadHeaderTimeout: 3 * time.Second,
		ReadTimeout: 5 * time.Second, WriteTimeout: 10 * time.Second,
		IdleTimeout: 30 * time.Second, MaxHeaderBytes: 8192}
	var once sync.Once
	stop := func() {
		once.Do(func() {
			// Also cover shutdown before the Serve goroutine registers its listener.
			defer listener.Close()
			ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
			defer cancel()
			if err := server.Shutdown(ctx); err != nil {
				_ = server.Close()
			}
			cleanup()
		})
	}
	go func() {
		if err := server.Serve(listener); err != nil && !errors.Is(err, http.ErrServerClosed) {
			log.Printf("painel administrativo indisponivel: %v; servidor do jogo continua ativo", err)
			stop()
		}
	}()
	return stop
}
