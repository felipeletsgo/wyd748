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
		// The artwork's editable strips are centered at x=350..458. The
		// slightly wider input hitboxes retain keyboard/mouse compatibility
		// with the previous logical form while still enclosing those strips.
		AccountRect:  Rect{X: 300, Y: 230, Width: 220, Height: 40},
		PasswordRect: Rect{X: 300, Y: 270, Width: 220, Height: 40},
		ButtonRect:   Rect{X: 332, Y: 326, Width: 134, Height: 31},
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

// Render draws only dynamic values and status. Labels, field chrome and the
// Login button are supplied by the official loginbox2.wyt artwork; no
// synthetic rectangle or duplicate label is used as a user-facing control.
func (f *LoginForm) Render(renderer graphics.ShapeRenderer) {
	if f == nil || renderer == nil {
		return
	}
	text, ok := renderer.(graphics.TextRenderer)
	if !ok {
		return
	}
	value := graphics.Color{R: 1, G: 1, B: 1, A: 1}
	muted := graphics.Color{R: 0.72, G: 0.76, B: 0.84, A: 1}
	account := f.Account
	if account == "" {
		account = "ENTER ACCOUNT"
		muted = graphics.Color{R: 0.58, G: 0.62, B: 0.70, A: 1}
	}
	text.DrawText(356, 245, account, 12, valueOrMuted(f.Account, value, muted))
	password := ""
	for range f.Password {
		password += "*"
	}
	if password == "" {
		password = "ENTER PASSWORD"
		text.DrawText(356, 281, password, 12, muted)
	} else {
		text.DrawText(356, 281, password, 12, value)
	}
	if f.Status != "" {
		text.DrawText(300, 390, f.Status, 12, muted)
	}
}

func valueOrMuted(value string, normal, muted graphics.Color) graphics.Color {
	if value == "" {
		return muted
	}
	return normal
}
