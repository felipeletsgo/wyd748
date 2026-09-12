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
		return nil, errors.New("ausente")
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
		t.Fatalf("autenticacao alterou conta: saves=%d acc=%+v", st.saves, acc)
	}
	if _, err := Authenticate(st, "felipe", "errada"); !errors.Is(err, ErrInvalidCredentials) {
		t.Fatalf("senha errada aceita: %v", err)
	}
	if _, err := Authenticate(st, "FELIPE", "felipe"); err != nil {
		t.Fatalf("login por hash falhou: %v", err)
	}
}

func TestCreateAccountIsEmptyHashedAndUnique(t *testing.T) {
	st := store.NewJSONStore(t.TempDir())
	acc, err := Create(st, "NovaConta", "Senha123!", "Senha123!")
	if err != nil {
		t.Fatal(err)
	}
	if acc.PasswordHash == "" || len(acc.Chars) != 0 {
		t.Fatalf("conta criada incorretamente: %+v", acc)
	}
	if _, err := Create(st, "novaconta", "Senha123!", "Senha123!"); !errors.Is(err, ErrUsernameUnavailable) {
		t.Fatalf("duplicata case-insensitive aceita: %v", err)
	}
}

func TestRegistrationValidation(t *testing.T) {
	st := store.NewJSONStore(t.TempDir())
	for _, tc := range []struct{ user, pass, confirmation string }{
		{"abc", "Senha123!", "Senha123!"},
		{"nome-invalido", "Senha123!", "Senha123!"},
		{"Conta", "abc", "abc"},
		{"Conta", "Senha123!", "Outra123!"},
	} {
		if _, err := Create(st, tc.user, tc.pass, tc.confirmation); err == nil {
			t.Fatalf("cadastro invalido aceito: %+v", tc)
		}
	}
}
