package account

import (
	"errors"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

type authMemoryStore struct {
	acc   *model.Account
	saves int
}

func (s *authMemoryStore) LoadAccount(string) (*model.Account, error) {
	if s.acc == nil {
		return nil, errors.New("missing")
	}
	copy := *s.acc
	return &copy, nil
}

func (s *authMemoryStore) SaveAccount(acc *model.Account) error {
	s.saves++
	copy := *acc
	s.acc = &copy
	return nil
}

func TestAuthenticateUsesPasswordHash(t *testing.T) {
	hash, err := HashPassword("felipe")
	if err != nil {
		t.Fatal(err)
	}
	st := &authMemoryStore{acc: &model.Account{Name: "felipe", PasswordHash: hash}}
	acc, err := Authenticate(st, "felipe", "felipe")
	if err != nil {
		t.Fatal(err)
	}
	if st.saves != 0 || acc.PasswordHash == "" {
		t.Fatalf("authentication changed the account: saves=%d acc=%+v", st.saves, acc)
	}
	if _, err := Authenticate(st, "felipe", "wrong"); !errors.Is(err, ErrInvalidCredentials) {
		t.Fatalf("wrong password accepted: %v", err)
	}
	if _, err := Authenticate(st, "FELIPE", "felipe"); err != nil {
		t.Fatalf("hash-based login failed: %v", err)
	}
}

func TestCreateAccountIsEmptyHashedAndUnique(t *testing.T) {
	st := store.NewJSONStore(t.TempDir())
	acc, err := Create(st, "NewUser", "Pass123!", "Pass123!")
	if err != nil {
		t.Fatal(err)
	}
	if acc.PasswordHash == "" || len(acc.Chars) != 0 {
		t.Fatalf("account created incorrectly: %+v", acc)
	}
	if _, err := Create(st, "newuser", "Pass123!", "Pass123!"); !errors.Is(err, ErrUsernameUnavailable) {
		t.Fatalf("case-insensitive duplicate accepted: %v", err)
	}
}

func TestRegistrationValidation(t *testing.T) {
	st := store.NewJSONStore(t.TempDir())
	for _, tc := range []struct{ user, pass, confirmation string }{
		{"abc", "Pass123!", "Pass123!"},
		{"invalid-name", "Pass123!", "Pass123!"},
		{"User", "abc", "abc"},
		{"User", "12345678901", "12345678901"},
		{"User", "Pass123!", "Other123!"},
	} {
		if _, err := Create(st, tc.user, tc.pass, tc.confirmation); err == nil {
			t.Fatalf("invalid registration accepted: %+v", tc)
		}
	}
}

func TestNativePasswordLengthBoundary(t *testing.T) {
	st := store.NewJSONStore(t.TempDir())
	const maximum = "1234567890"
	acc, err := Create(st, "Boundary", maximum, maximum)
	if err != nil {
		t.Fatalf("native ten-character password rejected: %v", err)
	}
	if _, err := Authenticate(st, acc.Name, maximum); err != nil {
		t.Fatalf("authentication at the native limit failed: %v", err)
	}

	hash, err := HashPassword("12345678901")
	if err != nil {
		t.Fatal(err)
	}
	legacy := &authMemoryStore{acc: &model.Account{Name: "legacy", PasswordHash: hash}}
	if _, err := Authenticate(legacy, "legacy", "12345678901"); !errors.Is(err, ErrInvalidCredentials) {
		t.Fatalf("password beyond the native limit accepted on the wire: %v", err)
	}
}
