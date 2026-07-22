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
