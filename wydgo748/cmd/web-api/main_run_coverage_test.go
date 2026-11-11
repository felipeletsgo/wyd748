package main

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func writeWebAPITestInputs(t *testing.T, pin, staff string) (string, string) {
	t.Helper()
	root := t.TempDir()
	configPath := filepath.Join(root, "server.txt")
	staffPath := filepath.Join(root, "staff.json")
	if err := os.WriteFile(configPath, []byte(pin), 0o600); err != nil {
		t.Fatal(err)
	}
	if staff != "" {
		if err := os.WriteFile(staffPath, []byte(staff), 0o600); err != nil {
			t.Fatal(err)
		}
	}
	return configPath, staffPath
}

func runWebAPITest(t *testing.T, configPath, staffPath string, extra ...string) error {
	t.Helper()
	t.Setenv("WYD_CONTROL_TOKEN", strings.Repeat("x", 32))
	args := []string{
		"-addr", "127.0.0.1:8082",
		"-origin", "https://admin.example.test",
		"-staff-file", staffPath,
		"-config", configPath,
		"-dev-http",
	}
	setCommandArgs(t, append(args, extra...)...)
	return run()
}

func TestRunRejectsInvalidStartupInputsBeforeOpeningDatabase(t *testing.T) {
	const validStaff = `{"operator":{"capabilities":["server.status"]}}`
	t.Run("admin PIN is required", func(t *testing.T) {
		configPath, staffPath := writeWebAPITestInputs(t, "", validStaff)
		err := runWebAPITest(t, configPath, staffPath)
		if err == nil || !strings.Contains(err.Error(), "admin_access_pin is required") {
			t.Fatalf("missing admin PIN error=%v", err)
		}
	})
	t.Run("trusted proxy must be a CIDR", func(t *testing.T) {
		configPath, staffPath := writeWebAPITestInputs(t, "admin_access_pin=001234\n", validStaff)
		err := runWebAPITest(t, configPath, staffPath, "-trusted-proxies", "192.0.2.1")
		if err == nil || !strings.Contains(err.Error(), "invalid trusted proxy CIDR") {
			t.Fatalf("invalid trusted proxy error=%v", err)
		}
	})
	t.Run("staff file must load", func(t *testing.T) {
		configPath, staffPath := writeWebAPITestInputs(t, "admin_access_pin=001234\n", "")
		err := runWebAPITest(t, configPath, staffPath)
		if err == nil || !strings.Contains(err.Error(), "staff file unavailable") {
			t.Fatalf("missing staff file error=%v", err)
		}
	})
	t.Run("control API must use loopback", func(t *testing.T) {
		configPath, staffPath := writeWebAPITestInputs(t, "admin_access_pin=001234\n", validStaff)
		err := runWebAPITest(t, configPath, staffPath, "-control-url", "http://0.0.0.0:8081")
		if err == nil || !strings.Contains(err.Error(), "loopback") {
			t.Fatalf("unsafe control API error=%v", err)
		}
	})
	t.Run("dedicated read-only database is required", func(t *testing.T) {
		configPath, staffPath := writeWebAPITestInputs(t, "admin_access_pin=001234\n", validStaff)
		t.Setenv("WYD_WEB_DATABASE_URL", "")
		err := runWebAPITest(t, configPath, staffPath)
		if err == nil || !strings.Contains(err.Error(), "WYD_WEB_DATABASE_URL is required") {
			t.Fatalf("missing database URL error=%v", err)
		}
	})
	t.Run("database connection errors stay sanitized", func(t *testing.T) {
		configPath, staffPath := writeWebAPITestInputs(t, "admin_access_pin=001234\n", validStaff)
		t.Setenv("WYD_WEB_DATABASE_URL", "invalid-database-url")
		err := runWebAPITest(t, configPath, staffPath)
		if err == nil || !strings.Contains(err.Error(), "read-only account database unavailable") ||
			strings.Contains(err.Error(), "invalid-database-url") {
			t.Fatalf("database failure was not sanitized: %v", err)
		}
	})
}
