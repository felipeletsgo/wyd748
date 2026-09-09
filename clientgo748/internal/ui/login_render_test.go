package ui

import (
	"testing"

	"wydclient748/internal/graphics"
)

type loginTextRenderer struct {
	texts []string
}

func (*loginTextRenderer) Initialize(uintptr) error { return nil }
func (*loginTextRenderer) BeginFrame()               {}
func (*loginTextRenderer) EndFrame()                 {}
func (*loginTextRenderer) Close() error              { return nil }
func (*loginTextRenderer) DrawRect(int32, int32, int32, int32, graphics.Color) {}
func (r *loginTextRenderer) DrawText(_ int32, _ int32, text string, _ int32, _ graphics.Color) {
	r.texts = append(r.texts, text)
}

func TestLoginFormRenderUsesArtworkForStaticControls(t *testing.T) {
	form := NewLoginForm(nil)
	form.Account = "alice"
	form.Password = []byte("secret")
	renderer := &loginTextRenderer{}

	form.Render(renderer)

	joined := ""
	for _, text := range renderer.texts {
		joined += "|" + text
	}
	for _, want := range []string{"alice", "******"} {
		if !containsText(renderer.texts, want) {
			t.Fatalf("rendered texts %q do not contain %q", joined, want)
		}
	}
	for _, forbidden := range []string{"ACCOUNT", "PASSWORD", "LOGIN"} {
		if containsText(renderer.texts, forbidden) {
			t.Fatalf("rendered static control %q despite official artwork", forbidden)
		}
	}
}

func containsText(texts []string, want string) bool {
	for _, text := range texts {
		if text == want {
			return true
		}
	}
	return false
}
