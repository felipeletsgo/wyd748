package main

import (
	"testing"

	"wydgo/internal/data"
)

func TestConfigPathFromArgs(t *testing.T) {
	tests := []struct {
		name string
		args []string
		want string
	}{
		{name: "default", want: "data/server.txt"},
		{name: "short separate", args: []string{"-config", "data/vps.txt"}, want: "data/vps.txt"},
		{name: "long separate", args: []string{"--config", "data/vps.txt"}, want: "data/vps.txt"},
		{name: "short equals", args: []string{"-config=data/vps.txt"}, want: "data/vps.txt"},
		{name: "long equals", args: []string{"--config=data/vps.txt"}, want: "data/vps.txt"},
		{name: "among flags", args: []string{"-addr", "127.0.0.1:8281", "-config", "custom.txt"}, want: "custom.txt"},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			if got := configPathFromArgs(tt.args); got != tt.want {
				t.Fatalf("configPathFromArgs(%q) = %q; want %q", tt.args, got, tt.want)
			}
		})
	}
}

func TestGeneratorExtraDisabledByDefault(t *testing.T) {
	if defaultGeneratorExtraPath != "" {
		t.Fatalf("gener-extra default=%q; fixtures de teste devem exigir opt-in", defaultGeneratorExtraPath)
	}
}

func TestConfiguredDatabaseURLPrecedenceAndRailwayFallback(t *testing.T) {
	cfg := data.DefaultServerConfig()
	t.Setenv("WYD_DATABASE_URL", "wyd-environment")
	t.Setenv("DATABASE_URL", "railway-environment")

	if got := configuredDatabaseURL(cfg); got != "wyd-environment" {
		t.Fatalf("WYD_DATABASE_URL=%q, quer precedencia sobre DATABASE_URL", got)
	}

	t.Setenv("WYD_DATABASE_URL", "")
	if got := configuredDatabaseURL(cfg); got != "railway-environment" {
		t.Fatalf("fallback DATABASE_URL=%q, quer railway-environment", got)
	}

	cfg.DatabaseURL = "configured"
	if got := configuredDatabaseURL(cfg); got != "configured" {
		t.Fatalf("database_url explicita=%q, quer configured", got)
	}

	cfg.DatabaseURL = ""
	cfg.DatabaseURLEnv = "CUSTOM_DATABASE_URL"
	t.Setenv("CUSTOM_DATABASE_URL", "")
	if got := configuredDatabaseURL(cfg); got != "" {
		t.Fatalf("env customizada vazia nao deve usar fallback Railway: %q", got)
	}
}
