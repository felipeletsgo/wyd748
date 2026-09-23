package account

import (
	"errors"
	"fmt"
	"os"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

var (
	ErrInvalidCredentials  = errors.New("account: invalid credentials")
	ErrUsernameUnavailable = errors.New("account: username unavailable")
)

type AuthStore interface {
	LoadAccount(name string) (*model.Account, error)
}

type RegistrationStore interface {
	AccountNameExists(name string) (bool, error)
	CreateAccount(acc *model.Account) error
}

// Authenticate validates the hashed password for packet 0x20D.
func Authenticate(st AuthStore, username, password string) (*model.Account, error) {
	if !validWireUsername(username) || !validWirePassword(password) {
		return nil, ErrInvalidCredentials
	}
	acc, err := st.LoadAccount(username)
	if err != nil {
		if os.IsNotExist(err) {
			// Keep a similar cost to the wrong-password path to reduce timing-based
			// account enumeration on the game endpoint.
			_, _ = HashPassword(password)
			return nil, ErrInvalidCredentials
		}
		return nil, err
	}
	ok, err := VerifyPassword(acc.PasswordHash, password)
	if err != nil {
		return nil, fmt.Errorf("account: corrupted hash for %q: %w", acc.Name, err)
	}
	if !ok {
		return nil, ErrInvalidCredentials
	}
	return acc, nil
}

func Create(st RegistrationStore, username, password, confirmation string) (*model.Account, error) {
	if !ValidUsername(username) {
		return nil, &ValidationError{Field: "username", Message: "use 4 to 12 letters or digits"}
	}
	if !ValidPassword(password) {
		return nil, &ValidationError{Field: "password", Message: "use 4 to 10 ASCII characters without spaces"}
	}
	if password != confirmation {
		return nil, &ValidationError{Field: "passwordConfirmation", Message: "passwords do not match"}
	}
	exists, err := st.AccountNameExists(username)
	if err != nil {
		return nil, err
	}
	if exists {
		return nil, ErrUsernameUnavailable
	}
	hash, err := HashPassword(password)
	if err != nil {
		return nil, err
	}
	acc := &model.Account{Name: username, PasswordHash: hash, Chars: []model.Char{}}
	if err := st.CreateAccount(acc); err != nil {
		if errors.Is(err, store.ErrAccountExists) {
			return nil, ErrUsernameUnavailable
		}
		return nil, err
	}
	return acc, nil
}
