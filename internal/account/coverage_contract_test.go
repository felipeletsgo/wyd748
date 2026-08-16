package account

import "testing"

func TestValidationErrorStringIncludesFieldAndMessage(t *testing.T) {
	err := (&ValidationError{Field: "username", Message: "invalid"}).Error()
	if err != "account: username: invalid" {
		t.Fatalf("ValidationError.Error=%q", err)
	}
}
