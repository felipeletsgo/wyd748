package ui

import (
	"errors"
	"testing"

	"wydclient748/internal/input"
)

func TestLoginFormFocusEditAndSubmit(t *testing.T) {
	var gotAccount string
	var gotPassword []byte
	form := NewLoginForm(func(account string, password []byte) error {
		gotAccount, gotPassword = account, append([]byte(nil), password...)
		return nil
	})
	for _, r := range "alice" {
		_ = form.HandleEvent(input.Event{Kind: input.KindText, Rune: r})
	}
	_ = form.HandleEvent(input.Event{Kind: input.KindMouseButtonDown, Button: 1, X: 310, Y: 300})
	for _, r := range "secret" {
		_ = form.HandleEvent(input.Event{Kind: input.KindText, Rune: r})
	}
	if err := form.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D}); err != nil {
		t.Fatal(err)
	}
	if gotAccount != "alice" || string(gotPassword) != "secret" {
		t.Fatalf("submitted %q/%q", gotAccount, gotPassword)
	}
	if len(form.Password) != 0 || form.Status != "Connecting..." {
		t.Fatalf("form after submit password=%q status=%q", form.Password, form.Status)
	}
}

func TestLoginFormRejectsEmptyCredentialsAndDoesNotCallSubmit(t *testing.T) {
	calls := 0
	form := NewLoginForm(func(string, []byte) error { calls++; return nil })
	err := form.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D})
	if !errors.Is(err, ErrCredentialsEmpty) || calls != 0 || form.Status != "Account and password are required." {
		t.Fatalf("err=%v calls=%d status=%q", err, calls, form.Status)
	}
}

func TestLoginFormSubmitFailurePreservesPasswordForRetry(t *testing.T) {
	form := NewLoginForm(func(string, []byte) error { return errors.New("offline") })
	form.Account = "alice"
	form.Password = []byte("secret")
	if err := form.HandleEvent(input.Event{Kind: input.KindKeyDown, Key: 0x0D}); err == nil {
		t.Fatal("expected submit error")
	}
	if string(form.Password) != "secret" || form.Status != "Login failed. Please try again." {
		t.Fatalf("password=%q status=%q", form.Password, form.Status)
	}
}
