package config

import "testing"

func TestDefaultIsValid(t *testing.T) {
	c := Default()
	if err := c.Validate(); err != nil {
		t.Fatalf("default config must validate: %v", err)
	}
	if c.AssetRoot != "assets/current" {
		t.Fatalf("default assets must remain inside clientgo748: %q", c.AssetRoot)
	}
	if c.ServerAddress != "127.0.0.1:8281" {
		t.Fatalf("default server address changed: %q", c.ServerAddress)
	}
}

func TestValidateServerAddress(t *testing.T) {
	for _, address := range []string{"", "127.0.0.1:8281", "localhost:8281", "[::1]:8281"} {
		if err := ValidateServerAddress(address); err != nil {
			t.Errorf("ValidateServerAddress(%q): %v", address, err)
		}
	}
	for _, address := range []string{"localhost", ":8281", "localhost:bad", "127.0.0.1:0"} {
		if err := ValidateServerAddress(address); err == nil {
			t.Errorf("ValidateServerAddress(%q) accepted invalid address", address)
		}
	}
}

func TestServerAddressFromEnvIsOptIn(t *testing.T) {
	t.Setenv("WYD_SERVER_ADDRESS", "")
	address, err := ServerAddressFromEnv("127.0.0.1:8281")
	if err != nil || address != "" {
		t.Fatalf("empty environment must keep offline mode: address=%q err=%v", address, err)
	}
	t.Setenv("WYD_SERVER_ADDRESS", "127.0.0.1:8281")
	address, err = ServerAddressFromEnv("127.0.0.1:8281")
	if err != nil || address != "127.0.0.1:8281" {
		t.Fatalf("configured environment not returned: address=%q err=%v", address, err)
	}
	t.Setenv("WYD_SERVER_ADDRESS", "not-an-endpoint")
	if _, err := ServerAddressFromEnv("127.0.0.1:8281"); err == nil {
		t.Fatal("malformed environment address was accepted")
	}
}

func TestServerEntriesFromEnvUsesDefaultWhenUnset(t *testing.T) {
	t.Setenv("WYD_SERVER_LIST", "")
	entries, err := ServerEntriesFromEnv("127.0.0.1:8281")
	if err != nil {
		t.Fatal(err)
	}
	if len(entries) != 1 || entries[0].Name != "WYD-Go Server" || entries[0].Channel != "Channel 1" || entries[0].Address != "127.0.0.1:8281" {
		t.Fatalf("entries=%+v", entries)
	}
}

func TestServerEntriesFromEnvParsesAndValidatesMultipleEntries(t *testing.T) {
	t.Setenv("WYD_SERVER_LIST", "Production|Main|127.0.0.1:8281;Test|localhost:8282")
	entries, err := ServerEntriesFromEnv("")
	if err != nil {
		t.Fatal(err)
	}
	if len(entries) != 2 || entries[0].Channel != "Main" || entries[1].Channel != "Channel 1" || entries[1].Address != "localhost:8282" {
		t.Fatalf("entries=%+v", entries)
	}
	t.Setenv("WYD_SERVER_LIST", "broken")
	if _, err := ServerEntriesFromEnv(""); err == nil {
		t.Fatal("malformed server list accepted")
	}
}

func TestValidateRejectsUnsupportedProtocol(t *testing.T) {
	c := Default()
	c.ProtocolVersion = "7.69"
	if err := c.Validate(); err == nil {
		t.Fatal("expected unsupported protocol to be rejected")
	}
}

func TestValidateRejectsSmallWindow(t *testing.T) {
	c := Default()
	c.WindowWidth = 100
	if err := c.Validate(); err == nil {
		t.Fatal("expected small window to be rejected")
	}
}
