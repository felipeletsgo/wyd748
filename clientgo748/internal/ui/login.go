// Package ui contains backend-independent controls for the client screens.
// It owns only value state; rendering is optional and supplied by a scene.
package ui

import (
	"errors"
	"strings"
	"unicode"

	"wydclient748/internal/graphics"
	"wydclient748/internal/input"
)

var ErrCredentialsEmpty = errors.New("login: account and password are required")

type Rect struct{ X, Y, Width, Height int32 }

func (r Rect) Contains(x, y int32) bool {
	return x >= r.X && y >= r.Y && x < r.X+r.Width && y < r.Y+r.Height
}

// LoginForm is a deterministic, keyboard-accessible login control set. The
// submit callback receives a copy of the password and is never called twice
// while a previous submission is pending.
type LoginForm struct {
	Account      string
	Password     []byte
	Focus        int // 0 account, 1 password
	Status       string
	Submitting   bool
	AccountRect  Rect
	PasswordRect Rect
	ButtonRect   Rect
	Submit       func(account string, password []byte) error
}

func NewLoginForm(submit func(string, []byte) error) *LoginForm {
	return &LoginForm{
		Focus: 0, Submit: submit,
		AccountRect:  Rect{X: 300, Y: 240, Width: 240, Height: 34},
		PasswordRect: Rect{X: 300, Y: 290, Width: 240, Height: 34},
		ButtonRect:   Rect{X: 350, Y: 350, Width: 140, Height: 38},
	}
}

func (f *LoginForm) HandleEvent(event input.Event) error {
	if f == nil {
		return errors.New("login: form is nil")
	}
	if event.Kind == input.KindMouseButtonDown && event.Button == 1 {
		switch {
		case f.AccountRect.Contains(event.X, event.Y):
			f.Focus = 0
		case f.PasswordRect.Contains(event.X, event.Y):
			f.Focus = 1
		case f.ButtonRect.Contains(event.X, event.Y):
			return f.submit()
		default:
			f.Focus = -1
		}
		return nil
	}
	if event.Kind == input.KindText && !f.Submitting && f.Focus >= 0 && unicode.IsGraphic(event.Rune) && event.Rune != '\u007f' {
		if f.Focus == 0 && len([]rune(f.Account)) < 15 {
			f.Account += string(event.Rune)
		}
		if f.Focus == 1 && len(f.Password) < 12 {
			f.Password = append(f.Password, string(event.Rune)...)
		}
		f.Status = ""
		return nil
	}
	if event.Kind == input.KindKeyDown && !f.Submitting {
		switch event.Key {
		case 0x08: // VK_BACK
			if f.Focus == 0 {
				rs := []rune(f.Account)
				if len(rs) > 0 {
					f.Account = string(rs[:len(rs)-1])
				}
			}
			if f.Focus == 1 && len(f.Password) > 0 {
				f.Password = f.Password[:len(f.Password)-1]
			}
		case 0x09:
			f.Focus = (f.Focus + 1) % 2
		case 0x0D:
			return f.submit()
		}
	}
	return nil
}

func (f *LoginForm) submit() error {
	if f.Submitting {
		return nil
	}
	if strings.TrimSpace(f.Account) == "" || len(f.Password) == 0 {
		f.Status = "Account and password are required."
		return ErrCredentialsEmpty
	}
	if f.Submit == nil {
		f.Status = "Login is unavailable."
		return errors.New("login: submit callback is unavailable")
	}
	f.Submitting = true
	password := append([]byte(nil), f.Password...)
	err := f.Submit(f.Account, password)
	clear(password)
	f.Submitting = false
	if err != nil {
		f.Status = "Login failed. Please try again."
		return err
	}
	clear(f.Password)
	f.Password = nil
	f.Status = "Connecting..."
	return nil
}

// Render draws the form's structure when the backend supports primitives.
// Text remains a separate font milestone; Status is still exposed for a real
// text renderer and for deterministic tests.
func (f *LoginForm) Render(renderer graphics.ShapeRenderer) {
	if f == nil || renderer == nil {
		return
	}
	renderer.DrawRect(260, 170, 320, 260, graphics.Color{R: .06, G: .08, B: .14, A: .96})
	renderer.DrawRect(f.AccountRect.X, f.AccountRect.Y, f.AccountRect.Width, f.AccountRect.Height, fieldColor(f.Focus == 0))
	renderer.DrawRect(f.PasswordRect.X, f.PasswordRect.Y, f.PasswordRect.Width, f.PasswordRect.Height, fieldColor(f.Focus == 1))
	renderer.DrawRect(f.ButtonRect.X, f.ButtonRect.Y, f.ButtonRect.Width, f.ButtonRect.Height, graphics.Color{R: .12, G: .34, B: .62, A: 1})
}

func fieldColor(focused bool) graphics.Color {
	if focused {
		return graphics.Color{R: .25, G: .48, B: .78, A: 1}
	}
	return graphics.Color{R: .16, G: .18, B: .24, A: 1}
}
