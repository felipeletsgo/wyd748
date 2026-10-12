// player-api is isolated from staff commands and game-state mutations.
package main

import (
	"context"
	"errors"
	"flag"
	"log/slog"
	"net/http"
	"os"
	"os/signal"
	"syscall"
	"time"
	"wydgo/internal/control"
	"wydgo/internal/playerportal"
)

func main() {
	if err := run(); err != nil {
		slog.Error("player-api stopped", "reason", err.Error())
		os.Exit(1)
	}
}
func run() error {
	addr := flag.String("addr", "127.0.0.1:8083", "literal loopback HTTP listener")
	origin := flag.String("origin", "", "exact public HTTPS origin")
	dev := flag.Bool("dev-http", false, "allow HTTP only on literal loopback for development")
	proxy := flag.Bool("trust-loopback-proxy", false, "trust a single X-Forwarded-For address from loopback Caddy")
	flag.Parse()
	if err := control.LoopbackAddress(*addr); err != nil {
		return err
	}
	databaseURL := os.Getenv("WYD_PLAYER_DATABASE_URL")
	if databaseURL == "" {
		return errors.New("WYD_PLAYER_DATABASE_URL is required (registration-only role)")
	}
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	db, err := playerportal.OpenDatabase(ctx, databaseURL)
	cancel()
	if err != nil {
		return err
	}
	defer db.Close()
	handler, err := playerportal.New(playerportal.Config{Origin: *origin, DevHTTP: *dev, TrustLoopbackProxy: *proxy, Backend: db})
	if err != nil {
		return err
	}
	srv := &http.Server{Addr: *addr, Handler: handler, ReadHeaderTimeout: 3 * time.Second, ReadTimeout: 5 * time.Second, WriteTimeout: 10 * time.Second, IdleTimeout: 30 * time.Second, MaxHeaderBytes: 8192}
	stopping, stop := signal.NotifyContext(context.Background(), os.Interrupt, syscall.SIGTERM)
	defer stop()
	finished := make(chan struct{})
	defer close(finished)
	go func() {
		select {
		case <-stopping.Done():
			ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
			defer cancel()
			_ = srv.Shutdown(ctx)
		case <-finished:
		}
	}()
	slog.Info("player-api started", "address", *addr, "origin", *origin)
	err = srv.ListenAndServe()
	if errors.Is(err, http.ErrServerClosed) {
		return nil
	}
	return err
}
