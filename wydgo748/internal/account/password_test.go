package account

import "testing"

func TestPasswordHashRoundTrip(t *testing.T) {
	hash, err := HashPassword("Senha123!")
	if err != nil {
		t.Fatal(err)
	}
	if hash == "Senha123!" {
		t.Fatal("senha foi armazenada em texto")
	}
	ok, err := VerifyPassword(hash, "Senha123!")
	if err != nil || !ok {
		t.Fatalf("senha correta recusada: ok=%v err=%v", ok, err)
	}
	ok, err = VerifyPassword(hash, "Errada123!")
	if err != nil || ok {
		t.Fatalf("senha errada aceita: ok=%v err=%v", ok, err)
	}
}
