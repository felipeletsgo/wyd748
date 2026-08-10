// Comando account-api expoe somente cadastro/saude. Ele nao conhece protocolo,
// mundo, personagens ou gameplay; compartilha apenas a camada de contas/store.
package main

import (
	"context"
	"errors"
	"flag"
	"log"
	"net/http"
	"net/netip"
	"os"
	"os/signal"
	"strings"
	"syscall"
	"time"

	"wydgo/internal/accountapi"
	"wydgo/internal/store"
)

func main() {
	addr := flag.String("addr", "127.0.0.1:8080", "endereco HTTP; publique via proxy HTTPS")
	databaseURLEnv := flag.String("database-url-env", "WYD_DATABASE_URL",
		"variavel de ambiente com a URL PostgreSQL")
	maxConns := flag.Int("database-max-conns", 4, "maximo de conexoes PostgreSQL")
	requestsPerMinute := flag.Int("requests-per-minute", 10, "limite de cadastros por IP/minuto")
	hashConcurrency := flag.Int("hash-concurrency", 4, "maximo de hashes PBKDF2 simultaneos")
	trustedProxyList := flag.String("trusted-proxies", "127.0.0.0/8,::1/128",
		"prefixos CIDR de proxies que podem definir X-Forwarded-For")
	flag.Parse()

	databaseURL := os.Getenv(*databaseURLEnv)
	if databaseURL == "" {
		log.Fatalf("account-api: variavel %s esta vazia", *databaseURLEnv)
	}
	st, err := store.NewPostgresStore(context.Background(), store.PostgresConfig{
		URL: databaseURL, MaxConns: int32(*maxConns),
	})
	if err != nil {
		log.Fatalf("account-api: abrir PostgreSQL: %v", err)
	}
	defer st.Close()
	trustedProxies := make([]netip.Prefix, 0)
	for _, raw := range strings.Split(*trustedProxyList, ",") {
		raw = strings.TrimSpace(raw)
		if raw == "" {
			continue
		}
		prefix, parseErr := netip.ParsePrefix(raw)
		if parseErr != nil {
			log.Fatalf("account-api: trusted proxy %q: %v", raw, parseErr)
		}
		trustedProxies = append(trustedProxies, prefix)
	}
	server := &http.Server{
		Addr: *addr,
		Handler: accountapi.NewWithConfig(st, accountapi.Config{
			RequestsPerMinute: *requestsPerMinute, HashConcurrency: *hashConcurrency,
			TrustedProxies: trustedProxies, Ready: st.Ping,
		}),
		ReadHeaderTimeout: 5 * time.Second,
		ReadTimeout:       10 * time.Second,
		WriteTimeout:      15 * time.Second,
		IdleTimeout:       60 * time.Second,
		MaxHeaderBytes:    8 << 10,
	}
	log.Printf("WYD-Go Account API escutando em http://%s", *addr)
	errCh := make(chan error, 1)
	go func() { errCh <- server.ListenAndServe() }()
	stop := make(chan os.Signal, 1)
	signal.Notify(stop, os.Interrupt, syscall.SIGTERM)
	select {
	case err = <-errCh:
		if err != nil && !errors.Is(err, http.ErrServerClosed) {
			log.Fatalf("account-api: %v", err)
		}
	case <-stop:
		ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
		defer cancel()
		if err = server.Shutdown(ctx); err != nil {
			log.Printf("account-api shutdown: %v", err)
		}
	}
}
