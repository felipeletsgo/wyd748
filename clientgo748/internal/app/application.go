// Package app orquestra recursos do processo sem conhecer Win32 ou OpenGL.
// Dependências injetáveis permitem provar aquisição, falha e teardown sem
// criar handles reais nos testes.
package app

import (
	"context"
	"errors"
	"fmt"
	"runtime"
	"sync"
	"time"

	"wydclient748/internal/assets"
	"wydclient748/internal/graphics"
	"wydclient748/internal/input"
	"wydclient748/internal/platform"
	"wydclient748/internal/protocol"
	"wydclient748/internal/scene"
)

// Options contém somente a fronteira observável do primeiro bootstrap.
type Options struct {
	Title  string
	Width  int
	Height int
	// LogoPath is optional for headless lifecycle tests. When set, the first
	// scene loads this official texture before entering the frame loop.
	LogoPath string
	// LogoSource is an optional authenticated source for the first texture.
	// Application takes ownership and closes it even when a later bootstrap
	// stage fails. LogoAssetPath is a manifest path, not a filesystem path.
	LogoSource    assets.TextureSource
	LogoAssetPath string
	// InitialScene permite registrar uma cena de teste ou uma tela futura sem
	// acoplar Application ao conteúdo de gameplay. Quando omitido, o logo
	// carregado vira a cena estática inicial.
	InitialScene   scene.Factory
	InitialSceneID scene.ID
	// SceneFactories contém todas as cenas que podem ser solicitadas durante o
	// processo. O mapa é copiado pelo Manager; o chamador continua dono apenas
	// das factories, não das instâncias criadas.
	SceneFactories map[scene.ID]scene.Factory
	// Session is optional during the graphical bootstrap. When configured,
	// Application owns its lifecycle: it connects before the first scene and
	// closes it before any scene or renderer resource is torn down.
	Session protocol.Session
	// SessionConnected executa na thread principal imediatamente depois de o
	// handshake concluir. SessionDisconnected executa depois de Close sempre
	// que a conexão chegou a ficar ativa, inclusive em falha parcial posterior.
	SessionConnected    func() error
	SessionDisconnected func()
	// SessionEventHandler é executado na thread principal para cada evento já
	// recebido. Quando configurado, Session deve implementar EventSession.
	SessionEventHandler   func(protocol.SessionEvent) error
	SessionEventsPerFrame int
	// SceneSynchronizer runs after session and input dispatch, before scene
	// update, so packets received in one frame resolve to one final scene.
	SceneSynchronizer func() error
}

// Application possui janela e renderer depois que cada estágio conclui.
type Application struct {
	options         Options
	window          platform.Window
	renderer        graphics.Renderer
	textureRenderer graphics.TextureRenderer

	windowOwned   bool
	rendererOwned bool
	sourceOwned   bool
	sessionOwned  bool
	sessionActive bool
	eventSession  protocol.EventSession
	sceneManager  *scene.Manager
	closeOnce     sync.Once
	closeErr      error
}

// New valida dependências antes que qualquer recurso externo seja criado.
func New(options Options, window platform.Window, renderer graphics.Renderer) (*Application, error) {
	if options.Title == "" {
		return nil, fmt.Errorf("clientgo748: application title is required")
	}
	if options.Width <= 0 || options.Height <= 0 {
		return nil, fmt.Errorf("clientgo748: invalid application size %dx%d", options.Width, options.Height)
	}
	if window == nil {
		return nil, fmt.Errorf("clientgo748: window dependency is required")
	}
	if renderer == nil {
		return nil, fmt.Errorf("clientgo748: renderer dependency is required")
	}
	if options.LogoSource != nil && options.LogoAssetPath == "" {
		return nil, fmt.Errorf("clientgo748: logo asset path is required when a source is configured")
	}
	if (options.SessionConnected != nil || options.SessionDisconnected != nil) && options.Session == nil {
		return nil, fmt.Errorf("clientgo748: session is required for session lifecycle callbacks")
	}
	var eventSession protocol.EventSession
	if options.SessionEventHandler != nil {
		if options.Session == nil {
			return nil, fmt.Errorf("clientgo748: session is required for a session event handler")
		}
		var ok bool
		eventSession, ok = options.Session.(protocol.EventSession)
		if !ok {
			return nil, fmt.Errorf("clientgo748: configured session does not provide events")
		}
		if options.SessionEventsPerFrame < 0 {
			return nil, fmt.Errorf("clientgo748: invalid session events per frame %d", options.SessionEventsPerFrame)
		}
	}
	return &Application{
		options:      options,
		window:       window,
		renderer:     renderer,
		sourceOwned:  options.LogoSource != nil,
		sessionOwned: options.Session != nil,
		eventSession: eventSession,
	}, nil
}

