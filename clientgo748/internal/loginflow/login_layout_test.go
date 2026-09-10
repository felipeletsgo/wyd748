package loginflow

import (
	"os"
	"testing"
	"wydclient748/internal/assets"
	"wydclient748/internal/graphics"
	"wydclient748/internal/input"
	"wydclient748/internal/login"
	"wydclient748/internal/ui"
)

func TestLoginLayoutMatchesTMProjectViewportComposition(t *testing.T) {
	for _, test := range []struct {
		width, height  int32
		panelX, panelY int32
		logoY          int32
	}{
		{800, 600, 292, 223, 10},
		{1024, 768, 404, 307, 32},
		{1280, 960, 532, 403, 56},
		{1600, 900, 692, 373, 15},
	} {
		layout := loginLayoutFor(viewportProbe{width: test.width, height: test.height})
		if layout.panel.X != test.panelX || layout.panel.Y != test.panelY {
			t.Fatalf("viewport %dx%d panel=%+v, want (%d,%d)", test.width, test.height, layout.panel, test.panelX, test.panelY)
		}
		if layout.logoLeft.X != test.width/2-loginLogoWidth || layout.logoRight.X != test.width/2 {
			t.Fatalf("viewport %dx%d logos=(%+v,%+v) not centered on viewport", test.width, test.height, layout.logoLeft, layout.logoRight)
		}
		if layout.logoLeft.Y != test.logoY || layout.logoRight.Y != test.logoY {
			t.Fatalf("viewport %dx%d logoY=(%d,%d), want %d", test.width, test.height, layout.logoLeft.Y, layout.logoRight.Y, test.logoY)
		}
		if layout.account.X != layout.panel.X+72 || layout.password.X != layout.panel.X+72 || layout.submit.X != layout.panel.X+68 {
			t.Fatalf("viewport %dx%d controls lost panel anchoring: panel=%+v account=%+v password=%+v submit=%+v", test.width, test.height, layout.panel, layout.account, layout.password, layout.submit)
		}
	}
}

func TestLoginLayoutUsesNativeHitboxes(t *testing.T) {
	l := loginLayoutFor(viewportProbe{width: 800, height: 600})
	controls := loginAssetControls(t)
	l.applyNativeControls(controls)
	if l.account.X != l.panel.X+82 || l.password.Y != l.panel.Y+57 || l.submit.Width != 70 {
		t.Fatalf("native hitboxes not applied: %+v", l)
	}
	if l.panel.Width != 215 || l.panel.Height != 153 {
		t.Fatalf("wrong asset dimensions: %+v", l.panel)
	}
	if l.accountText.X != l.account.X || l.accountText.Y != l.account.Y+3 || l.passwordText.Y != l.password.Y+3 {
		t.Fatalf("text detached from fields: %+v", l)
	}
}

func loginAssetControls(t *testing.T) []assets.SceneControl {
	t.Helper()
	data, err := os.ReadFile("../../assets/current/UI/SelServerScene2.bin")
	if err != nil {
		t.Fatal(err)
	}
	controls, err := assets.ParseScene(data)
	if err != nil {
		t.Fatal(err)
	}
	return controls
}

func TestLoginInputLayoutBeforeRenderAndAfterResize(t *testing.T) {
	scene, err := newLoginScene(login.NewSessionState(), VisualOptions{LoginControls: loginAssetControls(t)})
	if err != nil {
		t.Fatal(err)
	}
	s := scene.(*loginScene)
	for _, width := range []int32{800, 1024, 1280} {
		s.renderer = viewportProbe{width: width, height: 600}
		x, y := (width-215)/2+82, (int32(600)-153)/2+57
		if err := s.HandleEvent(input.Event{Kind: input.KindMouseButtonDown, Button: 1, X: x, Y: y}); err != nil {
			t.Fatal(err)
		}
		if s.form.Focus != 1 || s.form.PasswordRect.X != x || s.form.PasswordRect.Y != y {
			t.Fatalf("viewport %d input uses stale layout: %+v", width, s.form)
		}
	}
}

