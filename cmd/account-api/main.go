// Comando account-api expoe somente cadastro/saude. Ele nao conhece protocolo,
// mundo, personagens ou gameplay; compartilha apenas a camada de contas/store.
package main

import (
	"errors"
	"flag"
	"log"
	"net/http"
	"time"

	"wydgo/internal/accountapi"
	"wydgo/internal/store"
)

func main() {
	addr := flag.String("addr", "127.0.0.1:8080", "endereco HTTP; publique via proxy HTTPS")
	accounts := flag.String("accounts", "data/accounts", "diretorio de contas compartilhado com o servidor")
	flag.Parse()

	server := &http.Server{
		Addr:              *addr,
		Handler:           accountapi.New(store.NewJSONStore(*accounts)),
		ReadHeaderTimeout: 5 * time.Second,
		ReadTimeout:       10 * time.Second,
		WriteTimeout:      15 * time.Second,
		IdleTimeout:       60 * time.Second,
		MaxHeaderBytes:    8 << 10,
	}
	log.Printf("WYD-Go Account API escutando em http://%s", *addr)
	if err := server.ListenAndServe(); err != nil && !errors.Is(err, http.ErrServerClosed) {
		log.Fatalf("account-api: %v", err)
	}
}
