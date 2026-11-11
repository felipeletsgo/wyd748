package main

import (
	"strings"
	"testing"
)

func TestStartWebAdminValidatesPINFromEnvironment(t *testing.T) {
	t.Run("rejects invalid environment PIN", func(t *testing.T) {
		cfg := adminTestConfig(t)
		cfg.AdminAccessPIN = "123456"
		cfg.AdminAccessPINEnv = "WYD_TEST_ADMIN_PIN"
		t.Setenv(cfg.AdminAccessPINEnv, "12x456")
		if _, err := startWebAdmin(cfg, nil, nil, adminInputs{}); err == nil || !strings.Contains(err.Error(), "admin_access_pin") {
			t.Fatalf("invalid environment PIN error=%v", err)
		}
	})

	t.Run("environment value replaces file value", func(t *testing.T) {
		cfg := adminTestConfig(t)
		cfg.AdminAccessPIN = "bad"
		cfg.AdminAccessPINEnv = "WYD_TEST_ADMIN_PIN"
		cfg.DatabaseDriver = "sqlite"
		t.Setenv(cfg.AdminAccessPINEnv, "123456")
		if _, err := startWebAdmin(cfg, nil, nil, adminInputs{}); err == nil || !strings.Contains(err.Error(), "database_driver=postgres") {
			t.Fatalf("environment PIN was not validated before database selection: %v", err)
		}
	})
}