// Run mantém criação, frames e destruição na mesma thread do sistema, requisito
// de Win32/WGL. O retorno sempre inclui eventual erro de teardown.
func (a *Application) Run(ctx context.Context) (err error) {
	runtime.LockOSThread()
	defer runtime.UnlockOSThread()
	defer func() { err = errors.Join(err, a.Close()) }()

	if err := a.window.Open(a.options.Title, a.options.Width, a.options.Height); err != nil {
		return fmt.Errorf("clientgo748: initialize window: %w", err)
	}
	a.windowOwned = true
	if err := a.renderer.Initialize(a.window.Handle()); err != nil {
		return fmt.Errorf("clientgo748: initialize renderer: %w", err)
	}
	a.rendererOwned = true
	if a.options.Session != nil {
		if err := a.options.Session.Connect(); err != nil {
			return fmt.Errorf("clientgo748: connect session: %w", err)
		}
		a.sessionActive = true
		if a.options.SessionConnected != nil {
			if err := a.options.SessionConnected(); err != nil {
				return fmt.Errorf("clientgo748: initialize connected session state: %w", err)
			}
		}
	}
	if a.eventSession != nil {
		if err := a.eventSession.StartReceiving(); err != nil {
			return fmt.Errorf("clientgo748: start session receiver: %w", err)
		}
	}
	if a.options.LogoSource != nil || a.options.LogoPath != "" {
		textureRenderer, ok := a.renderer.(graphics.TextureRenderer)
		if !ok {
			return fmt.Errorf("clientgo748: renderer does not support the initial texture scene")
		}
		var texture assets.Texture
		if a.options.LogoSource != nil {
			texture, err = a.options.LogoSource.LoadTexture(a.options.LogoAssetPath)
			if err != nil {
				return fmt.Errorf("clientgo748: load protected initial logo: %w", err)
			}
		} else {
			texture, err = assets.LoadWYTFile(a.options.LogoPath)
			if err != nil {
				return fmt.Errorf("clientgo748: load initial logo: %w", err)
			}
		}
		if err := textureRenderer.UploadTexture(texture); err != nil {
			return fmt.Errorf("clientgo748: upload initial logo: %w", err)
		}
		a.textureRenderer = textureRenderer
	}
	if a.options.InitialScene != nil || a.textureRenderer != nil || len(a.options.SceneFactories) > 0 {
		initialID := a.options.InitialSceneID
		if initialID == "" {
			initialID = scene.ID("initial")
		}
		factories := make(map[scene.ID]scene.Factory, len(a.options.SceneFactories)+1)
		for id, factory := range a.options.SceneFactories {
			factories[id] = factory
		}
		if a.options.InitialScene != nil {
			factories[initialID] = a.options.InitialScene
		} else if a.textureRenderer != nil {
			textureRenderer := a.textureRenderer
			factories[initialID] = func() (scene.Scene, error) {
				return &textureScene{id: initialID, renderer: textureRenderer}, nil
			}
		}
		a.sceneManager = scene.New(factories)
		if err := a.sceneManager.Start(initialID); err != nil {
			return fmt.Errorf("clientgo748: start initial scene: %w", err)
		}
	}

	lastFrame := time.Now()
	for !a.window.ShouldClose() {
		select {
		case <-ctx.Done():
			return nil
		default:
		}
		events := a.window.PollEvents()
		if err := a.dispatchSessionEvents(); err != nil {
			return err
		}
		if a.sceneManager != nil {
			if err := a.sceneManager.Dispatch(events); err != nil {
				return err
			}
			if a.options.SceneSynchronizer != nil {
				if err := a.options.SceneSynchronizer(); err != nil {
					return fmt.Errorf("clientgo748: synchronize scene: %w", err)
				}
			}
		}
		if a.window.ShouldClose() {
			break
		}
		now := time.Now()
		delta := now.Sub(lastFrame)
		lastFrame = now
		a.renderer.BeginFrame()
		if a.sceneManager != nil {
			if err := a.sceneManager.Update(delta); err != nil {
				return err
			}
			if err := a.sceneManager.Render(); err != nil {
				return err
			}
		} else if a.textureRenderer != nil {
			a.textureRenderer.DrawTexture()
		}
		a.renderer.EndFrame()
	}
	return nil
}

