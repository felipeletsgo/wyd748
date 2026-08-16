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
gameplay_log = quiet
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
		cfg.InstancesPath != "data/instances.txt" ||
		cfg.NPCGenerLog != "verbose" || cfg.GameplayLog != "quiet" || cfg.TeleportPath != "custom/teleports.ini" ||
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

func TestLoadServerConfigParsesPostgresWithoutSecret(t *testing.T) {
	cfg, err := LoadServerConfig(writeServerConfig(t, `
database_driver=postgres
database_url_env=WYD_PRIVATE_DATABASE_URL
database_max_conns=6
`))
	if err != nil {
		t.Fatal(err)
	}
	if cfg.DatabaseDriver != "postgres" ||
		cfg.DatabaseURLEnv != "WYD_PRIVATE_DATABASE_URL" ||
		cfg.DatabaseMaxConns != 6 ||
		cfg.DatabaseURL != "" {
		t.Fatalf("configuracao PostgreSQL incorreta: %+v", cfg)
	}
}

func TestLoadServerConfigRejectsInvalidDatabaseSettings(t *testing.T) {
	for _, contents := range []string{
		"database_driver=mongo\n",
		"database_driver=postgres\ndatabase_max_conns=0\n",
		"database_driver=postgres\ndatabase_max_conns=65\n",
	} {
		if _, err := LoadServerConfig(writeServerConfig(t, contents)); err == nil {
			t.Fatalf("configuracao de banco invalida aceita: %q", contents)
		}
	}
}

func TestLoadServerConfigRejectsTypoAndInvalidAddress(t *testing.T) {
	for _, contents := range []string{
		"listen_adress=0.0.0.0:8281\n",
		"listen_address=0.0.0.0:99999\n",
		"accounts\n",
		"npcgener_log=debug\n",
		"gameplay_log=debug\n",
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

func TestLoadServerConfigParsesLoadtestSpawn(t *testing.T) {
	path := filepath.Join(t.TempDir(), "server.txt")
	content := "loadtest_spawn=1162,1700\nloadtest_account_prefix=bot\n"
	if err := os.WriteFile(path, []byte(content), 0o600); err != nil {
		t.Fatal(err)
	}
	cfg, err := LoadServerConfig(path)
	if err != nil {
		t.Fatal(err)
	}
	if cfg.LoadtestSpawn.X != 1162 || cfg.LoadtestSpawn.Y != 1700 ||
		cfg.LoadtestAccountPrefix != "bot" {
		t.Fatalf("loadtest config=%+v", cfg)
	}
}

func TestLoadServerConfigRejectsLoadtestPrefixWithoutSpawn(t *testing.T) {
	path := filepath.Join(t.TempDir(), "server.txt")
	if err := os.WriteFile(path, []byte("loadtest_account_prefix=bot\n"), 0o600); err != nil {
		t.Fatal(err)
	}
	if _, err := LoadServerConfig(path); err == nil {
		t.Fatal("prefixo loadtest sem spawn deveria ser recusado")
	}
}

func TestLoadServerConfigRejectsUnsafeLoadtestPrefix(t *testing.T) {
	path := writeServerConfig(t, "loadtest_spawn=1162,1700\nloadtest_account_prefix=bot!\n")
	if _, err := LoadServerConfig(path); err == nil {
		t.Fatal("prefixo loadtest com caractere nao alfabetico deveria ser recusado")
	}
}

func TestDebugAddressDefaultsToDisabled(t *testing.T) {
	if got := DefaultServerConfig().DebugAddress; got != "" {
		t.Fatalf("diagnostico deveria vir desligado por padrao, veio %q", got)
	}
}

func TestDefaultServerConfigUsesPackagedMapsAndNoInlineDatabaseURL(t *testing.T) {
	cfg := DefaultServerConfig()
	if cfg.HeightMapPath != "data/maps/HeightMap.dat" {
		t.Fatalf("HeightMap padrao=%q", cfg.HeightMapPath)
	}
	if cfg.AttributeMapPath != "data/maps/AttributeMap.dat" {
		t.Fatalf("AttributeMap padrao=%q", cfg.AttributeMapPath)
	}
	if cfg.DatabaseURL != "" || cfg.DatabaseURLEnv != "WYD_DATABASE_URL" {
		t.Fatalf("PostgreSQL padrao deve usar somente ambiente: url=%q env=%q", cfg.DatabaseURL, cfg.DatabaseURLEnv)
	}
	if cfg.NetworkAdmissionPath != "data/network_admission.json" {
		t.Fatalf("politica de rede padrao=%q", cfg.NetworkAdmissionPath)
	}
	if cfg.WorldCommandQueueCapacity != 1024 || cfg.AuthHashConcurrency != 4 {
		t.Fatalf("limites operacionais inesperados: world_queue=%d auth_hash=%d",
			cfg.WorldCommandQueueCapacity, cfg.AuthHashConcurrency)
	}
	if cfg.MaxConnections != 400 || cfg.MaxConnectionsPerIP != 4 || cfg.MaxAuthenticatedClientsPerIP != 4 {
		t.Fatalf("limites de conexao padrao=%d/%d/%d, quer 400/4/4",
			cfg.MaxConnections, cfg.MaxConnectionsPerIP, cfg.MaxAuthenticatedClientsPerIP)
	}
}

func TestLoadServerConfigParsesAndValidatesAuthenticatedClientLimit(t *testing.T) {
	cfg, err := LoadServerConfig(writeServerConfig(t, "max_authenticated_clients_per_ip=3\n"))
	if err != nil {
		t.Fatal(err)
	}
	if cfg.MaxAuthenticatedClientsPerIP != 3 {
		t.Fatalf("limite carregado=%d, quer 3", cfg.MaxAuthenticatedClientsPerIP)
	}
	for _, value := range []string{"0", "65"} {
		contents := "max_connections_per_ip=64\nmax_authenticated_clients_per_ip=" + value + "\n"
		if _, err := LoadServerConfig(writeServerConfig(t, contents)); err == nil {
			t.Fatalf("limite invalido %s foi aceito", value)
		}
	}
}
