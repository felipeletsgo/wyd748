package main

import (
	"context"
	"flag"
	"net"
	"os"
	"os/exec"
	"path/filepath"
	"strings"
	"testing"
	"time"

	"wydgo/internal/data"
)

func TestConfigPathFromArgs(t *testing.T) {
	tests := []struct {
		name string
		args []string
		want string
	}{
		{name: "default", want: "data/config/server.txt"},
		{name: "short separate", args: []string{"-config", "data/vps.txt"}, want: "data/vps.txt"},
		{name: "long separate", args: []string{"--config", "data/vps.txt"}, want: "data/vps.txt"},
		{name: "short equals", args: []string{"-config=data/vps.txt"}, want: "data/vps.txt"},
		{name: "long equals", args: []string{"--config=data/vps.txt"}, want: "data/vps.txt"},
		{name: "among flags", args: []string{"-addr", "127.0.0.1:8281", "-config", "custom.txt"}, want: "custom.txt"},
		{name: "short flag without value", args: []string{"-config"}, want: "data/config/server.txt"},
		{name: "long flag without value", args: []string{"--config"}, want: "data/config/server.txt"},
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
		t.Fatalf("gener-extra default=%q; test fixtures must require opt-in", defaultGeneratorExtraPath)
	}
}

func TestChannelProcessSelectsStandaloneOrConfiguredChannel(t *testing.T) {
	if got := channelProcess(data.ServerConfig{}); got != 0 {
		t.Fatalf("standalone process channel=%d, want zero", got)
	}
	cfg := data.ServerConfig{ChannelID: 3, ChannelPeers: []byte{1, 2}}
	if got := channelProcess(cfg); got != 3 {
		t.Fatalf("channel process=%d, want configured channel 3", got)
	}
}

func TestServerMainRejectsMissingConfiguration(t *testing.T) {
	config := filepath.Join(t.TempDir(), "missing-server-config.txt")
	cmd := exec.Command(os.Args[0], "-test.run=^TestServerMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_SERVER_MAIN_HELPER=1", "WYD_SERVER_MAIN_CONFIG="+config)
	output, err := cmd.CombinedOutput()
	if err == nil || !strings.Contains(string(output), filepath.Base(config)) {
		t.Fatalf("server continued without configuration: err=%v output=%s", err, output)
	}
}

func TestServerMainRejectsUnsafeListenerOverrides(t *testing.T) {
	config, err := filepath.Abs(filepath.Join("..", "..", "data", "config", "server.txt"))
	if err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct {
		name string
		args []string
		want string
	}{
		{name: "public diagnostics", args: []string{"-debug_address", "0.0.0.0:6060"}, want: "debug_address"},
		{name: "public control listener", args: []string{"-control-address", "0.0.0.0:9090"}, want: "control-address requires literal loopback"},
		{name: "control listener without token", args: []string{"-control-address", "127.0.0.1:9090"}, want: "control-address requires literal loopback"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			cmd := exec.Command(os.Args[0], "-test.run=^TestServerMainHelperProcess$")
			cmd.Env = append(os.Environ(), "WYD_SERVER_MAIN_HELPER=1",
				"WYD_SERVER_MAIN_CONFIG="+config,
				"WYD_SERVER_MAIN_ARGS="+strings.Join(tc.args, "\x1f"))
			output, err := cmd.CombinedOutput()
			if err == nil || !strings.Contains(string(output), tc.want) {
				t.Fatalf("server accepted unsafe %s override: err=%v output=%s", tc.name, err, output)
			}
		})
	}
}

func TestServerMainRejectsOccupiedControlListener(t *testing.T) {
	listener, err := net.Listen("tcp", "127.0.0.1:0")
	if err != nil {
		t.Fatal(err)
	}
	defer listener.Close()
	config, err := filepath.Abs(filepath.Join("..", "..", "data", "config", "server.txt"))
	if err != nil {
		t.Fatal(err)
	}
	cmd := exec.Command(os.Args[0], "-test.run=^TestServerMainHelperProcess$")
	cmd.Env = append(os.Environ(), "WYD_SERVER_MAIN_HELPER=1",
		"WYD_SERVER_MAIN_CONFIG="+config,
		"WYD_SERVER_MAIN_ARGS="+strings.Join([]string{"-control-address", listener.Addr().String()}, "\x1f"),
		"WYD_CONTROL_TOKEN=0123456789abcdef0123456789abcdef")
	output, err := cmd.CombinedOutput()
	if err == nil || !strings.Contains(string(output), "could not open the private control listener") {
		t.Fatalf("server accepted occupied control address: err=%v output=%s", err, output)
	}
}

