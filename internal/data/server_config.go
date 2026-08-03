package data

import (
	"bufio"
	"fmt"
	stdnet "net"
	"os"
	"strconv"
	"strings"

	"wydgo/internal/model"
)

// ServerConfig concentra configuracao operacional e parametros globais de
// balanceamento. Dados especificos continuam em NPCGener, SkillData, NPCs etc.
type ServerConfig struct {
	ListenAddress         string
	DatabaseDriver        string
	DatabaseURL           string
	DatabaseURLEnv        string
	DatabaseMaxConns      uint32
	SessionQueueCapacity  uint32
	NPCPath               string
	GeneratorPath         string
	AccountsPath          string
	GuildsPath            string
	GuildsTxtPath         string
	CharStatePath         string
	QuestsPath            string
	QuestZonesPath        string
	InitItemsPath         string
	BossPath              string
	ItemPath              string
	ItemNamePath          string
	ItemEffectPath        string
	SkillPath             string
	DropRatePath          string
	VolatilePath          string
	ReplictionPath        string
	MountPath             string
	CharacterTemplatePath string
	HeightMapPath         string
	AttributeMapPath      string
	TeleportPath          string
	NPCGenerLog           string
	// DebugAddress e o endereco do servidor de diagnostico (expvar em
	// /debug/vars e pprof em /debug/pprof). Vazio = desligado, que e o padrao.
	// O host DEVE ser loopback: esses endpoints expoem estado interno e perfil
	// do processo. Para acessar de fora, use tunel SSH.
	DebugAddress string
	// LoadtestSpawn/LoadtestAccountPrefix ficam desabilitados por padrao. Em
	// staging permitem materializar bots diretamente no mapa de teste, sem
	// alterar a coordenada de nascimento dos jogadores reais.
	LoadtestSpawn         model.CharacterSpawn
	LoadtestAccountPrefix string
	Gameplay              model.GameplayConfig
}

func DefaultServerConfig() ServerConfig {
	return ServerConfig{
		ListenAddress:         "0.0.0.0:8281",
		DatabaseDriver:        "postgres",
		DatabaseURLEnv:        "WYD_DATABASE_URL",
		DatabaseMaxConns:      8,
		SessionQueueCapacity:  256,
		NPCPath:               "data/npcs",
		GeneratorPath:         "data/NPCGener.txt",
		AccountsPath:          "data/accounts",
		GuildsPath:            "data/guilds.json",
		GuildsTxtPath:         "data/Guilds.txt",
		CharStatePath:         "data/charstate",
		QuestsPath:            "data/quests.json",
		QuestZonesPath:        "data/quest_zones.json",
		InitItemsPath:         "data/init_items.csv",
		BossPath:              "data/boss",
		ItemPath:              "data/itemlist.csv",
		ItemNamePath:          "data/Itemname.csv",
		ItemEffectPath:        "data/ItemEffect.h",
		SkillPath:             "data/SkillData.csv",
		DropRatePath:          "data/droprate.json",
		VolatilePath:          "data/volatiles.json",
		ReplictionPath:        "data/repliction.json",
		MountPath:             "data/mounts.json",
		CharacterTemplatePath: "data/character_templates.json",
		HeightMapPath:         "../Server Star Micronics/TMSRV/run/HeightMap.dat",
		AttributeMapPath:      "../Server Star Micronics/TMSRV/run/AttributeMap.dat",
		TeleportPath:          "data/teleports.ini",
		NPCGenerLog:           "summary",
		DebugAddress:          "", // diagnostico desligado por padrao
		LoadtestAccountPrefix: "",
		Gameplay:              model.DefaultGameplayConfig(),
	}
}

