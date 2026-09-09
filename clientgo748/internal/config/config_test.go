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
