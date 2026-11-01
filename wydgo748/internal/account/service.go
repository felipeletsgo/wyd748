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
	// ErrHashBusy means no password hash slot became free in time. The
	// attempt did no hash work and says nothing about the credentials.
	ErrHashBusy = errors.New("account: password hashing is busy")
)

// HashGate bounds password hashing apart from storage reads. Acquire blocks
// until a slot is free and returns false when the attempt must stop.
type HashGate interface {
	Acquire() bool
	Release()
}

type AuthStore interface {
	LoadAccount(name string) (*model.Account, error)
}

type RegistrationStore interface {
	AccountNameExists(name string) (bool, error)
	CreateAccount(acc *model.Account) error
}

// Authenticate validates the hashed password for packet 0x20D.
func Authenticate(st AuthStore, username, password string) (*model.Account, error) {
	return AuthenticateGated(st, username, password, nil)
}

// AuthenticateGated holds a hash slot only for the PBKDF2 work. The account
// read and later checks run without one, so slow storage cannot hold CPU
// capacity. A nil gate hashes without a limit.
func AuthenticateGated(st AuthStore, username, password string, gate HashGate) (*model.Account, error) {
	if !validWireUsername(username) || !validWirePassword(password) {
		return nil, ErrInvalidCredentials
	}
	acc, err := st.LoadAccount(username)
	if err != nil {
		// errors.Is also matches the store's wrapped not-found error;
		// os.IsNotExist does not unwrap it.
		if errors.Is(err, os.ErrNotExist) {
			// Keep a similar cost to the wrong-password path to reduce timing-based
			// account enumeration on the game endpoint.
			if gate != nil && !gate.Acquire() {
				return nil, ErrHashBusy
			}
			_, _ = HashPassword(password)
			if gate != nil {
				gate.Release()
			}
			return nil, ErrInvalidCredentials
		}
		return nil, err
	}
	if gate != nil && !gate.Acquire() {
		return nil, ErrHashBusy
	}
	ok, err := VerifyPassword(acc.PasswordHash, password)
	if gate != nil {
		gate.Release()
	}
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
