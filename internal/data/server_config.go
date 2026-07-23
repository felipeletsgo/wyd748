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
	NPCPath               string
	GeneratorPath         string
	AccountsPath          string
	GuildsPath            string
	GuildsTxtPath         string
	CharStatePath         string
	QuestsPath            string
	QuestZonesPath        string
	ItemPath              string
	ItemNamePath          string
	SkillPath             string
	DropRatePath          string
	VolatilePath          string
	MountPath             string
	CharacterTemplatePath string
	HeightMapPath         string
	AttributeMapPath      string
	TeleportPath          string
	NPCGenerLog           string
	Gameplay              model.GameplayConfig
}

func DefaultServerConfig() ServerConfig {
	return ServerConfig{
		ListenAddress:         "0.0.0.0:8281",
		NPCPath:               "data/npcs",
		GeneratorPath:         "data/NPCGener.txt",
		AccountsPath:          "data/accounts",
		GuildsPath:            "data/guilds.json",
		GuildsTxtPath:         "data/Guilds.txt",
		CharStatePath:         "data/charstate",
		QuestsPath:            "data/quests.json",
		QuestZonesPath:        "data/quest_zones.json",
		ItemPath:              "data/itemlist.csv",
		ItemNamePath:          "data/Itemname.csv",
		SkillPath:             "data/SkillData.csv",
		DropRatePath:          "data/droprate.json",
		VolatilePath:          "data/volatiles.json",
		MountPath:             "data/mounts.json",
		CharacterTemplatePath: "data/character_templates.json",
		HeightMapPath:         "../Server Star Micronics/TMSRV/run/HeightMap.dat",
		AttributeMapPath:      "../Server Star Micronics/TMSRV/run/AttributeMap.dat",
		TeleportPath:          "data/teleports.ini",
		NPCGenerLog:           "summary",
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
	setters := map[string]func(string) error{
		"listen_address":      func(v string) error { cfg.ListenAddress = v; return nil },
		"npcs":                func(v string) error { cfg.NPCPath = v; return nil },
		"npcgener":            func(v string) error { cfg.GeneratorPath = v; return nil },
		"accounts":            func(v string) error { cfg.AccountsPath = v; return nil },
		"guilds":              func(v string) error { cfg.GuildsPath = v; return nil },
		"guilds_txt":          func(v string) error { cfg.GuildsTxtPath = v; return nil },
		"charstate":           func(v string) error { cfg.CharStatePath = v; return nil },
		"quests":              func(v string) error { cfg.QuestsPath = v; return nil },
		"quest_zones":         func(v string) error { cfg.QuestZonesPath = v; return nil },
		"items":               func(v string) error { cfg.ItemPath = v; return nil },
		"itemnames":           func(v string) error { cfg.ItemNamePath = v; return nil },
		"skills":              func(v string) error { cfg.SkillPath = v; return nil },
		"droprates":           func(v string) error { cfg.DropRatePath = v; return nil },
		"volatiles":           func(v string) error { cfg.VolatilePath = v; return nil },
		"mounts":              func(v string) error { cfg.MountPath = v; return nil },
		"character_templates": func(v string) error { cfg.CharacterTemplatePath = v; return nil },
		"heightmap":           func(v string) error { cfg.HeightMapPath = v; return nil },
		"attributemap":        func(v string) error { cfg.AttributeMapPath = v; return nil },
		"teleports":           func(v string) error { cfg.TeleportPath = v; return nil },
		"npcgener_log":        func(v string) error { cfg.NPCGenerLog = strings.ToLower(v); return nil },
		"exp_minimum":         setUint32(&cfg.Gameplay.EXPMinimum),
		"exp_rate":            setUint32(&cfg.Gameplay.EXPRatePercent),
		"party_exp_bonus":     setUint32(&cfg.Gameplay.PartyEXPBonusPercent),
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
	if err := cfg.Gameplay.Validate(); err != nil {
		return ServerConfig{}, fmt.Errorf("%s: gameplay: %w", path, err)
	}
	return cfg, nil
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