func LoadServerConfig(path string) (ServerConfig, error) {
	cfg := DefaultServerConfig()
	file, err := os.Open(path)
	if err != nil {
		return ServerConfig{}, fmt.Errorf("abrir configuracao %q: %w", path, err)
	}
	defer file.Close()

	setUint32 := func(target *uint32) func(string) error {
		return func(value string) error {
			parsed, err := strconv.ParseUint(value, 10, 32)
			if err != nil {
				return fmt.Errorf("esperado inteiro sem sinal: %w", err)
			}
			*target = uint32(parsed)
			return nil
		}
	}
	setSpawn := func(target *model.CharacterSpawn) func(string) error {
		return func(value string) error {
			parts := strings.Split(value, ",")
			if len(parts) != 2 {
				return fmt.Errorf("esperado x,y")
			}
			x, err := strconv.ParseUint(strings.TrimSpace(parts[0]), 10, 16)
			if err != nil {
				return fmt.Errorf("x invalido: %w", err)
			}
			y, err := strconv.ParseUint(strings.TrimSpace(parts[1]), 10, 16)
			if err != nil {
				return fmt.Errorf("y invalido: %w", err)
			}
			target.X, target.Y = uint16(x), uint16(y)
			return nil
		}
	}
	setters := map[string]func(string) error{
		"listen_address":         func(v string) error { cfg.ListenAddress = v; return nil },
		"database_driver":        func(v string) error { cfg.DatabaseDriver = strings.ToLower(v); return nil },
		"database_url":           func(v string) error { cfg.DatabaseURL = v; return nil },
		"database_url_env":       func(v string) error { cfg.DatabaseURLEnv = v; return nil },
		"database_max_conns":     setUint32(&cfg.DatabaseMaxConns),
		"session_queue_capacity": setUint32(&cfg.SessionQueueCapacity),
		"npcs":                   func(v string) error { cfg.NPCPath = v; return nil },
		"npcgener":               func(v string) error { cfg.GeneratorPath = v; return nil },
		"accounts":               func(v string) error { cfg.AccountsPath = v; return nil },
		"guilds":                 func(v string) error { cfg.GuildsPath = v; return nil },
		"guilds_txt":             func(v string) error { cfg.GuildsTxtPath = v; return nil },
		"charstate":              func(v string) error { cfg.CharStatePath = v; return nil },
		"quests":                 func(v string) error { cfg.QuestsPath = v; return nil },
		"quest_zones":            func(v string) error { cfg.QuestZonesPath = v; return nil },
		"init_items":             func(v string) error { cfg.InitItemsPath = v; return nil },
		"boss":                   func(v string) error { cfg.BossPath = v; return nil },
		"items":                  func(v string) error { cfg.ItemPath = v; return nil },
		"itemnames":              func(v string) error { cfg.ItemNamePath = v; return nil },
		"itemeffects":            func(v string) error { cfg.ItemEffectPath = v; return nil },
		"skills":                 func(v string) error { cfg.SkillPath = v; return nil },
		"droprates":              func(v string) error { cfg.DropRatePath = v; return nil },
		"volatiles":              func(v string) error { cfg.VolatilePath = v; return nil },
		"repliction":             func(v string) error { cfg.ReplictionPath = v; return nil },
		"mounts":                 func(v string) error { cfg.MountPath = v; return nil },
		"character_templates":    func(v string) error { cfg.CharacterTemplatePath = v; return nil },
		"heightmap":              func(v string) error { cfg.HeightMapPath = v; return nil },
		"attributemap":           func(v string) error { cfg.AttributeMapPath = v; return nil },
		"teleports":              func(v string) error { cfg.TeleportPath = v; return nil },
		"npcgener_log":           func(v string) error { cfg.NPCGenerLog = strings.ToLower(v); return nil },
		"debug_address":          func(v string) error { cfg.DebugAddress = v; return nil },
		"loadtest_spawn":         setSpawn(&cfg.LoadtestSpawn),
		"loadtest_account_prefix": func(v string) error {
			cfg.LoadtestAccountPrefix = strings.ToLower(strings.TrimSpace(v))
			return nil
		},
		"exp_minimum":         setUint32(&cfg.Gameplay.EXPMinimum),
		"exp_rate":            setUint32(&cfg.Gameplay.EXPRatePercent),
		"party_exp_bonus":     setUint32(&cfg.Gameplay.PartyEXPBonusPercent),
		"compositor_base":     setUint32(&cfg.Gameplay.CompositorBaseChance),
		"compositor_refine_7": setUint32(&cfg.Gameplay.CompositorRefineChance[0]),
		"compositor_refine_8": setUint32(&cfg.Gameplay.CompositorRefineChance[1]),
		"compositor_refine_9": setUint32(&cfg.Gameplay.CompositorRefineChance[2]),
	}

	scanner := bufio.NewScanner(file)
	lineNumber := 0
	for scanner.Scan() {
		lineNumber++
		line := strings.TrimSpace(scanner.Text())
		if line == "" || strings.HasPrefix(line, "#") || strings.HasPrefix(line, ";") {
			continue
		}
		key, value, ok := strings.Cut(line, "=")
		if !ok {
			return ServerConfig{}, fmt.Errorf("%s:%d: esperado chave=valor", path, lineNumber)
		}
		key = strings.ToLower(strings.TrimSpace(key))
		value = strings.Trim(strings.TrimSpace(value), "\"")
		if value == "" {
			return ServerConfig{}, fmt.Errorf("%s:%d: valor vazio para %q", path, lineNumber, key)
		}
		set, known := setters[key]
		if !known {
			return ServerConfig{}, fmt.Errorf("%s:%d: chave desconhecida %q", path, lineNumber, key)
		}
		if err := set(value); err != nil {
			return ServerConfig{}, fmt.Errorf("%s:%d: %s: %w", path, lineNumber, key, err)
		}
	}
	if err := scanner.Err(); err != nil {
		return ServerConfig{}, fmt.Errorf("ler configuracao %q: %w", path, err)
	}
	if err := validateListenAddress(cfg.ListenAddress); err != nil {
		return ServerConfig{}, fmt.Errorf("%s: listen_address: %w", path, err)
	}
	switch cfg.NPCGenerLog {
	case "quiet", "summary", "verbose":
	default:
		return ServerConfig{}, fmt.Errorf("%s: npcgener_log invalido %q (use quiet, summary ou verbose)",
			path, cfg.NPCGenerLog)
	}
	switch cfg.DatabaseDriver {
	case "postgres", "json":
	default:
		return ServerConfig{}, fmt.Errorf("%s: database_driver invalido %q (use postgres ou json)",
			path, cfg.DatabaseDriver)
	}
	if cfg.DatabaseDriver == "postgres" {
		if cfg.DatabaseURL == "" && cfg.DatabaseURLEnv == "" {
			return ServerConfig{}, fmt.Errorf("%s: PostgreSQL exige database_url ou database_url_env", path)
		}
		if cfg.DatabaseMaxConns == 0 || cfg.DatabaseMaxConns > 64 {
			return ServerConfig{}, fmt.Errorf("%s: database_max_conns deve ficar entre 1 e 64", path)
		}
	}
	if cfg.SessionQueueCapacity == 0 || cfg.SessionQueueCapacity > 8192 {
		return ServerConfig{}, fmt.Errorf("%s: session_queue_capacity deve ficar entre 1 e 8192", path)
	}
	if cfg.DebugAddress != "" {
		if err := ValidateDebugAddress(cfg.DebugAddress); err != nil {
			return ServerConfig{}, fmt.Errorf("%s: debug_address: %w", path, err)
		}
	}
	if cfg.LoadtestAccountPrefix != "" {
		if cfg.LoadtestSpawn.X == 0 || cfg.LoadtestSpawn.Y == 0 {
			return ServerConfig{}, fmt.Errorf("%s: loadtest_account_prefix exige loadtest_spawn", path)
		}
		if !strings.HasPrefix(cfg.LoadtestAccountPrefix, "bot") || len(cfg.LoadtestAccountPrefix) > 8 {
			return ServerConfig{}, fmt.Errorf("%s: loadtest_account_prefix invalido", path)
		}
		for _, ch := range cfg.LoadtestAccountPrefix {
			if ch < 'a' || ch > 'z' {
				return ServerConfig{}, fmt.Errorf("%s: loadtest_account_prefix deve conter somente letras minusculas", path)
			}
		}
	}
	if err := cfg.Gameplay.Validate(); err != nil {
		return ServerConfig{}, fmt.Errorf("%s: gameplay: %w", path, err)
	}
	return cfg, nil
}

