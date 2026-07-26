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

// TestLoadServerConfigParsesCompositorKeys: as quatro chaves precisam chegar ao
// GameplayConfig. Na W2PP o equivalente (CompRate.txt) e lido para um array que
// nenhuma funcao consulta, e o parser so normaliza a primeira coluna -- ajustar
// o arquivo la nao muda nada no jogo. Aqui isso tem que valer.
func TestLoadServerConfigParsesCompositorKeys(t *testing.T) {
	path := writeServerConfig(t, `
compositor_base = 15
compositor_refine_7 = 1
compositor_refine_8 = 2
compositor_refine_9 = 4
`)
	cfg, err := LoadServerConfig(path)
	if err != nil {
		t.Fatal(err)
	}
	if cfg.Gameplay.CompositorBaseChance != 15 {
		t.Errorf("compositor_base=%d, quer 15", cfg.Gameplay.CompositorBaseChance)
	}
	if want := [3]uint32{1, 2, 4}; cfg.Gameplay.CompositorRefineChance != want {
		t.Errorf("bonus por refino=%v, quer %v", cfg.Gameplay.CompositorRefineChance, want)
	}
}

func TestLoadServerConfigUsesCompositorDefaults(t *testing.T) {
	cfg, err := LoadServerConfig(writeServerConfig(t, "exp_rate=100\n"))
	if err != nil {
		t.Fatal(err)
	}
	// Teto de 42: 10 de base mais quatro materiais +9.
	if cfg.Gameplay.CompositorBaseChance != 10 {
		t.Errorf("base padrao=%d, quer 10", cfg.Gameplay.CompositorBaseChance)
	}
	if want := [3]uint32{3, 5, 8}; cfg.Gameplay.CompositorRefineChance != want {
		t.Errorf("bonus padrao=%v, quer %v", cfg.Gameplay.CompositorRefineChance, want)
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
		"compositor_base=101\n",
		"compositor_refine_9=101\n",
		"compositor_refine_7=abc\n",
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