func TestServerMainStopsAtInvalidRequiredDataInputs(t *testing.T) {
	repoRoot, err := filepath.Abs(filepath.Join("..", ".."))
	if err != nil {
		t.Fatal(err)
	}
	invalidJSON := filepath.Join(t.TempDir(), "invalid.json")
	if err := os.WriteFile(invalidJSON, []byte("{"), 0o600); err != nil {
		t.Fatal(err)
	}
	invalidBosses := t.TempDir()
	if err := os.WriteFile(filepath.Join(invalidBosses, "broken.lua"), []byte("return {"), 0o600); err != nil {
		t.Fatal(err)
	}
	for _, tc := range []struct {
		name, config, want string
		key, value         string
		args, env          []string
	}{
		{name: "NPC directory", key: "npcs", value: "missing", want: "load NPCs"},
		{name: "base generators", key: "npcgener", value: "missing", want: "load NPCGener"},
		{name: "additional generators", key: "npcgener", value: "data/spawns/NPCGener.txt",
			args: []string{"-gener-extra", "missing"}, want: "load additional NPCGener"},
		{name: "teleports", key: "teleports", value: "missing", want: "load teleports"},
		{name: "network admission", key: "network_admission", value: "missing", want: "load network admission"},
		{name: "client integrity", key: "client_integrity", value: "missing", want: "load client integrity"},
		{name: "server messages", key: "language_dir", value: "missing", want: "load server messages"},
		{name: "item list", key: "items", value: "missing", want: "load catalog"},
		{name: "item names", key: "itemnames", value: "missing", want: "load catalog"},
		{name: "item effects", key: "itemeffects", value: "missing", want: "load catalog"},
		{name: "skills", key: "skills", value: "missing", want: "load catalog"},
		{name: "drop rates", key: "droprates", value: "missing", want: "load drop rates"},
		{name: "volatile items", key: "volatiles", value: "missing", want: "load volatile items/instances"},
		{name: "instance definitions", key: "instances", value: "missing", want: "load volatile items/instances"},
		{name: "Repliction", key: "repliction", value: "missing", want: "load Repliction"},
		{name: "mounts", key: "mounts", value: invalidJSON, want: "load mounts"},
		{name: "character templates", key: "character_templates", value: "missing", want: "load character templates"},
		{name: "height map", key: "heightmap", value: "missing", want: "load terrain"},
		{name: "attribute map", key: "attributemap", value: "missing", want: "load terrain"},
		{name: "quests", key: "quests", value: invalidJSON, want: "load quests"},
		{name: "quest zones", key: "quest_zones", value: invalidJSON, want: "load quest zones"},
		{name: "boss catalog", key: "boss", value: invalidBosses, want: "load bosses"},
		{name: "corrupted crystal", args: []string{"-corrupted_crystal", invalidJSON}, want: "load corrupted crystal"},
		{name: "Colosseum enabled startup", config: "colosseum_enabled=true\ndatabase_driver=postgres\ndatabase_url_env=WYD_TEST_EMPTY_DATABASE_URL\n",
			env: []string{"WYD_TEST_EMPTY_DATABASE_URL="}, want: "is empty"},
		{name: "initial world items", key: "init_items", value: "missing", want: "load world objects"},
		{name: "database URL missing", config: "database_driver=postgres\ndatabase_url_env=WYD_TEST_EMPTY_DATABASE_URL\n",
			env: []string{"WYD_TEST_EMPTY_DATABASE_URL="}, want: "is empty"},
		{name: "database URL invalid", config: "database_driver=postgres\ndatabase_url=invalid://database\n", want: "open PostgreSQL"},
		{name: "additional generators load successfully before database startup",
			config: "database_driver=postgres\ndatabase_url=invalid://database\n",
			args:   []string{"-gener-extra", "data/spawns/NPCGener.txt"}, want: "open PostgreSQL"},
		{name: "valid diagnostics override reaches database startup",
			config: "database_driver=postgres\ndatabase_url=invalid://database\n",
			args:   []string{"-debug_address", "127.0.0.1:6060"}, want: "open PostgreSQL"},
		{name: "private control listener binds with a valid token before database startup",
			config: "database_driver=postgres\ndatabase_url=invalid://database\n",
			args:   []string{"-control-address", "127.0.0.1:0"},
			env:    []string{"WYD_CONTROL_TOKEN=0123456789abcdef0123456789abcdef"}, want: "open PostgreSQL"},
	} {
		t.Run(tc.name, func(t *testing.T) {
			args := append([]string(nil), tc.args...)
			for i := 0; i+1 < len(args); i++ {
				if args[i] != "-control-address" || args[i+1] != "127.0.0.1:0" {
					continue
				}
				probe, err := net.Listen("tcp", args[i+1])
				if err != nil {
					t.Fatal(err)
				}
				args[i+1] = probe.Addr().String()
				if err := probe.Close(); err != nil {
					t.Fatal(err)
				}
			}
			config := filepath.Join(t.TempDir(), "server.txt")
			configContents := tc.config
			if configContents == "" && tc.key != "" {
				value := tc.value
				if value == "missing" {
					value = filepath.Join(t.TempDir(), "missing-input")
				}
				configContents = tc.key + "=" + value + "\n"
			}
			if err := os.WriteFile(config, []byte(configContents), 0o600); err != nil {
				t.Fatal(err)
			}
			ctx, cancel := context.WithTimeout(context.Background(), 60*time.Second)
			defer cancel()
			cmd := exec.CommandContext(ctx, os.Args[0], "-test.run=^TestServerMainHelperProcess$")
			cmd.Dir = repoRoot
			cmd.Env = append(os.Environ(), tc.env...)
			cmd.Env = append(cmd.Env, "WYD_SERVER_MAIN_HELPER=1", "WYD_SERVER_MAIN_CONFIG="+config)
			if len(args) > 0 {
				cmd.Env = append(cmd.Env, "WYD_SERVER_MAIN_ARGS="+strings.Join(args, "\x1f"))
			}
			output, err := cmd.CombinedOutput()
			if ctx.Err() != nil {
				t.Fatalf("server helper did not stop within its startup bound: %v; output=%s", ctx.Err(), output)
			}
			if err == nil || !strings.Contains(string(output), tc.want) {
				t.Fatalf("server did not fail at invalid %s input: err=%v output=%s", tc.name, err, output)
			}
		})
	}
}

func TestServerMainHelperProcess(t *testing.T) {
	if os.Getenv("WYD_SERVER_MAIN_HELPER") != "1" {
		return
	}
	args := []string{"server", "-config", os.Getenv("WYD_SERVER_MAIN_CONFIG")}
	if extra := os.Getenv("WYD_SERVER_MAIN_ARGS"); extra != "" {
		args = append(args, strings.Split(extra, "\x1f")...)
	}
	os.Args = args
	flag.CommandLine = flag.NewFlagSet("server", flag.ExitOnError)
	main()
}