func (a *Application) dispatchSessionEvents() error {
	if a.eventSession == nil {
		return nil
	}
	limit := a.options.SessionEventsPerFrame
	if limit == 0 {
		limit = 64
	}
	for _, event := range a.eventSession.DrainEvents(limit) {
		if err := a.options.SessionEventHandler(event); err != nil {
			return fmt.Errorf("clientgo748: handle session event: %w", err)
		}
	}
	return nil
}

// RequestScene agenda uma troca de cena para o próximo frame. Deve ser
// chamado no mesmo thread que executa Run; a API não cria uma segunda fila
// concorrente nem transfere ownership ao chamador.
func (a *Application) RequestScene(id scene.ID) error {
	if a.sceneManager == nil {
		return fmt.Errorf("clientgo748: scene manager is not initialized")
	}
	return a.sceneManager.Request(id)
}

// CurrentScene expõe apenas o identificador da cena, nunca a instância ou
// seus recursos.
func (a *Application) CurrentScene() (scene.ID, bool) {
	if a.sceneManager == nil {
		return "", false
	}
	return a.sceneManager.Current()
}

// Close libera somente recursos cuja inicialização transferiu ownership. A
// sessão é encerrada antes das cenas e do renderer; chamadas repetidas
// retornam o mesmo erro.
func (a *Application) Close() error {
	a.closeOnce.Do(func() {
		var errs []error
		if a.sessionOwned {
			a.sessionOwned = false
			if err := a.options.Session.Close(); err != nil {
				errs = append(errs, fmt.Errorf("clientgo748: close session: %w", err))
			}
		}
		if a.sessionActive {
			a.sessionActive = false
			if a.options.SessionDisconnected != nil {
				a.options.SessionDisconnected()
			}
		}
		if a.sceneManager != nil {
			if err := a.sceneManager.Close(); err != nil {
				errs = append(errs, fmt.Errorf("clientgo748: close scene manager: %w", err))
			}
			a.sceneManager = nil
		}
		if a.rendererOwned {
			a.rendererOwned = false
			if err := a.renderer.Close(); err != nil {
				errs = append(errs, fmt.Errorf("clientgo748: close renderer: %w", err))
			}
		}
		if a.sourceOwned {
			a.sourceOwned = false
			if err := a.options.LogoSource.Close(); err != nil {
				errs = append(errs, fmt.Errorf("clientgo748: close asset source: %w", err))
			}
		}
		if a.windowOwned {
			a.windowOwned = false
			if err := a.window.Close(); err != nil {
				errs = append(errs, fmt.Errorf("clientgo748: close window: %w", err))
			}
		}
		a.closeErr = errors.Join(errs...)
	})
	return a.closeErr
}

// textureScene mantém o primeiro recurso visual dentro do mesmo lifecycle das
// demais cenas. O renderer continua pertencendo à Application; a cena apenas
// solicita o desenho durante Render e não o fecha.
type textureScene struct {
	id       scene.ID
	renderer graphics.TextureRenderer
}

func (s *textureScene) ID() scene.ID                  { return s.id }
func (s *textureScene) Enter() error                  { return nil }
func (s *textureScene) HandleEvent(input.Event) error { return nil }
func (s *textureScene) Update(time.Duration) error    { return nil }
func (s *textureScene) Render() error                 { s.renderer.DrawTexture(); return nil }
func (s *textureScene) Exit() error                   { return nil }
func (s *textureScene) Close() error                  { return nil }
