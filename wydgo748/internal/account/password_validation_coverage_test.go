package account

import (
	"encoding/base64"
	"strings"
	"testing"
)

func TestValidationErrorFormatsFieldAndMessage(t *testing.T) {
	err := (&ValidationError{Field: "password", Message: "invalid characters"}).Error()
	if err != "account: password: invalid characters" {
		t.Fatalf("validation error=%q", err)
	}
}

func TestPasswordCharacterBoundaries(t *testing.T) {
	for _, tc := range []struct {
		name  string
		value string
		want  bool
	}{
		{name: "first printable ASCII", value: "!", want: true},
		{name: "last printable ASCII", value: "~", want: true},
		{name: "space", value: " ", want: false},
		{name: "control", value: "\x1f", want: false},
		{name: "delete", value: "\x7f", want: false},
		{name: "multibyte UTF-8", value: "á", want: false},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if got := passwordCharactersValid(tc.value); got != tc.want {
				t.Fatalf("passwordCharactersValid(%q)=%v want %v", tc.value, got, tc.want)
			}
		})
	}
}

func TestVerifyPasswordRejectsMalformedStoredHashes(t *testing.T) {
	salt := base64.RawStdEncoding.EncodeToString(make([]byte, 16))
	shortSalt := base64.RawStdEncoding.EncodeToString([]byte{1})
	shortKey := base64.RawStdEncoding.EncodeToString(make([]byte, 31))
	for _, tc := range []struct {
		name    string
		encoded string
		want    string
	}{
		{name: "wrong part count", encoded: "pbkdf2-sha256$100000$only-salt", want: "unknown hash format"},
		{name: "unknown algorithm", encoded: "scrypt$100000$" + salt + "$key", want: "unknown hash format"},
		{name: "non-numeric rounds", encoded: "pbkdf2-sha256$many$" + salt + "$key", want: "invalid hash iteration count"},
		{name: "rounds below minimum", encoded: "pbkdf2-sha256$99999$" + salt + "$key", want: "invalid hash iteration count"},
		{name: "rounds above maximum", encoded: "pbkdf2-sha256$2000001$" + salt + "$key", want: "invalid hash iteration count"},
		{name: "malformed salt", encoded: "pbkdf2-sha256$100000$%%%$key", want: "invalid salt"},
		{name: "short salt", encoded: "pbkdf2-sha256$100000$" + shortSalt + "$key", want: "invalid salt"},
		{name: "malformed key", encoded: "pbkdf2-sha256$100000$" + salt + "$%%%", want: "invalid hash"},
		{name: "short key", encoded: "pbkdf2-sha256$100000$" + salt + "$" + shortKey, want: "invalid hash"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			if _, err := VerifyPassword(tc.encoded, "password"); err == nil || !strings.Contains(err.Error(), tc.want) {
				t.Fatalf("VerifyPassword error=%v want text %q", err, tc.want)
			}
		})
	}
}
