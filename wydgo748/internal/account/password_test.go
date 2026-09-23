package account

import "testing"

func TestPasswordHashRoundTrip(t *testing.T) {
	hash, err := HashPassword("Password1!")
	if err != nil {
		t.Fatal(err)
	}
	if hash == "Password1!" {
		t.Fatal("password was stored as plaintext")
	}
	ok, err := VerifyPassword(hash, "Password1!")
	if err != nil || !ok {
		t.Fatalf("correct password was rejected: ok=%v err=%v", ok, err)
	}
	ok, err = VerifyPassword(hash, "Wrong123!")
	if err != nil || ok {
		t.Fatalf("wrong password was accepted: ok=%v err=%v", ok, err)
	}
}
