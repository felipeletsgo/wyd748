package account

import (
	"errors"
	"os"
	"testing"

	"wydgo/internal/model"
	"wydgo/internal/store"
)

func TestValidationErrorStringIncludesFieldAndMessage(t *testing.T) {
	err := (&ValidationError{Field: "username", Message: "invalid"}).Error()
	if err != "account: username: invalid" {
		t.Fatalf("ValidationError.Error=%q", err)
	}
}

type contractAuthStore struct {
	acc *model.Account
	err error
}
func (s contractAuthStore) LoadAccount(string) (*model.Account, error) { return s.acc, s.err }

type contractRegistrationStore struct {
	exists bool
	existsErr error
	createErr error
	created *model.Account
}
func (s *contractRegistrationStore) AccountNameExists(string) (bool, error) { return s.exists, s.existsErr }
func (s *contractRegistrationStore) CreateAccount(acc *model.Account) error { s.created = acc; return s.createErr }

func TestVerifyPasswordRejectsEveryMalformedEncodingFamily(t *testing.T) {
	cases := []string{
		"", "wrong$600000$a$b", "pbkdf2-sha256$bad$a$b",
		"pbkdf2-sha256$99999$a$b", "pbkdf2-sha256$2000001$a$b",
		"pbkdf2-sha256$100000$bad***$AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA",
		"pbkdf2-sha256$100000$MTIzNDU2Nzg5MDEyMzQ1Ng$bad***",
		"pbkdf2-sha256$100000$MTIzNDU2Nzg5MDEyMzQ1Ng$YQ",
	}
	for _, encoded := range cases {
		if ok, err := VerifyPassword(encoded, "password"); err == nil || ok {
			t.Fatalf("hash malformado %q => ok=%v err=%v", encoded, ok, err)
		}
	}
}

func TestAuthenticateCoversValidationMissingAccountStoreErrorAndCorruptHash(t *testing.T) {
	if _, err := Authenticate(contractAuthStore{}, "abc", "valid1"); !errors.Is(err, ErrInvalidCredentials) {
		t.Fatalf("username wire invalido => %v", err)
	}
	if _, err := Authenticate(contractAuthStore{}, "valid1", "abc"); !errors.Is(err, ErrInvalidCredentials) {
		t.Fatalf("password wire invalido => %v", err)
	}
	if _, err := Authenticate(contractAuthStore{err: os.ErrNotExist}, "valid1", "valid1"); !errors.Is(err, ErrInvalidCredentials) {
		t.Fatalf("conta ausente => %v", err)
	}
	boom := errors.New("backend")
	if _, err := Authenticate(contractAuthStore{err: boom}, "valid1", "valid1"); !errors.Is(err, boom) {
		t.Fatalf("erro de store foi mascarado: %v", err)
	}
	acc := &model.Account{Name: "valid1", PasswordHash: "corrupt"}
	if _, err := Authenticate(contractAuthStore{acc: acc}, "valid1", "valid1"); err == nil || errors.Is(err, ErrInvalidCredentials) {
		t.Fatalf("hash corrompido nao foi diagnosticado: %v", err)
	}
}

func TestCreateCoversLookupAndCommitFailureFamilies(t *testing.T) {
	lookupErr := errors.New("lookup")
	st := &contractRegistrationStore{existsErr: lookupErr}
	if _, err := Create(st, "Valid1", "Valid1", "Valid1"); !errors.Is(err, lookupErr) {
		t.Fatalf("erro de lookup foi mascarado: %v", err)
	}

	st = &contractRegistrationStore{exists: true}
	if _, err := Create(st, "Valid1", "Valid1", "Valid1"); !errors.Is(err, ErrUsernameUnavailable) {
		t.Fatalf("conta existente => %v", err)
	}

	st = &contractRegistrationStore{createErr: store.ErrAccountExists}
	if _, err := Create(st, "Valid1", "Valid1", "Valid1"); !errors.Is(err, ErrUsernameUnavailable) {
		t.Fatalf("corrida de criacao nao virou indisponibilidade: %v", err)
	}

	commitErr := errors.New("commit")
	st = &contractRegistrationStore{createErr: commitErr}
	if _, err := Create(st, "Valid1", "Valid1", "Valid1"); !errors.Is(err, commitErr) {
		t.Fatalf("erro de commit foi mascarado: %v", err)
	}

	st = &contractRegistrationStore{}
	acc, err := Create(st, "Valid1", "Valid1", "Valid1")
	if err != nil || acc == nil || st.created != acc || acc.PasswordHash == "" || len(acc.Chars) != 0 {
		t.Fatalf("criacao valida divergente acc=%+v created=%p err=%v", acc, st.created, err)
	}
}

func TestValidationRejectsNonASCIIAndControlCharacters(t *testing.T) {
	for _, username := range []string{"José", "abcd!", "abc def", "abc\x00def"} {
		if ValidUsername(username) || validWireUsername(username) {
			t.Fatalf("username invalido aceito: %q", username)
		}
	}
	for _, password := range []string{"senhá", "abc def", "abc\x00def", "abc\ndef"} {
		if ValidPassword(password) || validWirePassword(password) {
			t.Fatalf("password invalido aceito: %q", password)
		}
	}
}
