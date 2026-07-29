// Comando account-api expoe somente cadastro/saude. Ele nao conhece protocolo,
// mundo, personagens ou gameplay; compartilha apenas a camada de contas/store.
package main

import (
	"context"
	"errors"
	"flag"
	"log"
	"net/http"
	"os"
	"time"

	"wydgo/internal/accountapi"
	"wydgo/internal/store"
)

func main() {
	addr := flag.String("addr", "127.0.0.1:8080", "endereco HTTP; publique via proxy HTTPS")
	databaseURLEnv := flag.String("database-url-env", "WYD_DATABASE_URL",
		"variavel de ambiente com a URL PostgreSQL")
	maxConns := flag.Int("database-max-conns", 4, "maximo de conexoes PostgreSQL")
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
	server := &http.Server{
		Addr:              *addr,
		Handler:           accountapi.New(st),
		ReadHeaderTimeout: 5 * time.Second,
		ReadTimeout:       10 * time.Second,
		WriteTimeout:      15 * time.Second,
		IdleTimeout:       60 * time.Second,
		MaxHeaderBytes:    8 << 10,
	}
	log.Printf("WYD-Go Account API escutando em http://%s", *addr)
	if err = server.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
		log.Fatalf("account-api: %v", err)
	}
}
