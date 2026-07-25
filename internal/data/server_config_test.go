package data

import (
	"os"
	"path/filepath"
	"strings"
	"testing"
)

func writeServerConfig(t *testing.T, contents string) string {
	t.Helper()
	path := filepath.Join(t.TempDir(), "server.txt")
	if err := os.WriteFile(path, []byte(contents), 0o600); err != nil {
		t.Fatal(err)
	}
	return path
}

func TestLoadServerConfigOverridesDefaults(t *testing.T) {
	path := writeServerConfig(t, `
# comentario
listen_address = 127.0.0.1:9000
accounts = "/srv/wyd/accounts"
heightmap = data/maps/HeightMap.dat
attributemap = data/maps/AttributeMap.dat
npcgener_log = verbose
teleports = custom/teleports.ini
exp_minimum = 10000
exp_rate = 250
party_exp_bonus = 3
`)
	cfg, err := LoadServerConfig(path)
	if err != nil {
		t.Fatal(err)
	}
	if cfg.ListenAddress != "127.0.0.1:9000" ||
		cfg.AccountsPath != "/srv/wyd/accounts" ||
		cfg.NPCPath != "data/npcs" ||
		cfg.NPCGenerLog != "verbose" || cfg.TeleportPath != "custom/teleports.ini" ||
		cfg.HeightMapPath != "data/maps/HeightMap.dat" ||
		cfg.Gameplay.EXPMinimum != 10_000 || cfg.Gameplay.EXPRatePercent != 250 ||
		cfg.Gameplay.PartyEXPBonusPercent != 3 {
		t.Fatalf("configuracao incorreta: %+v", cfg)
	}
}

func TestLoadServerConfigRejectsTypoAndInvalidAddress(t *testing.T) {
	for _, contents := range []string{
		"listen_adress=0.0.0.0:8281\n",
		"listen_address=0.0.0.0:99999\n",
		"accounts\n",
		"npcgener_log=debug\n",
		"exp_minimum=-1\n",
		"exp_rate=abc\n",
		"exp_rate=100001\n",
		"party_exp_bonus=101\n",
	} {
		if _, err := LoadServerConfig(writeServerConfig(t, contents)); err == nil {
			t.Fatalf("configuracao invalida aceita: %q", strings.TrimSpace(contents))
		}
	}
}

// TestDebugAddressMustBeLoopback protege a decisao de seguranca: /debug/vars e
// /debug/pprof expoem estado interno e permitem disparar profiling. Um endereco
// publico tem de DERRUBAR o boot, nunca subir silenciosamente.
func TestDebugAddressMustBeLoopback(t *testing.T) {
	for _, address := range []string{
		"0.0.0.0:6060",
		"192.168.1.10:6060",
		":6060",
		"example.com:6060",
		"127.0.0.1:0",
		"127.0.0.1:99999",
		"127.0.0.1",
	} {
		if err := ValidateDebugAddress(address); err == nil {
			t.Errorf("endereco %q deveria ser recusado", address)
		}
	}
	for _, address := range []string{
		"127.0.0.1:6060",
		"localhost:6060",
		"[::1]:6060",
	} {
		if err := ValidateDebugAddress(address); err != nil {
			t.Errorf("endereco de loopback %q recusado: %v", address, err)
		}
	}
}

func TestLoadServerConfigRejectsPublicDebugAddress(t *testing.T) {
	path := writeServerConfig(t, "debug_address = 0.0.0.0:6060\n")
	if _, err := LoadServerConfig(path); err == nil {
		t.Fatal("configuracao com diagnostico publico deveria derrubar o boot")
	} else if !strings.Contains(err.Error(), "debug_address") {
		t.Fatalf("erro deveria citar debug_address: %v", err)
	}
}

func TestLoadServerConfigAcceptsLoopbackDebugAddress(t *testing.T) {
	path := writeServerConfig(t, "debug_address = 127.0.0.1:6060\n")
	cfg, err := LoadServerConfig(path)
	if err != nil {
		t.Fatalf("loopback deveria ser aceito: %v", err)
	}
	if cfg.DebugAddress != "127.0.0.1:6060" {
		t.Fatalf("DebugAddress=%q", cfg.DebugAddress)
	}
}

func TestDebugAddressDefaultsToDisabled(t *testing.T) {
	if got := DefaultServerConfig().DebugAddress; got != "" {
		t.Fatalf("diagnostico deveria vir desligado por padrao, veio %q", got)
	}
}
