package account

import (
	"errors"
	"fmt"
	"os"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

var (
	ErrInvalidCredentials  = errors.New("account: credenciais invalidas")
	ErrUsernameUnavailable = errors.New("account: nome de conta indisponivel")
)

type AuthStore interface {
	LoadAccount(name string) (*model.Account, error)
}

type RegistrationStore interface {
	AccountNameExists(name string) (bool, error)
	CreateAccount(acc *model.Account) error
}

// Authenticate valida a senha hash do pacote 0x20D.
func Authenticate(st AuthStore, username, password string) (*model.Account, error) {
	if !validWireUsername(username) || !validWirePassword(password) {
		return nil, ErrInvalidCredentials
	}
	acc, err := st.LoadAccount(username)
	if err != nil {
		if os.IsNotExist(err) {
			// Mantem custo semelhante ao caminho de senha errada e reduz enumeracao
			// por tempo no endpoint de jogo.
			_, _ = HashPassword(password)
			return nil, ErrInvalidCredentials
		}
		return nil, err
	}
	ok, err := VerifyPassword(acc.PasswordHash, password)
	if err != nil {
		return nil, fmt.Errorf("account: hash de %q corrompido: %w", acc.Name, err)
	}
	if !ok {
		return nil, ErrInvalidCredentials
	}
	return acc, nil
}

func Create(st RegistrationStore, username, password, confirmation string) (*model.Account, error) {
	if !ValidUsername(username) {
		return nil, &ValidationError{Field: "username", Message: "use de 4 a 12 letras ou numeros"}
	}
	if !ValidPassword(password) {
		return nil, &ValidationError{Field: "password", Message: "use de 4 a 12 caracteres ASCII sem espacos"}
	}
	if password != confirmation {
		return nil, &ValidationError{Field: "passwordConfirmation", Message: "as senhas nao coincidem"}
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