func TestLoginCloseAndNewAccountCallbacks(t *testing.T) {
	requested := 0
	scene, err := newLoginScene(login.NewSessionState(), VisualOptions{
		LoginControls: loginAssetControls(t),
		RequestClose:  func() error { requested++; return nil },
	})
	if err != nil {
		t.Fatal(err)
	}
	s := scene.(*loginScene)
	layout := loginLayoutFor(nil)
	layout.applyNativeControls(s.loginControls)
	if err := s.HandleEvent(input.Event{Kind: input.KindMouseButtonDown, Button: 1, X: layout.close.X + 1, Y: layout.close.Y + 1}); err != nil {
		t.Fatal(err)
	}
	if requested != 1 {
		t.Fatalf("close callback calls=%d want 1", requested)
	}
	if err := s.HandleEvent(input.Event{Kind: input.KindMouseButtonDown, Button: 1, X: layout.newAccount.X + 1, Y: layout.newAccount.Y + 1}); err != nil {
		t.Fatal(err)
	}
	if s.form.Status != "Account creation is unavailable." {
		t.Fatalf("new account status=%q", s.form.Status)
	}
}

func TestLoginControlsRejectMalformedAsset(t *testing.T) {
	for _, test := range []struct {
		name   string
		mutate func([]assets.SceneControl) []assets.SceneControl
	}{
		{"missing", func(c []assets.SceneControl) []assets.SceneControl { return c[:1] }},
		{"duplicate", func(c []assets.SceneControl) []assets.SceneControl { return append(c, c[0]) }},
		{"kind", func(c []assets.SceneControl) []assets.SceneControl {
			for i := range c {
				if c[i].Words[0] == 5121 {
					c[i].Kind = 2
				}
			}
			return c
		}},
		{"parent", func(c []assets.SceneControl) []assets.SceneControl {
			for i := range c {
				if c[i].Words[0] == 5121 {
					c[i].Words[1] = 0
				}
			}
			return c
		}},
		{"truncated", func(c []assets.SceneControl) []assets.SceneControl {
			for i := range c {
				if c[i].Words[0] == 5121 {
					c[i].Words = c[i].Words[:7]
				}
			}
			return c
		}},
		{"overflow", func(c []assets.SceneControl) []assets.SceneControl {
			for i := range c {
				if c[i].Words[0] == 5121 {
					c[i].Words[3] = 2147483647
				}
			}
			return c
		}},
		{"dimensions", func(c []assets.SceneControl) []assets.SceneControl {
			for i := range c {
				if c[i].Words[0] == 4608 {
					c[i].Words[5] = 210
				}
			}
			return c
		}},
	} {
		t.Run(test.name, func(t *testing.T) {
			if _, err := newLoginScene(login.NewSessionState(), VisualOptions{LoginControls: test.mutate(loginAssetControls(t))}); err == nil {
				t.Fatal("malformed resource accepted")
			}
		})
	}
}

func TestLoginSceneOwnsResourceWords(t *testing.T) {
	controls := loginAssetControls(t)
	s, err := newLoginScene(login.NewSessionState(), VisualOptions{LoginControls: controls})
	if err != nil {
		t.Fatal(err)
	}
	for i := range controls {
		clear(controls[i].Words)
	}
	l := s.(*loginScene).syncLayout()
	if l.account.X != l.panel.X+82 {
		t.Fatalf("scene aliases caller resource: %+v", l)
	}
}

type captionProbe struct {
	viewportProbe
	captions []loginCaption
}

func (p *captionProbe) DrawText(x, y int32, text string, _ int32, _ graphics.Color) {
	p.captions = append(p.captions, loginCaption{rect: ui.Rect{X: x, Y: y}, text: text})
}

func TestLoginRendersAllAssetCaptions(t *testing.T) {
	data, err := os.ReadFile("../../assets/current/UI/UIString.txt")
	if err != nil {
		t.Fatal(err)
	}
	table, err := assets.ParseUIStrings(data)
	if err != nil {
		t.Fatal(err)
	}
	probe := &captionProbe{viewportProbe: viewportProbe{width: 800, height: 600}}
	s, err := newLoginScene(login.NewSessionState(), VisualOptions{LoginControls: loginAssetControls(t), UIStrings: table, ShapeRenderer: probe})
	if err != nil {
		t.Fatal(err)
	}
	clear(table) // A cena não depende de mutações futuras nas opções.
	if err := s.Render(); err != nil {
		t.Fatal(err)
	}
	want := []string{"Login", "Account", "Password", "Login", "Close", "New Account"}
	if len(probe.captions) != 8 {
		t.Fatalf("rendered captions: %+v", probe.captions)
	}
	for i, caption := range want {
		got := probe.captions[i+2]
		if got.text != caption || got.rect.X < 292 || got.rect.Y < 223 || got.rect.Y > 376 {
			t.Fatalf("caption %d = %+v, want %q inside panel", i, got, caption)
		}
	}
	if _, err := loginCaptions(loginAssetControls(t), map[int32]string{}); err == nil {
		t.Fatal("missing strings accepted")
	}
}
