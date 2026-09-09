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
	serverEntries, err := config.ServerEntriesFromEnv(displayServerAddress(cfg.ServerAddress, serverAddress))
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
	loginTexture, err := assets.LoadWYTFile(loginTexturePath())
	if err != nil {
		return fmt.Errorf("load official login UI: %w", err)
	}
	loginLogoLeft, err := assets.LoadWYTFile(loginLogoLeftPath())
	if err != nil {
		return fmt.Errorf("load official left login logo: %w", err)
	}
	loginLogoRight, err := assets.LoadWYTFile(loginLogoRightPath())
	if err != nil {
		return fmt.Errorf("load official right login logo: %w", err)
	}
	serverTexture, err := assets.LoadWYTFile(serverSelectionTexturePath())
	if err != nil {
		return fmt.Errorf("load official server selection UI: %w", err)
	}
	var client *app.Application
	selectedAddress := displayServerAddress(cfg.ServerAddress, serverAddress)
	session := protocol.NewSession(selectedAddress, protocol.SessionOptions{})
	options := app.Options{
		Title:               "WYD 7.48",
		Width:               cfg.WindowWidth,
		Height:              cfg.WindowHeight,
		InitialSceneID:      loginflow.ServerSelectionSceneID,
		DeferSessionConnect: true,
		LogoPath:            "",
		SceneFactories: loginflow.SceneFactoriesWithVisuals(state, loginflow.VisualOptions{
			ShapeRenderer: renderer,
			ServerTexture: &serverTexture,
			Servers:       toLoginServerEntries(serverEntries),
			RequestClose: func() error {
				if client == nil {
					return fmt.Errorf("client application is not initialized")
				}
				return client.RequestClose()
			},
			SelectServer: func(entry loginflow.ServerEntry) error {
				if client == nil {
					return fmt.Errorf("client application is not initialized")
				}
				if err := session.SetAddress(entry.Address); err != nil {
					return err
				}
				if coordinator == nil {
					return fmt.Errorf("login coordinator is not initialized")
				}
				return coordinator.ServerSelected()
			},
			LoginTexture:   &loginTexture,
			LoginLogoLeft:  &loginLogoLeft,
			LoginLogoRight: &loginLogoRight,
			Authenticate: func(account string, password []byte) error {
				if coordinator == nil {
					return fmt.Errorf("login coordinator is not initialized")
				}
				// TMSelectServerScene connects only from B_LOGIN_OK, after the
				// endpoint has been selected and the account form is validated.
				if err := client.ConnectSession(); err != nil {
					return err
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
		Session: session,
		SessionConnected: func() error {
			if coordinator == nil {
				return fmt.Errorf("login coordinator is not initialized")
			}
			return coordinator.SessionConnected()
		},
		SessionDisconnected: func() {
			if coordinator != nil {
				coordinator.SessionDisconnected()
			}
		},
		SessionEventHandler: func(event protocol.SessionEvent) error {
			if coordinator == nil {
				return fmt.Errorf("login coordinator is not initialized")
			}
			_, err := coordinator.HandleSessionEvent(event)
			return err
		},
	}
	if protectedEnabled {
		options.LogoPath = ""
		options.LogoSource = protectedSource
		options.LogoAssetPath = protectedSettings.LogoPath
	}
	client, err = app.New(
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
	coordinator, err = loginflow.New(state, session, client, loginflow.Options{})
	if err != nil {
		_ = client.Close()
		return err
	}
	ctx, stop := signal.NotifyContext(context.Background(), os.Interrupt)
	defer stop()
	return client.Run(ctx)
}

func displayServerAddress(defaultAddress, configuredAddress string) string {
	if configuredAddress != "" {
		return configuredAddress
	}
	return defaultAddress
}

func toLoginServerEntries(entries []config.ServerEntry) []loginflow.ServerEntry {
	converted := make([]loginflow.ServerEntry, len(entries))
	for i, entry := range entries {
		converted[i] = loginflow.ServerEntry{Name: entry.Name, Address: entry.Address}
	}
	return converted
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

func loginTexturePath() string {
	if executable, err := os.Executable(); err == nil {
		candidate := filepath.Clean(filepath.Join(filepath.Dir(executable), "..", "assets", "current", "UI", "loginbox2.wyt"))
		if _, err := os.Stat(candidate); err == nil {
			return candidate
		}
	}
	return filepath.Join("assets", "current", "UI", "loginbox2.wyt")
}

func serverSelectionTexturePath() string {
	if executable, err := os.Executable(); err == nil {
		candidate := filepath.Clean(filepath.Join(filepath.Dir(executable), "..", "assets", "current", "UI", "ServerList2.wyt"))
		if _, err := os.Stat(candidate); err == nil {
			return candidate
		}
	}
	return filepath.Join("assets", "current", "UI", "ServerList2.wyt")
}

func loginLogoLeftPath() string {
	return loginAssetPath("logo1.wyt")
}

func loginLogoRightPath() string {
	return loginAssetPath("logo2.wyt")
}

func loginAssetPath(name string) string {
	if executable, err := os.Executable(); err == nil {
		candidate := filepath.Clean(filepath.Join(filepath.Dir(executable), "..", "assets", "current", "UI", name))
		if _, err := os.Stat(candidate); err == nil {
			return candidate
		}
	}
	return filepath.Join("assets", "current", "UI", name)
}
