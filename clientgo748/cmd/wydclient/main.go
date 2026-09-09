// Command wydclient compõe o primeiro processo gráfico real do client Go 7.48.
// A janela, o contexto e a primeira cena de textura já são concretos; rede e
// cenas de gameplay serão adicionadas somente após seus contratos nativos.
package main

import (
	"context"
	"fmt"
	"os"
	"os/signal"
	"path/filepath"

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
		app.Options{
			Title:    "WYD 7.48",
			Width:    cfg.WindowWidth,
			Height:   cfg.WindowHeight,
			LogoPath: initialLogoPath(),
		},
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

func initialLogoPath() string {
	if executable, err := os.Executable(); err == nil {
		candidate := filepath.Clean(filepath.Join(filepath.Dir(executable), "..", "CLIENT OFICIAL 7.48", "UI", "logo1.wyt"))
		if _, err := os.Stat(candidate); err == nil {
			return candidate
		}
	}
	return filepath.Join("CLIENT OFICIAL 7.48", "UI", "logo1.wyt")
}
