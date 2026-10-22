package main

import (
	"context"
	"crypto/sha256"
	"encoding/hex"
	"errors"
	"fmt"
	"io"
	"log"
	"log/slog"
	"net"
	"net/http"
	"os"
	"path/filepath"
	"runtime/debug"
	"sync"
	"time"

	"wydgo/internal/account"
	"wydgo/internal/control"
	"wydgo/internal/data"
	"wydgo/internal/logreader"
	"wydgo/internal/playerportal"
	"wydgo/internal/staffcontent"
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

func (c localAdminControl) AccountPresence(ctx context.Context, _ string, accounts []string) (map[string]bool, error) {
	source, ok := c.source.(control.AccountPresenceSource)
	if !ok {
		return nil, errors.New("account presence unavailable")
	}
	return source.AccountPresence(ctx, accounts)
}

func adminDatabaseURL(cfg data.ServerConfig) string {
	if url := os.Getenv("WYD_WEB_DATABASE_URL"); url != "" {
		return url
	}
	// Local-only convenience: a separate read-only store, not the game's writer.
	if cfg.DatabaseURL != "" {
		return cfg.DatabaseURL
	}
	return os.Getenv(cfg.DatabaseURLEnv)
}

// Failure disables only the panel. Never stop or replace the game process.
// adminInputs carries the static, boot-time panel data built in main.
type adminInputs struct {
	content *staffcontent.Index
	zones   *staffcontent.Zones
}

func startWebAdmin(cfg data.ServerConfig, source control.Source, registration account.RegistrationStore, inputs adminInputs) (func(), error) {
	if !cfg.WebAdminEnabled {
		return func() {}, nil
	}
	if err := control.LoopbackAddress(cfg.WebAdminAddress); err != nil {
		return nil, fmt.Errorf("web_admin_address: %w", err)
	}
	pin := cfg.AdminAccessPIN
	if cfg.AdminAccessPINEnv != "" {
		pin = os.Getenv(cfg.AdminAccessPINEnv)
	}
	if err := data.ValidateAdminAccessPIN(pin); err != nil {
		return nil, fmt.Errorf("admin_access_pin: %w", err)
	}
	trustedProxies, err := data.ParseTrustedProxies(cfg.WebAdminTrustedProxies)
	if err != nil {
		return nil, fmt.Errorf("web_admin_trusted_proxies: %w", err)
	}
	if cfg.DatabaseDriver != "postgres" {
		return nil, errors.New("integrated admin panel requires database_driver=postgres")
	}
	staff := func() (map[string]webadmin.Staff, error) { return webadmin.ReadStaff(cfg.WebAdminStaffPath) }
	if _, err := staff(); err != nil {
		return nil, fmt.Errorf("web_admin_staff: %w", err)
	}
	if info, err := os.Stat(filepath.Join(cfg.WebAdminStaticPath, "admin", "index.html")); err != nil || !info.Mode().IsRegular() {
		return nil, errors.New("admin panel build is missing; run tools/web-admin/Start-WYDAdmin.ps1 before starting tm.exe")
	}
	listener, err := net.Listen("tcp", cfg.WebAdminAddress)
	if err != nil {
		return nil, fmt.Errorf("admin panel port unavailable: %w", err)
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
		return nil, errors.New("admin panel database unavailable; check WYD_WEB_DATABASE_URL or the server PostgreSQL configuration")
	}
	defer func() {
		if !started {
			db.Close()
		}
	}()
	// A published panel keeps its loopback listener behind the HTTPS proxy;
	// the browser origin, Secure cookies and client IPs follow the proxy.
	origin, devHTTP := "http://"+cfg.WebAdminAddress, true
	if cfg.WebAdminOrigin != "" {
		origin, devHTTP = cfg.WebAdminOrigin, false
	}
	var verification *playerportal.Turnstile
	if !devHTTP {
		verification, err = playerportal.NewTurnstileAction(os.Getenv("WYD_TURNSTILE_SECRET"), origin, "staff_login")
		if err != nil {
			return nil, err
		}
	}
	events, _ := source.(control.GlobalDropSource)
	quiz, _ := source.(control.QuizSource)
	bosses, _ := source.(control.BossesSource)
	pvpDoor, _ := source.(control.PvPDoorSource)
	oxEvent, _ := source.(control.OXEventSource)
	colosseum, _ := source.(control.ColosseumSource)
	kick, _ := source.(control.KickSource)
	teleport, _ := source.(control.TeleportSource)
	premium, _ := source.(control.PremiumSource)
	sanctions, _ := source.(control.SanctionSource)
	notices, _ := source.(control.NoticeSource)
	shutdown, _ := source.(control.ShutdownSource)
	// Audit rows use the game's writer store: the panel database role may be
	// read-only. The stderr audit log remains when it is unavailable.
	auditStore, _ := registration.(control.AuditStore)
	accountHistory, _ := registration.(control.AccountHistoryStore)
	accountNotes, _ := registration.(control.AccountNotesStore)
	worldReads, _ := source.(control.WorldReadSource)
	var logReader webadmin.LogReader
	if cfg.WebAdminLogReaderSocket != "" {
		logReader = logreader.Client{Path: cfg.WebAdminLogReaderSocket}
	}
	var itemName func(uint16) string
	if items, ok := source.(interface{ ItemName(uint16) string }); ok {
		itemName = items.ItemName
	}
	handler, err := webadmin.New(webadmin.Config{
		Origin: origin, DevHTTP: devHTTP, AdminAccessPIN: pin, TrustedProxies: trustedProxies,
		RequireLoginProtection: !devHTTP, LoginVerifier: verification, AuthenticationLimiter: db.AuthenticationLimiter(),
		Staff: staff, Accounts: db, Registration: registration, Control: localAdminControl{source: source, started: time.Now()},
		Persistent: webadmin.NewAccountPersistentReader(db), Ready: db.Ping,
		GlobalDrop: events,
		Quiz:       quiz,
		Bosses:     bosses,
		PvPDoor:    pvpDoor,
		OXEvent:    oxEvent,
		Colosseum:  colosseum,
		Kick:       kick,
		Teleport:   teleport,
		Premium:    premium,
		Sanctions:  sanctions,
		Notices:    notices,
		Shutdown:   shutdown,
		AuditStore: auditStore,
		Metrics:    control.ReadRuntime,
		// Login history and staff actions use the writer store like the audit;
		// saved storage is read through the panel's read-only pool.
		AccountHistory: accountHistory,
		Storage:        db,
		ItemName:       itemName,
		Logs:           logReader,
		Content:        inputs.content,
		Zones:          inputs.zones,
		WorldReads:     worldReads,
		Notes:          accountNotes,
		Audit:          slog.New(slog.NewJSONHandler(os.Stderr, nil)),
	})
	if err != nil {
		return nil, err
	}
	site, err := handler.Site(cfg.WebAdminStaticPath)
	if err != nil {
		handler.Close()
		return nil, err
	}
	stop := serveWebAdmin(listener, site, func() { handler.Close(); db.Close() })
	started = true
	handler.RecordServerStart(append(serverBuildIdentity(), "channel_id", cfg.ChannelID)...)
	log.Printf("Admin panel: %s/admin/ (listener %s; account password + PIN; permissions from %s)",
		origin, cfg.WebAdminAddress, cfg.WebAdminStaffPath)
	return stop, nil
}

// serverBuildIdentity describes the running binary for the boot receipt: Go
// and VCS stamps when the build has them, plus the executable's SHA-256.
func serverBuildIdentity() []any {
	attrs := []any{"pid", os.Getpid()}
	if info, ok := debug.ReadBuildInfo(); ok {
		attrs = append(attrs, "go_version", info.GoVersion)
		for _, setting := range info.Settings {
			switch setting.Key {
			case "vcs.revision", "vcs.time", "vcs.modified":
				attrs = append(attrs, setting.Key, setting.Value)
			}
		}
	}
	if path, err := os.Executable(); err == nil {
		if file, err := os.Open(path); err == nil {
			hash := sha256.New()
			if _, err := io.Copy(hash, file); err == nil {
				attrs = append(attrs, "executable_sha256", hex.EncodeToString(hash.Sum(nil)))
			}
			_ = file.Close()
		}
	}
	return attrs
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
			log.Printf("admin panel unavailable: %v; game server remains active", err)
			stop()
		}
	}()
	return stop
}
