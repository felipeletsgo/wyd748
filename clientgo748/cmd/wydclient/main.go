// Command wydclient compõe o primeiro processo gráfico real do client Go 7.48.
// A janela e o contexto já são concretos; scene, rede e assets permanecem fora
// até que seus contratos nativos sejam rastreados e implementados.
package main

import (
	"context"
	"fmt"
	"os"
	"os/signal"

	"wydclient748/internal/app"
	"wydclient748/internal/config"
	"wydclient748/internal/graphics/wgl"
	"wydclient748/internal/platform/win32"
)

func main() {
	if err := run(); err != nil {
		fmt.Fprintln(os.Stderr, "WYD Client initialization failed:", err)
		os.Exit(1)
	}
}

func run() error {
	cfg := config.Default()
	if err := cfg.Validate(); err != nil {
		return err
	}

	client, err := app.New(
		app.Options{Title: "WYD 7.48", Width: cfg.WindowWidth, Height: cfg.WindowHeight},
		win32.New(),
		wgl.New(),
	)
	if err != nil {
		return err
	}
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt)
	defer stop()
	return client.Run(ctx)
}
