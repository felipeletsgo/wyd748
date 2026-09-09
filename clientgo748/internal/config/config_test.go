package config

import "testing"

func TestDefaultIsValid(t *testing.T) {
	if err := Default().Validate(); err != nil {
		t.Fatalf("default config must validate: %v", err)
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
