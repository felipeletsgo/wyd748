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
	"wydclient748/internal/assets"
	"wydclient748/internal/config"
	"wydclient748/internal/graphics/wgl"
	"wydclient748/internal/login"
	"wydclient748/internal/loginflow"
	"wydclient748/internal/platform/win32"
	"wydclient748/internal/protocol"
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
	serverAddress, err := config.ServerAddressFromEnv(cfg.ServerAddress)
	if err != nil {
		return err
	}
	var protectedSource assets.TextureSource
	protectedSettings, protectedEnabled, err := config.LoadProtectedAssetSettingsFromEnv()
	if err != nil {
		return fmt.Errorf("load protected asset settings: %w", err)
	}
	if protectedEnabled {
		packageData, err := os.ReadFile(protectedSettings.PackagePath)
		if err != nil {
			return fmt.Errorf("read protected asset package: %w", err)
		}
		protectedSource, err = assets.NewProtectedAssetCache(packageData, protectedSettings.ContentKey, protectedSettings.PublicKey)
		if err != nil {
			return fmt.Errorf("open protected asset package: %w", err)
		}
	}

	state := login.NewSessionState()
	var coordinator *loginflow.Coordinator
	renderer := wgl.New()
	options := app.Options{
		Title:    "WYD 7.48",
		Width:    cfg.WindowWidth,
		Height:   cfg.WindowHeight,
		LogoPath: initialLogoPath(),
		SceneFactories: loginflow.SceneFactoriesWithVisuals(state, loginflow.VisualOptions{
			ShapeRenderer: renderer,
			Authenticate: func(account string, password []byte) error {
				if coordinator == nil {
					return fmt.Errorf("login coordinator is not initialized")
				}
				return coordinator.Authenticate(account, password, [4]uint32{}, 0)
			},
			SelectCharacter: func(slot int32) error {
				if coordinator == nil {
					return fmt.Errorf("login coordinator is not initialized")
				}
				return coordinator.SelectCharacter(slot, 0, 0)
			},
		}),
		SessionEventsPerFrame: 64,
		SceneSynchronizer: func() error {
			if coordinator == nil {
				return nil
			}
			return coordinator.Synchronize()
		},
	}
	if serverAddress != "" {
		session := protocol.NewSession(serverAddress, protocol.SessionOptions{})
		options.Session = session
		options.SessionConnected = func() error {
			if coordinator == nil {
				return fmt.Errorf("login coordinator is not initialized")
			}
			return coordinator.SessionConnected()
		}
		options.SessionDisconnected = func() {
			if coordinator != nil {
				coordinator.SessionDisconnected()
			}
		}
		options.SessionEventHandler = func(event protocol.SessionEvent) error {
			if coordinator == nil {
				return fmt.Errorf("login coordinator is not initialized")
			}
			_, err := coordinator.HandleSessionEvent(event)
			return err
		}
	}
	if protectedEnabled {
		options.LogoPath = ""
		options.LogoSource = protectedSource
		options.LogoAssetPath = protectedSettings.LogoPath
	}
	client, err := app.New(
		options,
		win32.New(),
		renderer,
	)
	if err != nil {
		if protectedSource != nil {
			_ = protectedSource.Close()
		}
		return err
	}
	coordinator, err = loginflow.New(state, sessionSender(options.Session), client, loginflow.Options{})
	if err != nil {
		_ = client.Close()
		return err
	}
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt)
	defer stop()
	return client.Run(ctx)
}

func sessionSender(session protocol.Session) login.Sender {
	if sender, ok := session.(login.Sender); ok {
		return sender
	}
	return offlineSender{}
}

type offlineSender struct{}

func (offlineSender) Send([]byte, byte) error {
	return fmt.Errorf("login: network session is not configured")
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
