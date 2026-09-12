package account

import "fmt"

type ValidationError struct {
	Field   string
	Message string
}

func (e *ValidationError) Error() string {
	return fmt.Sprintf("account: %s: %s", e.Field, e.Message)
}

func ValidUsername(username string) bool {
	if len(username) < 4 || len(username) > 12 {
		return false
	}
	return usernameCharactersValid(username)
}

func validWireUsername(username string) bool {
	return len(username) >= 1 && len(username) <= 12 && usernameCharactersValid(username)
}

func usernameCharactersValid(username string) bool {
	for _, c := range []byte(username) {
		if (c < 'A' || c > 'Z') && (c < 'a' || c > 'z') && (c < '0' || c > '9') {
			return false
		}
	}
	return true
}

// O client 7.48 envia Password[12]. Somente ASCII visivel, exceto espaco,
// evita truncamento/mudanca de encoding entre site e client.
func ValidPassword(password string) bool {
	if len(password) < 4 || len(password) > 12 {
		return false
	}
	return passwordCharactersValid(password)
}

func validWirePassword(password string) bool {
	return len(password) >= 1 && len(password) <= 12 && passwordCharactersValid(password)
}

func passwordCharactersValid(password string) bool {
	for _, c := range []byte(password) {
		if c < 33 || c > 126 {
			return false
		}
	}
	return true
}