// ValidateDebugAddress exige loopback. /debug/vars e /debug/pprof expoem estado
// interno e permitem disparar profiling; expor isso na internet e um risco de
// vazamento e de DoS. Preferimos derrubar o boot a subir o diagnostico aberto:
// uma configuracao errada aqui e silenciosa e perigosa demais para tolerar.
//
// E exportada porque a flag de linha de comando sobrescreve o arquivo e precisa
// passar pela MESMA checagem -- senao `-debug_address 0.0.0.0:6060` furaria a
// validacao do boot.
func ValidateDebugAddress(address string) error {
	host, portText, err := stdnet.SplitHostPort(address)
	if err != nil {
		return err
	}
	port, err := strconv.Atoi(portText)
	if err != nil || port < 1 || port > 65535 {
		return fmt.Errorf("porta invalida %q", portText)
	}
	if host == "" {
		return fmt.Errorf("host vazio expoe o diagnostico em todas as interfaces; use 127.0.0.1")
	}
	if strings.EqualFold(host, "localhost") {
		return nil
	}
	ip := stdnet.ParseIP(host)
	if ip == nil {
		return fmt.Errorf("host %q nao e um IP de loopback; use 127.0.0.1", host)
	}
	if !ip.IsLoopback() {
		return fmt.Errorf("host %q nao e loopback; o diagnostico so pode escutar em 127.0.0.1 (use tunel SSH)", host)
	}
	return nil
}

func validateListenAddress(address string) error {
	_, portText, err := stdnet.SplitHostPort(address)
	if err != nil {
		return err
	}
	port, err := strconv.Atoi(portText)
	if err != nil || port < 1 || port > 65535 {
		return fmt.Errorf("porta invalida %q", portText)
	}
	return nil
}
