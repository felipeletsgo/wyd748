package account

import (
	"crypto/pbkdf2"
	"crypto/rand"
	"crypto/sha256"
	"crypto/subtle"
	"encoding/base64"
	"fmt"
	"strconv"
	"strings"
)

const (
	passwordAlgorithm  = "pbkdf2-sha256"
	passwordIterations = 600_000
	passwordSaltBytes  = 16
	passwordKeyBytes   = 32
)

func HashPassword(password string) (string, error) {
	salt := make([]byte, passwordSaltBytes)
	if _, err := rand.Read(salt); err != nil {
		return "", fmt.Errorf("account: gerar salt: %w", err)
	}
	key, err := pbkdf2.Key(sha256.New, password, salt, passwordIterations, passwordKeyBytes)
	if err != nil {
		return "", fmt.Errorf("account: derivar senha: %w", err)
	}
	return fmt.Sprintf("%s$%d$%s$%s", passwordAlgorithm, passwordIterations,
		base64.RawStdEncoding.EncodeToString(salt), base64.RawStdEncoding.EncodeToString(key)), nil
}

func VerifyPassword(encoded, password string) (bool, error) {
	parts := strings.Split(encoded, "$")
	if len(parts) != 4 || parts[0] != passwordAlgorithm {
		return false, fmt.Errorf("account: formato de hash desconhecido")
	}
	iterations, err := strconv.Atoi(parts[1])
	if err != nil || iterations < 100_000 || iterations > 2_000_000 {
		return false, fmt.Errorf("account: iteracoes de hash invalidas")
	}
	salt, err := base64.RawStdEncoding.DecodeString(parts[2])
	if err != nil || len(salt) < 16 {
		return false, fmt.Errorf("account: salt invalido")
	}
	want, err := base64.RawStdEncoding.DecodeString(parts[3])
	if err != nil || len(want) != passwordKeyBytes {
		return false, fmt.Errorf("account: hash invalido")
	}
	got, err := pbkdf2.Key(sha256.New, password, salt, iterations, len(want))
	if err != nil {
		return false, err
	}
	return subtle.ConstantTimeCompare(got, want) == 1, nil
}
