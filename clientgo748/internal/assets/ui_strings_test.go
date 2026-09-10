package assets

import (
	"os"
	"testing"
)

func TestUIStringsOfficialAsset(t *testing.T) {
	data, err := os.ReadFile("../../assets/current/UI/UIString.txt")
	if err != nil {
		t.Fatal(err)
	}
	strings, err := ParseUIStrings(data)
	if err != nil {
		t.Fatal(err)
	}
	for id, want := range map[int32]string{1: "Connect", 2: "Close", 5: "Login", 6: "Close", 7: "New Account", 8: "Account", 9: "Password"} {
		if strings[id] != want {
			t.Fatalf("index %d = %q, want %q", id, strings[id], want)
		}
	}
}

func TestUIStringsRejectInvalidTables(t *testing.T) {
	for _, data := range []string{"", "1", "bad Caption", "0 Caption", "1 A\n1 B", "2147483648 A"} {
		if _, err := ParseUIStrings([]byte(data)); err == nil {
			t.Fatalf("accepted %q", data)
		}
	}
}
